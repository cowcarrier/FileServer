#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
#pragma comment(lib, "ws2_32.lib")	//Winsock Library
#pragma warning(disable: 6386)



/*
 *Gets a Gets a thread identifier struct.
 *Recives requested files from client.
 *Passes files for further check for availability.
 */
DWORD WINAPI FileRequest(SOCKET* data);

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET serverSocket, clientSocket;
	struct sockaddr_in server, client;
	int c;
	HANDLE thread;
	

	printf("Initialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(8888);

	//Bind
	if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	puts("Bind done");

	//Listen to incoming connections
	listen(serverSocket, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);

	while (1)
	{
		clientSocket = accept(serverSocket, (struct sockaddr*)&client, &c);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("accept failed with error code : %d", WSAGetLastError());
			return 1;
		}

		puts("Connection accepted");
		

		
		thread = CreateThread(NULL, 0, FileRequest, (LPVOID)clientSocket, 0, NULL);
		if (thread == NULL)
		{
			puts("Thread failed");
			return 1;
		}

		
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}

DWORD WINAPI FileRequest(SOCKET* data)
{
	int found = 0;
	char* message = "Please enter the name of the file you are seeking for.";
	char* foundMessage = "File exists";
	char* notFoundMessage = "File doesn't exist, enter name agian";
	int recvSize;
	char clientReplay[2000];
	char buffer[BUFFER_SIZE];
	SOCKET clientSocket = (SOCKET)data;
	char pathIntro[] = "C:\\Files\\";
	char path[133];
	
	send(clientSocket, message, (int)strlen(message), 0);

	while (!found)
	{
		strcpy_s(path, sizeof( pathIntro), pathIntro);
		if ((recvSize = recv(clientSocket, clientReplay, sizeof(clientReplay) / sizeof(char), 0)) == SOCKET_ERROR)
		{
			puts("recv failed");

			return 1;
		}

		clientReplay[recvSize] = (char)NULL;

		char* fileName = (char*)malloc(recvSize * sizeof(char));
		if (fileName == 0)
		{
			return 1;
		}

		memcpy(fileName, clientReplay, recvSize * sizeof(char));
		fileName[recvSize] = (char)NULL;
		strcat_s(path, 100, fileName);
		FILE* file;

		if (fopen_s(&file, path, "rb") == 0)
		{

			send(clientSocket, foundMessage, (int)strlen(foundMessage), 0);
			memset(buffer, 0, BUFFER_SIZE);
			int length = 0;

			while ((length = (int)fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0)
			{
				if (send(clientSocket, buffer, length, 0) < 0)
				{
					printf("Send File: %s Failed\n", fileName);
					return 1;
				}
				memset(buffer, 0, BUFFER_SIZE);
			}

			fclose(file);
			printf("File: %s Transfer Successful!\n", fileName);
			char* end = "end";
			send(clientSocket, end, (int)strlen(message), 0);
			found++;
			
		}
		
		else
		{
			send(clientSocket, notFoundMessage, (int)strlen(notFoundMessage), 0);
		}
				
	}
	closesocket(clientSocket);
	return 0;
}


