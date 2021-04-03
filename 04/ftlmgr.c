// 주의사항
// 1. sectormap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. sectormap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(sectormap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sectormap.h"
// 필요한 경우 헤더 파일을 추가하시오.

//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//
void ftl_open();
void ftl_write(int lsn, char* sectorbuf);
void ftl_read(int lsn, char* sectorbuf);
void ftl_print();

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

typedef struct {
	int lsn;
	int psn;
} MapTblEntry;

typedef struct{
	MapTblEntry entry[DATAPAGES_PER_DEVICE];
} MapTbl;

MapTbl maptbl;

FILE *flashfp;
int free_blk;

void ftl_open()
{
	int i;
	// adress mapping table 초기화
	for(i=0 ; i < DATAPAGES_PER_DEVICE ; i++){
		maptbl.entry[i].psn = -1;
	}
	// free block's pbn 초기화
	free_blk = DATABLKS_PER_DEVICE; //맨 마지막 블럭으로 초기화 한 것 (BLOCKS_PER_DEVICE - 1)
    	// address mapping table에서 lbn 수는 DATABLKS_PER_DEVICE 동일
	return;
}
//
// 이 함수를 호출하기 전에 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 한다.
// 즉, 이 함수에서 메모리를 할당받으면 안된다.
//
void ftl_read(int lsn, char *sectorbuf)
{
	int ret;

	if(DATAPAGES_PER_DEVICE < lsn){ //올바른 범위가 아닐 때
		fprintf(stderr, "lsn[%d] not within range\n", lsn);
	}

	if(maptbl.entry[lsn].psn == -1){ //psn이 -1일 때 (매핑된 데이터가 없을 때)
		fprintf(stderr, "lsn[%d] : data doesn't exist\n", lsn);
	}
	char c = 0xFF;
//	char sector[SECTOR_SIZE];
	char pagebuf[PAGE_SIZE];

	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	
	if((ret = dd_read(maptbl.entry[lsn].psn, pagebuf)) < 0){
		fprintf(stderr, "read error\n");
		exit(1);
	}

	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	/*
	int j = 0;
	for(int i = 0 ; i < SECTOR_SIZE ; i++){
		if(sectorbuf[i] != c){
			sector[j] = sectorbuf[i];
			j++;
		}
	}

	if(strlen(sector) > 0){
		printf("%s\n", sector);
	}
*/
	return;
}


