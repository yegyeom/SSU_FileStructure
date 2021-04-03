#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#define SUFFLE_NUM 2000

typedef struct _Student
{
	char id[11];
	char name[21];
	char major[21];
	char addr[51];
} Student;

void GenRecordSequence(int* list, int n);
void swap(int* a, int* b);

int main(int argc, char* argv[]){
	Student Student;
	int* read_order_list; //레코드 번호들이 들어 있는 배열
	int num_of_records; //레코드 파일에 저장되어 있는 전체 레코드의 수
	struct timeval start, end;
	double difftime;
	int fd, length, i;

	if(argc != 2){
		fprintf(stderr, "usage : %sfile\n", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDONLY)) < 0){ //인자로 입력받은 파일 오픈
		fprintf(stderr, "open error\n");
		exit(1);
	}

	length = lseek(fd, 0, SEEK_END); //레코드 파일의 전체 크기
	num_of_records = length/100; //레코드 파일 갯수

	read_order_list = (int*)malloc(num_of_records*sizeof(int));//레코드 파일 개수만큼 동적할당

	for(i = 0 ; i < num_of_records ; i++) //입력한 레코드 수 만큼 배열에 넣음
		read_order_list[i] = i;

	GenRecordSequence(read_order_list, num_of_records);
	//read_order_list 배열에 추후 랜덤하게 읽어야 할 레코드 번호들이 순서대로 나열되어 저장
	gettimeofday(&start, NULL); //시간 측정 시작

	for(i = 0 ; i < num_of_records ; i++){
		if(lseek(fd, read_order_list[i] * sizeof(Student), SEEK_SET) < 0){
			fprintf(stderr, "fseek error\n");
			exit(1);
		}
		read(fd,(char*)&Student,sizeof(Student));
	}
	//'read_order_list'를 이용하여 표준 입력으로 받은 레코드 파일로부터 레코드를 random하게 읽어들이고 
	
	gettimeofday(&end, NULL); //시간 측정 끝
	difftime = (double)(end.tv_sec)+(double)(end.tv_usec) / 1000000.0 - (double)(start.tv_sec)-(double)(start.tv_usec) / 1000000.0;

	printf("#records : %d\ttimecost : %dus\n", num_of_records, (int)(difftime*1000000.0));
	return 0;
}

void GenRecordSequence(int* list, int n){ //read_order_list , num_of_records
	int i, j, k;

	srand((unsigned int)time(0));

	for(i = 0 ; i < n ; i++){
		list[i] = i;
	}

	for(i = 0 ; i < n ; i++){
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int* a, int* b){
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
