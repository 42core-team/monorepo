#ifndef PARSE_JSON_H
# define PARSE_JSON_H

# include "con_lib.h"
# include "utils.h"
# define JSMN_HEADER
# include "jsmn.h"

typedef struct s_json
{
	int			token_len;
	jsmntok_t	*tokens;
} t_json;

//parse_json_objects.c
void				ft_parse_cores(int token_ind, int token_len, jsmntok_t *tokens, char *json);
void				ft_parse_teams(int token_ind, int token_len, jsmntok_t *tokens, char *json);
void				ft_parse_resources(int token_ind, int token_len, jsmntok_t *tokens, char *json);
void				ft_parse_units(int token_ind, int token_len, jsmntok_t *tokens, char *json);
void				ft_parse_walls(int token_ind, int token_len, jsmntok_t *tokens, char *json);

//parse_json_utils.c
int				ft_find_token(jsmntok_t *tokens, unsigned int token_len, unsigned int iter, char **search_str, char *json);
int				ft_find_token_one(const char *search, int token_ind, int token_len, jsmntok_t *tokens, char *json);
long			ft_find_parse_long(const char *search, int *token_ind, int token_len, jsmntok_t *tokens, char *json);
unsigned long	ft_find_parse_ulong(const char *search, int *token_ind, int token_len, jsmntok_t *tokens, char *json);
char			*ft_find_parse_str(const char *search, int *token_ind, int token_len, jsmntok_t *tokens, char *json);

//encode_json.c
char	*ft_strjoin(char const *s1, char const *s2);
char	*ft_strjoin_free_1(char *s1, char *s2);
char	*ft_strjoin_free_1_2(char *s1, char *s2);
char	*ft_l_string(long num);
char	*ft_ul_string(unsigned long num);
char	*ft_all_action_json();
void	ft_reset_actions();

//formatter.c
char	*json_formatter(const char* json_input);

#endif
