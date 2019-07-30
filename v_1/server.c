#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <malloc.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include<unistd.h>

#define SIZE_ADDR sizeof(struct sockaddr_in)
#define PORT 7793
pthread_mutex_t mt[3] = {PTHREAD_MUTEX_INITIALIZER};

typedef struct pack
{
	int *counter;
	struct sockaddr_in cl_addr;
	char str[256];
} pack_t;


struct tid_l
{
	pack_t val;
	struct tid_l* next;
	struct tid_l* prev;
};

struct pocket
{
	struct tid_l **head, **tail;
	struct tid_l* ptr;
};

struct tid_l* AddList(struct tid_l** head,
			struct tid_l** tail,
			pack_t v)
{
//	pthread_mutex_lock(&mt[0]);
//	printf("ADD\n");
//	fflush(stdout);
	struct pocket p;
	if ((*head) == NULL)
	{
		(*head) = (struct tid_l*)malloc(sizeof(struct tid_l));
		(*head) -> val.counter = v.counter;
		(*head) -> val.cl_addr.sin_addr.s_addr = v.cl_addr.sin_addr.s_addr;
		(*head) -> val.cl_addr.sin_family = v.cl_addr.sin_family;
		(*head) -> val.cl_addr.sin_port = v.cl_addr.sin_port;
		strcpy((*head) -> val.str, v.str);
		(*head) -> next = NULL;
		(*head) -> prev = NULL;
		(*tail) = (*head);
	}
	else
	{
		struct tid_l *tmp = (struct tid_l*)malloc(sizeof(struct tid_l));
		tmp -> val.counter = v.counter;
		tmp -> val.cl_addr.sin_addr.s_addr = v.cl_addr.sin_addr.s_addr;
		tmp -> val.cl_addr.sin_family = v.cl_addr.sin_family;
		tmp -> val.cl_addr.sin_port = v.cl_addr.sin_port;
		strcpy(tmp -> val.str, v.str);
		tmp -> next = NULL;
		tmp -> prev = (*tail);
		(*tail) -> next = tmp;
		(*tail) = tmp;
	}
//	pthread_mutex_unlock(&mt[0]);
	return *tail;
}

int Remove(struct tid_l** head,
	struct tid_l**tail,
	struct tid_l *val)
{
	pthread_mutex_lock(&mt[0]);
//	printf("REMOVE\n");
//	fflush(stdout);
	struct tid_l *tmp = val;
	if ((*head)==(*tail))
	{
		free((*head));
		(*head) = (*tail) = NULL;
	}
	else if (tmp == (*head))
	{
		(*head) = tmp->next;
		(*head)->prev = NULL;
		free(tmp);
	}
	else if(tmp == (*tail))
	{
		(*tail) = tmp->prev;
		(*tail)->next = NULL;
		free(tmp);
	}
	else
	{
		tmp->prev->next = tmp->next;
		tmp->next->prev = tmp->prev;
		free(tmp);
	}
	pthread_mutex_unlock(&mt[0]);
	return 0;
}

struct tid_l *tid_h = NULL, *tid_t = NULL;

void func(char* str)
{
	if(str[0]!='\0')
		str[0] = '0';
}

void *handler(void* pocket)
{
	int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int sso = 1, size_cl = SIZE_ADDR;
	//char buff[256];
	struct sockaddr_in cl;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &sso, sizeof(sso));
//	sso = 0;
//	setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &sso, sizeof(sso));
	struct tid_l *tmp = (struct tid_l *) pocket;
	pack_t *pack = &(tmp->val);
//	pthread_mutex_lock(&mt[1]);
	printf("IN: %d\n", pack->cl_addr.sin_port);
	fflush(stdout);
//	pthread_mutex_unlock(&mt[1]);
	func(pack->str);
	sendto(socket_fd, pack->str, sizeof(pack->str), 0, (struct sockaddr *) &(pack->cl_addr), SIZE_ADDR);
	//printf("%d", cl.sin_port);
	recvfrom(socket_fd, pack->str, 256, 0,(struct sockaddr *) &cl, &size_cl);
	while(strcmp(pack->str,"exit"))
	{
		if(!strcmp(pack->str,"shutdown"))
		{
//			sendto(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, SIZE_ADDR);
			break;
		}
		func(pack->str);
		sendto(socket_fd, pack->str, 256, 0,(struct sockaddr *) &cl, SIZE_ADDR);
		size_cl = SIZE_ADDR;
		recvfrom(socket_fd, pack->str, 256, 0,(struct sockaddr *) &cl, &size_cl);
	}
	pthread_mutex_lock(&mt[2]);
	(*pack->counter)--;
	printf("--%d\n", *pack->counter);
	pthread_mutex_unlock(&mt[2]);
//	pthread_mutex_lock(&mt[1]);
	printf("OUT: %d\n", cl.sin_port);
	fflush(stdout);
//	pthread_mutex_unlock(&mt[1]);
	Remove(&tid_h, &tid_t, tmp);
	shutdown(socket_fd, SHUT_RDWR);
}

int main()
{
	pthread_t tid;
	pack_t pack;
	int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int sso = 1;
	int size_cl, counter = 0;
	struct sockaddr_in serv, cl;
	char buff[256];

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &sso, sizeof(sso));
//	sso = 0;
	//setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &sso, sizeof(sso));

	serv.sin_family = AF_INET;
	serv.sin_port = htons(PORT);
	serv.sin_addr.s_addr = inet_addr("127.0.0.1");

	bind(socket_fd, (struct sockaddr *)&serv, SIZE_ADDR);
	listen(socket_fd, 5);
	while(1)
	{
		size_cl = SIZE_ADDR;
		recvfrom(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, &size_cl);
	//	printf("%s", buff);
		if (!strcmp(buff, "shutdown"))
		{
			strcpy(buff, "Server is being closed in few time.");
			sendto(socket_fd, buff, 256, 0, (struct sockaddr *) &cl, SIZE_ADDR);
			break;
		}
		strcpy(pack.str, buff);
		pack.counter = &counter;
		pack.cl_addr.sin_family = cl.sin_family;
		pack.cl_addr.sin_port = cl.sin_port;
		pack.cl_addr.sin_addr.s_addr = cl.sin_addr.s_addr;
		pthread_mutex_lock(&mt[2]);
		counter++;
		printf("++%d\n", counter);
		pthread_mutex_unlock(&mt[2]);
		pthread_mutex_lock(&mt[0]);
		pthread_create(&tid, NULL, &handler, (AddList(&tid_h, &tid_t, pack)));
		pthread_mutex_unlock(&mt[0]);
	}
	while (counter!=0);
	shutdown(socket_fd, SHUT_RDWR);
	return 0;
}
