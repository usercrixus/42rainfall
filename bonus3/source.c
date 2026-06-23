#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    FILE *file;
    char pass[67];
    char message[66];
    int index;

    if (argc != 2)
        return -1;

    file = fopen("/home/user/end/.pass", "r");
    if (file == NULL)
        return -1;

    memset(pass, 0, sizeof(pass));
    memset(message, 0, sizeof(message));

    fread(pass, 1, 66, file);
    index = atoi(argv[1]);
    pass[index] = '\0';
    fread(message, 1, 65, file);
    fclose(file);

    if (strcmp(pass, argv[1]) == 0)
        execl("/bin/sh", "sh", NULL);
    else
        puts(message);

    return 0;
}
