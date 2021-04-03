#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	int fd1, fd2, count;
	char buf[BUFFER_SIZE];

	if(argc != 3){
		fprintf(stderr, "Usage : %s file\n", argv[0]); //argc가 3개가 아닐 경우 
		exit(1);
	}

	if((fd1 = open(argv[1], O_RDONLY)) < 0){ //원본파일을 읽기 전용으로  오픈
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	if((fd2 = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0){ //복사본파일 쓰기전용, mode644로 오픈
		fprintf(stderr, "open error for %s\n", argv[2]); 
		exit(1);
	}

	while((count = read(fd1, buf, 100)) > 0){ //fd1에 있는 파일을 100바이트씩 read하여 buf에 넣음, read해서 buf에 들어간 크기만큼 count에 넣어줌
		write(fd2, buf, count); //buf에 있는 count크기의 내용을 fd2, 즉 복사본 파일에 write해줌
	}
	
	close(fd1);
	close(fd2);

	exit(0);
}

