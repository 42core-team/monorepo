=====
HEADING="⚙️ function core_action_attack(...)"
PERMALINK="core_action_attack"
SIDEBAR_HEADING="⚙️ action_attack()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L98

## Description

Attacks a target with a unit.

This action not only used to damage any object, but it's also good for destroying deposits, picking up gem piles & lighting a bomb's fuse.

> **TIP**: There is friendly fire - you can damage your own units and core. So **BE CAREFUL!**

- Units can only attack one tile up, down, left or right; for more see [Action Position Limits](../../documentation/action_position_limits).
- Units can only attack if their action cooldown is 0, for more see [Cooldowns](../../documentation/cooldowns).

## Signature

```c
void core_action_attack(const t_obj *attacker, const t_obj *target);
```

## Parameters

- `const t_obj *attacker`: The unit that should attack
- `const t_obj *target`: The object the unit should attack

## Return

void

## Examples

```c
t_obj *target = ft_get_target();
t_obj *unit = ft_get_attacker_unit();

if (manhattan_distance(unit->pos, target->pos) <= 1 && target->s_unit.team_id != game.my_team_id)
{
	core_action_attack(unit, target);
}
```

## Related

- [🧩 struct s_obj](../objects/s_obj)
