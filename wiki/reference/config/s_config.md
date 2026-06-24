---
title: "🧩 struct s_config"
permalink: "s_config"
sidebarTitle: "🧩 s_config"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L123

## Description

Immutable info on how certain aspects of the game are set up.

[More Info on configs](documentation/configs)

## Signature

```c
typedef struct s_config
{
	unsigned long gridSize;
	unsigned long idle_income;
	unsigned long idle_income_timeout;
	unsigned long deposit_hp;
	unsigned long deposit_income;
	unsigned long gem_pile_income;
	unsigned long core_hp;
	unsigned long core_spawn_cooldown;
	unsigned long initial_balance;
	unsigned long wall_hp;
	unsigned long wall_build_cost;
	unsigned long bomb_hp;
	unsigned long bomb_countdown;
	unsigned long bomb_throw_cost;
	unsigned long bomb_reach;
	unsigned long bomb_damage;
	t_unit_config **units;
} t_config;
```

## Parameters

- `unsigned long gridSize`: The width & height of the map. It's always square.
- `unsigned long idle_income`: How much idle income you get every second.
- `unsigned long idle_income_timeout`: After how many ticks from the start of the game you'll stop receiving idle income.
- `unsigned long deposit_hp`: How much healthpoints a deposit has at the start of the game.
- `unsigned long deposit_income`: A gem pile object with what balance the deposit will drop at its previous position when it is destroyed.
- `unsigned long gem_pile_income`: How many gems a default gem pile object contains.
- `unsigned long core_hp`: How much healthpoints a core has at the start of the game.
- `unsigned long core_spawn_cooldown`: Ticks it takes after a unit was spawned before core can spawn another unit.
- `unsigned long initial_balance`: How many gems in their core a team starts with.
- `unsigned long wall_hp`: How many healthpoints a wall has at the start of the game.
- `unsigned long wall_build_cost`: How much it costs for a builder to build a wall.
- `unsigned long bomb_hp`: How many healthpoints a bomb has.
- `unsigned long bomb_countdown`: How many ticks a bomb takes to explode after being thrown.
- `unsigned long bomb_throw_cost`: How much it costs to throw a bomb.
- `unsigned long bomb_reach`: How big the explosion of a bomb is. See the [bombs documentation](documentation/bombs) page for more info on the bomb explosion pattern.
- `unsigned long bomb_damage`: How much damage a bomb does to objects hit by its explosion.
- `t_unit_config **units`: List of all unit types that are available in the game. NULL-terminated.

## Examples

```c
void ft_on_tick(unsigned long tick)
{
	if (tick < game.config.idle_income_timeout)
	{
		// focus less on miner & carrier spawning
		// ...
```

## Related

- [🧩 struct s_unit_config](reference/c/config/s_unit_config)
- [Configs Documentation](documentation/configs)
