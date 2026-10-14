package main

import "github.com/42core-team/go-client-lib/game"

func opponentCore(g *game.Game) *game.Object {
	return g.NearestObject(game.Position{}, func(obj *game.Object) bool {
		data, err := obj.CoreData()
		return err == nil && data.TeamID != g.MyTeamID && data.TeamID != 0
	})
}

func ownUnits(g *game.Game) []*game.Object {
	return g.ObjectsFilter(func(obj *game.Object) bool {
		data, err := obj.UnitData()
		return err == nil && data.TeamID == g.MyTeamID
	})
}
