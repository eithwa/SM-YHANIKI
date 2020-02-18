#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 1024
using namespace std;

#include "BlueSocket.h"

namespace BlueSocket {

	void GetFile(const char *ip = "127.0.0.1", const char *filename = "", const char *filesize = "0") {

		int file_size = atoi(filesize);
		printf("IP: %s\n", ip);
		printf("File Directory: %s\n", filename);
		printf("File size: %d\n", file_size);
		//Sleep(10000);
		FILE *fp = fopen(filename, "wb"); 
		if(fp == NULL){
			printf("Cannot open file, press any key to exit!\n");
			system("pause");
			exit(0);
		}
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = PF_INET;
		// sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		sockAddr.sin_addr.s_addr = inet_addr(ip);
		sockAddr.sin_port = htons(1234);
		connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));

		char buffer[BUF_SIZE] = {0}; 
		int nCount=-1;
		double buffer_count = 0;
		printf("Waiting to download file...\n");
		static clock_t start_time=clock();
		clock_t end_time=clock();
		
		int percentage=0;
		while( (nCount = recv(sock, buffer, BUF_SIZE, 0)) > 0 || nCount==-1)
		{
			if(nCount!=-1)
			{
				if(nCount>0)buffer_count+=nCount;
				fwrite(buffer, nCount, 1, fp);
				end_time=clock();
				if(end_time-start_time>500)
				{   
					// system("cls");
					start_time=end_time;
					// printf("buffer_count %d\n",buffer_count);
	                
					percentage= buffer_count*100/1024/file_size;
					draw_bar(percentage, buffer_count, file_size);
				}
			}   
		}
		percentage= buffer_count*100/1024/file_size; 
		draw_bar(percentage, buffer_count, file_size);
	    
		puts("File transfer success!");

		fclose(fp);
		closesocket(sock);
		WSACleanup();
		Sleep(1000);

		return;
	}
}