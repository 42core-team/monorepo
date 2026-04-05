---
title: "⚙️ method Bot.Attack(...)"
permalink: "go_Attack"
sidebarTitle: "⚙️ Attack()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/actions.go

## Description

Attacks a target with a unit.

This action not only used to damage any object, but it's also good for destroying deposits, picking up gem piles & lighting a bomb's fuse.

> [!WARNING]
> There is friendly fire - you can damage your own units and core. So **BE _CAREFUL!_**

- Units can only attack one tile up, down, left or right; for more see [Action Position Limits](documentation/action_position_limits).
- Units can only attack if their action cooldown is 0, for more see [Cooldowns](documentation/cooldowns).

## Signature

```go
func (b *Bot) Attack(attacker, target *game.Object)
```

## Parameters

- `attacker *game.Object`: The unit that should attack
- `target *game.Object`: The object the unit should attack

## Return

void (nil-safe: does nothing if either argument is nil)

## Examples

```go
target := getTarget(g)
unit := getAttackerUnit(g)

if g.Distance(unit.Pos, target.Pos) <= 1 && target.IsEnemy(g.MyTeamID) {
	b.Attack(unit, target)
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
