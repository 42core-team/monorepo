package COREGAME

import (
	"fmt"
	"os"
	"strconv"

	"github.com/42core-team/go-client-lib/internal"
	"github.com/42core-team/go-client-lib/shared"
	"github.com/42core-team/go-client-lib/shared/schemas/actions"
)

// CoreGameBot represents a game bot instance that connects to the game server.
// It provides a clean API for controlling units and interacting with the game.
type Bot struct {
	conn     *internal.Connection
	teamName string
	teamId   uint
}

// NewCoreGameBot creates a new game bot with the given team name.
// It reads the team ID from the first command line argument and server
// connection details from environment variables (SERVER_IP and SERVER_PORT).
//
// Args:
//   - teamName: The name of your bot/team
//
// Returns a new CoreGameBot instance or an error if connection fails.
//
// Environment Variables:
//   - SERVER_IP: Server IP address (default: 127.0.0.1)
//   - SERVER_PORT: Server port (default: 4444)
//
// Command Line Args:
//   - args[1]: Team ID (required)
//
// Example:
//
//	bot, err := bot.NewCoreGameBot("MyBot")
//	if err != nil {
//	    panic(err)
//	}
func NewCoreGameBot(teamName string) (*Bot, error) {

	envIp, exists := os.LookupEnv("SERVER_IP")
	if !exists {
		fmt.Println("Environment variable SERVER_IP not set")
		envIp = "127.0.0.1"
	}

	envPort, exists := os.LookupEnv("SERVER_PORT")
	if !exists {
		fmt.Println("Environment variable SERVER_PORT not set")
		envPort = "4444"
	}

	if len(os.Args) < 2 {
		return nil, fmt.Errorf("team id not provided as first argument")
	}

	id := os.Args[1]
	if id == "" {
		return nil, fmt.Errorf("team id not provided as first argument")
	}
	teamId, err := strconv.Atoi(id)
	if err != nil {
		return nil, fmt.Errorf("team id not a number")
	}

	serverAddr := fmt.Sprintf("%s:%s", envIp, envPort)

	fmt.Printf("Connecting to server at %s...\n", serverAddr)
	conn, err := internal.NewConnection(serverAddr, uint(teamId))
	if err != nil {
		return nil, fmt.Errorf("could not create bot: %w", err)
	}

	return &Bot{conn: conn, teamName: teamName, teamId: uint(teamId)}, nil
}

func (bot *Bot) GetGame() *shared.Game {
	return bot.conn.GetGame()
}

func (bot *Bot) CreateUnit(unitType shared.UnitType) error {
	// TODO: check if spawning is possible
	bot.conn.GetActionQueue().Add(actions.NewActionCreate(unitType))
	return nil
}

func (bot *Bot) Move(object *shared.Object, pos shared.Position) error {
	bot.conn.GetActionQueue().Add(actions.NewActionMove(object.Id, pos.X, pos.Y))
	return nil
}

func (bot *Bot) SimplePathfind(object *shared.Object, target shared.Position) shared.Position {
	if object == nil {
		return shared.Position{}
	}

	game := bot.GetGame()
	if game == nil || game.Config.GridSize == 0 {
		return object.Pos
	}

	current := object.Pos
	if current == target {
		return current
	}

	grid := int(game.Config.GridSize)
	blocked := buildBlockedGrid(game, object.Id)

	goals := buildGoalSet(target, grid, blocked)
	if len(goals) == 0 {
		return current
	}

	next, ok := findNextStepBFS(current, target, goals, grid, blocked)
	if !ok {
		return current
	}
	return next
}

func buildBlockedGrid(game *shared.Game, selfID uint) map[int]bool {
	blocked := make(map[int]bool)
	grid := int(game.Config.GridSize)

	for i := range game.Objects {
		obj := &game.Objects[i]
		if !obj.IsAlive() || obj.Id == selfID {
			continue
		}
		x := int(obj.Pos.X)
		y := int(obj.Pos.Y)
		if x < 0 || y < 0 || x >= grid || y >= grid {
			continue
		}
		blocked[y*grid+x] = true
	}

	return blocked
}

