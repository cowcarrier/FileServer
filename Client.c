#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
#pragma comment(lib, "ws2_32.lib")
# pragma warning(disable: 6386)

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char serverReply[2000];
	char input[2000];
	int recvSize;
	int found = 0;
	
	char buffer[BUFFER_SIZE];
	FILE* file;
	PCWSTR IP = L"192.168.0.1";

	printf("Initialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");
	InetPtonW(AF_INET, IP, &server.sin_addr.s_addr);
	//server.sin_addr.s_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

	puts("Connected");
	if ((recvSize = recv(s, serverReply, 2000, 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
		return 1;
	}

	serverReply[recvSize] = (char)NULL;
	printf("%s\n", &serverReply);
	while (!found)
	{
		
		if (scanf_s("%s", input, (unsigned int)sizeof(input)) == EOF)
		{
			return 1;
		}

		int i = 0;
		while (input[i] != (char)NULL)
			i++;

		char* fileName = (char*)malloc(i * sizeof(char));
		if (fileName == 0)
		{
			return 1;
		}

		memcpy(fileName, input, i * sizeof(char));

		fileName[i] = (char)NULL;

		send(s, fileName, i, 0);
		if ((recvSize = recv(s, serverReply, 2000, 0)) == SOCKET_ERROR)
		{
			puts("recv failed");
			return 1;
		}

		serverReply[recvSize] = (char)NULL;
		puts(serverReply);
		
		if (!strcmp(serverReply, "File exists"))
		{
			FILE *fp;
			; //Under Windows is "wb", which means to open a write-only binary
			if (fopen_s(&fp, fileName, "wb")!=0)
			{
				printf("File: %s Can Not Open To Writen", fileName);
				return 1;
			}
			else
			{
				memset(buffer, 0, BUFFER_SIZE);
				int length = 0;
				while ((length = recv(s, buffer, BUFFER_SIZE, 0)) > 0 && strcmp(buffer, "end"))
				{
					if (fwrite(buffer, sizeof(char), length, fp) < length)
					{
						printf("File: %s Write Failed\n", fileName);
						return 1;
					}
					memset(buffer, 0, BUFFER_SIZE);
				}

				printf("Receive File: %s From Server Successful!\n", fileName);
				
			}
			found++;
			fclose(fp);
			closesocket(s);
			WSACleanup();
			
			
		}
	}

	
	return 0;
}