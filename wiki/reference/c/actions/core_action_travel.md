---
title: "⚙️ function core_action_travel(...)"
permalink: "core_action_travel"
sidebarTitle: "⚙️ action_travel()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L147

## Description

Makes your unit move across the grid, choosing the most efficient path around obstacles to your goal. If something is in the way, the action will even handle destroying the obstacles for you. It does this using [Dijkstras Algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm).

One call makes the unit take one step. Call it again on later ticks to keep moving.

The function works via a weight function. You can either create a custom one for advanced logic, but a basic default weight function is provided in the client lib as well. To use it, just pass NULL instead of a function, and the default logic will be used. You can find the default weight function [here](https://github.com/42core-team/monorepo/blob/bbf439b8ec83d229b0183fb2d712d8b888258dfc/bots/c/client_lib/src/public/action_travel.c#L131).

When making a custom weight function, you can exclude an object from potentially being destroyed to shorten the path if its in the way by returning `CORE_TRAVEL_BLOCKED` from the weight function when it is the input. This is e.g. highly recommended for your own units and especially your core, because otherwise a travel function might deem it most efficient to start killing your own troops. Any finite weight allows travel to route through it and attack the occupant when it becomes the next step.

If the goal cannot be reached, the unit tries to move as close as possible anyways.

You do not have to use travel. If you want to write your own pathfinder or movement rules, call [`core_action_move`](reference/c/actions/core_action_move) and [`core_action_attack`](reference/c/actions/core_action_attack) directly. For advanced bots, this is highly recommended.

## Signature

```c
void core_action_travel(const t_obj *unit, t_pos pos,
						unsigned int (*get_weight)(t_pos, const t_obj *));
```

## Parameters

- `unit`: One of your units. It must be ready to act (`action_cooldown <= 0`).
- `pos`: The goal. It may be anywhere inside the grid and does not need to be empty.
- `get_weight`: Returns the dijkstra-cost of entering a position for this unit, `CORE_TRAVEL_BLOCKED` to make it impassable, or
  pass `NULL` for the default policy.

## Example

```c
void move_towards(t_obj *unit, t_pos goal)
{
	core_action_travel(unit, goal, NULL);
}
```

This custom policy refuses to enter any occupied position, so the unit only uses open ground:

```c
// weight function
static unsigned int avoid_occupied_positions(t_pos pos, const t_obj *unit)
{
	(void)unit;
	return core_get_obj_from_pos(pos) ? CORE_TRAVEL_BLOCKED : 1;
}

core_action_travel(unit, goal, avoid_occupied_positions);
```

## Related

- [⚙️ function core_action_move(...)](reference/c/actions/core_action_move)
- [⚙️ function core_action_attack(...)](reference/c/actions/core_action_attack)
- [🧩 struct s_obj](reference/c/objects/s_obj)
- [🧩 struct s_pos](reference/c/objects/s_pos)
