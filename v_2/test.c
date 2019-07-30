#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
int main(int argc, char *argv[])
{
	pid_t pid;
	int wa;
	for(int i=0; i<5; i++)
	{
		pid = fork();
		if (pid == 0)
			execl("./client"," ", "lapa1", "sata2", "rata3", "exit\n", NULL);
		//else wait(&wa);
	}
//	wait(&wa);
	return 0;
}
