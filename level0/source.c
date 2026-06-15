#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char	*args[2];
	gid_t	egid;
	uid_t	euid;

	if (atoi(argv[1]) == 423)
	{
		args[0] = strdup("/bin/sh");
		args[1] = NULL;
		egid = getegid();
		euid = geteuid();
		setresgid(egid, egid, egid);
		setresuid(euid, euid, euid);
		execv("/bin/sh", args);
	}
	else
	{
		fwrite("No !\n", 1, 5, stderr);
	}
	return 0;
}
