#include "parse_json.h"

static char *ft_direction_to_str(t_direction dir)
{
	switch (dir)
	{
		case DIR_UP:	return "u";
		case DIR_RIGHT:	return "r";
		case DIR_DOWN:	return "d";
		case DIR_LEFT:	return "l";
		default:		return "u";
	}
}

char *ft_create_json(void)
{
	char *json = calloc(1, sizeof(char));
	if (!json)
		ft_perror_exit("Failed to allocate memory for JSON string\n");

	t_action_create *creates = game.actions.creates;
	unsigned int ind = 0;
	while (ind < game.actions.creates_count && creates)
	{
		json = ft_strjoin_free_1(json, "{\"type\":\"create\",\"type_id\":");
		char *type_id_str = ft_ul_string(creates[ind].type_id);
		json = ft_strjoin_free_1_2(json, type_id_str);
		free(type_id_str);
		json = ft_strjoin_free_1(json, "},");
		ind++;
	}
	return json;
}

char *ft_travel_json(void)
{
	char *json = calloc(1, sizeof(char));
	if (!json)
		ft_perror_exit("Failed to allocate memory for JSON string\n");

	t_action_travel *travels = game.actions.travels;
	unsigned int ind = 0;
	while (ind < game.actions.travels_count && travels)
	{
		json = ft_strjoin_free_1(json, "{\"type\":\"move\",\"unit_id\":");
		char *id_str = ft_ul_string(travels[ind].id);
		json = ft_strjoin_free_1_2(json, id_str);
		free(id_str);
		json = ft_strjoin_free_1(json, ",\"dir\":\"");
		json = ft_strjoin_free_1(json, ft_direction_to_str(travels[ind].direction));
		json = ft_strjoin_free_1(json, "\"},");
		ind++;
	}
	return json;
}

void	ft_reset_actions()
{
	if (game.actions.creates != NULL)
		free(game.actions.creates);
	game.actions.creates = NULL;
	game.actions.creates_count = 0;

	if (game.actions.travels != NULL)
		free(game.actions.travels);
	game.actions.travels = NULL;
	game.actions.travels_count = 0;
}

char *ft_all_action_json(void)
{
	char *json = calloc(1, sizeof(char));
	if (!json)
		ft_perror_exit("Failed to allocate memory for JSON string\n");

	json = ft_strjoin_free_1(json, "{\"actions\":[");
	
	char *create_part = ft_create_json();
	char *travel_part = ft_travel_json();
	json = ft_strjoin_free_1_2(json, create_part);
	json = ft_strjoin_free_1_2(json, travel_part);
	free(create_part);
	free(travel_part);

	size_t len = strlen(json);
	if (len > 0 && json[len - 1] == ',')
		json[len - 1] = '\0';

	json = ft_strjoin_free_1(json, "]}");
	json = ft_strjoin_free_1(json, "\n");
	return json;
}
