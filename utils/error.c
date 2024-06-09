#include "utils.h"

void ft_print_error(char *str, char *func_name)
{
	fprintf(stderr, "%s:%s\n", func_name, str);
}
