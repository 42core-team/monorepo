#ifndef CORE_LIB_H
#define CORE_LIB_H

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>

// ----- OBJECTS -----

/// @brief Type of object
typedef enum e_obj_type
{
	OBJ_CORE,
	OBJ_UNIT,
	OBJ_RESOURCE,
	OBJ_WALL,
	OBJ_MONEY,
	OBJ_BOMB
} t_obj_type;

/// @brief Object state.
/// @details Uninitialized objects should only have their type, state, data, team_id & unit_type read and set.
typedef enum e_obj_state
{
	STATE_UNINITIALIZED = 1,
	STATE_ALIVE = 2,
	STATE_DEAD = 3
} t_obj_state;

/// @brief Type of unit.
typedef enum e_unit_type
{
	UNIT_WARRIOR = 0,
	UNIT_MINER = 1,
	UNIT_CARRIER = 2,
	UNIT_BUILDER = 3,
	UNIT_BOMBERMAN = 4
} t_unit_type;

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
	/// @brief The unique id of the obj
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
			/// @brief Countdown to the next tick the unit can move, defined by it's speed & how much it's carrying.
			unsigned long move_cooldown;
		} s_unit;
		struct
		{
			/// @brief The amount of money the resource / money contains.
			unsigned long balance;
		} s_resource_money;
		struct
		{
			/// @brief How much longer the bomb will take to explode.
			unsigned long countdown;
		} s_bomb;
	};
} t_obj;

// ----- CONFIG -----

/// @brief Determines what a unit can build
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
	/// @brief The units build type.
	t_build_type build_type;
	/// @brief The time a unit waits between moves if it is not carrying money.
	unsigned long speed;
	/// @brief The minimum time a unit waits between moves.
	unsigned long min_speed;
	/// @brief Whether the unit can build walls or bombs.
	bool can_build;
} t_unit_config;
typedef struct s_config
{
	/// @brief The width & height of the map.
	unsigned long gridSize;
	/// @brief How much idle income you get every second.
	unsigned long idle_income;
	/// @brief How many ticks you get idle income.
	unsigned long idle_income_timeout;
	/// @brief How much healthpoints a resource has at the start of the game.
	unsigned long resource_hp;
	/// @brief How much income you get when you destroy a resource.
	unsigned long resource_income;
	/// @brief How much money a money object contains.
	unsigned long money_obj_income;
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
	/// @brief How much damage a bomb does to objects hit by its explosion.
	unsigned long bomb_damage;
	/// @brief List of all unit types that are available in the game. NULL-terminated.
	t_unit_config **units;
} t_config;

// ----- GENERAL -----

/// @brief Contains all the data about the game. Read it to your liking! Access it anywhere by typing `game.`
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
	 * @brief The id of the team that you are playing for. Saved in your cores team_id field.
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

/// @brief Starts the connection lib up, initializes the game, connects to the server & starts the game.
/// @param team_name Name of your team
/// @param argc Argument count from main function
/// @param argv Arguments from main function
/// @param tick_callback Function that will be called every game tick once new server data is available.
/// @param debug Set to true to enable extensive logging.
/// @return 0 on success, another number on failure.
int core_startGame(const char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug);

// ----- GETTER FUNCTIONS -----

/// GETTER FUNCTIONS are used to get information about the current game state.

/// @brief Get any object based on its id.
/// @return The object or NULL if no such object exists.
t_obj *core_get_obj_from_id(unsigned long id);

/// @brief Get any object based on its position.
/// @return The object at the position or NULL if no such object exists.
t_obj *core_get_obj_from_pos(t_pos pos);

/// @brief Get all objects matching a custom condition.
/// @param condition Selection function pointer returning if the inputted object should be selected
/// @return Null-terminated array of selected objects or NULL if no condition is provided or no objects match the condition.
t_obj **core_get_objs_customCondition(bool (*condition)(const t_obj *));

/// @brief Get the first object matching a custom condition.
/// @param condition Selection function pointer returning if the inputted object should be selected
/// @return The first object that matches the condition or NULL if no such object exists or no condition is provided.
t_obj *core_get_obj_customCondition_first(bool (*condition)(const t_obj *));

/// @brief Get the nearest object to a given position matching a custom condition.
/// @param pos Position to search from
/// @param condition Selection function pointer returning if the inputted object should be selected
/// @return The nearest object that matches the condition or NULL if no such object exists or no condition is provided.
t_obj *core_get_obj_customCondition_nearest(t_pos pos, bool (*condition)(const t_obj *));

/// @brief Get the unit config for a specific unit type.
/// @param type The type of unit to get the config for.
/// @return The unit config or NULL if no such unit type or unit config exists.
t_unit_config *core_get_unitConfig(t_unit_type type);

// ----- ACTION FUNCTIONS -----

// ACTION FUNCTIONS are used to perform actions in the game, like creating units, moving them, attacking, etc. Their changes are applied between ticks.

/// @brief Create a new unit of specified type.
/// @details The unit will be uninitialized, meaning you can read & write only read its type, state, data, team_id & unit_type.
/// @param unit_type The type of unit to create.
/// @return A newly created, uninitialized unit object or NULL if the unit could not be created.
t_obj *core_action_createUnit(t_unit_type unit_type);

/// @brief Moves a unit to a specific position.
/// @details Units can only move one tile up, down, left or right; and only if their move_cooldown is 0.
/// @param unit The unit that should move.
/// @param pos The position where the unit should move to.
void core_action_move(const t_obj *unit, t_pos pos);

/// @brief Attacks a target position with a unit.
/// @details Units can only attack one tile up, down, left or right; and only if their move_cooldown is 0.
/// @param attacker The unit that should attack.
/// @param pos The position where the unit should attack.
void core_action_attack(const t_obj *attacker, t_pos pos);

/// @brief Gives money to another object or drops it on the floor.
/// @param source The object that the money should be transferred from.
/// @param target_pos The position of the object to transfer the money to, or the non-occupied position where the money should be dropped.
/// @param amount The amount of money to transfer or drop.
void core_action_transferMoney(const t_obj *source, t_pos target_pos, unsigned long amount);

/// @brief Builds a new object.
/// @details Units can only build one tile up, down, left or right.
/// @param builder The unit that should build a new object. What will be built depends on the buildType of the builder unit.
/// @param pos The position where the object should be built.
void core_action_build(const t_obj *builder, t_pos pos);

// ----- PRINT FUNCTIONS -----

// PRINT FUNCTIONS are used to print information about the game state to the console.

/// @brief Prints all information about the current game state of a given object.
/// @param obj The object to print information about.
void core_print_object(const t_obj *obj);

/// @brief Prints all objects that match a custom condition.
/// @param condition Selection function pointer returning if the inputted object should be selected
void core_print_objects(bool (*condition)(const t_obj *));

/// @brief Prints a selected unit config.
/// @param unit_type The type of unit to print the config for.
void core_print_config_unit(t_unit_type unit_type);

/// @brief Prints the entire game config and all unit configs
void core_print_config(void);

#endif // CORE_LIB_H
