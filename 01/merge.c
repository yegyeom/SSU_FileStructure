#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	int fd1, fd2;
	int length;
	char buf[BUFFER_SIZE];
	char buf2[BUFFER_SIZE];

	if(argc != 3){
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}

	if((fd1 = open(argv[1], O_RDWR)) < 0 ){ //읽기,쓰기 가능하게 파일1 오픈
		fprintf(stderr, "open error for %sfile\n", argv[1]);
		exit(1);
	}

	if((fd2 = open(argv[2], O_RDONLY)) < 0){ //읽기 전용으로 파일2 오픈
		fprintf(stderr, "open error for %sfile\n", argv[2]);
		exit(1);
	}
	
	if((read(fd1, buf, BUFFER_SIZE)) < 0){ //파일1 내용을 read하여 buf에 넣음
		fprintf(stderr, "read error for %sfile\n", argv[1]);
		exit(1);
	}

	if((length = read(fd2, buf2, BUFFER_SIZE)) < 0){ //파일2 내용을 read하여 buf2에 넣고 read 한 크기를 length에 넣음
		fprintf(stderr, "read error for %sfile\n", argv[2]);
		exit(1);
	}

	write(fd1, buf2, length); //위에서 read한만큼 buf2있는 내용을 파일1에 write

	close(fd1);
	close(fd2);

	exit(0);
}

