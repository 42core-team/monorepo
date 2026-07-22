---
title: "⚙️ function core_debug_addObjectInfo(...)"
permalink: "debug_addObjectInfo"
sidebarTitle: "⚙️ debug_addObjectInfo()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/debug.c

## Description

Attaches an arbitrary string to your units or core that will be displayed in the object's tooltip if someone hovers over the object in the visualizer. May be helpful for a variety of debugging purposes.

You can also format strings dynamically by using the function the same way as one would use `printf`. All flags work as normal. Check out the example below for details.

This function can be called multiple times in a given tick; all the strings passed to it will be appended together.

> [!WARNING]
> Opponents cannot see your debug data during the game, but may be able to view it once the game has concluded through the visualizer. (Better not reveal your super-secret strategy...)

## Signature

```c
void core_debug_addObjectInfo(const t_obj *obj, const char *format, ...);
```

## Parameters

- `const t_obj *obj`: The object which the debug info will be attached to.
- `const char *info`: The string which will be attached to the debug data.

## Return

void

## Examples

```c
core_action_travel(own_team_warriors[i], ft_get_core_opponent()->pos, NULL);
core_debug_addObjectInfo(units[i], "I am a warrior! 🗡️ - I am heading for the opponent core at [%d,%d]! 🏰\n", ft_get_core_opponent()->pos.x, ft_get_core_opponent()->pos.y);
```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
