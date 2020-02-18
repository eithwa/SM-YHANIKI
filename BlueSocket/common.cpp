#include <stdio.h>
#include <string>

using namespace std;

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
}