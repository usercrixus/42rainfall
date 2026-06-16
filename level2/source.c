#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *p(void)
{
	char buffer[64];
	unsigned int retaddr;

	fflush(stdout);
	gets(buffer);
	if ((retaddr & 0xB0000000) == 0xB0000000)
	{
		printf("(%p)\n", (void *)retaddr);
		_exit(1);
	}
	puts(buffer);
	return strdup(buffer);
}

int main(void)
{
	p();
	return 0;
}

