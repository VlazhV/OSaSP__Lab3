//TODO process: int *processNumber

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>


int dirWalk(char *path, int nProcesses, char *seq);
char *getAbsPath (char *relPath);
int findSeq(char *seq, char *fileName);
int createChildProcesses(int *processNumber, char *fileName, char *seq);


int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "command seq dir N\n 1)seq - sequence of bytes to search\n 2)dir - to start search\n 3)N - max count of processes-children\n");
		return -1;
	}

	
	char *absPath;
	if (!(absPath = getAbsPath(argv[2])))
	{
		perror("error m1 : getAbsPath() failed");
		return 1;
	}
	
	int N = strtol(argv[3], NULL, 10);
	
	int nFound = dirWalk(absPath, N, argv[1]);
	
	if (nFound < 0)
	{
		perror("error m2 : dirWalk() failed");
		return 2;
	}
	
	printf("nFound = %d\n", nFound);
	
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
	static int nProcesses = 1;
	DIR *curDir;
	int nFound = 0;
	int addFound = 0;	

	if (!(curDir = opendir(path)))
	{
		fprintf(stderr, "error dW1: opendir() failed at  '%s'", path);
		perror("");
		return -1;
	}
	
	char *newPath;
	if (!(newPath = (char*)calloc(PATH_MAX, 1)))
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
			if ((addFound = dirWalk(newPath, nProcesses, seq)) < 0)
			{
				fprintf(stderr, "error dW3: dirWalk() failed at '%s'", newPath);
				perror("");
				continue;
			}
			nFound += addFound;
		}
		else if (dire->d_type == DT_REG)
		{		
			//create process	
			if (nProcesses > maxnProcesses)
			{
				int wstatus;
				wait(&wstatus);				
			}
							
			if ((addFound = createChildProcesses(&nProcesses, seq, newPath)) < 0)
			{
				fprintf(stderr, "error dw4: findSeq() failed at '%s'", newPath);
				perror("");
				continue;
			}

			nFound += addFound;
		}
		
	}	
	
	free(newPath);
	if (closedir(curDir))
	{
		fprintf(stderr, "error dW2: closedir() failed at  '%s'", path);
		perror("");
		return -2;
	}
	
	return nFound;	
}



int findSeq(char *seq, char* fileName)
{
	const int sizeSeq = strlen(seq);
	
	char *buffer;
	if (!(buffer = (char *)calloc(sizeSeq, 1)))
	{
		perror("error fS3 : calloc() failed");
		return -3;
	}
	
	
	FILE *file;
	if (!(file = fopen(fileName, "rb")))
	{
		perror("error fS1 : fopen() failed");
		free(buffer);
		return -1;
	}
	
	
	
	int nFound = 0;
	long int offset = 0;	
	

	while (!feof(file))
	{
		if (fseek(file, offset, SEEK_SET))
			perror("error fS4 : fseek() failed");			
			
		fread(buffer, 1, sizeSeq, file);
		
		++offset;
		
		if (strcmp(buffer, seq) == 0)
			++nFound;
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

	switch(cpid)
	{
	case -1: return -1;
	
	case 0:
		nFound = findSeq(seq, fileName);
		if (nFound >= 0)
			printf("pid = %d\t№ = %d\n%s\nFound = %d\n\n", getpid(), *processNumber, fileName, nFound);
		return nFound;
			
	default: 
		++(*processNumber);
		return 0;		
	}	
}





