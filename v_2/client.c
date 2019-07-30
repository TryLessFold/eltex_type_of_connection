#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define SIZE_ADDR sizeof(struct sockaddr)
int main(int argc, char *argv[])
{
	if (argc == 1) return 0;
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(7778);
	serv.sin_addr.s_addr = inet_addr("127.0.0.1");
	int i = 1, size_serv = SIZE_ADDR;
	char buff[256];
	strcpy(buff,argv[i]);
//	fgets(buff, 256, stdin);
	if(strcmp(buff,"exit\n"))
	{
		sendto(fd, buff, 256, 0, (struct sockaddr* ) &serv, SIZE_ADDR);
		recvfrom(fd, buff, 256, 0, (struct sockaddr* ) &serv, &size_serv);
		printf("%s\n", buff);
	}
	i++;
	if (i>=argc)
		strcpy(buff, "exit\n");
	else
		strcpy(buff, argv[i]);
//	fgets(buff, 256, stdin);
	while(strcmp(buff,"exit\n"))
	{
		i++;
		if ((!strcmp(buff, "shutdown\n")))
		{
			//sendto(fd, buff, 256, 0, (struct sockaddr* ) &serv, SIZE_ADDR);
			serv.sin_family = AF_INET;
			serv.sin_port = htons(7778);
			serv.sin_addr.s_addr = inet_addr("127.0.0.1");
			sendto(fd, buff, 256, 0, (struct sockaddr*) &serv, SIZE_ADDR);
		}
		else if ((connect(fd,(struct sockaddr* ) &serv, sizeof(serv)))!=-1)
		{
			send(fd, buff, 256, 0);
			recv(fd, buff, 256, 0);
//			printf("%s\n",buff);
		}
		else
		{
			printf("Connection error\n");
		}
		if(i<argc)
			strcpy(buff,argv[i]);
		else
			strcpy(buff,"exit\n");
//		fgets(buff, 256, stdin);
	}
	if ((strcmp(buff, "shutdown")))
		sendto(fd, buff, 256, 0, (struct sockaddr *) &serv, SIZE_ADDR);
	close(fd);
	return 0;
}
