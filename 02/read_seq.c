#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#define SUFFLE_NUM 2000

typedef struct _Student
{
	char id[11];
	char name[21];
	char major[21];
	char addr[51];
} Student;

int main(int argc, char* argv[])
{
	Student Student;
	struct timeval start, end;
	double difftime;
	int fd;
	int i, length, num;
	
	if(argc != 2){
		fprintf(stderr, "usage : %sfile\n", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDONLY)) < 0){ //인자로 입력받은 파일 오픈
		fprintf(stderr, "open error\n");
		exit(1);
	}

	length = lseek(fd, 0, SEEK_END); //레코드 파일의 전체 크기
	num = length/100; //레코드 파일 갯수

	gettimeofday(&start, NULL); //시간 측정 시작

	for (i = 0 ; i < num ; i++){
		read(fd, (char *)&Student, sizeof(Student));
	}
	gettimeofday(&end, NULL); //시간 측정 끝

	difftime = (double)(end.tv_sec) + (double)(end.tv_usec) / 1000000.0 - (double)(start.tv_sec)-(double)(start.tv_usec) / 1000000.0; //걸린 시간

	printf("#records : %d\ttimecost : %dus\n", num,(int)(difftime*1000000.0));

	return 0;
}
