package core

import "github.com/42core-team/go-client-lib/game"

// ObjectByID returns the object in [Game] with the given id.
// It returns an error if no such object exists.
func ObjectByID(id uint) (*game.Object, error) { return Game.ObjectByID(id) }

// ObjectAtPos returns the object at pos, or nil if the tile is empty.
func ObjectAtPos(pos game.Position) *game.Object { return Game.ObjectAtPos(pos) }

// ObjectsFilter returns all objects in [Game] for which predicate returns true.
// It returns nil if predicate is nil.
func ObjectsFilter(predicate func(*game.Object) bool) []*game.Object {
	return Game.ObjectsFilter(predicate)
}

// ObjectsFilterCount returns the number of objects in [Game] for which
// predicate returns true. A nil predicate matches every object.
func ObjectsFilterCount(predicate func(*game.Object) bool) uint {
	return Game.ObjectsFilterCount(predicate)
}

// UnitsByName returns the units of the own team whose name is name.
// It returns nil if name is empty.
func UnitsByName(name string) []*game.Object { return Game.UnitsByName(name) }

// UnitsByNameCount returns the number of units of the own team whose name is
// name. It returns 0 if name is empty.
func UnitsByNameCount(name string) uint { return Game.UnitsByNameCount(name) }

// NearestObject returns the object closest to pos for which predicate returns
// true, measured by [Distance]. Ties are broken in favor of the object that
// appears first in [Game]. It returns nil if no object matches or predicate is
// nil.
func NearestObject(pos game.Position, predicate func(*game.Object) bool) *game.Object {
	return Game.NearestObject(pos, predicate)
}

// IsPosValid reports whether pos lies on the board.
func IsPosValid(pos game.Position) bool { return Game.IsPosValid(pos) }

// Distance returns the Manhattan distance between a and b.
func Distance(a, b game.Position) int { return Game.Distance(a, b) }

// Log formats according to a format specifier, like [fmt.Printf], and writes
// the result followed by a newline to standard output.
func Log(format string, args ...any) { Game.Log(format, args...) }

// PrintField writes the current tick and an ASCII map of the board to
// standard output. Each tile shows the first letter of its object type,
// upper case for own objects and lower case for all others,
// or a dot if it is empty.
func PrintField() { Game.PrintField() }

// PrintObject writes the properties of obj to standard output.
func PrintObject(obj *game.Object) { Game.PrintObject(obj) }

// PrintObjects calls [PrintObject] for each object, separated by "---" lines.
func PrintObjects(objects []*game.Object) { Game.PrintObjects(objects) }

// PrintConfig writes a summary of the game configuration to standard output.
func PrintConfig() { Game.PrintConfig() }
