---
title: "⚙️ function core_debug_addObjectInfo(...)"
permalink: "debug_addObjectInfo"
sidebarTitle: "⚙️ debug_addObjectInfo()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/debug.c#L50

## Description

Attaches an arbitrary string to any object (typically an object or your core) that will be displayed in the object's tooltip if someone hovers over the object in the visualizer. May be helpful for a variety of debugging purposes.

This function can be called multiple times in a given tick, all of the string the function is called with will be appended together at the end.

> [!WARNING]
> Opponents cannot see your debug data during the game, but may be able to view it once the game has concluded through the visualizer. (Better not reveal your super-secret strategy...)

## Signature

```c
void core_debug_addObjectInfo(const t_obj *obj, const char *info);
```

## Parameters

- `const t_obj *obj`: The object which the debug info will be attached to.
- `const char *info`: The string which will be attached to the debug data.

## Return

void

## Examples

```c
if (path.length > 0)
	core_action_move(own_team_warriors[i], path.steps[0]);
core_debug_addObjectInfo(own_team_warriors[i], "Moving towards opponent core\n");
core_action_attack(own_team_warriors[i], ft_get_core_opponent());
```

## Related

- [🧩 struct s_obj](reference/objects/s_obj)
