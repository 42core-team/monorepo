#ifndef CON_LIB_H
#define CON_LIB_H

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

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
typedef struct s_core
{
	/// @brief The id of the core.
	unsigned long id;
	/// @brief The id of the team that owns the core.
	unsigned long team_id;
	/// @brief The x coordinate of the core.
	unsigned long x;
	/// @brief The y coordinate of the core.
	unsigned long y;
	/// @brief The current healthpoints of the core.
	unsigned long hp;
} t_core;
typedef struct s_resource
{
	/// @brief The id of the resource.
	unsigned long id;
	unsigned long value;
	/// @brief The x coordinate of the resource.
	unsigned long x;
	/// @brief The y coordinate of the resource.
	unsigned long y;
	/// @brief The current healthpoints of the resource.
	unsigned long hp;
} t_resource;
typedef struct s_unit
{
	/// @brief The id of the unit.
	unsigned long id;
	/// @brief Which type of unit this is.
	unsigned long type_id;
	/// @brief The id of the team that owns the unit.
	unsigned long team_id;
	/// @brief The x coordinate of the unit.
	unsigned long x;
	/// @brief The y coordinate of the unit.
	unsigned long y;
	/// @brief The current healthpoints of the unit.
	unsigned long hp;
} t_unit;

typedef struct s_unit_config
{
	/// @brief The name of the unit.
	char *name;
	/// @brief The type id of the unit.
	unsigned long type_id;
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
	t_team_config *team_configs;
	/// @brief List of all unit types that are available in the game. The array is terminated by an element with type_id 0.
	t_unit_config *unit_configs;
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
	 * @brief The config contains base informations about the game that don't change like the map size and the unit types.
	 */
	t_config config;
	/**
	 * @brief The id of the team that you are playing for.
	 */
	unsigned long my_team_id;
	/**
	 * @brief The status of the game. Can be OK, PAUSED, END or WAIT_FOR_CLIENTS.
	 */
	t_status status;
	/**
	 * @brief List of all teams and their informations. The array is terminated by an element with id 0.
	 */
	t_team *teams;
	/**
	 * @brief List of all cores and their informations. The array is terminated by an element with id 0.
	 */
	t_core *cores;
	/**
	 * @brief List of all resources and their informations. The array is terminated by an element with id 0.
	 */
	t_resource *resources;
	/**
	 * @brief List of all units and their informations. The array is terminated by an element with id 0.
	 */
	t_unit *units;
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

void ft_init_con(int *argc, char **argv);
void ft_close_con();
void ft_enable_debug();
void ft_loop(void (*ft_user_loop)());

// actions.c
void ft_travel_to_id(unsigned long id, unsigned long x, unsigned long y);
void ft_travel_to(t_unit *unit, unsigned long x, unsigned long y);
void ft_travel_dir_id(unsigned long id, long x, long y);
void ft_travel_dir(t_unit *unit, long x, long y);
void ft_create_type_id(unsigned long type_id);
void ft_create(t_unit_config *unit_config);
void ft_attack_id(unsigned long attacker_id, unsigned long target_id);
void ft_attack(t_unit *attacker, t_unit *target);

// print_utils.c
void ft_print_status();
void ft_print_teams();
void ft_print_cores();
void ft_print_resources();
void ft_print_units();
void ft_print_game_config();
void ft_print_all();

#endif
