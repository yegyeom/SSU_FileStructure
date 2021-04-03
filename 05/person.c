#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void modify_header(FILE*fp, int n_totalpage, int n_totalrecord, int n_lastpage, int n_lastrecord);

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
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 그런 후 이 레코드를 저장할 페이지를 readPage()를 통해 프로그램 상에
// 읽어 온 후 pagebuf에 recordbuf에 저장되어 있는 레코드를 저장한다. 그 다음 writePage() 호출하여 pagebuf를 해당 페이지 번호에
// 저장한다. pack() 함수에서 readPage()와 writePage()를 호출하는 것이 아니라 pack()을 호출하는 측에서 pack() 함수 호출 후
// readPage()와 writePage()를 차례로 호출하여 레코드 쓰기를 완성한다는 의미이다.
// 
void pack(char *recordbuf, const Person *p)
{
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->sn, p->name, p->age, p->addr, p->phone, p->email);
}
// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다. 이 함수가 언제 호출되는지는
// 위에서 설명한 pack()의 시나리오를 참조하면 된다.
//
void unpack(const char *recordbuf, Person *p)
{
	int j = 0, i = 0;
	char tmp[1024] = {0};

	if(recordbuf[i] == '*'){
		memset(p, ' ', sizeof(Person));
		return;
	}

	while(recordbuf[i] != '#')
		tmp[j++] = recordbuf[i++];
	tmp[j] = '\0';
	strcpy(p->sn, tmp);
	memset(tmp, '\0', 1024);
	i++;
	j=0;

	while(recordbuf[i] != '#')
		tmp[j++] = recordbuf[i++];
	tmp[j] = '\0';
	strcpy(p->name, tmp);
	memset(tmp, '\0', 1024);
	i++;
	j=0;

	while(recordbuf[i] != '#')
		tmp[j++] = recordbuf[i++];
	tmp[j] = '\0';
	strcpy(p->age, tmp);
	memset(tmp, '\0', 1024);
	i++;
	j=0;

	while(recordbuf[i] != '#')
		tmp[j++] = recordbuf[i++];
	tmp[j] = '\0';
	strcpy(p->addr, tmp);
	memset(tmp, '\0', 1024);
	i++;
	j=0;

	while(recordbuf[i] != '#')
		tmp[j++] = recordbuf[i++];
	tmp[j] = '\0';
	strcpy(p->phone, tmp);
	memset(tmp, '\0', 1024);
	i++;
	j=0;

	while(recordbuf[i] != '#')
		tmp[j++] = recordbuf[i++];
	tmp[j] = '\0';
	strcpy(p->email, tmp);
	memset(tmp, '\0', 1024);
	i++;
	j=0;
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값을 구조체에 저장한 후 아래의 insert() 함수를 호출한다.
//
void insert(FILE *fp, const Person *p)
{
	char recordbuf[RECORD_SIZE];
	char pagebuf[PAGE_SIZE];
	int totalpage, totalrecord, lastpage, lastrecord;
	int nextpage, nextrecord;
	int maxrecord = PAGE_SIZE / RECORD_SIZE;
	int count = 0;
	char check[4];
	
	memset(recordbuf, (char)0xFF, RECORD_SIZE);
	pack(recordbuf, p); //입력받은 정보들 packing	
	
	readPage(fp, pagebuf, 0); //header page read
	memcpy(&totalpage, pagebuf, sizeof(int));
	memcpy(&totalrecord, pagebuf+4, sizeof(int));
	memcpy(&lastpage, pagebuf+4*2, sizeof(int));
	memcpy(&lastrecord, pagebuf+4*3, sizeof(int));
	
	if((lastpage==-1)&&(lastrecord==-1)){ //삭제된 레코드가 없을 때
		if(totalrecord==0){ //1. 첫 레코드 저장일 때 (저장된 레코드 하나도 없을 때)
			memset(pagebuf,(char)0xFF, PAGE_SIZE);
			memcpy(pagebuf, recordbuf, RECORD_SIZE);
			writePage(fp, pagebuf, 1);
			modify_header(fp, 1, 1, -2, -2); //총 페이지수 증가
		}
		else { //2. 첫 저장은 아닌데 삭제 된 게 없을 때
			memset(pagebuf, (char)0xFF, PAGE_SIZE);
			readPage(fp, pagebuf, totalpage-1); //마지막 page read
			while(count!=maxrecord){
				memset(check, 0, sizeof(check));
				memcpy(check, pagebuf+RECORD_SIZE*count, 1); //첫 글자 읽고
				if(!isdigit(*check)){
					memcpy(pagebuf+RECORD_SIZE*count, recordbuf, RECORD_SIZE);
					writePage(fp, pagebuf, totalpage-1);
					modify_header(fp, 0, 1, -2, -2);
					break;
				}
				if(isdigit(*check))
					count++;
			}
			if(count == maxrecord){ //마지막 페이지 꽉 찼으면
				memset(pagebuf, (char)0xFF, PAGE_SIZE);
				memcpy(pagebuf, recordbuf, RECORD_SIZE);
				writePage(fp, pagebuf, totalpage);
				modify_header(fp, 1, 1, -2, -2);
			}	
		}

	}
	else if((lastpage!=-1)&&(lastrecord!=-1)){ //삭제된 레코드 있을 때
			memset(pagebuf, (char)0xFF, PAGE_SIZE);
			readPage(fp, pagebuf, lastpage);

			memcpy(&nextpage, pagebuf+RECORD_SIZE*lastrecord+1, 4);
			memcpy(&nextrecord, pagebuf+RECORD_SIZE*lastrecord+5, 4);

			memcpy(pagebuf+RECORD_SIZE*lastrecord, recordbuf, RECORD_SIZE);
			writePage(fp, pagebuf, lastpage);
			
			modify_header(fp, 0, 1, nextpage, nextrecord); 
	}

}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *sn)
{
	int count = 1;
	int check = 0;
	int a;
	char star = '*';
	int totalpage, totalrecord, lastpage, lastrecord;
	int maxrecord = PAGE_SIZE/RECORD_SIZE;
	char record[RECORD_SIZE];
	char pagebuf[PAGE_SIZE];
	Person person;

	readPage(fp, pagebuf, 0); //header page read
	memcpy(&totalpage, pagebuf, sizeof(int));
	memcpy(&totalrecord, pagebuf+4, sizeof(int));
	memcpy(&lastpage, pagebuf+4*2, sizeof(int));
	memcpy(&lastrecord, pagebuf+4*3, sizeof(int));

	while(count<totalpage){
		memset(pagebuf, (char)0xFF, PAGE_SIZE);
		readPage(fp, pagebuf, count);
		check=0;
		while(check<maxrecord){
			memset(record, (char)0xFF, sizeof(record));
			memcpy(record, pagebuf+RECORD_SIZE*check, RECORD_SIZE);
			unpack(record, &person);
			if(strcmp(person.sn,sn)==0){
				a=-1;
				break;
			}
			if((count==totalpage-1)&&(check==maxrecord-1)){
				printf("input sn doesn't exist\n");
				exit(1);
			}
			check++;
		}
		if(a==-1){
			break;
		}
		count++;
	}

	memcpy(record, &star, sizeof(char));
	memcpy(record+1, &lastpage, sizeof(int));
	memcpy(record+5, &lastrecord, sizeof(int));

	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	
	readPage(fp, pagebuf, count);

	memcpy(pagebuf+RECORD_SIZE*check, record, RECORD_SIZE);
	

	writePage(fp, pagebuf, count); 
	
	modify_header(fp, 0, 0, count, check); 
}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	char inputsn[14];
	char check[PAGE_SIZE];
	char buffer[100] = {0,};
	int num;
	int num0=1;
	int num1=0;
	int num2=-1;
	struct stat buf;
	Person p;

	if(access(argv[2], F_OK) < 0){ //파일 존재하지 않을 때
		fp = fopen(argv[2], "w+"); //person.dat
	}
	else if(access(argv[2], F_OK) == 0){ //파일 존재시
		fp = fopen(argv[2], "r+");
	}
	
	if(strcmp(argv[1],"i")==0){ //insert
		strcpy(p.sn,argv[3]);
		strcpy(p.name, argv[4]);
		strcpy(p.age, argv[5]);
		strcpy(p.addr, argv[6]);
		strcpy(p.phone, argv[7]);
		strcpy(p.email, argv[8]);
		//구조체에 정보 저장	
		if(stat("person.dat", &buf) < 0){
			fprintf(stderr, "stat error\n");
			exit(1);
		}
		if(buf.st_size == 0){ //파일이 비었다면
			char headerpage[PAGE_SIZE];	
			memset(headerpage, (char)0xFF, PAGE_SIZE);
			
			memcpy(headerpage, &num0, sizeof(int));
			memcpy(headerpage+4, &num1, sizeof(int));
			memcpy(headerpage+4*2, &num2, sizeof(int));
			memcpy(headerpage+4*3, &num2, sizeof(int));

			writePage(fp, headerpage, 0); //header page 생성
			
		}
		insert(fp, &p);
	}
	else if(strcmp(argv[1],"d")==0){ //delete
		memset(inputsn, 0, sizeof(inputsn));
		strcpy(inputsn, argv[3]); //삭제하고픈 레코드의 주민번호
		delete(fp, inputsn);
	}
	fclose(fp);
	return 1;
}

