#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h> 

void createChildren0_0(int nChild)
{
	if (nChild < 1)
		return;
	pid_t p = fork();
	
	switch(p)
	{
	case -1:
		perror("fork() failed");
		return;
	case 0:
		printf("CHILD: my pid is %d\n", getpid());
		printf("CHILD: my parent's pid is %d\n", getppid());
//		printf("CHILD: ");
		system("date -u");
		return;
	default:
		createChildren0_0(nChild - 1);
		return;
	}
	
}

int main()
{
	int nProcs = 2;
	createChildren0_0(nProcs);
	
	int *ws;
	wait(ws);

	return 0;
}
