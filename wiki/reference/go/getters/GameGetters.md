---
title: "⚙️ Game getters"
permalink: "go_GameGetters"
sidebarTitle: "⚙️ Game getters"
---

```go
func (g *Game) ObjectByID(id uint) (*Object, error)
func (g *Game) ObjectAtPos(pos Position) *Object
func (g *Game) ObjectsFilter(predicate func(*Object) bool) []*Object
func (g *Game) ObjectsFilterCount(predicate func(*Object) bool) uint
func (g *Game) UnitsByName(name string) []*Object
func (g *Game) UnitsByNameCount(name string) uint
func (g *Game) NearestObject(from Position, predicate func(*Object) bool) *Object
```

A nil predicate returns no results from `ObjectsFilter` and `NearestObject`; `ObjectsFilterCount(nil)` counts every object. `ObjectAtPos`, `NearestObject`, and name lookup return nil when no object matches. `ObjectByID` returns an error when absent. Name lookup includes only your team's units.

## Template helpers

Own/opponent getters live in the template's `my-core-bot/getters.go`, not in the library. These ordinary functions take the current game and can be edited to suit your bot:

```go
func ownCore(g *game.Game) *game.Object
func opponentCore(g *game.Game) *game.Object
func ownUnits(g *game.Game) []*game.Object
func opponentUnits(g *game.Game) []*game.Object
func nearestOpponentUnit(g *game.Game, pos game.Position) *game.Object
func nearestDeposit(g *game.Game, pos game.Position) *game.Object
func nearestGemPile(g *game.Game, pos game.Position) *game.Object
func nearestDepositOrGemPile(g *game.Game, pos game.Position) *game.Object
```

Opponent helpers match any team other than your own, just like the C template. Core and nearest-object helpers return nil when nothing matches; unit-list helpers return a nil slice. Nearest searches use Manhattan distance and keep the first match on ties. Core searches use `(0, 0)` as their origin.

To migrate from the former library methods, replace `g.MyCore()` with `ownCore(g)`, `g.EnemyCore()` with `opponentCore(g)`, `g.TeamUnits()` with `ownUnits(g)`, and `g.EnemyUnits()` with `opponentUnits(g)`, using the template's `getters.go`.

## Generic filtering

```go
deposits := g.ObjectsFilter(func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
nearest := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
```
