package core

import (
	"container/heap"
	"fmt"
	"math"

	"github.com/42core-team/go-client-lib/game"
	"github.com/42core-team/go-client-lib/internal"
)

// A TravelAction describes how [Travel] treats a tile and its occupant.
type TravelAction int

const (
	TravelBlock  TravelAction = iota // tile cannot be entered
	TravelPass                       // tile can be entered; an occupant blocks this tick but is expected to move
	TravelAttack                     // tile can be entered after its occupant has been destroyed
)

// A TileTravelability describes the cost of routing a unit through a tile.
// Weight is the cost added to a route for entering the tile, roughly the
// number of ticks it takes, and is ignored if Action is [TravelBlock].
// Unknown Action values are treated as [TravelBlock].
type TileTravelability struct {
	Weight uint
	Action TravelAction
}

// A TravelPolicy returns the [TileTravelability] of the tile at pos for unit.
// It is called once for every tile on the board each time [Travel] plans a
// route.
type TravelPolicy func(pos game.Position, unit *game.Object) TileTravelability

// CreateUnit queues the creation of a unit with the given name, built from
// the components with the given IDs. If name is empty, the server generates
// a name.
//
// CreateUnit returns an error only if no game is running. Costs, cooldowns
// and component rules are checked by the server.
func CreateUnit(name string, components ...string) error {
	if err := checkClient(); err != nil {
		return fmt.Errorf("create unit: %w", err)
	}
	var customName *string
	if name != "" {
		customName = &name
	}
	componentIDs := append(make([]string, 0, len(components)), components...)
	connection.Load().ActionQueue().Add(internal.Action{Type: internal.ActionCreate, Name: customName, Components: componentIDs})
	return nil
}

// Move queues a move of unit to pos.
//
// Move returns an error if no game is running, unit is not a unit or pos is
// not on the board. Cooldown, ownership, occupancy and range are checked by
// the server.
func Move(unit *game.Object, pos game.Position) error {
	if err := checkClient(); err != nil {
		return fmt.Errorf("move: %w", err)
	}
	if err := checkUnit(unit); err != nil {
		return fmt.Errorf("move: %w", err)
	}
	if err := checkPosition(pos); err != nil {
		return fmt.Errorf("move: %w", err)
	}
	connection.Load().ActionQueue().Add(internal.Action{Type: internal.ActionMove, UnitID: unit.ID, TargetPos: pos})
	return nil
}

// Attack queues an attack of attacker on target.
//
// Attack returns an error if no game is running, attacker is not a unit or
// target is nil. Cooldown, ownership and range are checked by the server.
func Attack(attacker, target *game.Object) error {
	if err := checkClient(); err != nil {
		return fmt.Errorf("attack: %w", err)
	}
	if err := checkUnit(attacker); err != nil {
		return fmt.Errorf("attack: %w", err)
	}
	if target == nil {
		return fmt.Errorf("attack target: %w", game.ErrNilObject)
	}
	connection.Load().ActionQueue().Add(internal.Action{Type: internal.ActionAttack, UnitID: attacker.ID, TargetID: target.ID})
	return nil
}

// TransferGems queues a transfer of amount gems from source, which must be a
// unit or a core, to the tile at targetPos.
//
// TransferGems returns an error if no game is running, source is neither a
// unit nor a core, or targetPos is not on the board. Available gems,
// cooldown, ownership and range are checked by the server.
func TransferGems(source *game.Object, targetPos game.Position, amount uint) error {
	if err := checkClient(); err != nil {
		return fmt.Errorf("transfer gems: %w", err)
	}
	if source == nil {
		return fmt.Errorf("transfer gems source: %w", game.ErrNilObject)
	}
	if source.Type != game.ObjectUnit && source.Type != game.ObjectCore {
		return fmt.Errorf("transfer gems source %d must be a unit or core: %w", source.ID, game.ErrWrongObjectType)
	}
	if err := checkPosition(targetPos); err != nil {
		return fmt.Errorf("transfer gems: %w", err)
	}
	connection.Load().ActionQueue().Add(internal.Action{Type: internal.ActionTransfer, UnitID: source.ID, TargetPos: targetPos, Amount: amount})
	return nil
}

