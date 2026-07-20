#ifndef CORE_LIB_H
#define CORE_LIB_H

#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// ----- OBJECTS -----

/// @brief Type of object
typedef enum e_obj_type
{
	OBJ_CORE,
	OBJ_UNIT,
	OBJ_DEPOSIT,
	OBJ_WALL,
	OBJ_GEM_PILE
} t_obj_type;

/// @brief Position structure for 2D coordinates. 0 indexed. Valid coordinates are 0,1,2,...gridSize-3,gridSize-2,gridSize-1.
typedef struct s_pos
{
	/// @brief X coordinate
	unsigned short x;
	/// @brief Y coordinate
	unsigned short y;
} t_pos;

/// @brief Definition of a units properties resulting from its components.
typedef struct s_unit_properties
{
	int hp;
	int base_action_cooldown;
	int balance_per_cooldown_step;
	int max_balance;
	int damage_reduction_percent;
	int damage_core;
	int damage_unit;
	int damage_object;
	int post_spawn_core_cooldown;
} t_unit_properties;

/// @brief Game object structure representing all entities in the game
typedef struct s_obj
{
	/// @brief Type of the obj
	t_obj_type type;
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
			/// @brief The current gems of the core.
			unsigned long gems;
			/// @brief Countdown to the next tick the core can spawn a unit, defined by core_spawn_cooldown.
			unsigned long spawn_cooldown;
		} s_core;
		struct
		{
			/// @brief The id of the team that owns the unit.
			unsigned long team_id;
			/// @brief The amount of gems the unit is carrying.
			unsigned long gems;
			/// @brief Countdown to the next tick the unit can act. Negative means ready and standing still.
			long action_cooldown;
			/// @brief Null-terminated string array of all component ids the unit has.
			char **components;
			/// @brief The properties of the unit, derived from its components.
			t_unit_properties properties;
			/// @brief Custom name of the unit, or a generated rogue-style name if none was set on spawn.
			char *name;
		} s_unit;
		struct
		{
			/// @brief The amount of gems the deposit / gem pile contains.
			unsigned long gems;
		} s_deposit_gems_pile;
	};
} t_obj;

// ----- GENERAL -----

/// @brief Contains all the data about the game. Read it to your liking! Access it anywhere by typing `game.`
typedef struct s_game
{
	/**
	 * @brief The elapsed ticks since the game started.
	 */
	unsigned long elapsed_ticks;
	/**
	 * @brief The id of the team that you are playing for. Saved in your cores team_id field.
	 */
	unsigned long my_team_id;
	/**
	 * @brief The size of the grid. The grid is square, so this is the width and height of the grid.
	 */
	unsigned short grid_size;
	/**
	 * @brief List of all objects (units, cores, deposits, etc.) and their informations. NULL-terminated.
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

// ----- ACTION FUNCTIONS -----

// ACTION FUNCTIONS are used to perform actions in the game, like creating units, moving them, attacking, etc. Their changes are applied between ticks.

/// @brief Create a new unit with the given components.
/// @param name Custom name for the unit, or NULL for a random rogue-style name.
/// @param component First component id (variadic, NULL-terminated).
void core_action_createUnit(const char *name, char *component, ...);

/// @brief Moves a unit to a specific position.
/// @details Units can only move one tile up, down, left or right; and only if their action_cooldown is 0 or less.
/// @param unit The unit that should move.
/// @param pos The position where the unit should move to. Must be next to the unit object.
void core_action_move(const t_obj *unit, t_pos pos);

/// @brief Weight returned by a travel policy for a position that must not be entered.
#define CORE_TRAVEL_BLOCKED UINT_MAX

/// @brief Travels one optimal step toward a position using a weighted shortest-path search.
/// @param unit The unit that should travel.
/// @param pos The destination position.
/// @param get_weight Function returning the cost of entering a position, or NULL for the default action-cost estimate.
void core_action_travel(const t_obj *unit, t_pos pos, unsigned int (*get_weight)(t_pos, const t_obj *));

/// @brief Attacks a target position with a unit.
/// @details Units can only attack one tile up, down, left or right; and only if their action_cooldown is 0 or less.
/// @param attacker The unit that should attack.
/// @param target The object that the unit should attack.
void core_action_attack(const t_obj *attacker, const t_obj *target);

/// @brief Gives gems to another object or drops it on the floor.
/// @param source The object that the gems should be transferred from.
/// @param target_pos The position of the object to transfer the gems to, or the non-occupied position where the gems should be dropped.
/// @param amount The amount of gems to transfer or drop.
void core_action_transferGems(const t_obj *source, t_pos target_pos, unsigned long amount);

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
t_obj **core_get_objs_filter(bool (*condition)(const t_obj *));

/// @brief Get all of your units with the given name.
/// @param name Unit name to match.
/// @return Null-terminated array of matching owned units or NULL if name is NULL or no owned units match. Free the returned array, not the objects.
t_obj **core_get_units_byName(const char *name);

/// @brief Get the count of all of your units with the given name.
/// @param name Unit name to match.
/// @return The count of matching owned units, or 0 if name is NULL or no owned units match.
unsigned int core_get_units_byName_count(const char *name);

/// @brief Get the nearest object to a given position matching a custom condition.
/// @param pos Position to search from
/// @param condition Selection function pointer returning if the inputted object should be selected
/// @return The nearest object that matches the condition or NULL if no such object exists or no condition is provided.
t_obj *core_get_obj_filter_nearest(t_pos pos, bool (*condition)(const t_obj *));

/// @brief Get the count of all objects matching a custom condition.
/// @param condition Selection function pointer returning if the inputted object should be selected
/// @return The count of objects that match the condition or 0 if no such object exists
unsigned int core_get_objs_filter_count(bool (*condition)(const t_obj *));

// ----- DEBUG FUNCTIONS -----

/// @brief Add debug information to an object for visualization
/// @details This function accumulates debug strings for objects. Multiple calls per tick will append the strings.
/// @param obj The object to attach debug info to
/// @param format printf-style format debug string to add
/// @param ... Format arguments matching the format string
void core_debug_addObjectInfo(const t_obj *obj, const char *format, ...);

/// @brief Add a step to the end of the debug path of an object for visualization
/// @param unit The unit to attach the debug path step to
/// @param pos The position to add as a step to the debug path
void core_debug_addObjectPathStep(const t_obj *unit, t_pos pos);

#endif // CORE_LIB_H
