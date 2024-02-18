#include "parse_json.h"

char	*ft_create_json()
{
	char	*json = calloc(1, sizeof(char));
	t_action_create	*creates = game.actions.creates;

	unsigned int ind = 0;
	while (game.actions.creates_count > ind && creates)
	{
		json = ft_strjoin_free_1(json, "{\"Create\":{");
		json = ft_strjoin_free_1(json, "\"type_id\":");
		json = ft_strjoin_free_1_2(json, ft_ul_string(creates[ind].type_id));
		json = ft_strjoin_free_1(json, "}},");
		ind++;
	}
	return (json);
}

char	*ft_travel_json()
{
	char	*json = calloc(1, sizeof(char));
	t_action_travel	*travels = game.actions.travels;

	unsigned int ind = 0;
	while (game.actions.travels_count > ind && travels)
	{
		json = ft_strjoin_free_1(json, "{\"Travel\":{");
		json = ft_strjoin_free_1(json, "\"id\":");
		json = ft_strjoin_free_1_2(json, ft_l_string(travels[ind].id));

		json = ft_strjoin_free_1(json, ",\"travel_type\":");
		if (travels[ind].is_vector)
		{
			json = ft_strjoin_free_1(json, "{\"Vector\":{");
			json = ft_strjoin_free_1(json, "\"x\":");
			json = ft_strjoin_free_1_2(json, ft_l_string(travels[ind].x));
			json = ft_strjoin_free_1(json, ",\"y\":");
			json = ft_strjoin_free_1_2(json, ft_l_string(travels[ind].y));
		}
		else
		{
			json = ft_strjoin_free_1(json, "{\"Position\":{");
			json = ft_strjoin_free_1(json, "\"x\":");
			json = ft_strjoin_free_1_2(json, ft_l_string(travels[ind].x));
			json = ft_strjoin_free_1(json, ",\"y\":");
			json = ft_strjoin_free_1_2(json, ft_l_string(travels[ind].y));
		}
		json = ft_strjoin_free_1(json, "}}}},");
		ind++;
	}
	return (json);
}

char	*ft_attack_json()
{
	char	*json = calloc(1, sizeof(char));
	t_action_attack	*attacks = game.actions.attacks;

	unsigned int ind = 0;
	while (game.actions.attacks_count > ind && attacks)
	{
		json = ft_strjoin_free_1(json, "{\"Attack\":{");
		json = ft_strjoin_free_1(json, "\"attacker_id\":");
		json = ft_strjoin_free_1_2(json, ft_ul_string(attacks[ind].attacker_id));
		json = ft_strjoin_free_1(json, ",\"target_id\":");
		json = ft_strjoin_free_1_2(json, ft_ul_string(attacks[ind].target_id));
		json = ft_strjoin_free_1(json, "}},");
		ind++;
	}
	return (json);
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

	if (game.actions.attacks != NULL)
		free(game.actions.attacks);
	game.actions.attacks = NULL;
	game.actions.attacks_count = 0;
}

char	*ft_all_action_json()
{
	char	*json = calloc(1, sizeof(char));

	json = ft_strjoin_free_1(json, "{\"actions\":[");

	json = ft_strjoin_free_1_2(json, ft_create_json());
	json = ft_strjoin_free_1_2(json, ft_travel_json());
	json = ft_strjoin_free_1_2(json, ft_attack_json());

	if (json[strlen(json) - 1] == ',')
		json[strlen(json) - 1] = '\0';

	json = ft_strjoin_free_1(json, "]}\n");

	return (json);
}
