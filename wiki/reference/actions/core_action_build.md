=====
HEADING="⚙️ function core_action_build(...)"
PERMALINK="core_action_build"
SIDEBAR_HEADING="⚙️ action_build()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L155

## Description

Builds a new object.

The `t_build_type` of the builder unit in the config will determine what gets built (e.g. bomb or wall).

Objects can only be built one tile up, down, left or right from the builder; and only if their builders action_cooldown is 0.

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
