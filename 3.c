


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

#define BLK_SIZE 512

int dirWalk(char *path, int nProcesses, char *seq);
char *getAbsPath (char *relPath);
int findSeq(char *seq, char *fileName, int *byteLook);
int createChildProcesses(int *processNumber, char *fileName, char *seq);


int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "command seq dir N\n 1)seq - sequence of bytes to search\n 2)dir - to start search\n 3)N - max count of processes-children\n");
		return -1;
	}

	
	char *absPath;
	absPath = getAbsPath(argv[2]);
	if (!absPath)
	{
		perror("error m1 : getAbsPath() failed");
		return 1;
	}
	
		
		char *endptr;
		int N;		
		N = strtol(argv[3], &endptr, 10);
		if (errno == ERANGE)
		{
			perror("error m5: cannot convert");
			return 5;
		}
		
		
		if (endptr[0] != '\0' && argv[3][0] != '\0')
		{		
			fprintf(stderr, "error m6: cannot convert(N must be full correct)\n");
			return 6;
			
		}
		
		endptr = NULL;
		
		if (N <= 0)
		{
			fprintf(stderr, "error m4: max count of processes-children must be a positive value\n");
			return 4;
		}
		

	int nFilesFound = dirWalk(absPath, N, argv[1]);
	if (nFilesFound < 0)
	{
		perror("error m2 : dirWalk() failed");
		return 2;
	}
	
	int wstatus;
	for (int i = 0; i < N; ++i)
		if (waitpid(-1, &wstatus, 0) == -1)
			perror("error m3: wait() failed at the end");
		else if (WIFEXITED(wstatus))
			if (WEXITSTATUS(wstatus) == 1)
				++nFilesFound;
			else if (WEXITSTATUS(wstatus))			
				perror("error m7: child process terminated unsuccessfully");
		
	
	printf("\n======================================\nCount Files Found = %d\n", nFilesFound);
	return 0;
}



char *getAbsPath (char *relPath)
{
	if (relPath[0] == '/')
		return relPath;
	else
	{
		char curDir[PATH_MAX];
		if (!getcwd(curDir, PATH_MAX))
		{
			perror("error gap1 : getcwd() failed");
			return NULL;	
		}

		return strcat(strcat(curDir, "/"), relPath);
	}
	
}


int dirWalk(char *path, int maxnProcesses, char *seq)
{
	static int nProcesses = 0;
	int nFilesFound = 0;
	int add_nFilesFound = 0;
	DIR *curDir = opendir(path);

	if (!curDir)
	{
		fprintf(stderr, "error dW1: opendir() failed at  '%s'", path);
		perror("");
		return -1;
	}
	
	char *newPath = (char*)calloc(PATH_MAX, 1);
	if (!newPath)
	{
		fprintf(stderr, "error dW5: no memory at '%s'", path);
		perror("");
		closedir(curDir);
		return -5;
	}
	struct dirent *dire;
	while (dire = readdir(curDir))	
	{
		
		if ( strcmp(dire->d_name, ".") == 0 || strcmp(dire->d_name, "..") == 0 ) 
			continue;
		
		strcpy(newPath, path);
		strcat(strcat(newPath, "/"), dire->d_name);
		
		if (dire->d_type == DT_DIR) 
		{			
			add_nFilesFound = dirWalk(newPath, maxnProcesses, seq);
			if (add_nFilesFound < 0)
			{
				fprintf(stderr, "error dW3: dirWalk() failed at '%s'", newPath);
				perror("");
				continue;
			}
			nFilesFound += add_nFilesFound;
		}
		else if (dire->d_type == DT_REG && dire->d_type != DT_LNK)
		{		
			//create process	
			if (nProcesses >= maxnProcesses)
			{
				int wstatus;		
				if (waitpid(-1, &wstatus, 0) == -1)
				{
					fprintf(stderr, "error dW5: wait() failed at '%s'", newPath);
					perror(" ");				
					continue;	
				}				
				else if (WIFEXITED(wstatus))
					if (WEXITSTATUS(wstatus) == 1)
						++nFilesFound;
					else if (WEXITSTATUS(wstatus))
						perror("error dW6: child process terminated unsuccessfully");
						
				--nProcesses;	
			}
			
			if ((createChildProcesses(&nProcesses, newPath, seq)) < 0)
			{
				fprintf(stderr, "error dw4: createChildProcesses() failed at '%s'", newPath);
				perror(" ");
				continue;
			}
		}
		
	}	
	
	free(newPath);
	if (closedir(curDir))
	{
		fprintf(stderr, "error dW2: closedir() failed at  '%s'", path);
		perror(" ");
		return -2;
	}
	
	return nFilesFound;	
}


int findSeq(char *seq, char* fileName, int *byteLook)
{
	const int sizeSeq = strlen(seq);
	
	char *buffer = (char *)calloc(sizeSeq, 1);
	if (!buffer)
	{
		perror("error fS3 : calloc() failed");
		return -3;
	}
	
	
	FILE *file = fopen(fileName, "rb");
	if (!file)
	{
		fprintf(stderr, "error fS1 : fopen() failed at %s", fileName);
		perror("");
		free(buffer);
		return -1;
	}
	
	
	int nFound = 0;
	long int offset = 0;	
	char flagFirst = 1;
	
	*byteLook = -1;
	while (!feof(file))
	{
		if (fseek(file, offset, SEEK_SET))
			perror("error fS4 : fseek() failed");
		
		if (flagFirst)
		{		
			*byteLook = fread(buffer, 1, sizeSeq, file);
			flagFirst = 0;
		}
		else
		{
			fread(buffer, 1, sizeSeq, file);
			++(*byteLook);
		}

		if (strcmp(buffer, seq) == 0)
			++nFound;				
		++offset;				
	} 	
	
	
	free(buffer);
		
	if (fclose(file))
	{
		perror("error fS2 : fclose() failed");
		return -3;
	}
		
	return nFound;
}

int createChildProcesses(int *processNumber, char *fileName, char *seq)
{
	pid_t cpid = fork();
	int nFound;
	int nBytesLook;

	switch(cpid)
	{
	case -1: return -1;
	
	case 0:
		nFound = findSeq(seq, fileName, &nBytesLook);
		if (nFound >= 0)
			printf("pid = %d\tActive Processes = %d\t %s \tFound = %d\t Bytes Looked = %d\n\n", getpid(), 1 + *processNumber, fileName, nFound, nBytesLook);
		
		_exit(!!nFound);
			
	default: 
		++(*processNumber);
		return 0;		
	}	
}


