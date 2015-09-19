#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<assert.h>
#include<string.h>
#include<unistd.h>


int main(int argc, char *argv[])
{
	char buffer[1000] = {0};
	
	memset(buffer,0,1000);
	FILE *rtf;
	size_t bytesRead;
	if(argc < 3){
		printf("Error: <fileName> <numberOfBytes> <outputFileName>");
		exit(1);
	}
	int count= atoi(argv[1]);
	int fp = open("/dev/mypipe",O_RDWR);
	rtf = fopen(argv[2], "a");
	
	if((bytesRead = read(fp,buffer,count)) < 0)
	{
    		printf("Error in buffer read\n");
		exit(1);
	}
	
	fwrite (buffer, sizeof(char), sizeof(buffer), rtf);
	
	fclose(rtf);
	close(fp);
	return 0;
}
