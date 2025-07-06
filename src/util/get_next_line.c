#include "socket.h"
#define BUFFER_SIZE 1024

static size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize)
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

static char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t	len_s1;
	size_t	len_s2;
	char	*dst;

	if (!s1 || !s2)
		return (0);
	len_s1 = strlen(s1);
	len_s2 = strlen(s2);
	dst = malloc((len_s1 + len_s2 + 1) * sizeof(char));
	if (!dst)
		return (0);
	ft_strlcpy(dst, s1, len_s1 + 1);
	strncat(dst, s2, len_s1 + len_s2 + 1);
	return (dst);
}

static ssize_t	ft_endline(const char *s, ssize_t read_bytes)
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

static int	ft_append_buff(char **temp, char *buff)
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

static ssize_t	ft_readnextline(ssize_t *i, int fd, char *buff, char **temp)
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

static char	*ft_returnfree(char **temp)
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
	ft_strlcpy(retstr, temp, i + 2);
	ft_strlcpy(temp, temp + i + 1, strlen(temp + i + 1) + 1);
	return (retstr);
}
