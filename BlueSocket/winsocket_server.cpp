#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <iostream>
#include <iomanip>
#include <time.h>

#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define BUF_SIZE 1024

using namespace std;

#include "BlueSocket.h"

namespace BlueSocket {
	unsigned long GetFileLength ( FILE * fileName)
	{
		unsigned long pos = ftell(fileName);
		unsigned long len = 0;
		fseek ( fileName, 0L, SEEK_END );
		len = ftell ( fileName );
		fseek ( fileName, pos, SEEK_SET );
		return len;
	}

	void draw_bar(int percentage, int buffer_count, int file_size)
	{
		string bar;
		bar+="[";
		for(int i=0; i<20; i++)
		{
			if(percentage/5>=i)
			{
				bar+="|";
			}
			else
			{
				bar+=" ";
			}
		}
		bar+="] ";
		printf("%s ",bar.c_str());
		printf("%3d%% (%d/%d) %.2lfm\n", percentage, (int)buffer_count/1024, (int)file_size/1024, (double)file_size/(double)1024/1024);
	}

	//int main(int argc, char *argv[]){
	void SendFile(const char *ip = "127.0.0.1", const char *filename = "") {

		printf("IP: %s\n", ip);
		printf("File Directory: %s\n", filename);
	    
		FILE *fp = fopen(filename, "rb");
		if(fp == NULL){
			printf("Cannot open file, press any key to exit!\n");
			system("pause");
			exit(0);
		}

		double file_size = GetFileLength(fp);//byte
		std::cout<<"File size "<<setprecision(2)<<fixed<<(double)file_size/1024/1024<<" m"<<std::endl;
		std::cout<<"Waiting to upload file..."<<std::endl;
		static clock_t start_time=clock();
		clock_t end_time=clock();

		WSADATA wsaData;
		WSAStartup( MAKEWORD(2, 2), &wsaData);
		SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = PF_INET;
		//sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		//sockAddr.sin_addr.s_addr = inet_addr(clientip);
		// sockAddr.sin_addr.s_addr = inet_addr("163.13.164.129");
		sockAddr.sin_addr.s_addr = inet_addr(ip);
		sockAddr.sin_port = htons(1234);
		bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
		listen(servSock, 20);
		SOCKADDR clntAddr;
		int nSize = sizeof(SOCKADDR);
		SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);

		char buffer[BUF_SIZE] = {0};
		int nCount;
		double buffer_count = 0;
		int percentage=0;
		while( (nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0 ){
			send(clntSock, buffer, nCount, 0);
			if(nCount>0)buffer_count+=nCount;
			//std::cout<<buffer<<"  buffer_count"<<buffer_count<<std::endl;
			end_time=clock();
			if(end_time-start_time>500)
			{   
				// system("cls");
				start_time=end_time;
				// printf("buffer_count %d\n",buffer_count);
	            
				percentage= buffer_count*100/file_size;
				draw_bar(percentage, buffer_count, file_size);
			}  
		}
		percentage= buffer_count*100/file_size;
		draw_bar(percentage, buffer_count, file_size);
		shutdown(clntSock, SD_SEND);
		recv(clntSock, buffer, BUF_SIZE, 0); 
		fclose(fp);
		closesocket(clntSock);
		closesocket(servSock);
		printf("File uploaded success\n");
		WSACleanup();
		Sleep(1000);
		//system("pause");

		return;
	}
}