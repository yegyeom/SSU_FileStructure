#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	int fd;
	int num1, num2, length;
	off_t fsize;
	char buf[BUFFER_SIZE];
	//입력받은 오프셋과 삭제 바이트 수를 문자열에서 상수로 전환
	num1 = atoi(argv[2]);
	num2 = atoi(argv[3]); 
	
	if(argc != 4){
		fprintf(stderr, "usage : %sfile\n", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDWR)) < 0){ //파일을 읽기, 쓰기 전용으로 오픈
		fprintf(stderr, "open error\n");
		exit(1);
	}

	if((fsize = lseek(fd, 0, SEEK_END)) < 0){ //파일의 전체 사이즈
		fprintf(stderr, "lseek error\n");
		exit(1);
	}

	if(((fsize)<(num1+num2))&&(fsize>num1)){ //삭제 바이트 수만큼 데이터가 존재하지 않는 경우 마지막 바이트까지삭제
		ftruncate(fd, num1); 
	}

	else{

		if((lseek(fd, num1+num2, SEEK_SET)) < 0){ //lseek을 사용해서 offset 이동
			fprintf(stderr, "lseek error\n");
			exit(1);
		}

		if((length = read(fd, buf, BUFFER_SIZE)) < 0){ //이동한 위치부터 끝까지 파일 read
			fprintf(stderr, "read error\n");
			exit(1);
		}
	
		ftruncate(fd, num1); //입력받은 오프셋만큼만 남기고 나머지 삭제
	
		lseek(fd, num1, SEEK_SET); //lseek을 사용해서 offset 이동

		if((write(fd, buf, length)) < 0){ //buf에 넣어놨던 내용을 write
			fprintf(stderr, "write error\n");
			exit(1);
			}
		}

	close(fd);
	exit(0);
}
