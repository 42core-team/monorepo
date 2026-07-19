---
title: "⚙️ function core_action_travel(...)"
permalink: "core_action_travel"
sidebarTitle: "⚙️ action_travel()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L147

## Description

Makes your unit move across the grid, choosing the most efficient path around obstacles to your goal.

One call makes the unit take one step. Call it again on later ticks to keep moving.

An optional weight function decides how costly each position is. Its return value also decides whether the position is
passable: return `CORE_TRAVEL_BLOCKED` when the position must not be entered or its occupant must not be attacked. Pass
`NULL` to use the default policy.

The unit then does one of two things:

- If the next position is empty, it moves there.
- If an attackable object blocks the next position, it attacks it.

If the goal cannot be reached, the unit moves towards the reachable position closest to it. If the unit cannot get any closer, no action is added.

## Signature

```c
void core_action_travel(const t_obj *unit, t_pos pos,
						unsigned int (*get_weight)(t_pos, const t_obj *));
```

## Parameters

- `unit`: One of your units. It must be ready to act (`action_cooldown <= 0`).
- `pos`: The goal. It may be anywhere inside the grid and does not need to be empty.
- `get_weight`: Returns the cost of entering a position for this unit, `CORE_TRAVEL_BLOCKED` to make it impassable, or
  pass `NULL` for the default policy.

The callback should only inspect the game and return a value. Do not add actions or change state inside it. Each
position's weight is requested once per travel call.

For an occupied position, any finite weight allows travel to route through it and attack the occupant when it becomes
the next step. `CORE_TRAVEL_BLOCKED` is exactly `UINT_MAX`; it is reserved and is not a valid finite weight. The largest
finite weight is therefore `UINT_MAX - 1`. Travel never attacks a friendly unit or core, even if a custom callback
returns a finite weight for it.

A custom callback replaces the default damage and gem-capacity checks. Returning a finite weight for any non-friendly
occupant opts into attacking it, so only do that when the unit can actually clear that object.

## Default behavior

The default weight is `1` for open ground. For an occupied position, it estimates the attacks required to destroy the
object from its health, armor, and the traveling unit's relevant damage property, then adds one for moving onto the
cleared position. Travel therefore prefers a short open detour over repeatedly attacking a durable obstacle.

The default policy returns `CORE_TRAVEL_BLOCKED` for:

- a friendly unit or core;
- a unit when `damage_unit` is not positive;
- a core when `damage_core` is not positive;
- a wall or deposit when `damage_object` is not positive; and
- a gem pile that does not fit within the unit's remaining `max_balance`.

These checks answer whether the unit can eventually clear the object, not whether it can destroy it in one attack.

## Example

```c
void move_towards(t_obj *unit, t_pos goal)
{
	core_action_travel(unit, goal, NULL);
}
```

This custom policy refuses to enter any occupied position, so the unit only uses open ground:

```c
static unsigned int avoid_occupied_positions(t_pos pos, const t_obj *unit)
{
	(void)unit;
	return core_get_obj_from_pos(pos) ? CORE_TRAVEL_BLOCKED : 1;
}

core_action_travel(unit, goal, avoid_occupied_positions);
```

## When no action is added

Travel does nothing when:

- `unit` is `NULL` or is not a unit;
- the unit is still on cooldown;
- the goal is outside the grid;
- the unit is already at the goal;
- no reachable position gets the unit closer to the goal.

## About weights

Travel prefers routes with the lowest total entry weight; the starting position adds no cost. Finite weights range
from `0` through `UINT_MAX - 1`. `CORE_TRAVEL_BLOCKED` (`UINT_MAX`) is skipped rather than added to the route cost.
Travel uses Dijkstra's algorithm for every call.

You do not have to use travel. If you want to write your own pathfinder or movement rules, call [`core_action_move`](reference/c/actions/core_action_move) and [`core_action_attack`](reference/c/actions/core_action_attack) directly.

## Related

- [⚙️ function core_action_move(...)](reference/c/actions/core_action_move)
- [⚙️ function core_action_attack(...)](reference/c/actions/core_action_attack)
- [🧩 struct s_obj](reference/c/objects/s_obj)
- [🧩 struct s_pos](reference/c/objects/s_pos)
