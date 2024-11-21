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
typedef struct s_team
{
	/// @brief The id of the team.
	unsigned long id;
	/// @brief Current balance of the team.
	unsigned long balance;
} t_team;

typedef enum e_obj_type
{
	OBJ_UNIT,
	OBJ_CORE,
	OBJ_RESOURCE
} t_obj_type;

typedef struct s_obj
{
	/// @brief Type of the obj
	t_obj_type	type;

	/// @brief The id of the obj
	unsigned long id;
	/// @brief The x coordinate of the obj
	unsigned long x;
	/// @brief The y coordinate of the obj
	unsigned long y;
	/// @brief The current healthpoints of the obj
	unsigned long hp;
	union {
		struct
		{
			/// @brief The id of the team that owns the core.
			unsigned long team_id;
		}	s_core;
		struct
		{
			/// @brief Which type of unit this is.
			unsigned long type_id;
			/// @brief The id of the team that owns the unit.
			unsigned long team_id;
		}	s_unit;
	};
} t_obj;

typedef enum e_unit_type
{
	UNIT_WARRIOR = 1,
	UNIT_WORKER = 2,
	UNIT_TANK = 3,
	UNIT_ARCHER = 4
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
	unsigned long dmg_core;
	/// @brief How much damage the unit deals to units.
	unsigned long dmg_unit;
	/// @brief How much damage the unit deals to resources.
	unsigned long dmg_resource;
	/// @brief How far away the unit can maximally attack.
	unsigned long max_range;
	/// @brief How far away the unit can minimally attack.
	unsigned long min_range;
	/// @brief How far the unit can travel in one second.
	unsigned long speed;
} t_unit_config;
typedef struct s_team_config
{
	/// @brief The id of the team.
	unsigned long id;
	/// @brief The name of the team.
	char *name;
} t_team_config;
typedef struct s_resource_config
{
	/// @brief What type of resource this is.
	unsigned long type_id;
	/// @brief Initial healthpoints of the resource.
	unsigned long hp;
	/// @brief How much balance you would get if you destroy the whole resource.
	unsigned long balance_value;
} t_resource_config;
typedef struct s_config
{
	/// @brief The height of the map.
	unsigned long height;
	/// @brief The width of the map.
	unsigned long width;
	/// @brief How much idle income you get every second.
	unsigned long idle_income;
	/// @brief How much healthpoints a core has at the start of the game.
	unsigned long core_hp;
	/// @brief List of all teams with their id and name. The array is terminated by an element with id 0.
	t_team_config *teams;
	/// @brief List of all unit types that are available in the game. The array is terminated by an element with type_id 0.
	t_unit_config *units;
	/// @brief List of all resource types that are available in the game. The array is terminated by an element with type_id 0.
	t_resource_config *resources;
} t_config;

typedef struct s_action_create
{
	unsigned long type_id;
} t_action_create;
typedef struct s_action_travel
{
	unsigned long id;
	bool is_vector;
	unsigned long x;
	unsigned long y;
} t_action_travel;
typedef struct s_action_attack
{
	unsigned long attacker_id;
	unsigned long target_id;
} t_action_attack;
typedef struct s_actions
{
	t_action_create *creates;
	unsigned int creates_count;
	t_action_travel *travels;
	unsigned int travels_count;
	t_action_attack *attacks;
	unsigned int attacks_count;
} t_actions;