void ftl_write(int lsn, char *sectorbuf)
{
	SpareData data1;
	int ret;
	char buf[SPARE_SIZE];
	char* tmp_page = (char*)malloc(PAGE_SIZE);
	char* tmp_sectorbuf = (char*)malloc(SECTOR_SIZE);
	char* tmp_sparebuf = (char*)malloc(SPARE_SIZE);
	int psn = -1; 
	int beforepsn, i;
	if(DATAPAGES_PER_DEVICE < lsn){ //올바른 범위가 아닐 때
		fprintf(stderr, "lsn not within range\n");
		exit(1);
	}

	if(maptbl.entry[lsn].psn != -1){ //다른 freepage 찾기
		beforepsn = maptbl.entry[lsn].psn;
		fseek(flashfp, PAGE_SIZE * i + SECTOR_SIZE, SEEK_SET);
		fread(&data1, sizeof(SpareData), 1, flashfp);
		data1.is_invalid = 1; //동일 lsn인 데이터가 쓰이므로 is_invalid 값 1로 설정
		data1.lpn = lsn;	
		memcpy(buf,&data1,SPARE_SIZE);

		fseek(flashfp, PAGE_SIZE*beforepsn + SECTOR_SIZE, SEEK_SET);
		fwrite(buf, sizeof(buf), 1, flashfp); //SpareData 구조체 메모리에 write
	}

	for(i = 0 ; i<BLOCKS_PER_DEVICE*PAGES_PER_BLOCK ; i++){
		if(i / PAGES_PER_BLOCK == free_blk) //free block 위치면 다시
			continue;

		fseek(flashfp, PAGE_SIZE * i + SECTOR_SIZE, SEEK_SET);
		fread(&data1, sizeof(SpareData), 1, flashfp); 
		if(data1.is_invalid == -1){ //첫 write인 page(is_invalid -1)면
			psn = i;
			break;
				}
	}
	
	char* gar_pagebuf = (char*)malloc(PAGE_SIZE);
	memset(gar_pagebuf, (char)0xFF, PAGE_SIZE);

	if(psn == -1){ //free block을 제외한 모든 페이지에 모두 data가 있으면
		for(int j = 0 ; j < BLOCKS_PER_DEVICE*PAGES_PER_BLOCK ; j++){
			fseek(flashfp, PAGE_SIZE * j + SECTOR_SIZE, SEEK_SET);
			fread(&data1, sizeof(SpareData), 1, flashfp);
			if(data1.is_invalid == 1){  // garbage collection 실행
				int garbageblock = j / PAGES_PER_BLOCK; //is_invalid = 1 인 페이지가 존재하는 블럭 => garbage block
				for (int k = 0 ; k < PAGES_PER_BLOCK ; k++){			
					fseek(flashfp, BLOCK_SIZE*garbageblock + PAGE_SIZE*k + SECTOR_SIZE, SEEK_SET); //garbage block의 모든 페이지
					fread(&data1, sizeof(SpareData), 1, flashfp);
					if(data1.is_invalid == 0){ //garbagepage가 아니면
						fseek(flashfp, BLOCK_SIZE*garbageblock + PAGE_SIZE*k, SEEK_SET); //PAGE_SIZE만큼 read하여
						fread(gar_pagebuf, PAGE_SIZE, 1, flashfp);
						fseek(flashfp, BLOCK_SIZE * free_blk + PAGE_SIZE*k, SEEK_SET); //free block에 write
						fwrite(gar_pagebuf, PAGE_SIZE, 1, flashfp);

						maptbl.entry[data1.lpn].psn = free_blk*PAGES_PER_BLOCK + k; //매핑 테이블의 psn 갱신
					}
				}
				for(int m = 0 ; m < PAGES_PER_BLOCK ; m++){ //옮겨간 블럭에서 (free block)
					fseek(flashfp, BLOCK_SIZE*free_blk + PAGE_SIZE*m + SECTOR_SIZE, SEEK_SET);
					fread(&data1, sizeof(SpareData), 1, flashfp);
					if(data1.is_invalid == -1){ //아무 데이터도 없는 page가 있으면 그 위치가 psn
						psn = free_blk*PAGES_PER_BLOCK + m;
						break;
					}
				}

				free_blk = garbageblock; 
				//garbage block 지우고 그 block이 새로운 free block이 됨
				if((ret = dd_erase(garbageblock)) < 0){ 
					fprintf(stderr, "garbage block erase error\n");
					exit(1);
				}
				break;
			}
		}
	}

	maptbl.entry[lsn].psn = psn; //매핑 테이블의 psn 갱신

	//buffer 초기화
	memset(tmp_page, (char)0xFF, PAGE_SIZE); 
	memset(tmp_sectorbuf, (char)0xFF, SECTOR_SIZE); 
	memset(tmp_sparebuf, (char)0xFF, SPARE_SIZE); 

	memcpy(tmp_sectorbuf,sectorbuf,strlen(sectorbuf));
	memcpy(tmp_page, tmp_sectorbuf, SECTOR_SIZE); //tmp_page에 sectordata 복사

	data1.is_invalid = 0; //빈 page에 처음 데이터를 쓰므로 is_invalid 0
	data1.lpn = lsn;
	memcpy(tmp_page+SECTOR_SIZE,&data1,SPARE_SIZE); //tmp_page에 sparedata (lsn,is_invalid) 복사

	if((ret = dd_write(psn, tmp_page)) < 0){
		fprintf(stderr, "write error\n");
		exit(1);
	}
	
	return;
}

void ftl_print()
{
	printf("lpn ppn\n");
	for (int i = 0 ; i < DATAPAGES_PER_DEVICE ; i++){
		printf("%d %d\n", i, maptbl.entry[i].psn);
	}
	printf("free block's pbn=%d\n", free_blk);
	return;
}

