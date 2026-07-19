---
title: "⚙️ function core_action_transferGems(...)"
permalink: "core_action_transferGems"
sidebarTitle: "⚙️ action_transferGems()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L170

## Description

Gives gems to another object or drops it on the floor.

If the position points to an object that can hold gems (Cores / Units / Gem Piles), the gems will be transferred to that object, otherwise a new gem pile object will be created on the floor.

Only units are able to drop gems on the floor.

- Units can only transfer gems if their action cooldown is 0 or less; see [`baseActionCooldown`](documentation/unit_builder#baseactioncooldown). Cores do not have an action cooldown.
- Objects can only transfer gems one tile up, down, left or right; for more see [Action Position Limits](documentation/action_position_limits). The only exception to this is outlined in the tip box below.

The transferring object must be a Core or Unit of your team, and the target object must be a Core or Unit of any team, a Gem Pile, or an empty grid position.

> [!TIP]
> Unlike other action parameters, the server won't stop executing the transfer gems action if the gems amount parameter is set higher than the possible amount. That means you can pass `99999`, even if the source object is only holding `42` gems, to transfer the maximum possible amount of gems anyways.

> [!TIP]
> A unit can transfer gems to or from a surrounded core when it is as close to that core as possible by [Manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry). This exception may allow a transfer across more than one position. It does not apply when dropping gems on an empty position.

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

## Examples

This uses the default [`core_action_travel`](reference/c/actions/core_action_travel#example) policies.

```c
t_obj *nearest_deposit = ft_get_deposit_nearest(obj->pos);
if (nearest_deposit && obj->s_unit.gems <= 0)
	core_action_travel(obj, nearest_deposit->pos, NULL, NULL);
else
{
	core_action_travel(obj, ft_get_core_own()->pos, NULL, NULL);
	core_action_transferGems(obj, ft_get_core_own()->pos, 99999);
}
```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
