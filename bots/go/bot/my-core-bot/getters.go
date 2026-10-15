package main

import (
	"github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

func ownCore() *game.Object {
	return core.NearestObject(game.Position{}, isOwnCore(core.Game.MyTeamID))
}

func opponentCore() *game.Object {
	return core.NearestObject(game.Position{}, isOpponentCore(core.Game.MyTeamID))
}

func nearestDeposit(pos game.Position) *game.Object {
	return core.NearestObject(pos, isDeposit)
}

func nearestGemPile(pos game.Position) *game.Object {
	return core.NearestObject(pos, isGemPile)
}

func nearestDepositOrGemPile(pos game.Position) *game.Object {
	return core.NearestObject(pos, isDepositOrGemPile)
}

func ownUnits() []*game.Object {
	return core.ObjectsFilter(isOwnUnit(core.Game.MyTeamID))
}

func opponentUnits() []*game.Object {
	return core.ObjectsFilter(isOpponentUnit(core.Game.MyTeamID))
}

func nearestOpponentUnit(pos game.Position) *game.Object {
	return core.NearestObject(pos, isOpponentUnit(core.Game.MyTeamID))
}

func isDeposit(obj *game.Object) bool {
	if obj == nil {
		return false
	}
	return obj.Type == game.ObjectDeposit
}

func isOwnCore(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		data, err := obj.CoreData()
		if err != nil {
			return false
		}
		return data.TeamID == myTeamID
	}
}

func isOpponentCore(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		data, err := obj.CoreData()
		if err != nil {
			return false
		}
		return data.TeamID != myTeamID
	}
}

func isGemPile(obj *game.Object) bool {
	if obj == nil {
		return false
	}
	return obj.Type == game.ObjectGemPile
}

func isDepositOrGemPile(obj *game.Object) bool {
	if isDeposit(obj) {
		return true
	}
	return isGemPile(obj)
}

func isOwnUnit(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		if obj == nil {
			return false
		}
		if obj.Type != game.ObjectUnit {
			return false
		}
		data, err := obj.UnitData()
		if err != nil {
			return false
		}
		return data.TeamID == myTeamID
	}
}

func isOpponentUnit(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		if obj == nil {
			return false
		}
		if obj.Type != game.ObjectUnit {
			return false
		}
		data, err := obj.UnitData()
		if err != nil {
			return false
		}
		return data.TeamID != myTeamID
	}
}
