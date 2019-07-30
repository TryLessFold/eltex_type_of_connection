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
#define NUM_THREADS 3
/*
struct tid_l
{
	pthread_t *val;
	struct tid_l* next;
	struct tid_l* prev;
};
*/
typedef struct pack
{
	int *flag;
	int *counter;
	char str[256];
	struct sockaddr_in cl_addr;
} pack_t;

/*
void AddList(struct tid_l** head, struct tid_l** tail, pthread_t v)
{
	printf("ADD\n");
	fflush(stdout);
	if ((*head) == NULL)
	{
		(*head) = (struct tid_l*)malloc(sizeof(struct tid_l));
		(*head) -> val = &v;
		(*head) -> next = NULL;
		(*head) -> prev = NULL;
		(*tail) = (*head);
	}
	else
	{
		struct tid_l *tmp = (struct tid_l*)malloc(sizeof(struct tid_l));
		tmp -> val = &v;
		tmp -> next = NULL;
		tmp -> prev = (*tail);
		(*tail) -> next = tmp;
		(*tail) = tmp;
	}
}

int Remove(struct tid_l** head, struct tid_l**tail, pthread_t val)
{
	printf("REMOVE\n");
	fflush(stdout);
	struct tid_l *tmp = (*head);
	while(!pthread_equal(val, *(tmp->val)))
	{
		tmp = tmp->next;
		if (tmp == NULL)
			return -1;
	}
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
	return 0;
}

struct tid_l *tid_h = NULL, *tid_t = NULL;
*/
void func(char* str)
{
	if(str[0]!='\0')
		str[0] = '0';
}

void *handler(void* pocket)
{
	int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int sso = 1, size_cl = SIZE_ADDR;
	char buff[256];
	struct sockaddr_in cl;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &sso, sizeof(sso));
	sso = 0;
	setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &sso, sizeof(sso));
	pack_t *pack = (pack_t *) pocket;
	func(pack->str);
	sendto(socket_fd, pack->str, sizeof(pack->str), 0, (struct sockaddr *) &(pack->cl_addr), SIZE_ADDR);
	recvfrom(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, &size_cl);
	while(strcmp(buff,"exit\n"))
	{
		if(!strcmp(buff,"shutdown\n"))
		{
//			sendto(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, SIZE_ADDR);
			break;
		}
		func(buff);
		sendto(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, SIZE_ADDR);
		size_cl = SIZE_ADDR;
		recvfrom(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, &size_cl);
	}
	(*pack->counter)--;
}


void *handler_const(void *status)
{
	int *stat = (int* ) status;
	key_t key;
	pack_t pack;
	int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int sso = 1, size_cl = SIZE_ADDR;
	char buff[256];
	struct sockaddr_in cl, serv;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &sso, sizeof(sso));
	sso = 0;
	setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &sso, sizeof(sso));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(7778);
	serv.sin_addr.s_addr = inet_addr("127.0.0.1");
	sendto(socket_fd, buff, 256, 0, (struct sockaddr* ) &serv, SIZE_ADDR);
	while(1)
	{
		//sendto(socket_fd, pack_t->str, sizeof(pack->str), 0, (struct sockaddr *) &(pack->cl_addr), SIZE_ADDR);
		size_cl = SIZE_ADDR;
		recvfrom(socket_fd, (char *) &pack, sizeof(pack), 0,(struct sockaddr *) &serv, &size_cl);
		strcpy(buff, pack.str);
		cl.sin_family = pack.cl_addr.sin_family;
		cl.sin_port = pack.cl_addr.sin_port;
		cl.sin_addr = pack.cl_addr.sin_addr;
		while (strcmp(buff, "exit\n"))
		{
			if (!strcmp(buff,"shutdown\n"))
			{
				//strcpy(buff, "Server is being closed in few time.");
				//sendto(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, SIZE_ADDR);
				(*stat) = -1;
				break;
			}
			func(buff);
			sendto(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, SIZE_ADDR);
			size_cl = SIZE_ADDR;
			recvfrom(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, &size_cl);
		}
		(*stat) = 0;
	}
}
int main()
{
	pthread_t tid, tid_const[NUM_THREADS];
	pack_t pack;
	int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int sso = 1, hand_status[NUM_THREADS], j = 0, exit_stat = 0;
	int size_cl, counter = 0;
	struct sockaddr_in serv, cl, hand_addr[NUM_THREADS] = {0};
	char buff[256];

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &sso, sizeof(sso));
	sso = 0;
	setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &sso, sizeof(sso));

	serv.sin_family = AF_INET;
	serv.sin_port = htons(7778);
	serv.sin_addr.s_addr = inet_addr("127.0.0.1");

	bind(socket_fd, (struct sockaddr *)&serv, SIZE_ADDR);

	for (int i = 0; i<NUM_THREADS; i++)
	{
		pthread_create(&tid_const[i], NULL, &handler_const, &hand_status[i]);
		size_cl = SIZE_ADDR;
		recvfrom(socket_fd, buff, 256, 0, (struct sockaddr *) &hand_addr[i], &size_cl);
	}

	while(exit_stat == 0)
	{
		size_cl = SIZE_ADDR;
		for(int i = 0; i< NUM_THREADS; i++) printf("%4d", hand_status[i]);
		printf("%4d\n", counter);
		recvfrom(socket_fd, buff, 256, 0,(struct sockaddr *) &cl, &size_cl);
		if (!strcmp(buff, "shutdown\n"))
		{
			strcpy(buff, "Server is being closed in few time.");
			sendto(socket_fd, buff, 256, 0, (struct sockaddr *) &cl, SIZE_ADDR);
			break;
		}
		strcpy(pack.str, buff);
		pack.cl_addr.sin_family = cl.sin_family;
		pack.cl_addr.sin_port = cl.sin_port;
		pack.cl_addr.sin_addr.s_addr = cl.sin_addr.s_addr;
		for(j = 0; j < NUM_THREADS; j++)
		{
			if (hand_status[j] == -1) exit_stat = 1;
			if (hand_status[j] == 0)
			{
				sendto(socket_fd,(char *) &pack, sizeof(pack), 0, (struct sockaddr* ) &hand_addr[j], SIZE_ADDR);
				hand_status[j] = 1;
				break;
			}
		}
		if(j >= NUM_THREADS)
		{
			pack.counter = &counter;
			pthread_create(&tid, NULL, &handler, &pack);
			counter++;
		}
	}
	for(int i = 0; i<NUM_THREADS; i++)
	{
		while(hand_status[i]>0);
		if(hand_status[i] == 0)
			pthread_cancel(tid_const[i]);
	}
	while(counter>0);
	shutdown(socket_fd, SHUT_RDWR);
	return 0;
}
