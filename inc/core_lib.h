#ifndef CORE_LIB_H
#define CORE_LIB_H

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>

typedef enum e_obj_type
{
	OBJ_CORE,
	OBJ_UNIT,
	OBJ_RESOURCE,
	OBJ_WALL,
	OBJ_MONEY,
	OBJ_BOMB
} t_obj_type;
typedef enum e_obj_state
{
	STATE_UNINITIALIZED = 1,
	STATE_ALIVE = 2,
	STATE_DEAD = 3
} t_obj_state;

typedef struct s_pos
{
	unsigned short x;
	unsigned short y;
} t_pos;

typedef struct s_obj
{
	/// @brief Type of the obj
	t_obj_type type;
	/// @brief State of the obj
	t_obj_state state;
	/// @brief Custom data, save whatever you want here.
	void *data;
	/// @brief The id of the obj
	unsigned long id;
	/// @brief The position of the obj
	t_pos pos;
	/// @brief The current healthpoints of the obj
	unsigned long hp;
	union
	{
		struct
		{
			/// @brief The id of the team that owns the core.
			unsigned long team_id;
			/// @brief The current balance of the core.
			unsigned long balance;
		} s_core;
		struct
		{
			/// @brief Which type of unit this is.
			unsigned long unit_type;
			/// @brief The id of the team that owns the unit.
			unsigned long team_id;
			/// @brief The amount of money the unit is carrying.
			unsigned long balance;
			/// @brief The next time the unit can move, as defined by it's speed & how much it's carrying.
			unsigned long next_movement_opp;
		} s_unit;
		struct
		{
			/// @brief The amount of money the resource is carrying.
			unsigned long balance;
		} s_resource_money;
		struct
		{
			/// @brief How much longer the bomb will take to explode.
			unsigned long countdown;
		} s_bomb;
	};
} t_obj;

typedef enum e_unit_type
{
	UNIT_WARRIOR = 0,
	UNIT_MINER = 1,
	UNIT_CARRIER = 2,
	UNIT_BUILDER = 3,
	UNIT_BOMBERMAN = 4
} t_unit_type;
typedef enum e_build_type
{
	BUILD_TYPE_NONE = 0,
	BUILD_TYPE_WALL = 1,
	BUILD_TYPE_BOMB = 2
} t_build_type;

typedef struct s_unit_config
{
	/// @brief The name of the unit.
	char *name;
	/// @brief The unit type of the unit.
	t_unit_type unit_type;
	/// @brief What the unit costs to create.
	unsigned long cost;
	/// @brief How much healthpoints the unit has.
	unsigned long hp;
	/// @brief How much damage the unit deals to cores.
	unsigned long dmg_core;
	/// @brief How much damage the unit deals to units.
	unsigned long dmg_unit;
	/// @brief How much damage the unit deals to resources.
	unsigned long dmg_resource;
	/// @brief How much damage the unit deals to walls.
	unsigned long dmg_wall;
	/// @brief How much damage the unit deals to bombs.
	unsigned long dmg_bomb;
	/// @brief The units build type.
	t_build_type build_type;
	/// @brief The maximum reach the unit can attack. Unused for direct hit attack type units.
	unsigned long attack_reach;
	/// @brief The time a unit waits between moves.
	unsigned long speed;
	/// @brief The minimum time a unit waits between moves.
	unsigned long min_speed;
	/// @brief Whether the unit can build walls.
	bool can_build;
} t_unit_config;
typedef struct s_config
{
	/// @brief The height of the map.
	unsigned long height;
	/// @brief The width of the map.
	unsigned long width;
	/// @brief How many ticks there are in one second.
	unsigned long tick_rate;
	/// @brief How much idle income you get every second.
	unsigned long idle_income;
	/// @brief How many ticks you get idle income.
	unsigned long idle_income_timeout;
	/// @brief How much healthpoints a resource has at the start of the game.
	unsigned long resource_hp;
	/// @brief How much income you get when you destroy a resource.
	unsigned long resource_income;
	/// @brief How much healthpoints a core has at the start of the game.
	unsigned long core_hp;
	/// @brief How much money a team starts with.
	unsigned long initial_balance;
	/// @brief How much healthpoints a wall has at the start of the game.
	unsigned long wall_hp;
	/// @brief How much it costs for a builder to build a wall.
	unsigned long wall_build_cost;
	/// @brief How much healthpoints a bomb has.
	unsigned long bomb_hp;
	/// @brief How many ticks a bomb takes to explode after being thrown.
	unsigned long bomb_countdown;
	/// @brief How much it costs to throw a bomb.
	unsigned long bomb_throw_cost;
	/// @brief How big the explosion of a bomb is.
	unsigned long bomb_reach;
	/// @brief How much damage a bomb does to cores.
	unsigned long bomb_damage;
	/// @brief List of all unit types that are available in the game. NULL-terminated.
	t_unit_config **units;
} t_config;

