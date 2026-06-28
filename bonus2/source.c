#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	language;

void	greetuser(char *src)
{
	char	greeting[64];

	switch (language)
	{
		case 1:
			strcpy(greeting, "Hyvää päivää ");
			break;
		case 2:
			strcpy(greeting, "Goedemiddag! ");
			break;
		case 0:
			strcpy(greeting, "Hello ");
			break;
	}
	strcat(greeting, src);
	puts(greeting);
}

int	main(int argc, char **argv)
{
	char	dest[76];
	char	src[76];
	char	*lang;

	if (argc != 3)
		return 1;
	memset(dest, 0, sizeof(dest));
	strncpy(dest, argv[1], 40);
	strncpy(dest + 40, argv[2], 32);
	lang = getenv("LANG");
	if (lang)
	{
		if (!memcmp(lang, "fi", 2))
			language = 1;
		else if (!memcmp(lang, "nl", 2))
			language = 2;
	}
	memcpy(src, dest, sizeof(src));
	greetuser(src);
	return 0;
}
