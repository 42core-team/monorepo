---
title: "⚙️ function core_action_createUnit(...)"
permalink: "core_action_createUnit"
sidebarTitle: "⚙️ action_createUnit()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L136

## Description

Requests a new unit built from component IDs. Use the event's [Unit Builder](documentation/unit_builder) to choose the components and see the unit's properties, cost, and core spawn cooldown.

Every unit starts with the event's default cost and properties. Each component changes those properties and adds to the cost.

No unit is created when a component is unknown, there are too many components, the design breaks an event rule, the core cannot afford it, or `s_core.spawn_cooldown > 0`.

The unit appears on a later tick, at the nearest empty position around the core. If the grid has no empty position, creation fails.

## Signature

```c
void core_action_createUnit(const char *name, char *component, ...);
```

## Parameters

- `const char *name`: Custom unit name. Pass `NULL` for a generated rogue-style name.
- `char *component`: First component ID from the Unit Builder, or `NULL` for no components.
- `...`: Any remaining component IDs. The list must end with `NULL`.

Component IDs may be repeated when the resulting design satisfies the builder's component-count limit and validity rules.

## Return

`void`. Creation failures are reported with the next game state. No unit is added when creation fails.

## Examples

```c
// One component and a custom name.
core_action_createUnit("Warrior", "combat", NULL);

// Repeated components. Use only if this design is valid in your event's builder.
core_action_createUnit("Tank", "armor", "health", "health", NULL);

// Use a generated name.
core_action_createUnit(NULL, "speed", NULL);
```

## Related

- [Unit Builder](documentation/unit_builder)
- [🧩 struct s_obj](reference/c/objects/s_obj)
