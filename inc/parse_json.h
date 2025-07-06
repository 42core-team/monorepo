#ifndef PARSE_JSON_H
# define PARSE_JSON_H

#include <string.h>

#include "core_lib.h"
# include "json.h"

//parse_json_state.c
void	ft_parse_json_state(char *json);

//parse_json_config.c
void	ft_parse_json_config(char *json);


//encode_json_actions.c
char	*ft_all_action_json();
void	ft_reset_actions();

//encode_json_login.c
char	*ft_create_login_msg(char *team_name, int argc, char **argv);

#endif
