#ifndef CON_LIB_H
#define CON_LIB_H

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

typedef enum e_status
{
	/// @brief The game is running.
	STATUS_OK = 0,
	/// @brief The game is paused.
	STATUS_PAUSED = 1,
	/// @brief The game is over.
	STATUS_END = 2,
	/// @brief The game is waiting for clients to connect.
	STATUS_WAIT_FOR_CLIENTS = 3
} t_status;

typedef enum e_obj_type
{
	OBJ_UNIT,
	OBJ_CORE,
	OBJ_RESOURCE
} t_obj_type;
typedef enum e_obj_state
{
	STATE_UNINITIALIZED = 1,
	STATE_ALIVE = 2,
	STATE_DEAD = 3
}	t_obj_state;

typedef enum e_direction
{
	DIR_UP = 0,
	DIR_RIGHT = 1,
	DIR_DOWN = 2,
	DIR_LEFT = 3
}	t_direction;

typedef struct s_pos
{
	unsigned short x;
	unsigned short y;
}	t_pos;

typedef struct s_obj
{
	/// @brief Type of the obj
	t_obj_type	type;
	/// @brief State of the obj
	t_obj_state	state;
	/// @brief Custom data, save whatever you want here.
	void	*data;
	/// @brief The id of the obj
	unsigned long id;
	/// @brief The position of the obj
	t_pos pos;
	/// @brief The current healthpoints of the obj
	unsigned long hp;
	union {
		struct
		{
			/// @brief The id of the team that owns the core.
			unsigned long team_id;
			/// @brief The current balance of the core.
			unsigned long balance;
		}	s_core;
		struct
		{
			/// @brief Which type of unit this is.
			unsigned long type_id;
			/// @brief The id of the team that owns the unit.
			unsigned long team_id;
			/// @brief The amount of money the unit is carrying.
			unsigned long balance;
		}	s_unit;
	};
} t_obj;

typedef enum e_unit_type
{
	UNIT_WARRIOR = 0,
	UNIT_MINER = 1
} t_unit_type;

typedef struct s_unit_config
{
	/// @brief The name of the unit.
	char *name;
	/// @brief The type id of the unit.
	t_unit_type type_id;
	/// @brief What the unit costs to create.
	unsigned long cost;
	/// @brief How much healthpoints the unit has.
	unsigned long hp;
	/// @brief How much damage the unit deals to cores.
	long dmg_core;
	/// @brief How much damage the unit deals to units.
	long dmg_unit;
	/// @brief How much damage the unit deals to resources.
	long dmg_resource;
	/// @brief How much damage the unit deals to walls.
	long dmg_wall;
	/// @brief How far the unit can travel in one second.
	unsigned long speed;
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
	/// @brief List of all unit types that are available in the game. NULL-terminated.
	t_unit_config **units;
} t_config;

typedef struct s_action_create
{
	unsigned long type_id;
} t_action_create;
typedef struct s_action_travel
{
	unsigned long id;
	t_direction direction;
} t_action_travel;
typedef struct s_actions
{
	t_action_create *creates;
	unsigned int creates_count;
	t_action_travel *travels;
	unsigned int travels_count;
} t_actions;

typedef struct s_game
{
	/**
	 * @brief The status of the game. Can be OK, PAUSED, END or WAIT_FOR_CLIENTS.
	 */
	t_status status;
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
	 * @brief List of all cores and their informations. NULL-terminated.
	 */
	t_obj **cores;
	/**
	 * @brief List of all resources and their informations. NULL-terminated.
	 */
	t_obj **resources;
	/**
	 * @brief List of all walls and their informations. NULL-terminated.
	 */
	t_obj **walls;
	/**
	 * @brief List of all units and their informations. NULL-terminated.
	 */
	t_obj **units;
	/**
	 * @brief List of all actions that will be send to the server when your function ends.
	 */
	t_actions actions;
} t_game;

