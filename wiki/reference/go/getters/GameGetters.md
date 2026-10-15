---
title: "⚙️ Game getters"
permalink: "go_GameGetters"
sidebarTitle: "⚙️ Game getters"
---

```go
func ObjectByID(id uint) (*game.Object, error)
func ObjectAtPos(pos game.Position) *game.Object
func ObjectsFilter(predicate func(*game.Object) bool) []*game.Object
func ObjectsFilterCount(predicate func(*game.Object) bool) uint
func UnitsByName(name string) []*game.Object
func UnitsByNameCount(name string) uint
func NearestObject(from game.Position, predicate func(*game.Object) bool) *game.Object
```

A nil predicate returns no results from `ObjectsFilter` and `NearestObject`; `ObjectsFilterCount(nil)` counts every object. `ObjectAtPos`, `NearestObject`, and name lookup return nil when no object matches. `ObjectByID` returns an error when absent. Name lookup includes only your team's units.

## Template helpers

Own/opponent getters live in the template's `my-core-bot/getters.go`, not in the library. These ordinary functions read the global game state and can be edited to suit your bot:

```go
func ownCore() *game.Object
func opponentCore() *game.Object
func ownUnits() []*game.Object
func opponentUnits() []*game.Object
func nearestOpponentUnit(pos game.Position) *game.Object
func nearestDeposit(pos game.Position) *game.Object
func nearestGemPile(pos game.Position) *game.Object
func nearestDepositOrGemPile(pos game.Position) *game.Object
```

Opponent helpers match any team other than your own, just like the C template. Core and nearest-object helpers return nil when nothing matches; unit-list helpers return a nil slice. Nearest searches use Manhattan distance and keep the first match on ties. Core searches use `(0, 0)` as their origin.

To migrate existing helpers, remove the `g *game.Game` parameter, use `core.Game.MyTeamID`, and call the package-level getters below. Call template helpers as `ownCore()`, `opponentCore()`, `ownUnits()`, and `opponentUnits()`.

## Generic filtering

```go
deposits := core.ObjectsFilter(func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
nearest := core.NearestObject(unit.Pos, func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
```
