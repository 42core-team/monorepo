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

Travel has two optional policies: a weight function decides how costly each position is, and a break function decides
whether the traveling unit may attack an object blocking the route. Pass `NULL` for either policy to use its default.

The unit then does one of two things:

- If the next position is empty, it moves there.
- If an attackable object blocks the next position, it attacks it.

If the goal cannot be reached, the unit moves towards the reachable position closest to it. If the unit cannot get any closer, no action is added.

## Signature

```c
void core_action_travel(const t_obj *unit, t_pos pos,
						unsigned int (*get_weight)(t_pos, const t_obj *),
						bool (*can_break)(const t_obj *, const t_obj *));
```

## Parameters

- `unit`: One of your units. It must be ready to act (`action_cooldown <= 0`).
- `pos`: The goal. It may be anywhere inside the grid and does not need to be empty.
- `get_weight`: Returns the cost of entering a position for this unit, or `NULL` for the default.
- `can_break`: Returns whether this unit may break an object, or `NULL` for the default.

The callbacks should only inspect the game and return a value. Do not add actions or change state inside them. Each
position's weight is requested at most once per travel call, and `can_break` is called only for occupied positions.

The two callbacks are independent: you can customize one and pass `NULL` for the other. Travel never attacks a
friendly unit or core, even if a custom `can_break` callback returns `true` for it.

## Default behavior

The default weight is `1` for open ground. For an occupied position, it estimates the attacks required to destroy the
object from its health, armor, and the traveling unit's relevant damage property, then adds one for moving onto the
cleared position. Travel therefore prefers a short open detour over repeatedly attacking a durable obstacle.

The default break policy:

- never attacks a friendly unit or core;
- attacks units only when `damage_unit` is positive;
- attacks cores only when `damage_core` is positive;
- attacks walls and deposits only when `damage_object` is positive; and
- attacks a gem pile only when the unit's remaining `max_balance` can hold the entire pile.

These checks answer whether the unit can eventually clear the object, not whether it can destroy it in one attack.

## Example

```c
void move_towards(t_obj *unit, t_pos goal)
{
	core_action_travel(unit, goal, NULL, NULL);
}
```

To customize only weighting while retaining the safe default break policy:

```c
static unsigned int avoid_occupied_positions(t_pos pos, const t_obj *unit)
{
	(void)unit;
	return core_get_obj_from_pos(pos) ? 20 : 1;
}

core_action_travel(unit, goal, avoid_occupied_positions, NULL);
```

## When no action is added

Travel does nothing when:

- `unit` is `NULL` or is not a unit;
- the unit is still on cooldown;
- the goal is outside the grid;
- the unit is already at the goal;
- no reachable position gets the unit closer to the goal.

## About weights

Travel prefers routes with the lowest total entry weight; the starting position adds no cost. Weights are unsigned, so
custom callbacks may return zero or a positive value. Travel uses Dijkstra's algorithm for every call.

You do not have to use travel. If you want to write your own pathfinder or movement rules, call [`core_action_move`](reference/c/actions/core_action_move) and [`core_action_attack`](reference/c/actions/core_action_attack) directly.

## Related

- [⚙️ function core_action_move(...)](reference/c/actions/core_action_move)
- [⚙️ function core_action_attack(...)](reference/c/actions/core_action_attack)
- [🧩 struct s_obj](reference/c/objects/s_obj)
- [🧩 struct s_pos](reference/c/objects/s_pos)
