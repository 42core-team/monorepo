---
title: "⚙️ function core_action_transferGems(...)"
permalink: "core_action_transferGems"
sidebarTitle: "⚙️ action_transferGems()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Gives gems to another object or drops it on the floor.

If the position points to an object that can hold gems (Cores / Units / Gem Piles), the gems will be transferred to that object, otherwise a new gem pile object will be created on the floor.

Only units are able to drop gems on the floor.

- Units can only transfer gems if their action cooldown is 0 or less; see [`baseActionCooldown`](documentation/unit_builder#baseactioncooldown). Cores do not have an action cooldown.
- Units can transfer gems to or from a surrounded core when it is as close to that core as possible by [Manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry). This exception may allow a transfer across more than one position. It does not apply when dropping gems on an empty position.

The transferring object must be a Core or Unit of your team, and the target position must be a grid cell containing a Core or Unit of any team, a Gem Pile, or an empty grid position.

The amount must be exact. The action fails if the source does not have enough gems or the destination unit cannot hold the full amount.

## Signature

```c
void core_action_transferGems(const t_obj *source, t_pos target_pos, unsigned long amount);
```

## Parameters

- `const t_obj *source`: The object that the gems should be transferred from. (e.g. Core / Unit)
- `t_pos target_pos`: The position of the object to transfer the gems to, or the empty position where a gem pile should be dropped
- `unsigned long amount`: The amount of gems to transfer or drop

## Return

void

Action functions are queued and are executed between ticks, meaning their results are only reflected in the game state on the next `ft_on_tick()` call.

## Examples

This uses the default [`core_action_travel`](reference/c/actions/core_action_travel#example) policies.

```c
t_obj *nearest_deposit = ft_get_deposit_nearest(obj->pos);
if (nearest_deposit && obj->s_unit.gems <= 0)
	core_action_travel(obj, nearest_deposit->pos, NULL);
else
{
	core_action_travel(obj, ft_get_core_own()->pos, NULL);
	core_action_transferGems(obj, ft_get_core_own()->pos, obj->s_unit.gems);
}
```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
