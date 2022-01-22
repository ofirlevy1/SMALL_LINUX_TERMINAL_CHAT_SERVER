#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>


#define MAX_SOCKETS 10 //maximum concurrent users allowed in chat
int sockets[MAX_SOCKETS];
char names[MAX_SOCKETS][11];
int listenSocket;
void* listenFunc(void* val);
pthread_mutex_t userArrLock;

void exitWithErrorMessage(const char*);
void addUser(int newSocket, char name[]);

int main()
{
	int i;
	pthread_mutex_init(&userArrLock, NULL);
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(27015);
	

	
	for (i = 0; i < MAX_SOCKETS; i++)
		sockets[i] = -1;
	
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == -1)
		exitWithErrorMessage("Socket creation failed!");
	
	
	if (bind(listenSocket, (const struct sockaddr*) &server, sizeof(server)) < 0)
		exitWithErrorMessage("binding failed!");
	
	printf("socket creation and binding successfull!\n");

	if (listen(listenSocket, 10) < 0)
		exitWithErrorMessage("listen failed!");

	pthread_t tempThread;
	pthread_create(&tempThread, 0, listenFunc, NULL);
	char buff[200];
	char messageWithName[211];
	char msg[100];

	printf("debug - starting while loop on main func\n");
	int whileCounter = 0;
	int j, recvRes;
	while(1)
	{			
		for(i = 0; i < MAX_SOCKETS; i++)
		{
			//locking to not intercept with adding new user to array by listen thread.
			pthread_mutex_lock(&userArrLock);
			//when we have a message from a socket
			if(sockets[i] != -1)
			{
				recvRes = recv(sockets[i], (void*)buff, 200, 0);
				//printf("debug - calling recv on non-empty socket..\n");
				if (recvRes > 0)
				{
					strcat(messageWithName, names[i]);
					strcat(messageWithName, ": ");
					strcat(messageWithName, buff);
					//printf("server broadcasting %s to everyone\n ", buff);
					//we send it the everyone else
					for(j = 0; j < MAX_SOCKETS; j++)
					{
						
						if(sockets[j] != -1 && i != j)
						{
							send(sockets[j], (void*)messageWithName, strlen(messageWithName) + 1, 0);
						}
					}
					printf("%s\n", messageWithName);
					messageWithName[0] = '\0';
					//printf("server finished broadcasting  %s  to everyone\n ", buff);
				}
				else if (recvRes == 0) //connection was closed by client, or there was a problem with it.
				{
					sprintf(msg, "server - %s has left the chat!\n", names[i]);
					broadcastMessage(msg);
					sockets[i] = -1;
				}
			//printf("debug - recv failed. continuing loop\n");
			}
			pthread_mutex_unlock(&userArrLock);
		}
	}
	return 0;
}

void broadcastMessage(char msg[])
{
	int j;
	for(j = 0; j < MAX_SOCKETS; j++)
	{
		if(sockets[j] != -1)
			send(sockets[j], (void*)msg, strlen(msg) + 1, 0);

	}
}



void* listenFunc(void* val)
{
	int newSocket_fd;
	char name[11];
	char msg[100];
	while(true)
	{
		newSocket_fd = accept(listenSocket, NULL, NULL);
		
		if (recv(newSocket_fd, (void*)name, 11, 0) > 0)
		{
			fcntl(newSocket_fd, F_SETFL, fcntl(newSocket_fd, F_GETFL, 0) | O_NONBLOCK);
			addUser(newSocket_fd, name);
			sprintf(msg, "server - %s has joined the chat!\n", name);
			puts(msg);
			broadcastMessage(msg);
		}			
	}
}

void addUser(int newSocket, char name[])
{
	int i;
	pthread_mutex_lock(&userArrLock);
	for(i = 0; i < MAX_SOCKETS; i++)
	{
		if(sockets[i] == -1)
		{
			sockets[i] = newSocket;			
			strcpy(names[i], name);
			break;
		}	
	}
	pthread_mutex_unlock(&userArrLock);
}

void exitWithErrorMessage(const char* msg)
{
		puts(msg);
		printf("\n");
		exit(1);
}