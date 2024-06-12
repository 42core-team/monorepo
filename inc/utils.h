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
size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize);
void	ft_free_all();
void	ft_free_game();
void	ft_perror_exit(char *msg);
void	ft_print_error(char *str, const char *func_name);

#endif
