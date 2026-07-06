---
title: "⚙️ function core_action_build(...)"
permalink: "core_action_build"
sidebarTitle: "⚙️ action_build()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/actions.c#L119

## Description

Builds a new object.

The `e_build_type` of the builder unit in the config will determine what gets built (e.g. bomb or wall). ([More info on bombs](documentation/bombs)).

The builder must hold enough money to be able to afford the object it is building as defined by the [config](documentation/configs).

- Objects can only be built one tile up, down, left or right from the builder; for more see [Action Position Limits](documentation/action_position_limits).
- Builders objects can only build if their action cooldown is 0 or less, for more see [Cooldowns](documentation/cooldowns).

## Signature

```c
void core_action_build(const t_obj *builder, t_pos pos);
```

## Parameters

- `const t_obj *builder`: The unit object that has a unit type that is able to build. Must have a `t_build_type`that is not `BUILD_TYPE_NONE` in the config.
- `t_pos pos`: The position to build the object at.

## Return

void

## Examples

```c
t_obj *unit = ft_get_wall_builder_unit();
if (unit && unit->s_unit.gems >= game.config.wall_build_cost)
{
	core_action_build(unit, (t_pos){ unit->pos.x + 1, unit->pos.y });
}
```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
- [🔢 enum e_build_type](reference/c/objects/e_build_type)
- [🧩 struct s_pos](reference/c/objects/s_pos)
