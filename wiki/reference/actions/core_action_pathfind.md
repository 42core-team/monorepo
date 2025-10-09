=====
HEADING="⚙️ function core_action_pathfind(...)"
PERMALINK="core_action_pathfind"
SIDEBAR_HEADING="⚙️ action_pathfind()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L41

## Description

Very simple pathfinding logic implementation. It won't help you win, but it can be used to easily get started while you figure out how the game works.

When you call it, it will determine the next move to make and then immediately either move there or attack objects in its way, provided the objects in its way aren't your units or core.

## Signature

```c
void core_action_pathfind(const t_obj *unit, t_pos pos);
```

## Parameters

- `const t_obj *unit`: The unit that should move
- `t_pos pos`: The position where the unit should move to

## Return

void

## Examples

```c
t_obj **own_team_warriors = core_get_objs_filter(ft_is_own_team_warrior);
for (int i = 0; own_team_warriors && own_team_warriors[i]; i++)
{
	core_action_pathfind(own_team_warriors[i], ft_get_core_opponent()->pos);
}
```

## Related

- [🧩 struct s_obj](../objects/s_obj)
- [🧩 struct s_pos](../objects/s_pos)
