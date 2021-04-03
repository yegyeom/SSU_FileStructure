#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	int fd;
	char buf[BUFFER_SIZE];
	int num;
	num = atoi(argv[2]); //입력받은 offset을 문자열에서 상수로 전환

	if(argc != 4){
		fprintf(stderr, "usage : %s file\n", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDWR)) < 0){ //파일을 읽기,쓰기 용도로 오픈
		fprintf(stderr, "open error\n");
		exit(1);
	}

	if((read(fd, buf, BUFFER_SIZE)) < 0){ //파일의 내용을 read하여 buf에 넣음
		fprintf(stderr, "read error\n");
		exit(1);
	}

	if((lseek(fd, num, SEEK_SET)) < 0){ //lseek을 이용하여 파일의 offset을 덮어쓰기 할 위치로 이동
		fprintf(stderr, "lseek error\n");
		exit(1);
	}

	if((write(fd, argv[3], strlen(argv[3]))) < 0 ){ //파일에 입력한 데이터를 덮어씀
		fprintf(stderr, "write error\n");
		exit(1);
	}


	close(fd);
	exit(0);
}
