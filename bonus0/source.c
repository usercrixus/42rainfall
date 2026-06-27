#include <stdio.h>
#include <string.h>
#include <unistd.h>

char	*p(char *dest, char *s)
{
	char	buf[4096];

	puts(s);
	read(0, buf, 4096);
	*strchr(buf, '\n') = 0;
	return strncpy(dest, buf, 20);
}

char	*pp(char *dest)
{
	char	first[20];
	char	last[20];

	p(first, " - ");
	p(last, " - ");
	strcpy(dest, first);
	size_t	len = strlen(dest);
	dest[len] = ' ';
	dest[len + 1] = '\0';
	return strcat(dest, last);
}

int	main(int argc, char **argv)
{
	char	s[42];

	(void)argc;
	(void)argv;
	pp(s);
	puts(s);
	return 0;
}
