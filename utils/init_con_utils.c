#include "utils.h"

static char *ft_create_str(char *team_name, char *id)
{
	char	*msg;
	int		len = 0;

	if (team_name != NULL)
		len += strlen(team_name);
	if (id != NULL)
		len += strlen(id);

	msg = malloc(sizeof(char) * (len + 26));
	if (team_name != NULL && id != NULL)
		sprintf(msg, "{\"id\": %s, \"name\": \"%s\"}\n", id, team_name);
	else if (id != NULL)
		sprintf(msg, "{\"id\": %s}\n", id);
	else if (team_name != NULL)
		sprintf(msg, "{\"id\": 1, \"name\": \"%s\"}\n", team_name);
	else
		sprintf(msg, "{\"id\": 1}\n");
	return (msg);
}

char	*ft_create_login_msg(char *team_name, int *argc, char **argv)
{
	char	*msg, *env_id;

	env_id = getenv("CORE_ID");
	if (*argc == 2)
		msg = ft_create_str(team_name, argv[1]);
	else if (env_id != NULL)
		msg = ft_create_str(team_name, env_id);
	else
		msg = ft_create_str(team_name, NULL);
	return (msg);
}
