#ifndef UTILS_H
# define UTILS_H

# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <stdlib.h>

# include "con_lib.h"

//utils.c
void	ft_parse_json_config(char *json);
void	ft_parse_json_state(char *json);
char	**ft_create_array(const int count, ...);
char	*ft_substr(char const *s, unsigned int start, size_t len);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
void	ft_free_all();
void	ft_free_game();

#endif
