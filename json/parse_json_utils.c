#include "parse_json.h"

int	ft_find_token(jsmntok_t *tokens, unsigned int token_len, unsigned int iter, char **search_str, char *json)
{
	while (iter < token_len)
	{
		if (tokens[iter].type == 4)
		{
			if (ft_strncmp(json + tokens[iter].start, *search_str, tokens[iter].end - tokens[iter].start) == 0)
			{
				search_str++;
				if (*search_str != 0)
					return ft_find_token(tokens, token_len, iter, search_str, json);
				return (iter + 1);
			}
		}
		iter++;
	}
	return (-1);
}

int	ft_find_token_one(const char *search, int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	char	**search_str;

	search_str = ft_create_array(1, search);
	token_ind = ft_find_token(tokens, token_len, token_ind, search_str, json);
	free(search_str);
	return (token_ind);
}

unsigned long ft_find_parse_ulong(const char *search, int *token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int temp_token_ind = *token_ind;
	temp_token_ind = ft_find_token_one(search, temp_token_ind, token_len, tokens, json);
	if (temp_token_ind == -1)
		return 0;
	if (temp_token_ind >= token_len)
	{
		fprintf(stderr, "Token index out of bounds for key: %s\n", search);
		return 0;
	}
	*token_ind = temp_token_ind;
	return strtoul(json + tokens[temp_token_ind].start, NULL, 10);
}

char *ft_find_parse_str(const char *search, int *token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int temp_token_ind = *token_ind;
	temp_token_ind = ft_find_token_one(search, temp_token_ind, token_len, tokens, json);
	if (temp_token_ind == -1)
		return NULL;
	if (temp_token_ind >= token_len)
	{
		fprintf(stderr, "Token index out of bounds for key: %s\n", search);
		return NULL;
	}
	*token_ind = temp_token_ind;
	return strndup(json + tokens[temp_token_ind].start, tokens[temp_token_ind].end - tokens[temp_token_ind].start);
}