// Travel queues the next step of unit along the cheapest route to target,
// as determined by policy. The step is a [Move] onto the next tile or, if
// that tile is occupied by an object that policy marks as [TravelAttack],
// an [Attack] on that object. An occupant marked as [TravelPass] causes
// Travel to wait instead. The chosen route is added to the unit's debug path
// with [AddObjectPathStep].
//
// If target cannot be reached, Travel heads for the reachable tile closest
// to target. If unit is already at target or cannot make progress, Travel
// queues nothing and returns nil.
//
// A nil policy selects the default policy, which matches the C client:
//   - empty tiles cost 1;
//   - friendly units can be passed and cost more the longer their cooldown;
//   - friendly cores block;
//   - gem piles block if the unit cannot carry more gems;
//   - all other objects are attacked and cost the number of attacks needed
//     to destroy them, plus one.
//
// Travel returns an error if no game is running, unit is not a unit, or the
// position of unit or target is not on the board. As with Move and Attack,
// gameplay rules are checked by the server.
func Travel(unit *game.Object, target game.Position, policy TravelPolicy) error {
	if err := checkClient(); err != nil {
		return fmt.Errorf("travel: %w", err)
	}
	if _, err := unit.UnitData(); err != nil {
		return fmt.Errorf("travel: %w", err)
	}
	if err := checkPosition(unit.Pos); err != nil {
		return fmt.Errorf("travel origin: %w", err)
	}
	if err := checkPosition(target); err != nil {
		return fmt.Errorf("travel target: %w", err)
	}
	plan, ok := makeTravelPlan(Game, unit, target, policy)
	if !ok {
		return nil
	}
	for _, step := range plan.path {
		AddObjectPathStep(unit, step)
	}
	if plan.obstacle != nil {
		if plan.action == TravelAttack {
			return Attack(unit, plan.obstacle)
		}
		return nil
	}
	return Move(unit, plan.path[0])
}

type travelNode struct {
	distance uint64
	previous int
	weight   uint
	action   TravelAction
	visited  bool
}

type travelPlan struct {
	path     []game.Position
	obstacle *game.Object
	action   TravelAction
}

type travelHeapItem struct {
	index    int
	distance uint64
}
type travelHeap []travelHeapItem

func (h travelHeap) Len() int           { return len(h) }
func (h travelHeap) Less(i, j int) bool { return h[i].distance < h[j].distance }
func (h travelHeap) Swap(i, j int)      { h[i], h[j] = h[j], h[i] }
func (h *travelHeap) Push(value any)    { *h = append(*h, value.(travelHeapItem)) }
func (h *travelHeap) Pop() any {
	old := *h
	item := old[len(old)-1]
	*h = old[:len(old)-1]
	return item
}

func makeTravelPlan(g *game.Game, unit *game.Object, target game.Position, policy TravelPolicy) (travelPlan, bool) {
	if g == nil || unit == nil || !g.IsPosValid(unit.Pos) || !g.IsPosValid(target) || unit.Pos == target {
		return travelPlan{}, false
	}
	if _, err := unit.UnitData(); err != nil {
		return travelPlan{}, false
	}
	if policy == nil {
		objectsByPosition := make(map[game.Position]*game.Object, len(g.Objects))
		for _, object := range g.Objects {
			if _, exists := objectsByPosition[object.Pos]; !exists {
				objectsByPosition[object.Pos] = object
			}
		}
		policy = func(pos game.Position, unit *game.Object) TileTravelability {
			return defaultTileTravelabilityForObject(objectsByPosition[pos], unit)
		}
	}

	size := int(g.Config.GridSize)
	count := size * size
	start := int(unit.Pos.Y)*size + int(unit.Pos.X)
	destination := int(target.Y)*size + int(target.X)
	nodes := make([]travelNode, count)
	for i := range nodes {
		pos := game.Position{X: uint(i % size), Y: uint(i / size)}
		travelability := policy(pos, unit)
		nodes[i] = travelNode{distance: math.MaxUint64, previous: -1, weight: travelability.Weight, action: travelability.Action}
		if travelability.Action != TravelPass && travelability.Action != TravelAttack {
			nodes[i].action = TravelBlock
		}
	}

	nodes[start].distance, nodes[start].previous = 0, start
	queue := &travelHeap{{index: start, distance: 0}}
	heap.Init(queue)
	for queue.Len() > 0 {
		item := heap.Pop(queue).(travelHeapItem)
		current := item.index
		if nodes[current].visited || item.distance != nodes[current].distance {
			continue
		}
		nodes[current].visited = true
		if current == destination {
			break
		}
		neighbors, neighborCount := travelNeighbors(current, size)
		for _, next := range neighbors[:neighborCount] {
			if nodes[next].action == TravelBlock || nodes[next].visited {
				continue
			}
			step := uint64(nodes[next].weight)
			if nodes[current].distance > math.MaxUint64-step {
				continue
			}
			distance := nodes[current].distance + step
			if distance >= nodes[next].distance {
				continue
			}
			nodes[next].distance = distance
			nodes[next].previous = current
			heap.Push(queue, travelHeapItem{index: next, distance: distance})
		}
	}

	chosen := destination
	if nodes[chosen].distance == math.MaxUint64 {
		chosen = start
		closest := g.Distance(unit.Pos, target)
		for i := range nodes {
			if nodes[i].distance == math.MaxUint64 {
				continue
			}
			pos := game.Position{X: uint(i % size), Y: uint(i / size)}
			distance := g.Distance(pos, target)
			if distance < closest || (distance == closest && nodes[i].distance < nodes[chosen].distance) {
				chosen, closest = i, distance
			}
		}
	}
	if chosen == start {
		return travelPlan{}, false
	}

	reversed := make([]game.Position, 0, count)
	for step := chosen; step != start; step = nodes[step].previous {
		if step < 0 || nodes[step].previous < 0 {
			return travelPlan{}, false
		}
		reversed = append(reversed, game.Position{X: uint(step % size), Y: uint(step / size)})
	}
	path := make([]game.Position, len(reversed))
	for i := range reversed {
		path[len(reversed)-1-i] = reversed[i]
	}
	nextIndex := int(path[0].Y)*size + int(path[0].X)
	return travelPlan{path: path, obstacle: g.ObjectAtPos(path[0]), action: nodes[nextIndex].action}, true
}

