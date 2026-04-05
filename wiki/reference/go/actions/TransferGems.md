---
title: "⚙️ method Bot.TransferGems(...)"
permalink: "go_TransferGems"
sidebarTitle: "⚙️ TransferGems()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/actions.go

## Description

Gives gems to another object or drops it on the floor.

If the position points to an object that can hold gems (Cores / Units / Gem Piles), the gems will be transferred to that object, otherwise a new gem pile object will be created on the floor.

Only units are able to drop gems on the floor.

- Objects can only transfer gems if their action cooldown is 0, for more see [Cooldowns](documentation/cooldowns).
- Objects can only transfer gems one tile up, down, left or right; for more see [Action Position Limits](documentation/action_position_limits). The only exception to this is outlined in the tip box below.

The transferring object must be a Core or Unit of your team, and the target object must be a Core or Unit of any team, a Gem Pile, or an empty grid position.

> [!TIP]
> Unlike other action parameters, the server won't stop executing the transfer gems action if the gems amount parameter is set higher than the possible amount. That means you can pass `99999`, even if the source object is only holding `42` gems, to transfer the maximum possible amount of gems anyways.

> [!TIP]
> But what if my core is surrounded by units? How will I get gems to and from it? -> The transferGems action will work back and forth between a unit and its core provided the unit is at _the closest possible unoccupied position_ to its core in [Manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry). If the core is surrounded, the unit must simply get as close as possible for this action to work then, as determined by a floodfill algorithm and the [manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry). This does not apply to dropping money on the floor.

## Signature

```go
func (b *Bot) TransferGems(source *game.Object, targetPos game.Position, amount uint)
```

## Parameters

- `source *game.Object`: The object that the gems should be transferred from. (e.g. Core / Unit)
- `targetPos game.Position`: The position of the object to transfer the gems to, or the non-occupied position where the gems pile should be dropped
- `amount uint`: The amount of gems to transfer or drop

## Return

void (nil-safe: does nothing if source is nil)

## Examples

```go
nearestDeposit := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
udata := unit.GetUnitData()
if nearestDeposit != nil && udata != nil && (udata.Gems == nil || *udata.Gems == 0) {
	nextPos := b.SimplePathfind(unit, nearestDeposit.Pos)
	b.Move(unit, nextPos)
} else {
	myCore := g.MyCore()
	if myCore != nil {
		nextPos := b.SimplePathfind(unit, myCore.Pos)
		b.Move(unit, nextPos)
		b.TransferGems(unit, myCore.Pos, 99999)
	}
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
