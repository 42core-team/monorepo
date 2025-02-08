#include "utils.h"
#include "parse_json.h"

char	**ft_create_array(const int count, ...)
{
	va_list	args;
	char	**array;
	int		i;

	va_start(args, count);
	array = malloc(sizeof(char *) * (count + 1));
	if (!array)
		return (0);
	i = 0;
	while (i < count)
	{
		array[i] = va_arg(args, char *);
		i++;
	}
	array[i] = 0;
	va_end(args);
	return (array);
}

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	size_t	len_s;
	char	*ptr;

	if (!s)
		return (0);
	if (start >= strlen(s))
		len_s = 0;
	else
		len_s = strlen(&s[start]);
	if (len > len_s)
		len = len_s;
	ptr = malloc((len + 1) * sizeof(char));
	if (!ptr)
		return (0);
	if (len > 0)
		ft_strlcpy(ptr, &s[start], len + 1);
	else
		ptr[0] = 0;
	return (ptr);
}

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t			i;
	unsigned char	*sc1;
	unsigned char	*sc2;

	i = 0;
	sc1 = (unsigned char *) s1;
	sc2 = (unsigned char *) s2;
	if (n == 0)
		return (0);
	while (s1[i] && s2[i] && s1[i] == s2[i] && i < n - 1)
		i++;
	return (sc1[i] - sc2[i]);
}

size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t	len_src;
	size_t	i;

	len_src = strlen(src);
	if (dstsize == 0)
		return (len_src);
	i = 0;
	while (i < dstsize - 1 && i < len_src)
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = 0;
	return (len_src);
}

void	ft_free_game()
{
	if (game.cores != NULL)
	{
		for (int i = 0; game.cores[i] != NULL; i++)
			free(game.cores[i]);
		free(game.cores);
		game.cores = NULL;
	}
	if (game.resources != NULL)
	{
		for (int i = 0; game.resources[i] != NULL; i++)
			free(game.resources[i]);
		free(game.resources);
		game.resources = NULL;
	}
	if (game.units != NULL)
	{
		for (int i = 0; game.units[i] != NULL; i++)
			free(game.units[i]);
		free(game.units);
		game.units = NULL;
	}
	if (game.walls != NULL)
	{
		for (int i = 0; game.walls[i] != NULL; i++)
			free(game.walls[i]);
		free(game.walls);
		game.walls = NULL;
	}
}

void	ft_free_config()
{
	int	ind;

	ind = 0;
	if (game.config.units != NULL)
	{
		while (game.config.units[ind].type_id != 0)
		{
			free(game.config.units[ind].name);
			ind++;
		}
		free(game.config.units);
	}
}

void	ft_free_all()
{
	ft_free_game();
	ft_free_config();
	ft_reset_actions();
}

void	ft_perror_exit(char *msg)
{
	perror(msg);
	ft_free_all();
	exit(EXIT_FAILURE);
}
