

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h> 
#include <time.h>

#define TIME_MAX 12
char *myTime(char *tmBuf)
{
	if (!tmBuf)
	{
		perror("error mT1: myTime(null)");
		return NULL;
	}
	
	struct timespec mtime;
	if (clock_gettime(CLOCK_REALTIME, &mtime) < 0)
	{
		perror("error mT3 : clock_gettime() failed");
		return NULL;
	}
	
	struct tm *hms = localtime(&(mtime.tv_sec));
	if (!hms)
	{
		perror("error mT4: localtime() failed");
		return NULL;
	}
		
	if (sprintf(tmBuf, "%d:%d:%d:%d", hms->tm_hour, hms->tm_min, hms->tm_sec, (int)mtime.tv_nsec/1000000) < 0)
	{
		perror("error mT2 : sprintf() failed");
		return NULL;
	}
	return tmBuf;
}

int createChildProcesses(int nChild, char *tmBuf)
{
	if (nChild < 1)
		return 0;
	pid_t p = fork();
	
	switch(p)
	{
	case -1:
		perror("fork() failed");
		return -1;
	case 0:
		printf("CHILD : my pid is %d\t my parent's pid is %d\t%s\n",getpid(), getppid(), myTime(tmBuf) != NULL ? tmBuf : "time error"); 				
		_exit(0);
	default:

		createChildProcesses(nChild - 1, tmBuf);
		return 0;
	}
	
}

int main()
{
	int nProcs = 2;
 	char *tmBuf = (char *)calloc(TIME_MAX, 1);
 	if (!tmBuf)
 	{
 		perror("error m4: calloc() failed");
 		return 4;
 	}
 	
	if (createChildProcesses(nProcs, tmBuf) == -1)
	{
		perror("error m3 : cannot create child process");
		return 3;
	}

	printf("PARENT : my pid is %d\t my parent's pid is %d\t%s\n",getpid(), getppid(), myTime(tmBuf) != NULL ? tmBuf : "time error");
	
	system("ps -x");

		
	int ws;
	pid_t pid;
	for (int i = 0; i < nProcs; ++i)
	{
		pid = waitpid(-1,&ws, 0);
		if ( pid  == -1 )
			perror("error m1; waitpid() failed");		
		else if (WIFEXITED(ws))
			if (WEXITSTATUS(ws))
			{
				fprintf(stderr, "error m2: child process(%d) terminated  unsuccessfully", pid);
				perror(" ");
			}
	}

	free(tmBuf);	
	return 0;
}
