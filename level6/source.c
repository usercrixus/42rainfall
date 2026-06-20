#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int n(void)
{
	return system("/bin/cat /home/user/level7/.pass");
}

int m(void)
{
	return puts("Nope");
}

int main(int argc, char **argv)
{
	char *buf;
	int (**fp)(void);

	buf = malloc(0x40);
	fp = malloc(4);
	*fp = m;
	strcpy(buf, argv[1]);
	return (*fp)();
}