typedef struct s_game
{
	/**
	 * @brief The elapsed ticks since the game started.
	 */
	unsigned long elapsed_ticks;
	/**
	 * @brief The config contains base informations about the game that don't change like the map size and the unit types.
	 */
	t_config config;
	/**
	 * @brief The id of the team that you are playing for.
	 */
	unsigned long my_team_id;
	/**
	 * @brief List of all objects (units, cores, resources, etc.) and their informations. NULL-terminated.
	 */
	t_obj **objects;
} t_game;

/**
 * This variable contains all the data about the game.
 * It gets updated every time your function is called.
 */
extern t_game game;

int ft_game_start(char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug);

// ------------------ getter -----------------
/*
 * @brief Get any object based on its id
 */
t_obj *core_get_obj_from_id(unsigned long id);
/*
 * @brief Get the object at a specific position or null
 */
t_obj *core_get_obj_from_pos(t_pos pos);
/*
 * @brief Get all objects that match a custom condition
 * @param condition A function pointer that takes a t_obj pointer and returns a bool indicating if the object matches the condition
 * @return An array of pointers to t_obj that match the condition, NULL-terminated
 */
t_obj **core_get_obj_customCondition(bool (*condition)(t_obj *));
/*
 * @brief Get the first object that matches a custom condition
 * @param condition A function pointer that takes a t_obj pointer and returns a bool indicating if the object matches the condition
 * @return The first object that matches the condition or NULL if no such object exists or no condition is provided
 */
t_obj *core_get_obj_customCondition_first(bool (*condition)(t_obj *));
/*
 * @brief Get the nearest object to the given position that matches a custom condition
 * @param pos The position to search from
 * @param condition A function pointer that takes a t_obj pointer and returns a bool indicating if the object matches the condition
 * @return The nearest object that matches the condition or NULL if no such object exists or no condition is provided
 */
t_obj *core_get_obj_customCondition_nearest(t_pos pos, bool (*condition)(t_obj *));
// --------------- unit config getter ---------------
/**
 * @brief Get the unit config by unit_type
 */
t_unit_config *core_get_unitConfig(t_unit_type type);
// --------------------- utils ----------------
int core_util_distance(t_pos pos1, t_pos pos2);

// --------------- actions.c ---------------
/**
 * @brief Creates a unit of a specific type. Same as ft_create, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param unit_type Which type of unit should be created.
 */
t_obj *ft_create_unit(t_unit_type unit_type);
/**
 * @brief Moves a unit to a specific position.
 *
 * @param unit The unit that should be moved.
 * @param pos The position the unit should move to.
 */
void ft_move(t_obj *unit, t_pos pos);
/**
 * @brief Determines next direction to move to reach goal, then calls ft_move().
 *
 * @param unit The unit that should be moved.
 * @param direction The target position.
 */
void ft_travel_to_pos(t_obj *unit, t_pos pos);
/**
 * @brief Attacks a target with a unit.
 *
 * @param attacker The unit that should attack.
 * @param pos The position that should be attacked.
 */
void ft_attack(t_obj *attacker, t_pos pos);
/**
 * @brief Transfers money from one object to another.
 *
 * @param source The object that should transfer the money.
 * @param target The object that should receive the money.
 * @param amount The amount of money that should be transferred.
 */
void ft_transfer_money(t_obj *source, t_pos target_pos, unsigned long amount);
/**
 * @brief Builds a wall at a specific position. Won't work if unit isn't a builder.
 *
 * @param builder The builder that should build the wall.
 * @param pos The position where the wall should be built.
 */
void ft_build(t_obj *builder, t_pos pos);

void core_util_print_object(t_obj *obj);
void core_util_print_objects(bool (*condition)(t_obj *));

#endif // CORE_LIB_H
