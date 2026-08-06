---
title: "⚙️ function core_action_move(...)"
permalink: "core_action_move"
sidebarTitle: "⚙️ action_move()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Moves a unit to a specific position.

- Units can only move one tile up, down, left or right; for more see [Action Position Limits](documentation/action_position_limits).
- Units can only move if their action cooldown is 0 or less; see [`baseActionCooldown`](documentation/unit_builder#baseactioncooldown).

## Signature

```c
void core_action_move(const t_obj *unit, t_pos pos);
```

## Parameters

- `const t_obj *unit`: The unit that should move
- `t_pos pos`: The position where the unit should move to

## Return

void

Action functions are queued and are executed between ticks, meaning their results are only reflected in the game state on the next `ft_on_tick()` call.

## Examples

```c
t_obj *target = ft_get_target();
t_obj *moving_unit = ft_get_moving_unit();
t_pos next = moving_unit->pos;

if (moving_unit->pos.x < target->pos.x)
	next.x++;
else if (moving_unit->pos.x > target->pos.x)
	next.x--;
else if (moving_unit->pos.y < target->pos.y)
	next.y++;
else if (moving_unit->pos.y > target->pos.y)
	next.y--;

if ((next.x != moving_unit->pos.x || next.y != moving_unit->pos.y)
	&& core_get_obj_from_pos(next) == NULL)
	core_action_travel(moving_unit, next, NULL);
```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
- [🧩 struct s_pos](reference/c/objects/s_pos)
- [⚙️ function core_action_travel(...)](reference/c/actions/core_action_travel)
