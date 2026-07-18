package main

import "github.com/42core-team/go-client-lib/game"

func ownCore(g *game.Game) *game.Object {
	return g.MyCore()
}

func opponentCore(g *game.Game) *game.Object {
	return g.EnemyCore()
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
		data := obj.GetUnitData()
		return data != nil && data.TeamID == myTeamID
	}
}

func isOpponentUnit(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		if obj == nil || obj.Type != game.ObjectUnit {
			return false
		}
		data := obj.GetUnitData()
		return data != nil && data.TeamID != myTeamID
	}
}
