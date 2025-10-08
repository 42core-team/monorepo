=====
HEADING="⚙️ function core_action_transferGems(...)"
PERMALINK="core_action_transferGems"
SIDEBAR_HEADING="⚙️ action_transferGems()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L139

## Description

Gives gems to another object or drops it on the floor.

If the position points to an object, the gems will be transferred to that object, otherwise a new gem pile object will be created on the floor.

> **TIP**: But what if my core is surrounded by units? How will I get gems to and from it? -> The transferGems action will work back and forth between a unit and its core provided the unit is at *the closest possible unoccupied position* to its core in Manhattan distance. If the core is surrounded, the unit must simply get as close as possible for this action to work then, as determined by a floodfill algorithm and the manhattan distance.

> **TIP**: Unlike other action parameters, the server won't stop executing the transfer gems action if the gems amount parameter is set higher than the possible amount. That means you can pass `99999`, even if the source object is only holding `42` gems, to transfer the maximum possible amount of gems anyways.

## Signature

```c
void core_action_transferGems(const t_obj *source, t_pos target_pos, unsigned long amount);
```

## Parameters

- `const t_obj *source`: The object that the gems should be transferred from. (e.g. Core / Unit)
- `const t_obj *target_pos`: The position of the object to transfer the gems to, or the non-occupied position where the gems pile should be dropped
- `unsigned long amount`: The amount of gems to transfer or drop

## Return

void

## Examples

```c
t_obj *nearest_deposit = ft_get_deposit_nearest(obj->pos);
if (nearest_deposit && obj->s_unit.gems <= 0)
	core_action_pathfind(obj, nearest_deposit->pos);
else
{
	core_action_pathfind(obj, ft_get_core_own()->pos);
	core_action_transferGems(obj, ft_get_core_own()->pos, -1);
}
```

## Related
