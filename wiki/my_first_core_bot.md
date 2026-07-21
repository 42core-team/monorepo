In this guide, you will build a small bot that creates a custom unit and sends it towards the opponent. You will learn how unit design, game ticks, object searches, and travel fit together.

---

## Before you start

If you have not created a team and cloned your repository yet, follow the [Getting Started Guide](README) first. Run `make` once and watch the replay in the visualizer before changing anything. Pay attention to when units act, how cooldown bars change, and which objects block movement.

Open `my-core-bot/src/main.c`. Your starting bot may already resemble the finished example below; this guide explains why each part exists so you can change it deliberately instead of treating it as magic.

## Build your first unit

There are no fixed Warrior or Miner enums anymore. Open the **Unit Builder** for your event on the CORE website and assemble a unit from the components available there. The builder calculates its cost and final properties, checks the event's construction rules, and gives you the component IDs needed by the C API.

Read the [Unit Builder and property guide](documentation/unit_builder) while making the unit. Do not copy a component list from another event: component IDs, modifications, costs, limits, and validity rules can differ.

For this example, assume the builder offers a `combat` component:

```c
core_action_createUnit("Warrior", "combat", NULL);
```

The first argument is your unit's name. The remaining arguments are component IDs. The list must end with `NULL`:

```c
core_action_createUnit("Tank", "armor", "health", "health", NULL);
```

Repeated components are allowed only when the builder accepts the finished design. Passing `NULL` as the name asks the game to generate one.

The call requests a new unit. The unit does not appear in the current tick. If the request works, you will see the unit on a later tick. Creation can fail because the core is on spawn cooldown, lacks gems, has no free spawn position, or because the component design is invalid. See the [`core_action_createUnit` reference](reference/c/actions/core_action_createUnit) for all failure cases.

## The game loop

Every CORE bot starts the library and gives it a callback:

```c
void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("My CORE Bot", argc, argv, ft_on_tick, false);
}
```

`core_startGame` calls `ft_on_tick` whenever a new game state arrives. In that function, you read the current state and choose actions. Those actions happen between ticks. Then `game` is updated and your function is called again.

That boundary explains two important rules:

- A unit requested now is not available in `game.objects` until a later callback.
- Object pointers may change between ticks. Store an object's `id` instead of keeping a `t_obj *` for the next callback.

## Choose how the unit travels

`core_action_travel` makes your unit move across the grid, choosing the most efficient path around obstacles to your
goal. One call adds one move or attack. Start with its built-in policies:

```c
core_action_travel(unit, goal, NULL);
```

The third argument is an optional tile-travelability function. `NULL` selects the default policy. A custom function
returns a `t_tile_travelability` containing the pathfinding weight and one of three actions: `CORE_TRAVEL_BLOCK`
excludes the position, `CORE_TRAVEL_PASS` routes through it but waits while it is occupied, and `CORE_TRAVEL_ATTACK`
attacks its occupant.

The defaults price open ground at one action and estimate how many attacks an obstacle needs. Friendly units are soft
obstacles: their remaining cooldown raises the route cost, but travel can wait for them to move instead of treating
them as walls. Friendly cores remain blocked. The policy checks the matching unit property: `damage_unit` for units,
`damage_core` for cores, and `damage_object` for deposits and walls. A gem pile is breakable only if the unit has enough
unused `max_gems` to collect the whole pile.

The starter bot keeps an older two-argument name as a small wrapper:

```c
void ft_travel_attack(const t_obj *unit, t_pos pos)
{
	core_action_travel(unit, pos, NULL);
}
```

This is not another pathfinder; it only preserves the familiar helper name. Use `core_action_travel` directly when you
want to supply a custom policy. The full [`core_action_travel` reference](reference/c/actions/core_action_travel)
documents its signature, tile travelability, defaults, and unreachable goals.

Travel is optional. If you want to write your own pathfinder or movement rules, use [`core_action_move`](reference/c/actions/core_action_move) to step into an empty position next to the unit and [`core_action_attack`](reference/c/actions/core_action_attack) to attack a target next to it.

## Put the bot together

```c
#include "bot.h"

#include <stdlib.h>

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("My CORE Bot", argc, argv, ft_on_tick, false);
}

void ft_on_tick(unsigned long tick)
{
	(void)tick;

	t_obj *own_core = ft_get_core_own();
	if (own_core && own_core->s_core.spawn_cooldown == 0)
		core_action_createUnit("Warrior", "combat", NULL);

	t_obj *target = ft_get_core_opponent();
	t_obj **units = ft_get_units_own();
	for (size_t i = 0; target && units && units[i]; i++)
		ft_travel_attack(units[i], target->pos);

	free(units);
}
```

Replace `"combat"` with the component list produced by your event's Unit Builder.

The spawn-cooldown check avoids requests that cannot work yet. It does not check cost because the cost belongs to the design shown in the builder. Once the core runs out of gems, creation fails until your strategy earns more.

`ft_get_units_own()` returns a newly allocated, `NULL`-terminated array of pointers to the current objects. The objects belong to the library, but the array belongs to you, which is why the code frees the array and not its elements.

The loop calls travel once per unit. Units whose action cooldown is positive queue nothing; ready units each plan from the current state. Since every call queues at most one adjacent move or attack, repeating this on later ticks advances the route and reacts to changes on the board.

Run `make` and inspect the replay. If creation reports an invalid component, return to the Unit Builder: the tutorial's example component is not a promise about your event.

## Make it yours

The unit name and component list are available in `unit->s_unit.name` and the `NULL`-terminated `unit->s_unit.components` array. Use [`core_get_units_byName`](reference/c/getters/core_get_units_byName) when different designs need different jobs. For example, mining units can target deposits while combat units target the opponent.

The defaults are a baseline, not a strategy. Should a fragile carrier avoid enemies? Should a demolition unit prefer a
short route through a wall? Should units wait for crowded corridors? Supply custom tile travelability for those choices.
If that does not fit your strategy, use move and attack directly.

Continue with the [getter filtering guide](documentation/getter_filtering), [action execution order](documentation/action_execution_order), and the [Unit Builder property guide](documentation/unit_builder).
