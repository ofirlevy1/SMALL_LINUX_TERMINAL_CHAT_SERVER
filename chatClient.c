
#include <sys/socket.h>
#include <stdbool.h>
#include <pthread.h> //don't forget pthread flag when compiling!
#include <string.h>
#include <arpa/inet.h> 
#include <stdio.h>

void* recieveLoop(void*);
void* sendLoop(void*);
int mySocket;
void exitWithErrorMessage(const char*);

int main()
{
	char clientsName[11];
	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	printf("Please enter your name(max 10 characters): ");
	fgets(clientsName, 11, stdin);
	clientsName[strlen(clientsName) - 1] = '\0';
	
	if(mySocket == -1)
		exitWithErrorMessage("Socket creation failed!");
	printf("socket created\n");
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr("79.177.208.85");
	server.sin_family = AF_INET;
	server.sin_port = htons(27015);
	
	if(connect(mySocket, (struct sockaddr*)&server, sizeof(server)) < 0)
		exitWithErrorMessage("failed to create a connection with server!");
	printf("connected to server\n");
	
	if(send(mySocket, (const void*)clientsName, strlen(clientsName) + 1, 0) < 0)
			exitWithErrorMessage("failed sending your name to server!\n");
	
	
	pthread_t recvThread, sendThread;
	int recievingThread = pthread_create(&recvThread, NULL, recieveLoop, NULL);
	int sendingThread = pthread_create(&sendThread, NULL, sendLoop, NULL);

	pthread_join(recvThread,NULL);
	pthread_join(sendThread, NULL);
	
	return 0;
}

void* recieveLoop(void* val)
{
	char buffer[101];
	while(recv(mySocket, (void*)buffer, 100, 0) > 0)
	{
		puts(buffer);
		printf("\n");
	}
}

void* sendLoop(void* val)
{
	char msg[201];
	
	while(true)
	{
		fgets(msg, 201, stdin);
		msg[strlen(msg) - 1] = '\0';
		if(send(mySocket, (const void*)msg, strlen(msg) + 1, 0) < 0)
			exitWithErrorMessage("failed sending your message to server!\n");
	}
}

void exitWithErrorMessage(const char* str)
{
	puts(str);
	printf("\n");
	exit(1);
}