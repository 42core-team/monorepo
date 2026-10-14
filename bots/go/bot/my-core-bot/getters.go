package main

import "github.com/42core-team/go-client-lib/game"

func ownCore(g *game.Game) *game.Object {
	return g.NearestObject(game.Position{}, isOwnCore(g.MyTeamID))
}

func opponentCore(g *game.Game) *game.Object {
	return g.NearestObject(game.Position{}, isOpponentCore(g.MyTeamID))
}

func nearestDeposit(g *game.Game, pos game.Position) *game.Object {
	return g.NearestObject(pos, isDeposit)
}

func nearestGemPile(g *game.Game, pos game.Position) *game.Object {
	return g.NearestObject(pos, isGemPile)
}

func nearestDepositOrGemPile(g *game.Game, pos game.Position) *game.Object {
	return g.NearestObject(pos, isDepositOrGemPile)
}

func ownUnits(g *game.Game) []*game.Object {
	return g.ObjectsFilter(isOwnUnit(g.MyTeamID))
}

func opponentUnits(g *game.Game) []*game.Object {
	return g.ObjectsFilter(isOpponentUnit(g.MyTeamID))
}

func nearestOpponentUnit(g *game.Game, pos game.Position) *game.Object {
	return g.NearestObject(pos, isOpponentUnit(g.MyTeamID))
}

func isDeposit(obj *game.Object) bool {
	return obj != nil && obj.Type == game.ObjectDeposit
}

func isOwnCore(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		data, err := obj.CoreData()
		return err == nil && data.TeamID == myTeamID
	}
}

func isOpponentCore(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		data, err := obj.CoreData()
		return err == nil && data.TeamID != myTeamID && data.TeamID != 0
	}
}

func isGemPile(obj *game.Object) bool {
	return obj != nil && obj.Type == game.ObjectGemPile
}

func isDepositOrGemPile(obj *game.Object) bool {
	return isDeposit(obj) || isGemPile(obj)
}

func isOwnUnit(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		if obj == nil || obj.Type != game.ObjectUnit {
			return false
		}
		data, err := obj.UnitData()
		return err == nil && data.TeamID == myTeamID
	}
}

func isOpponentUnit(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		if obj == nil || obj.Type != game.ObjectUnit {
			return false
		}
		data, err := obj.UnitData()
		return err == nil && data.TeamID != myTeamID && data.TeamID != 0
	}
}
