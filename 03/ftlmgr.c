#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "flash.h"
// 필요한 경우 헤더파일을 추가한다

FILE *flashfp;	// fdevicedriver.c에서 사용

int dd_erase(int pbn);
int dd_write(int ppn, char *pagebuf);
int dd_read(int ppn, char *pagebuf);
//
// 이 함수는 FTL의 역할 중 일부분을 수행하는데 물리적인 저장장치 flash memory에 Flash device driver를 이용하여 데이터를
// 읽고 쓰거나 블록을 소거하는 일을 한다 (동영상 강의를 참조).
// flash memory에 데이터를 읽고 쓰거나 소거하기 위해서 fdevicedriver.c에서 제공하는 인터페이스를
// 호출하면 된다. 이때 해당되는 인터페이스를 호출할 때 연산의 단위를 정확히 사용해야 한다.
// 읽기와 쓰기는 페이지 단위이며 소거는 블록 단위이다.
// 
int main(int argc, char *argv[])
{	
	char sectorbuf[SECTOR_SIZE];//512
	char sector[SECTOR_SIZE];
	char sparebuf[SPARE_SIZE]; //16
	char spare[SPARE_SIZE];
	char pagebuf[PAGE_SIZE]; //4
	char *blockbuf;
	int ret;

	// flash memory 파일 생성: 위에서 선언한 flashfp를 사용하여 flash 파일을 생성한다. 그 이유는 fdevicedriver.c에서 
	//                 flashfp 파일포인터를 extern으로 선언하여 사용하기 때문이다.
	
	if(strcmp(argv[1],"c")==0){ 
		if((flashfp = fopen(argv[2], "w+")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}

		int ppn = atoi(argv[3]); //100
		
		blockbuf = (char*)malloc(sizeof(char)*BLOCK_SIZE);
		
		memset(blockbuf, (char)0xFF, BLOCK_SIZE); //blockbuf 초기화

		fseek(flashfp, 0, SEEK_SET);

		for(int i = 0 ; i <ppn ; i++){
			fwrite((void*)blockbuf, 1, BLOCK_SIZE, flashfp);
		}
	}

	if(strcmp(argv[1],"w")==0){ 
		//페이지 쓰기 : pagebuf의 섹터와 스페어에 각각 입력된 데이터를 정확히 저장하고 난 후 해당 인터페이스 호출
		if((flashfp = fopen(argv[2], "r+")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}

		int ppn = atoi(argv[3]); //physical page number
	
		fseek(flashfp, 0, SEEK_END);
		int fsize = ftell(flashfp);
		
		int num = fsize/PAGE_SIZE;
		if((num-1) < ppn){ //입력받은 ppn이 범위에서 벗어날 때
			fprintf(stderr, "ppn not within rage\n"); 
			exit(1);
		}

		memset(pagebuf, (char)0xFF, PAGE_SIZE); //pagebuf 초기화
		memset(sectorbuf, (char)0xFF, SECTOR_SIZE); //sectorbuf 초기화
		memset(sparebuf, (char)0xFF, SPARE_SIZE); //sparebuf 초기화
		
		blockbuf = (char*)malloc(sizeof(char)*BLOCK_SIZE);
		memset(blockbuf, (char)0xFF, BLOCK_SIZE);

		fseek(flashfp, PAGE_SIZE*ppn, SEEK_SET);
		char ch = fgetc(flashfp);
		char c = (char)0xFF;
		int pagenum;
		int blocknum;
		int count = 0;
		int i;

		if(ch != c){ //쓰려는 위치에 이미 데이터가 있으면
			fseek(flashfp, 0, SEEK_SET);
			int j = 0;
			for(i=0 ; i<(fsize/BLOCK_SIZE) ; i++){
				fseek(flashfp, i*BLOCK_SIZE, SEEK_SET);
				count = 0;
				for(j=1 ; j<=PAGE_NUM ; j++){
					fseek(flashfp, i*PAGE_SIZE, SEEK_CUR);
					if((ch=fgetc(flashfp)) == c)
						count++;
				}
				if(count==PAGE_NUM){
					blocknum = i;
					break;
				}
			}
			fseek(flashfp, BLOCK_SIZE*(ppn/4), SEEK_SET);
			for(i=0 ; i < PAGE_NUM ; i++){
				fseek(flashfp, BLOCK_SIZE*(ppn/4)+i*PAGE_SIZE, SEEK_SET);
				pagenum = ftell(flashfp)/PAGE_SIZE;
				if(pagenum == ppn){
					fseek(flashfp, PAGE_SIZE , SEEK_CUR);
					continue;
				}
				dd_read(pagenum, pagebuf);
				dd_write(blocknum*PAGE_NUM+i, pagebuf);
			}
			dd_erase(ppn/4);

			memset(pagebuf, (char)0xFF, PAGE_SIZE);
			fseek(flashfp, BLOCK_SIZE*(ppn/4), SEEK_SET);
			for(i=0 ; i < PAGE_NUM ; i++){
				memset(pagebuf, (char)0xFF, PAGE_SIZE);
				pagenum = ftell(flashfp)/PAGE_SIZE;
				if(pagenum == ppn){
					memcpy(pagebuf, argv[4], strlen(argv[4]));
					memcpy(pagebuf+SECTOR_SIZE, argv[5], strlen(argv[5]));

					dd_write(ppn, pagebuf);
				}
				else{
					dd_read(blocknum*PAGE_NUM + i, pagebuf);
					dd_write(pagenum, pagebuf);
				}
			}
			dd_erase(blocknum);
		}

		else{ //쓰려는 위치에 데이터가 없으면
			memcpy(pagebuf, argv[4], strlen(argv[4])); //sectordata 입력
			memcpy(pagebuf+SECTOR_SIZE, argv[5], strlen(argv[5])); //spare값 입력

			if((ret = dd_write(ppn, pagebuf)) < 0) {
				fprintf(stderr, "write error\n");
				exit(1);
			}	
		}
	}

	// 페이지 읽기: pagebuf를 인자로 사용하여 해당 인터페이스를 호출하여 페이지를 읽어 온 후 여기서 섹터 데이터와
	//                  스페어 데이터를 분리해 낸다
	if(strcmp(argv[1],"r")==0){
		if((flashfp = fopen(argv[2], "r+")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}
		char c = 0xFF;
		int ppn = atoi(argv[3]);
		
		fseek(flashfp, 0, SEEK_END);
		int fsize = ftell(flashfp);

		int num = fsize/PAGE_SIZE;
		if((num - 1) < ppn){
			fprintf(stderr, "ppn not within range\n");
			exit(1);
		}
	
		if((ret = dd_read(ppn, pagebuf)) < 0){
			fprintf(stderr, "read error\n");
			exit(1);
		}
		memset(sector, 0, SECTOR_SIZE); //출력할 sector배열 초기화
		memset(spare, 0, SPARE_SIZE); //출력할 spare배열 초기봐

		int j = 0;
		memset(sectorbuf, 0, SECTOR_SIZE); //sectorbuf 초기화
		memcpy(sectorbuf, pagebuf, SECTOR_SIZE); //pagebuf에서 sectorbuf로 sectordata 복사
		for (int i = 0 ; i <SECTOR_SIZE ; i++){
			if(sectorbuf[i] != c){ //sectorbuf에서 0xFF를 만나지 않을 때만 sector에 입력
				sector[j] = sectorbuf[i];
				j++;
			}
		}

		memset(sparebuf, 0, SPARE_SIZE);
		memcpy(sparebuf, pagebuf + SECTOR_SIZE, SPARE_SIZE); //pagebuf에서 sparebuf로 sparedata 복사
		j = 0;
		for(int i = 0 ; i < SPARE_SIZE ; i++){
			if(sparebuf[i] != c){ //sparebuf에서 0xFF를 만나지 않을 때만 spare에 입력
				spare[j] = sparebuf[i];
				j++;
			}
		}

		if(strlen(sector) > 0 && strlen(spare) > 0){
			printf("%s %s", sector, spare);
		}
	}
	
	if(strcmp(argv[1],"e")==0){ //erase
		if((flashfp = fopen(argv[2],"r+")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}

		int pbn = atoi(argv[3]);

		if((ret = dd_erase(pbn)) < 0){
			fprintf(stderr, "erase error\n");
			exit(1);
		}
	}

	fclose(flashfp);

	return 0;
}
