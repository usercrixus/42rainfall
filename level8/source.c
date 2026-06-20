#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char	*auth;
char	*service;

int main(int argc, char **argv)
{
	char	buf[128];

	(void)argc;
	(void)argv;
	while (1)
	{
		printf("%p, %p \n", auth, service);
		if (!fgets(buf, 128, stdin))
			break;
		if (!memcmp(buf, "auth ", 5))
		{
			auth = malloc(4);
			*(int *)auth = 0;
			if (strlen(buf + 5) <= 30)
				strcpy(auth, buf + 5);
		}
		if (!memcmp(buf, "reset", 5))
			free(auth);
		if (!memcmp(buf, "service", 6))
			service = strdup(buf + 7);
		if (!memcmp(buf, "login", 5))
		{
			if (*((int *)auth + 8))
				system("/bin/sh");
			else
				fwrite("Password:\n", 1, 10, stdout);
		}
	}
	return 0;
}
