#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<assert.h>
#include<string.h>
#include<unistd.h>


int main(int argc, char *argv[])
{
	char buf[1000] = {0};
	int fp = open("/dev/mypipe",O_RDWR);
	FILE *rfp;
	int count= atoi(argv[1]); 
	size_t read=0,written=0;
	if(argc < 3){
		printf("Error: <fileName> <numberOfBytes> <inputFileName>");
		exit(1);
	}
	rfp = fopen(argv[2], "r");
	if (rfp == NULL) {
  		fprintf(stderr, "Can't open input file in.list!\n");
  		exit(1);
	}
	
	
	if(read = fread (buf,1,count,rfp)){
		printf("Written to buffer:%s\n",buf);
		written  = write(fp,buf,read);
		if(written < 0){
			printf("Error in wrtting to device buffer");
		}			  
    }
	else
		printf("Error in file reading\n");
	fclose(rfp);
	close(fp);
	return 0;
}