/**
 * This variable contains all the data about the game.
 * It gets updated every time your function is called.
*/
extern t_game game;

void ft_init_con(char *team_name, int *argc, char **argv);
void ft_close_con();
void ft_enable_debug();
void ft_loop(void (*ft_init_func)(void *ptr), void (*ft_user_loop)(void *ptr), void (*ft_on_exit)(void *ptr), void *ptr);

// ------------------ getter -----------------
/*
 * @brief Get any object based on its id
 */
t_obj	*ft_get_obj_from_id(unsigned long id);
// --------------- core getter ---------------
/**
 * @brief Get my core
 */
t_obj	*ft_get_my_core(void);
/**
 * @brief Get the first opponent core
 */
t_obj	*ft_get_first_opponent_core(void);
/**
 * @brief Get the nearest core to the given obj
 */
t_obj	*ft_get_nearest_core(t_obj *obj);
// --------------- unit getter ---------------
/**
 * @brief Returns a pointer array of all units
 */
t_obj **ft_get_all_units();
/**
 * @brief Allocates a pointer array of all my units
 */
t_obj	**ft_get_my_units(void);
/**
 * @brief Allocates a pointer array of all opponent units
 */
t_obj	**ft_get_opponent_units(void);
/**
 * @brief Get the nearest unit to the given unit
 */
t_obj	*ft_get_nearest_unit(t_obj *unit);
/**
 * @brief Get the nearest opponent unit to the given unit
 */
t_obj	*ft_get_nearest_opponent_unit(t_obj *unit);
/**
 * @brief Get the nearest team unit to the given unit
 */
t_obj	*ft_get_nearest_team_unit(t_obj *unit);
// --------------- resource getter ---------------
/**
 * @brief Get the nearest resource to the given unit
 */
t_obj	*ft_get_nearest_resource(t_obj *unit);
// --------------- unit config getter ---------------
/**
 * @brief Get the unit config by type_id
 */
t_unit_config	*ft_get_unit_config(t_unit_type type);
// ---------------- get utils ----------------
double	ft_distance(t_obj *obj1, t_obj *obj2);

// --------------- actions.c ---------------
/**
 * @brief Creates a unit of a specific type. Same as ft_create, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param type_id Which type of unit should be created.
 */
t_obj	*ft_create_unit(t_unit_type type_id);
/**
 * @brief Moves a unit to a specific position.
 * 
 * @param unit The unit that should be moved.
 * @param direction The direction the unit should move.
 */
void	ft_move(t_obj *unit, t_direction direction);
/**
 * @brief Determines next direction to move to reach goal, then calls ft_move().
 * 
 * @param unit The unit that should be moved.
 * @param direction The target position.
 */
void	ft_travel_to_pos(t_obj *unit, t_pos pos);

// -------------- print_utils.c --------------
/**
 * @brief Prints the current game status into stdout
 */
void ft_print_status();
/**
 * @brief Prints the current game cores with inforamtion about their id, team_id, x, y and hp into stdout
 *
 */
void ft_print_cores();
/**
 * @brief Prints the current game resources with inforamtion about their id, value, x, y and hp into stdout
 *
 */
void ft_print_resources();
/**
 * @brief Prints the current game units with inforamtion about their id, type_id, team_id, x, y and hp into stdout
 */

void ft_print_units();
/**
 * @brief Prints a unit config with inforamtion about their type_id, name, cost, hp, dmg_core, dmg_unit, max_range, min_range and speed into stdout
 *
 * @param unit_config Pointer to the unit config
 */
void ft_print_unit_config(const t_unit_config *unit_config);
/**
 * @brief Prints the current game config with inforamtion about their height, width, idle_income, core_hp, teams and units into stdout
 *
 */
void ft_print_game_config();
/**
 * @brief Prints every info about the gameconfig, status, teams, cores, resources and units into stdout.
 *
 */
void ft_print_all();

#endif
