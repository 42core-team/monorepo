package coregame

import (
	"github.com/42core-team/go-client-lib/game"
	"github.com/42core-team/go-client-lib/internal"
)

func (b *Bot) CreateUnit(unitType game.UnitType) {
	b.connection.ActionQueue().Add(internal.Action{
		Type:     internal.ActionCreate,
		UnitType: unitType,
	})
}

func (b *Bot) Move(unit *game.Object, pos game.Position) {
	if unit == nil {
		return
	}
	b.connection.ActionQueue().Add(internal.Action{
		Type:      internal.ActionMove,
		UnitID:    unit.ID,
		TargetPos: pos,
	})
}

func (b *Bot) Attack(attacker, target *game.Object) {
	if attacker == nil || target == nil {
		return
	}
	b.connection.ActionQueue().Add(internal.Action{
		Type:     internal.ActionAttack,
		UnitID:   attacker.ID,
		TargetID: target.ID,
	})
}

func (b *Bot) TransferGems(source *game.Object, targetPos game.Position, amount uint) {
	if source == nil {
		return
	}
	b.connection.ActionQueue().Add(internal.Action{
		Type:      internal.ActionTransfer,
		UnitID:    source.ID,
		TargetPos: targetPos,
		Amount:    amount,
	})
}

func (b *Bot) Build(builder *game.Object, pos game.Position) {
	if builder == nil {
		return
	}
	b.connection.ActionQueue().Add(internal.Action{
		Type:      internal.ActionBuild,
		UnitID:    builder.ID,
		TargetPos: pos,
	})
}

func (b *Bot) SimplePathfind(unit *game.Object, targetPos game.Position) game.Position {
	if unit == nil || unit.Type != game.ObjectUnit {
		return game.Position{}
	}
	if unit.Pos.X == targetPos.X && unit.Pos.Y == targetPos.Y {
		return unit.Pos
	}

	data := unit.GetUnitData()
	if data == nil || (data.ActionCooldown != nil && *data.ActionCooldown > 0) {
		return unit.Pos
	}

	posOptionY := game.Position{X: unit.Pos.X, Y: unit.Pos.Y}
	if targetPos.Y != unit.Pos.Y {
		if targetPos.Y > unit.Pos.Y {
			posOptionY.Y = unit.Pos.Y + 1
		} else {
			posOptionY.Y = unit.Pos.Y - 1
		}
	}

	posOptionX := game.Position{X: unit.Pos.X, Y: unit.Pos.Y}
	if targetPos.X != unit.Pos.X {
		if targetPos.X > unit.Pos.X {
			posOptionX.X = unit.Pos.X + 1
		} else {
			posOptionX.X = unit.Pos.X - 1
		}
	}

	priorityX := 0
	priorityY := 0

	if !b.game.IsPosValid(posOptionY) || posOptionY.Y == unit.Pos.Y {
		priorityY += 500
	}
	if !b.game.IsPosValid(posOptionX) || posOptionX.X == unit.Pos.X {
		priorityX += 500
	}

	dx := abs(int(targetPos.X) - int(unit.Pos.X))
	dy := abs(int(targetPos.Y) - int(unit.Pos.Y))
	if dx > dy {
		priorityY++
	} else {
		priorityX++
	}

	objAtX := b.game.ObjectAtPos(posOptionX)
	objAtY := b.game.ObjectAtPos(posOptionY)

	if objAtX != nil {
		priorityX += 50
	}
	if objAtY != nil {
		priorityY += 50
	}

	if objAtX != nil && objAtX.IsFriendly(b.game.MyTeamID) {
		priorityX += 100
	}
	if objAtY != nil && objAtY.IsFriendly(b.game.MyTeamID) {
		priorityY += 100
	}

	if priorityX < 250 && priorityX < priorityY {
		if objAtX != nil && !objAtX.IsFriendly(b.game.MyTeamID) {
			b.Attack(unit, objAtX)
		} else if objAtX == nil {
			return posOptionX
		}
	}

	if priorityY < 250 {
		if objAtY != nil && !objAtY.IsFriendly(b.game.MyTeamID) {
			b.Attack(unit, objAtY)
		} else if objAtY == nil {
			return posOptionY
		}
	}

	return unit.Pos
}

func abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
}
