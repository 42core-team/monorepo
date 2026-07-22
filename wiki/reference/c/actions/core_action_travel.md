---
title: "⚙️ function core_action_travel(...)"
permalink: "core_action_travel"
sidebarTitle: "⚙️ action_travel()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Makes your unit move across the grid, choosing the most efficient path around obstacles to your goal. If something is in the way, the action will even handle destroying the obstacles for you. It does this using [Dijkstra's Algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm).

One call makes the unit take one step. Call it again on later ticks to keep moving.

If the goal cannot be reached, the unit tries to move as close as possible anyway.

You do not have to use travel. If you want to write your own pathfinder or movement rules, call [`core_action_move`](reference/c/actions/core_action_move) and [`core_action_attack`](reference/c/actions/core_action_attack) directly. For advanced bots, this is highly recommended.

### Tile travelability

As an input, the algorithm used by travel requires a travelability value for each tile on the grid. Travelability is made up of a pathfinding weight (the cost of traveling through that tile) and an action (e.g. to be able to categorically disallow walking through certain tiles the unit is unable to break).

You can provide a custom tile-travelability function for advanced logic, or pass `NULL` to use the default policy.

The action is separate from the weight:

- `CORE_TRAVEL_BLOCK` excludes the position from the route.
- `CORE_TRAVEL_PASS` allows routing through the position. The unit moves when it is empty and waits when it is occupied.
- `CORE_TRAVEL_ATTACK` allows routing through the position and attacks its occupant when it becomes the next step.

## Signature

```c
void core_action_travel(const t_obj *unit, t_pos pos,
							t_tile_travelability (*get_tile_travelability)(t_pos, const t_obj *));
```

## Parameters

- `unit`: One of your units. It must be ready to act (`action_cooldown <= 0`).
- `pos`: The goal. It may be anywhere inside the grid and does not need to be empty.
- `get_tile_travelability`: Returns the pathfinding weight and action for a position, or `NULL` for the default policy.

## Return

void

Action functions are queued and are executed between ticks, meaning their results are only reflected in the game state on the next `ft_on_tick()` call.

## Example

```c
void move_towards(t_obj *unit, t_pos goal)
{
	core_action_travel(unit, goal, NULL);
}
```

This custom policy refuses to route through any occupied position:

```c
static t_tile_travelability avoid_occupied_positions(t_pos pos, const t_obj *unit)
{
	(void)unit;
	if (core_get_obj_from_pos(pos)) return (t_tile_travelability){0, CORE_TRAVEL_BLOCK};
	return (t_tile_travelability){1, CORE_TRAVEL_PASS};
}

core_action_travel(unit, goal, avoid_occupied_positions);
```

## Related

- [⚙️ function core_action_move(...)](reference/c/actions/core_action_move)
- [⚙️ function core_action_attack(...)](reference/c/actions/core_action_attack)
- [🧩 struct s_obj](reference/c/objects/s_obj)
- [🧩 struct s_pos](reference/c/objects/s_pos)
