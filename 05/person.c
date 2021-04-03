#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "person.h"
//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 ���ų� ���� ���ڵ带 ������ ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. �� ������ ������ �аų� ��� �մϴ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// ���ο� ���ڵ带 ������ �� �͹̳ηκ��� �Է¹��� ������ Person ����ü�� ���� �����ϰ�, pack() �Լ��� ����Ͽ�
// ���ڵ� ���Ͽ� ������ ���ڵ� ���¸� recordbuf�� �����. �׷� �� �� ���ڵ带 ������ �������� readPage()�� ���� ���α׷� ��
// �о� �� �� pagebuf�� recordbuf�� ����Ǿ� �ִ� ���ڵ带 �����Ѵ�. �� ���� writePage() ȣ���Ͽ� pagebuf�� �ش� ������ ��ȣ��
// �����Ѵ�. pack() �Լ����� readPage()�� writePage()�� ȣ���ϴ� ���� �ƴ϶� pack()�� ȣ���ϴ� ������ pack() �Լ� ȣ�� ��
// readPage()�� writePage()�� ���ʷ� ȣ���Ͽ� ���ڵ� ���⸦ �ϼ��Ѵٴ� �ǹ��̴�.
// 
void pack(char *recordbuf, const Person *p)
{
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->sn, p->name, p->age, p->addr, p->phone, p->email);
}
// 
// �Ʒ��� unpack() �Լ��� recordbuf�� ����Ǿ� �ִ� ���ڵ带 ����ü�� ��ȯ�� �� ����Ѵ�. �� �Լ��� ���� ȣ��Ǵ�����
// ������ ������ pack()�� �ó������� �����ϸ� �ȴ�.
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
// ���ο� ���ڵ带 �����ϴ� ����� �����ϸ�, �͹̳ηκ��� �Է¹��� �ʵ尪�� ����ü�� ������ �� �Ʒ��� insert() �Լ��� ȣ���Ѵ�.
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
	pack(recordbuf, p); //�Է¹��� ������ packing	
	
	readPage(fp, pagebuf, 0); //header page read
	memcpy(&totalpage, pagebuf, sizeof(int));
	memcpy(&totalrecord, pagebuf+4, sizeof(int));
	memcpy(&lastpage, pagebuf+4*2, sizeof(int));
	memcpy(&lastrecord, pagebuf+4*3, sizeof(int));
	
	if((lastpage==-1)&&(lastrecord==-1)){ //������ ���ڵ尡 ���� ��
		if(totalrecord==0){ //1. ù ���ڵ� ������ �� (����� ���ڵ� �ϳ��� ���� ��)
			memset(pagebuf,(char)0xFF, PAGE_SIZE);
			memcpy(pagebuf, recordbuf, RECORD_SIZE);
			writePage(fp, pagebuf, 1);
			modify_header(fp, 1, 1, -2, -2); //�� �������� ����
		}
		else { //2. ù ������ �ƴѵ� ���� �� �� ���� ��
			memset(pagebuf, (char)0xFF, PAGE_SIZE);
			readPage(fp, pagebuf, totalpage-1); //������ page read
			while(count!=maxrecord){
				memset(check, 0, sizeof(check));
				memcpy(check, pagebuf+RECORD_SIZE*count, 1); //ù ���� �а�
				if(!isdigit(*check)){
					memcpy(pagebuf+RECORD_SIZE*count, recordbuf, RECORD_SIZE);
					writePage(fp, pagebuf, totalpage-1);
					modify_header(fp, 0, 1, -2, -2);
					break;
				}
				if(isdigit(*check))
					count++;
			}
			if(count == maxrecord){ //������ ������ �� á����
				memset(pagebuf, (char)0xFF, PAGE_SIZE);
				memcpy(pagebuf, recordbuf, RECORD_SIZE);
				writePage(fp, pagebuf, totalpage);
				modify_header(fp, 1, 1, -2, -2);
			}	
		}

	}
	else if((lastpage!=-1)&&(lastrecord!=-1)){ //������ ���ڵ� ���� ��
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
// �ֹι�ȣ�� ��ġ�ϴ� ���ڵ带 ã�Ƽ� �����ϴ� ����� �����Ѵ�.
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
	FILE *fp;  // ���ڵ� ������ ���� ������
	char inputsn[14];
	char check[PAGE_SIZE];
	char buffer[100] = {0,};
	int num;
	int num0=1;
	int num1=0;
	int num2=-1;
	struct stat buf;
	Person p;

	if(access(argv[2], F_OK) < 0){ //���� �������� ���� ��
		fp = fopen(argv[2], "w+"); //person.dat
	}
	else if(access(argv[2], F_OK) == 0){ //���� �����
		fp = fopen(argv[2], "r+");
	}
	
	if(strcmp(argv[1],"i")==0){ //insert
		strcpy(p.sn,argv[3]);
		strcpy(p.name, argv[4]);
		strcpy(p.age, argv[5]);
		strcpy(p.addr, argv[6]);
		strcpy(p.phone, argv[7]);
		strcpy(p.email, argv[8]);
		//����ü�� ���� ����	
		if(stat("person.dat", &buf) < 0){
			fprintf(stderr, "stat error\n");
			exit(1);
		}
		if(buf.st_size == 0){ //������ ����ٸ�
			char headerpage[PAGE_SIZE];	
			memset(headerpage, (char)0xFF, PAGE_SIZE);
			
			memcpy(headerpage, &num0, sizeof(int));
			memcpy(headerpage+4, &num1, sizeof(int));
			memcpy(headerpage+4*2, &num2, sizeof(int));
			memcpy(headerpage+4*3, &num2, sizeof(int));

			writePage(fp, headerpage, 0); //header page ����
			
		}
		insert(fp, &p);
	}
	else if(strcmp(argv[1],"d")==0){ //delete
		memset(inputsn, 0, sizeof(inputsn));
		strcpy(inputsn, argv[3]); //�����ϰ��� ���ڵ��� �ֹι�ȣ
		delete(fp, inputsn);
	}
	fclose(fp);
	return 1;
}

void modify_header(FILE *fp, int n_totalpage, int n_totalrecord, int n_lastpage, int n_lastrecord){ //header page �� �������ִ� �Լ�
	char header[PAGE_SIZE];
	char n_header[PAGE_SIZE];
	int totalpage, totalrecord, lastpage, lastrecord;

	memset(n_header, (char)0xFF, PAGE_SIZE);

	readPage(fp, header, 0);
	memcpy(&totalpage, header, sizeof(int));
	memcpy(&totalrecord, header+4, sizeof(int));
	memcpy(&lastpage, header+4*2, sizeof(int));
	memcpy(&lastrecord, header+4*3, sizeof(int));
	
	if(n_totalpage != 0){ //page ����
		totalpage = totalpage + n_totalpage;
	}
	if(n_totalrecord != 0){ //record ����
		totalrecord = totalrecord + n_totalrecord;
	}
	if((n_lastpage != -2)&&(n_lastrecord != -2)){ //�ֱ� ������ ���ڵ� �ٲ�
		lastpage = n_lastpage;
		lastrecord = n_lastrecord;
	}
	//������ ������ ����
	memcpy(n_header, &totalpage, sizeof(int));
	memcpy(n_header+4, &totalrecord, sizeof(int));
	memcpy(n_header+4*2, &lastpage, sizeof(int));
	memcpy(n_header+4*3, &lastrecord, sizeof(int));
	
	writePage(fp, n_header, 0);
}

