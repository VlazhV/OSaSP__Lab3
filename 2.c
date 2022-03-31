//TODO hh : MM : ss : ms

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h> 

void createChildProcesses(int nChild)
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
		printf("CHILD : my pid is %d\t my parent's pid is %d\n",getpid(), getppid()); 		
		printf("CHILD %d:\n", getpid());		
		system("date");

		_exit(0);
	default:
//		sleep(1);
		createChildProcesses(nChild - 1);
		return;
	}
	
}

int main()
{
	int nProcs = 2;
	createChildProcesses(nProcs);
	

	printf("PARENT : my pid is %d\t my parent's pid is %d\n",getpid(), getppid()); 		
	printf("PARENT %d:\n", getpid());		
	system("date");

	system("ps -x");	
		
	int ws;
	pid_t pid;
	if ( (pid = waitpid(-1, &ws, 0)) == -1)
		perror("error m1; waitpid() failed");		
	else if (WIFEXITED(ws))
		if (WEXITSTATUS(ws))
		{
			fprintf(stderr, "error m2: child process(%d) terminated  unsuccessfully", pid);
			perror(" ");
		}

	
	return 0;
}
