package core

import "github.com/42core-team/go-client-lib/game"

func ObjectByID(id uint) (*game.Object, error)   { return Game.ObjectByID(id) }
func ObjectAtPos(pos game.Position) *game.Object { return Game.ObjectAtPos(pos) }
func ObjectsFilter(predicate func(*game.Object) bool) []*game.Object {
	return Game.ObjectsFilter(predicate)
}
func ObjectsFilterCount(predicate func(*game.Object) bool) uint {
	return Game.ObjectsFilterCount(predicate)
}
func UnitsByName(name string) []*game.Object { return Game.UnitsByName(name) }
func UnitsByNameCount(name string) uint      { return Game.UnitsByNameCount(name) }
func NearestObject(pos game.Position, predicate func(*game.Object) bool) *game.Object {
	return Game.NearestObject(pos, predicate)
}
func IsPosValid(pos game.Position) bool   { return Game.IsPosValid(pos) }
func Distance(a, b game.Position) int     { return Game.Distance(a, b) }
func Log(format string, args ...any)      { Game.Log(format, args...) }
func PrintField()                         { Game.PrintField() }
func PrintObject(obj *game.Object)        { Game.PrintObject(obj) }
func PrintObjects(objects []*game.Object) { Game.PrintObjects(objects) }
func PrintConfig()                        { Game.PrintConfig() }
