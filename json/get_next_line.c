#include "get_next_line.h"
#include "parse_json.h"

ssize_t	ft_endline(const char *s, ssize_t read_bytes)
{
	ssize_t	i;
	char	c;

	if (!s)
		return (-1);
	if (read_bytes == 0)
		return (strlen(s) - 1);
	i = 0;
	c = '\n';
	while (s[i] != 0 && s[i] != c)
		i++;
	if (s[i] == c)
		return (i);
	return (-1);
}

int	ft_append_buff(char **temp, char *buff)
{
	char	*temp2;

	if (!*temp)
	{
		*temp = strdup(buff);
		if (!*temp)
			return (0);
	}
	else
	{
		temp2 = ft_strjoin(*temp, buff);
		if (!temp2)
			return (0);
		free(*temp);
		*temp = temp2;
	}
	return (1);
}

ssize_t	ft_readnextline(ssize_t *i, int fd, char *buff, char **temp)
{
	ssize_t	bytes_read;

	bytes_read = 1;
	if (*i == -1)
	{
		while (*i == -1 && bytes_read > 0)
		{
			bytes_read = read(fd, buff, BUFFER_SIZE);
			if (bytes_read != -1)
				buff[bytes_read] = 0;
			if (bytes_read > 0)
			{
				if (!ft_append_buff(temp, buff))
					return (-1);
			}
			*i = ft_endline(*temp, bytes_read);
		}
		return (bytes_read);
	}
	return (-1);
}

char	*ft_returnfree(char **temp)
{
	free(*temp);
	*temp = 0;
	return (0);
}

char	*get_next_line(int fd)
{
	static char	*temp = 0;
	char		buff[BUFFER_SIZE + 1];
	ssize_t		bytes_read;
	ssize_t		i;
	char		*retstr;

	if (BUFFER_SIZE <= 0 || fd < 0)
		return (0);
	i = ft_endline(temp, -1);
	bytes_read = ft_readnextline(&i, fd, buff, &temp);
	if (bytes_read <= 0 && i == -1)
		return (ft_returnfree(&temp));
	retstr = malloc(sizeof(char) * (i + 2));
	if (!retstr || !temp)
		return (ft_returnfree(&temp));
	strlcpy(retstr, temp, i + 2);
	strlcpy(temp, temp + i + 1, strlen(temp + i + 1) + 1);
	return (retstr);
}
