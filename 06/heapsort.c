#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
int pagenum, maxrecord, recordnum;

void heapify(char **heaparray, int recordnum);
void heap(char **heaparray, int *recordnum);
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	if(fseek(fp, PAGE_SIZE*pagenum, SEEK_SET) != 0){
		fprintf(stderr, "fseek error\n");
		exit(1);
	}
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	if(fseek(fp, PAGE_SIZE*pagenum, SEEK_SET) != 0){
		fprintf(stderr, "fseek error\n");
		exit(1);
	}
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며, 
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
//
void buildHeap(FILE *inputfp, char **heaparray)
{
	int count, rn;
	long long sn1, sn2;
	int num;
	char pagebuf[PAGE_SIZE];
	char tmprecord[RECORD_SIZE];
	int fsize;
	char check[4];
	char test[RECORD_SIZE];
	char tmp1[RECORD_SIZE];
	char tmp2[RECORD_SIZE];
	char temp[RECORD_SIZE];
	char temp2[RECORD_SIZE];
	char *ptr1, *ptr2;
	
	for(int i = 0 ; i < recordnum ; i++){
		heaparray[i] = (char*)malloc(sizeof(char)*RECORD_SIZE);
	}
	
	maxrecord = PAGE_SIZE/RECORD_SIZE;
	num=0;
	//배열에 레코드 삽입
	for(int i = 1 ; i < pagenum ; i++){
		count = 0;
		memset(pagebuf, 0, PAGE_SIZE);
		readPage(inputfp, pagebuf, i);
		while(count != maxrecord){
			memset(check, 0, sizeof(check));
			memcpy(check, pagebuf+RECORD_SIZE*count, 1);
			if(isdigit(*check)){
				strncpy(heaparray[num], &pagebuf[RECORD_SIZE*count], RECORD_SIZE);
				count++;
				num++;
			}
			else
				break;
		}
	} 
	
	rn = recordnum;

	for(int i = 0 ; i < recordnum ; i++){
		heapify(heaparray, rn);
		heap(heaparray, &rn);
	} //heap

}

void heapify(char **heaparray, int recordnum){
	long long sn1, sn2;
	char *ptr1, *ptr2;
	char tmp1[RECORD_SIZE];
	char tmp2[RECORD_SIZE];
	char temp[RECORD_SIZE];

	for(int j = 1 ; j < recordnum ; j++){
			int child = j;
			do{
				int root = (child-1)/2;
				
				memset(tmp1, 0, RECORD_SIZE);
				memset(tmp2, 0, RECORD_SIZE);
				memcpy(tmp1, heaparray[root], RECORD_SIZE);
				memcpy(tmp2, heaparray[child], RECORD_SIZE);
				ptr1 = strtok(tmp1, "#");
				sn1 = atoll(ptr1);
				ptr2 = strtok(tmp2, "#");
				sn2 = atoll(ptr2);
				
				if(sn1 < sn2){
					memset(temp, 0, RECORD_SIZE);
					memcpy(temp, heaparray[root], RECORD_SIZE);
					memset(heaparray[root], 0, RECORD_SIZE);
					memcpy(heaparray[root], heaparray[child], RECORD_SIZE);
					memcpy(heaparray[child], temp, RECORD_SIZE);
				}
				child = root;
			}while(child!=0);
		}
}

void heap(char **heaparray, int *recordnum){
	char temp[RECORD_SIZE];

	memset(temp, 0, RECORD_SIZE);
	memcpy(temp, heaparray[0], RECORD_SIZE);

	memset(heaparray[0], 0, RECORD_SIZE);
	memcpy(heaparray[0], heaparray[*recordnum-1], RECORD_SIZE);

	memset(heaparray[*recordnum-1], 0, RECORD_SIZE);
	memcpy(heaparray[*recordnum-1], temp, RECORD_SIZE);

	--(*recordnum);
}

//
// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
//
void makeSortedFile(FILE *outputfp, char **heaparray)
{
	char pagebuf[PAGE_SIZE];
	int root, rn;

	for(int i=1; i<pagenum; i++){
		memset(pagebuf, 0xFF, PAGE_SIZE);
		for(int j=0; j<maxrecord; j++){
			if(recordnum == 0)
				break;
			root = 0;
			memcpy(pagebuf+(j*RECORD_SIZE), heaparray[root], RECORD_SIZE);
			memcpy(heaparray[root], heaparray[--recordnum], RECORD_SIZE);

			rn = recordnum;

			for(int k = 0 ; k < recordnum ; k++){
				heapify(heaparray, rn);
				heap(heaparray, &rn);
			} //heap
		}
		writePage(outputfp, pagebuf, i);
	}

}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp;	// 정렬된 레코드 파일의 파일 포인터
	char pagebuf[PAGE_SIZE];

	if(argc != 4){
		fprintf(stderr, "usage : %s s <input record file name><output record file name>\n", argv[0]);
		exit(1);
	}
	
	if(access(argv[2], F_OK) < 0){
		fprintf(stderr, "%s doesn't exist\n", argv[2]);
		exit(1);
	}
	
	if((inputfp = fopen(argv[2], "r+")) == NULL){
		fprintf(stderr, "fopen error for %s\n", argv[1]);
		exit(1);
	}
	
	if((outputfp = fopen(argv[3], "w+")) == NULL){
		fprintf(stderr, "fopen error for %s\n", argv[2]);
		exit(1);
	}

	memset(pagebuf, 0, PAGE_SIZE);
	readPage(inputfp, pagebuf, 0);

	fseek(inputfp, 0, SEEK_SET);
	memcpy(&pagenum, pagebuf, sizeof(int));
	memcpy(&recordnum, pagebuf+4, sizeof(int));

	char *heaparray[recordnum];

	buildHeap(inputfp, heaparray);
	
	memset(pagebuf, 0, PAGE_SIZE);
	readPage(inputfp, pagebuf, 0);
	writePage(outputfp, pagebuf, 0); //헤더 페이지 복사

	makeSortedFile(outputfp, heaparray);
	
	return 1;
}