typedef struct s_game
{
	/**
	 * @brief The status of the game. Can be OK, PAUSED, END or WAIT_FOR_CLIENTS.
	 */
	t_status status;
	/**
	 * @brief The config contains base informations about the game that don't change like the map size and the unit types.
	 */
	t_config config;
	/**
	 * @brief The id of the team that you are playing for.
	 */
	unsigned long my_team_id;
	/**
	 * @brief List of all teams and their informations. The array is terminated by an element with id 0.
	 */
	t_team *teams;
	/**
	 * @brief List of all cores and their informations. The array is terminated by an element with id 0.
	 */
	t_obj *cores;
	/**
	 * @brief List of all resources and their informations. The array is terminated by an element with id 0.
	 */
	t_obj *resources;
	/**
	 * @brief List of all units and their informations. The array is terminated by an element with id 0.
	 */
	t_obj *units;
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
void ft_loop(void (*ft_init_func)(void *ptr), void (*ft_user_loop)(void *ptr), void *ptr);

// ------------------ getter -----------------
/*
 * @brief Get any object based on its id
 */
t_obj	*ft_get_obj_from_id(unsigned long id);
// --------------- team getter ---------------
/**
 * @brief Get the referance to my team
 */
t_team	*ft_get_my_team(void);
/**
 * @brief Get the referance to the first opponent team
 */
t_team	*ft_get_first_opponent_team(void);
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
 * @brief Lets a unit travel to a specific coordinate. Same as ft_travel_to, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param id Which unit should travel.
 * @param x To which x coordinate the unit should travel.
 * @param y To which y coordinate the unit should travel.
 */
void ft_travel_to_id(unsigned long id, unsigned long x, unsigned long y);
/**
 * @brief Lets a unit travel to a specific coordinate. Same as ft_travel_to_id, besides that this function takes a pointer to a unit instead of an id.
 *
 * @param unit Pointer to the unit that should travel.
 * @param x To which x coordinate the unit should travel.
 * @param y To which y coordinate the unit should travel.
 */
void ft_travel_to(t_obj *unit, unsigned long x, unsigned long y);
/**
 * @brief Lets a unit start to travel into a specific direction. Same as ft_travel_dir, besides that this function takes an id instead of a pointer to a unit. When x and y are both 0, the unit will stop traveling.
 *
 * @param id Which unit should travel.
 * @param x x vector of the direction the unit should travel.
 * @param y y vector of the direction the unit should travel.
 */
void ft_travel_dir_id(unsigned long id, long x, long y);
/**
 * @brief Lets a unit start to travel into a specific direction. Same as ft_travel_dir_id, besides that this function takes a pointer to a unit instead of an id. When x and y are both 0, the unit will stop traveling.
 *
 * @param id Which unit should travel.
 * @param x x vector of the direction the unit should travel.
 * @param y y vector of the direction the unit should travel.
 */
void ft_travel_dir(t_obj *unit, long x, long y);
/**
 * @brief Lets a unit travel to another obj. Same as ft_travel_to_id, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param id Which unit should travel.
 * @param target To which obj the unit should travel.
 */
void ft_travel_to_id_obj(unsigned long id, t_obj *target);
/**
 * @brief Lets a unit travel to another obj. Same as ft_travel_to_id_obj, besides that this function takes a pointer to a unit instead of an id.
 *
 * @param unit Pointer to the unit that should travel.
 * @param target Pointer to the obj that the unit should travel to.
 */
void ft_travel_to_obj(t_obj *unit, t_obj *target);
/**
 * @brief Creates a unit of a specific type. Same as ft_create, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param type_id Which type of unit should be created.
 */
void ft_create_type_id(t_unit_type type_id);
/**
 * @brief Creates a unit of a specific type. Same as ft_create_type_id, besides that this function takes a pointer to a unit instead of an id.
 *
 * @param unit_config Pointer to the unit config that should be created.
 */
void ft_create(t_unit_config *unit_config);
/**
 * @brief Lets a unit attack another unit. Same as ft_attack, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param attacker_id Which unit should be used to attack.
 * @param target_id Which unit should be attacked.
 */
void ft_attack_id(unsigned long attacker_id, unsigned long target_id);
/**
 * @brief Lets a unit attack another unit. Same as ft_attack_id, besides that this function takes a pointer to a unit instead of an id.
 *
 * @param attacker_unit Pointer to the unit that should be used to attack.
 * @param target_obj Pointer to the obj that should be attacked.
 */
void ft_attack(t_obj *attacker, t_obj *target);
/**
 * @brief Travel and attack a target. The unit will travel to the target and attack it. Same as calling ft_travel_to_obj and ft_attack.
 *
 * @param attacker_unit Pointer to the unit that should be used to attack.
 * @param attack_obj Pointer to the unit that should be attacked.
 */
void ft_travel_attack(t_obj *attacker_unit, t_obj *attack_obj);

// -------------- print_utils.c --------------
/**
 * @brief Prints the current game status into stdout
 */
void ft_print_status();
/**
 * @brief Prints the current game teams with inforamtion about their id and balance into stdout
 *
 */
void ft_print_teams();
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
 * @brief Prints a team config with inforamtion about their id and name into stdout
 *
 * @param team_config Pointer to the team config
 */
void ft_print_team_config(const t_team_config *team_config);
/**
 * @brief Prints a unit config with inforamtion about their type_id, name, cost, hp, dmg_core, dmg_unit, max_range, min_range and speed into stdout
 *
 * @param unit_config Pointer to the unit config
 */
void ft_print_unit_config(const t_unit_config *unit_config);
/**
 * @brief Prints a resource config with inforamtion about their type_id and hp into stdout
 *
 * @param unit_config Pointer to the resource config
 */
void ft_print_resource_config(const t_resource_config *resource_config);
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
