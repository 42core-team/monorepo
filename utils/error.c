#include "utils.h"

void ft_print_error(char *str, const char *func_name)
{
	fprintf(stderr, "%s:%s\n", func_name, str);
}
