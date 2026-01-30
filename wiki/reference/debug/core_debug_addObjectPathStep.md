---
title: "⚙️ function core_debug_addObjectPathStep(...)"
permalink: "debug_addObjectPathStep"
sidebarTitle: "⚙️ debug_addObjectPathStep()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/debug.c#L54

## Description

Used to build a debug path which displays where a unit will move next in the visualizer. This is done by calling the function multiple times, one for each grid tile the unit is planning on traversing, which will then be appended together by the core library for your convenience.

> [!WARNING]
> Opponents cannot see your debug path during the game, but may be able to view it once the game has concluded through the visualizer.

## Signature

```c
void core_debug_addObjectPathStep(const t_obj *unit, t_pos pos);
```

## Parameters

- `const t_obj *unit`: The unit which the debug path step will be attached to.
- `t_pos pos`: The position the unit is planning to move to.

## Return

void

## Examples

```c
for (size_t j = 0; j < planned_path.length; j++)
	core_debug_addObjectPathStep(unit, planned_path.steps[j]);
```

## Related

- [🧩 struct s_obj](reference/objects/s_obj)
- [🧩 struct s_pos](reference/objects/s_pos)
