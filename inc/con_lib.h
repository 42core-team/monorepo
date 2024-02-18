#ifndef CON_LIB_H
#define CON_LIB_H

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

typedef enum e_status
{
	STATUS_OK = 0,
	STATUS_PAUSED = 1,
	STATUS_END = 2,
	STATUS_WAIT_FOR_CLIENTS = 3
} t_status;
typedef struct s_team
{
	unsigned long id;
	unsigned long balance;
} t_team;
typedef struct s_core
{
	unsigned long id;
	unsigned long team_id;
	unsigned long x;
	unsigned long y;
	unsigned long hp;
} t_core;
typedef struct s_resource
{
	unsigned long id;
	unsigned long value;
	unsigned long x;
	unsigned long y;
	unsigned long hp;
} t_resource;
typedef struct s_unit
{
	unsigned long id;
	unsigned long type_id;
	unsigned long team_id;
	unsigned long x;
	unsigned long y;
	unsigned long hp;
} t_unit;

typedef struct s_unit_config
{
	char *name;
	unsigned long type_id;
	unsigned long cost;
	unsigned long hp;
	unsigned long dmg_core;
	unsigned long dmg_unit;
	unsigned long dmg_resource;
	unsigned long max_range;
	unsigned long min_range;
	unsigned long speed;
} t_unit_config;
typedef struct s_team_config
{
	unsigned long id;
	char *name;
} t_team_config;
typedef struct s_config
{
	unsigned long height;
	unsigned long width;
	unsigned long idle_income;
	unsigned long core_hp;
	t_team_config *team_configs;
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
	t_config config;
	t_status status;
	t_team *teams;
	t_core *cores;
	t_resource *resources;
	t_unit *units;
	t_actions actions;
} t_game;

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
