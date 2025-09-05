# Configs

If you are in an event, **look at the config**! They contain invaluable info.

## Config Types

- The Game Config contains all gameplay-related settings: Units and their damage values, deposit gem counts, and bomb countdowns.
- The Server Config contains settings about how the game runs, e.g. timeouts.

## Where do I find the Config

Once you've cloned your bot, find the configs in the configs/ folder.

If it's easier, you can also print the config using the provided core library functions.

## Fun fact

You can totally edit the configs in your local folder, and it will work. It's a lot of fun to play around with and see how stable and reactive your bot is, but it won't have an effect on games played on the website or in the final tournament.

## Library Structs

### `t_unit_config`

- `name`: The name of the unit.
- `unit_type`: The unit type of the unit.
- `cost`: What the unit costs to create.
- `hp`: How much healthpoints the unit has.
- `dmg_core`: How much damage the unit deals to cores.
- `dmg_unit`: How much damage the unit deals to units.
- `dmg_deposit`: How much damage the unit deals to deposits.
- `dmg_wall`: How much damage the unit deals to walls.
- `build_type`: The units build type.
- `baseActionCooldown`: The time a unit waits between moves if it is not carrying any gems.
- `maxActionCooldown`: The maximum boundary of a units wait time between moves if it's carrying a ton of gems.
- `can_build`: Whether the unit can build walls or bombs.

```c
typedef struct s_unit_config
{
	char *name;
	t_unit_type unit_type;
	unsigned long cost;
	unsigned long hp;
	unsigned long dmg_core;
	unsigned long dmg_unit;
	unsigned long dmg_deposit;
	unsigned long dmg_wall;
	t_build_type build_type;
	unsigned long baseActionCooldown;
	unsigned long maxActionCooldown;
	bool can_build;
} t_unit_config;
```

> **NOTE**: Building won't be within the scope of this Rush, so don't worry about constructing walls or blowing things up with bombs.

### `t_build_type`

```c
typedef enum e_build_type
{
	BUILD_TYPE_NONE = 0,
	BUILD_TYPE_WALL = 1,
	BUILD_TYPE_BOMB = 2
} t_build_type;
```

Determines whether and what the unit will build if you use it as the builder argument when executing a build action.

### `t_config`

- `gridSize`: The width & height of the map. It's always square.
- `idle_income`: How much idle income you get every second.
- `idle_income_timeout`: After how many ticks from the start of the game you'll stop receiving idle income.
- `deposit_hp`: How much healthpoints a deposit has at the start of the game.
- `deposit_income`: A gem pile object with what balance the deposit will drop at its previous position when it is destroyed.
- `gem_pile_income`: How many gems a default gem pile object contains.
- `core_hp`: How much healthpoints a core has at the start of the game.
- `initial_balance`: How many gems in their core a team starts with.
- `wall_hp`: How many healthpoints a wall has at the start of the game.
- `wall_build_cost`: How much it costs for a builder to build a wall.
- `bomb_hp`: How many healthpoints a bomb has.
- `bomb_countdown`: How many ticks a bomb takes to explode after being thrown.
- `bomb_throw_cost`: How much it costs to throw a bomb.
- `bomb_reach`: How big the explosion of a bomb is. (Plus-shaped, like amazing bomberman. This value describes the radius.)
- `bomb_damage`: How much damage a bomb does to objects hit by its explosion.
- `units`: List of all unit types that are available in the game. NULL-terminated.

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
