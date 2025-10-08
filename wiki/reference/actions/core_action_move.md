=====
HEADING="⚙️ function core_action_move(...)"
PERMALINK="core_action_move"
SIDEBAR_HEADING="⚙️ action_move()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#32

## Description

Moves a unit to a specific position.
Units can only move one tile up, down, left or right; and only if their action_cooldown is 0.

## Signature

```c
void core_action_move(const t_obj *unit, t_pos pos);
```

## Parameters

- `const t_obj *unit`: The unit that should move
- `t_pos pos`: The position where the unit should move to

## Return

void

## Examples

```c
t_obj *target = ft_get_target();
t_obj *moving_unit = ft_get_moving_unit();

if (moving_unit->pos.x < target->pos.x)
{
	core_action_move(moving_unit, (t_pos){moving_unit->pos.x + 1, moving_unit->pos.y});
}
if (moving_unit->pos.y < target->pos.y)
{
	core_action_move(moving_unit, (t_pos){moving_unit->pos.x, moving_unit->pos.y + 1});
}
// ...
```

## Related
