#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
int main(int argc, char *argv[])
{
	pid_t pid;
	int wa;
	for(int i=0; i<50; i++)
	{
//		sleep(1);
		pid = fork();
		if (pid == 0)
			execl("./client"," ", "lapa1", "sata2", "rata3", "exit", NULL);
		//else wait(&wa);
	}
//	wait(&wa);
	return 0;
}
