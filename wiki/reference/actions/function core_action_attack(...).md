## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L124

## Description

Attacks a target position with a unit.

This action not only used to damage any object, but it's also good for destroying deposits, picking up gem piles & lighting a bomb's fuse.

Units can only attack one tile up, down, left or right; and only if their action_cooldown is 0.

> **TIP**: There is friendly fire - you can damage your own units and core. So **BE CAREFUL!**

## Signature

```c
void core_action_attack(const t_obj *attacker, t_pos pos);
```

## Parameters

- `attacker`: The unit that should attack
- `pos`: The position where the unit should attack

## Examples

```c
t_obj *target = ft_get_target();
t_obj *unit = ft_get_attacker_unit();

if (manhattan_distance(unit->pos, target->pos) <= 1)
{
	core_action_attack(unit, target->pos);
}
```

## Related
