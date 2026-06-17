#include <stdio.h>
#include <stdlib.h>

int m;

int p(char *format)
{
	return printf(format);
}

int n(void)
{
	char buffer[520];

	fgets(buffer, 512, stdin);
	p(buffer);
	if (m == 16930116)
		return system("/bin/cat /home/user/level5/.pass");
	return m;
}

int main(void)
{
	n();
	return 0;
}
