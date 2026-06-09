#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char c[68];

typedef struct s_block
{
	int id;
	char *buffer;
}	t_block;

void m(void)
{
	printf("%s - %d\n", c, (int)time(NULL));
}

int main(int argc, char **argv)
{
	t_block	*first;
	t_block	*second;
	FILE	*password;

	(void)argc;
	first = malloc(sizeof(*first));
	first->id = 1;
	first->buffer = malloc(8);
	second = malloc(sizeof(*second));
	second->id = 2;
	second->buffer = malloc(8);
	strcpy(first->buffer, argv[1]);
	strcpy(second->buffer, argv[2]);
	password = fopen("/home/user/level8/.pass", "r");
	fgets(c, 68, password);
	puts("~~");
	return 0;
}
