#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	int fd, length;
	char buf[BUFFER_SIZE];
	int num1, num2;
	off_t fsize;

	num1 = atoi(argv[2]); //오프셋
	num2 = atoi(argv[3]); //읽기 바이트 수

	if(argc != 4){
		fprintf(stderr, "Usage : %s file\n", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDONLY)) < 0){ //읽기 전용으로 파일 오픈
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	
	fsize = lseek(fd, 0, SEEK_END); //파일의 전체 사이즈

	if((lseek(fd, num1, SEEK_SET)) < 0){ //lseek을 이용하여 offset이동
		fprintf(stderr, "lseek error\n");
		exit(1);
	}

	if((length = read(fd, buf, num2)) < 0){ //offset부터 읽기 바이트 수만큼 read하여 length에 넣음
		fprintf(stderr, "read error\n");
		exit(1);
	}
	//데이터가 읽기 바이트 만큼 존재
	if(length > 0)	{
		write(1, buf, length); //표준 출력으로 일기 바이트만큼 파일 내용 출력
	}
	//데이터가 읽기 바이트 만큼 존재하지 않을 때
	if(length == 0){
		write(1, buf, fsize); //표준 출력으로 전체 파일 내용 출력
	}
	close(fd);
	exit(0);
}
