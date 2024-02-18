#include "parse_json.h"

char	*ft_strjoin(char const *s1, char const *s2)
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

char	*ft_strjoin_free_1(char *s1, char *s2)
{
	char	*result;

	result = ft_strjoin(s1, s2);
	free(s1);
	return (result);
}

char	*ft_strjoin_free_1_2(char *s1, char *s2)
{
	char	*result;

	result = ft_strjoin(s1, s2);
	free(s1);
	free(s2);
	return (result);
}

char*	ft_l_string(long num)
{
	char	*buffer = malloc(21);

	if (buffer)
		snprintf(buffer, 21, "%ld", num);
	return (buffer);
}

char*	ft_ul_string(unsigned long num)
{
	char	*buffer = malloc(21);

	if (buffer)
		snprintf(buffer, 21, "%lu", num);
	return (buffer);
}
