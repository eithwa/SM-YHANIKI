
#ifndef BLUESOCKET_H
#define BLUESOCKET_H

namespace BlueSocket {
	unsigned long GetFileLength ( FILE * fileName);
	void draw_bar(int percentage, int buffer_count, int file_size);
	void SendFile(const char *ip, const char *filename);
	void GetFile(const char *ip, const char *filename, const char *filesize);
}

#endif // BLUESOCKET_H