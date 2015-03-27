#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define SIZE 512
#define SECTOR_SIZE 512
//#define FILE_NAME "dump"
//#define FILE_NAME "fat12_4_size_4097"
#define FILE_NAME "/dev/sdb1"

int print_array(const unsigned char ptr[], int size){
	printf("\n");
	int i = 0;
	for(i=0; i<size; ++i)
		printf("%02x ", ptr[i]);
	printf("\n");
	return 0;
}

int print_sector(const unsigned char ptr[]){
	printf("\n");
	int i, j;
	int columns = 16;
	int rows = SECTOR_SIZE/columns;
	for(i=0; i<rows; ++i) {
		for(j=0; j<columns; ++j)
			printf("%02x ", ptr[i*columns+j]);
		printf("\n");
	}
	return 0;
}

int read_len(int fd, unsigned char buffer[], int offset, int len){
	int ret = 0;
	int gotten;

	memset(buffer, 0x00, len);
	if( lseek(fd, offset, SEEK_SET) < 0){
		printf ("lseek error\n");
		ret = -1;
		goto __func_end;
	}
	if( (gotten = read(fd, buffer, len)) <= 0 ) {
		printf ("read return %d\n", gotten);
		ret = -1;
		goto __func_end;
	}

__func_end: 
	return ret;
}

int write_len(int fd, const unsigned char buffer[], int offset, int len){
	int ret = 0;
	int gotten;

	if( lseek(fd, offset, SEEK_SET) < 0){
		printf ("lseek error\n");
		ret = -1;
		goto __func_end;
	}
	if( (gotten = write(fd, buffer, len)) != len ) {
		printf ("write return %d\n", gotten);
		ret = -1;
		goto __func_end;
	}

__func_end: 
	return ret;
}


int read_sector(int fd, unsigned char buffer[], int index){
	return read_len(fd, buffer, index*SECTOR_SIZE, SECTOR_SIZE);
}

int write_sector(int fd, const unsigned char buffer[], int index){
	return write_len(fd, buffer, index*SECTOR_SIZE, SECTOR_SIZE);
}



int get_fat(unsigned char fat[], int size){
	fat[0] = 0xf8;
	fat[1] = 0xff;
	fat[2] = 0xff;
	int numClusters = size/2048;
	if(size%2048==0 && size!=0) numClusters -= 1;
	int i;
	for(i=0; i<numClusters; ++i){
		int index = i+3;
		int start = 3*(i/2)+3;
		if(i%2==0){
			fat[start] = index & (0xff);
			char upper = fat[start+1] & (0xf0);
			char lower = (index >> 8) & (0x0f);
			fat[start+1] = upper | lower;
		}
		else{
			char upper = (index << 4) & (0xf0);
			char lower = fat[start+1] & (0x0f);
			fat[start+1] = upper | lower;
			fat[start+2] = (index >> 4) & (0xff);
		}
	}
	int start = 3*(i/2)+3;
	if(i%2==0){
		fat[start] = 0xff;
		char upper = fat[start+1] & (0xf0);
		char lower = 0x0f;
		fat[start+1] = upper | lower;
	}
	else{
		char upper = 0xf0;
		char lower = fat[start+1] & (0x0f);
		fat[start+1] = upper | lower;
		fat[start+2] = 0xff;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;

	if(argc < 2){
		printf("argc=%d < 2\n", argc);
		return -1;
	}

	printf ("hello\n");

	unsigned char file[999999];
	int size = 0;
	
	int finput = open(argv[1], O_RDONLY);
	if(finput < 0){
		printf("argv[1] not found\n");
		ret = -1;
		goto __func_end;
	}
	printf("%d\n", finput);
	int gotten;
	while( (gotten = read(finput, file+size, 512)) >0 ){
		size += gotten;
	}
	if(close(finput)<0){
		ret = -1;
		goto __func_end;
	}
	

	printf("argv[1] size = %d", size);

	unsigned char buf[SECTOR_SIZE];
	memset(buf, 0x00, SECTOR_SIZE);
	get_fat(buf, size);
	print_sector(buf);


	int fd = open(FILE_NAME, O_RDWR);
	if(fd < 0){
		ret = -1;
		goto __func_end;
	}

	printf("%d\n", fd);


	//write fat 1
	ret = write_sector(fd, buf, 1);
	if(ret < 0) goto __func_end;

	//write fat 2
	ret = write_sector(fd, buf, 3);
	if(ret < 0) goto __func_end;


	//read root dir
	ret = read_sector(fd, buf, 5);
	if(ret < 0) goto __func_end;

	buf[60] = size & 0xff;
	buf[61] = (size >> 8) & 0xff;
	buf[62] = (size >> 8*2) & 0xff;
	buf[63] = (size >> 8*3) & 0xff;

	print_sector(buf);

	//write root dir
	ret = write_sector(fd, buf, 5);
	if(ret < 0) goto __func_end;

	//write file
	ret = write_len(fd, file, 37*SECTOR_SIZE, size);
	if(ret < 0) goto __func_end;


	//close
	if(close(fd)<0){
		ret = -1;
		goto __func_end;
	}

	printf ("ok\n");
__func_end: 
	return ret;
}