func buildGoalSet(target shared.Position, grid int, blocked map[int]bool) map[int]bool {
	goals := make(map[int]bool)
	tx := int(target.X)
	ty := int(target.Y)
	if tx < 0 || ty < 0 || tx >= grid || ty >= grid {
		return goals
	}

	targetKey := ty*grid + tx
	if !blocked[targetKey] {
		goals[targetKey] = true
		return goals
	}

	for _, n := range neighbors(tx, ty, grid) {
		key := n[1]*grid + n[0]
		if !blocked[key] {
			goals[key] = true
		}
	}

	return goals
}

func findNextStepBFS(start shared.Position, target shared.Position, goals map[int]bool, grid int, blocked map[int]bool) (shared.Position, bool) {
	sx := int(start.X)
	sy := int(start.Y)
	if sx < 0 || sy < 0 || sx >= grid || sy >= grid {
		return start, false
	}

	startKey := sy*grid + sx
	if goals[startKey] {
		return start, true
	}

	queue := []int{startKey}
	visited := map[int]bool{startKey: true}
	prev := map[int]int{}

	found := -1
	bestKey := startKey
	bestDist := manhattanDistance(start, target)

	for len(queue) > 0 {
		cur := queue[0]
		queue = queue[1:]

		cx := cur % grid
		cy := cur / grid
		curPos := shared.NewPosition(uint(cx), uint(cy))
		d := manhattanDistance(curPos, target)
		if d < bestDist {
			bestDist = d
			bestKey = cur
		}

		if goals[cur] {
			found = cur
			break
		}

		for _, n := range neighbors(cx, cy, grid) {
			nx := n[0]
			ny := n[1]
			nKey := ny*grid + nx
			if visited[nKey] || blocked[nKey] {
				continue
			}
			visited[nKey] = true
			prev[nKey] = cur
			queue = append(queue, nKey)
		}
	}

	targetKey := found
	if targetKey == -1 {
		targetKey = bestKey
		if targetKey == startKey {
			return start, false
		}
	}

	stepKey := targetKey
	for {
		parent, ok := prev[stepKey]
		if !ok {
			return start, false
		}
		if parent == startKey {
			break
		}
		stepKey = parent
	}

	return shared.NewPosition(uint(stepKey%grid), uint(stepKey/grid)), true
}

func neighbors(x, y, grid int) [][2]int {
	result := make([][2]int, 0, 4)
	if x > 0 {
		result = append(result, [2]int{x - 1, y})
	}
	if x+1 < grid {
		result = append(result, [2]int{x + 1, y})
	}
	if y > 0 {
		result = append(result, [2]int{x, y - 1})
	}
	if y+1 < grid {
		result = append(result, [2]int{x, y + 1})
	}
	return result
}

func manhattanDistance(a, b shared.Position) uint {
	var dx uint
	if a.X > b.X {
		dx = a.X - b.X
	} else {
		dx = b.X - a.X
	}

	var dy uint
	if a.Y > b.Y {
		dy = a.Y - b.Y
	} else {
		dy = b.Y - a.Y
	}

	return dx + dy
}

func (bot *Bot) Attack(object *shared.Object, target *shared.Object) error {
	if !object.IsReadyForAction() {
		return fmt.Errorf("unit %d is on cooldown for %d more ticks", object.Id, object.ObjectData.(shared.UnitData).ActionCooldown)
	}
	bot.conn.GetActionQueue().Add(actions.NewActionAttack(object.Id, target.Id))
	return nil
}

func (bot *Bot) TransferGems(source *shared.Object, targetPos shared.Position, amount uint) error {
	if !source.IsReadyForAction() {
		return fmt.Errorf("unit %d is on cooldown for %d more ticks", source.Id, source.ObjectData.(shared.UnitData).ActionCooldown)
	}

	bot.conn.GetActionQueue().Add(actions.NewActionTransferGems(source.Id, targetPos.X, targetPos.Y, amount))
	return nil
}

func (bot *Bot) Build(builder *shared.Object, pos shared.Position) error {
	if !builder.IsReadyForAction() {
		return fmt.Errorf("unit %d is on cooldown for %d more ticks", builder.Id, builder.ObjectData.(shared.UnitData).ActionCooldown)
	}
	bot.conn.GetActionQueue().Add(actions.NewActionBuild(builder.Id, pos.X, pos.Y))
	return nil
}

func (bot *Bot) Run(callback func(game *shared.Game)) error {
	bot.conn.SetTickCallback(callback)
	return bot.conn.Start(bot.teamId, bot.teamName)
}