func travelNeighbors(index, size int) ([4]int, int) {
	var neighbors [4]int
	count := 0
	x, y := index%size, index/size
	if x > 0 {
		neighbors[count] = index - 1
		count++
	}
	if x+1 < size {
		neighbors[count] = index + 1
		count++
	}
	if y > 0 {
		neighbors[count] = index - size
		count++
	}
	if y+1 < size {
		neighbors[count] = index + size
		count++
	}
	return neighbors, count
}

func defaultTileTravelability(g *game.Game, pos game.Position, unit *game.Object) TileTravelability {
	return defaultTileTravelabilityForObject(g.ObjectAtPos(pos), unit)
}

func defaultTileTravelabilityForObject(object, unit *game.Object) TileTravelability {
	if object == nil {
		return TileTravelability{Weight: 1, Action: TravelPass}
	}
	unitData, err := unit.UnitData()
	if err != nil {
		return TileTravelability{Action: TravelBlock}
	}
	if object.IsFriendly(unitData.TeamID) {
		if object.Type == game.ObjectCore {
			return TileTravelability{Action: TravelBlock}
		}
		wait := uint(1)
		if data, err := object.UnitData(); err == nil && data.ActionCooldown > 0 {
			wait = uint(data.ActionCooldown)
		}
		if wait >= math.MaxUint32 {
			return TileTravelability{Weight: math.MaxUint32, Action: TravelPass}
		}
		return TileTravelability{Weight: wait + 1, Action: TravelPass}
	}
	if object.Type == game.ObjectGemPile && (unitData.Properties.MaxGems == 0 || unitData.Gems >= unitData.Properties.MaxGems) {
		return TileTravelability{Action: TravelBlock}
	}

	damage := unitData.Properties.DamageObject
	switch object.Type {
	case game.ObjectCore:
		damage = unitData.Properties.DamageCore
	case game.ObjectUnit:
		damage = unitData.Properties.DamageUnit
		if target, err := object.UnitData(); damage > 0 && err == nil {
			reduction := target.Properties.DamageReductionPercent
			if reduction < 0 {
				reduction = 0
			}
			if reduction > 100 {
				reduction = 100
			}
			reduced := (uint64(damage)*uint64(100-reduction) + 50) / 100
			if reduced == 0 {
				reduced = 1
			}
			damage = uint(reduced)
		}
	case game.ObjectGemPile:
		damage = 1
	}
	if damage == 0 {
		return TileTravelability{Action: TravelBlock}
	}
	attacks := uint64(object.Hp) / uint64(damage)
	if object.Hp%damage != 0 {
		attacks++
	}
	weight := uint(math.MaxUint32)
	if attacks < math.MaxUint32 {
		weight = uint(attacks + 1)
	}
	return TileTravelability{Weight: weight, Action: TravelAttack}
}
