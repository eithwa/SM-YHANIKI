
#ifndef BLUESOCKET_H
#define BLUESOCKET_H

namespace BlueSocket {

	typedef struct packet {
		packet(const char* _ip, const char* _dir, int _fileSize);
		const char* ip;
		const char* dir;
		int fileSize;
	} Packet;

	unsigned long GetFileLength ( FILE * fileName);
	void draw_bar(int percentage, int buffer_count, int file_size);
	void SendFile(const char *ip, const char *filename);
	void GetFile(const char *ip, const char *filename, int filesize);

	int SendFile_RageThread(void* data);
	int GetFile_RageThread(void* data);
}

#endif // BLUESOCKET_H