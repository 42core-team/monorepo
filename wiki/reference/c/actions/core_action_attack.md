---
title: "⚙️ function core_action_attack(...)"
permalink: "core_action_attack"
sidebarTitle: "⚙️ action_attack()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L164

## Description

Attacks a target with a unit.

This action damages units, cores, deposits, and walls. Attacking a gem pile collects its gems instead.

> [!WARNING]
> There is friendly fire - you can damage your own units and core. So **BE _CAREFUL!_**

- Units can only attack one tile up, down, left or right; for more see [Action Position Limits](documentation/action_position_limits).
- Units can only attack if their action cooldown is 0 or less; see [`baseActionCooldown`](documentation/unit_builder#baseactioncooldown).

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

- [🧩 struct s_obj](reference/c/objects/s_obj)
- [⚙️ function core_action_travel(...)](reference/c/actions/core_action_travel)
