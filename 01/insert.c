#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	int fd;
	int num;
	int length;
	off_t fsize;
	char buf[BUFFER_SIZE];

	num = atoi(argv[2]); //입력받은 오프셋을 문자열에서 상수로 전환

	if(argc != 4){
		fprintf(stderr, "usage : %s file\n", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDWR)) < 0){ //파일을 읽기,쓰기 용도로 오픈
		fprintf(stderr, "open error\n");
		exit(1);
	}
	
	fsize = lseek(fd, 0, SEEK_END); //파일 전체 사이즈

/*	if((read(fd, buf, BUFFER_SIZE)) < 0){
		fprintf(stderr, "read error\n");
		exit(1);
	}
*/
	if((lseek(fd, num, SEEK_SET)) < 0){ //lseek을 이용하여 끼워넣기 할 오프셋으로 이동
		fprintf(stderr, "lseek error\n");
		exit(1);
	}

	if((length = read(fd, buf, BUFFER_SIZE)) < 0){ //파일을 오프셋위치부터 read하여 buf에 넣고 read한 파일의 크기를 length에 넣어줌
		fprintf(stderr, "read error\n");
		exit(1);
	}

	lseek(fd, num, SEEK_SET); //입력 받았던 오프셋으로 이동

	if((write(fd, argv[3], strlen(argv[3]))) < 0){ //입력받은 데이터를 write
		fprintf(stderr, "write error\n");
		exit(1);
	}

	ftruncate(fd,(num + strlen(argv[3]))); //파일의 처음 ~ 입력받은 데이터까지만 남기고 나머지 삭제

	write(fd, buf, length); //buf에 있던 내용 다시 파일에 write
	
	close(fd);
	exit(0);
}
