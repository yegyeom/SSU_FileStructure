#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include "person.h"
//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 �� ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. ��, ������ ������ �аų� ��� �մϴ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ��ġ�� �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// �־��� ���ڵ� ���Ͽ��� ���ڵ带 �о� heap�� ����� ������. Heap�� �迭�� �̿��Ͽ� ����Ǹ�, 
// heap�� ������ Chap9���� ������ �˰����� ������. ���ڵ带 ���� �� ������ ������ ����Ѵٴ� �Ϳ� �����ؾ� �Ѵ�.
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
	//�迭�� ���ڵ� ����
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
// �ϼ��� heap�� �̿��Ͽ� �ֹι�ȣ�� �������� ������������ ���ڵ带 �����Ͽ� ���ο� ���ڵ� ���Ͽ� �����Ѵ�.
// Heap�� �̿��� ������ Chap9���� ������ �˰����� �̿��Ѵ�.
// ���ڵ带 ������� ������ ���� ������ ������ ����Ѵ�.
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
	FILE *inputfp;	// �Է� ���ڵ� ������ ���� ������
	FILE *outputfp;	// ���ĵ� ���ڵ� ������ ���� ������
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
	writePage(outputfp, pagebuf, 0); //��� ������ ����

	makeSortedFile(outputfp, heaparray);
	
	return 1;
}