void modify_header(FILE *fp, int n_totalpage, int n_totalrecord, int n_lastpage, int n_lastrecord){ //header page 값 수정해주는 함수
	char header[PAGE_SIZE];
	char n_header[PAGE_SIZE];
	int totalpage, totalrecord, lastpage, lastrecord;

	memset(n_header, (char)0xFF, PAGE_SIZE);

	readPage(fp, header, 0);
	memcpy(&totalpage, header, sizeof(int));
	memcpy(&totalrecord, header+4, sizeof(int));
	memcpy(&lastpage, header+4*2, sizeof(int));
	memcpy(&lastrecord, header+4*3, sizeof(int));
	
	if(n_totalpage != 0){ //page 증가
		totalpage = totalpage + n_totalpage;
	}
	if(n_totalrecord != 0){ //record 증가
		totalrecord = totalrecord + n_totalrecord;
	}
	if((n_lastpage != -2)&&(n_lastrecord != -2)){ //최근 삭제된 레코드 바뀜
		lastpage = n_lastpage;
		lastrecord = n_lastrecord;
	}
	//수정된 값으로 써줌
	memcpy(n_header, &totalpage, sizeof(int));
	memcpy(n_header+4, &totalrecord, sizeof(int));
	memcpy(n_header+4*2, &lastpage, sizeof(int));
	memcpy(n_header+4*3, &lastrecord, sizeof(int));
	
	writePage(fp, n_header, 0);
}

