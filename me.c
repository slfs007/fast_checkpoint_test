#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define SIZE_WORD	1024 * 1024 * 10

int AS0[SIZE_WORD];
int AS1[SIZE_WORD];
unsigned char WR[SIZE_WORD];
unsigned char cur;
unsigned char lock;// 0 1 2

void PrepareForNextCheckpoint( void)
{
	int i;

	if ( 1 == cur)
	{
		for ( i = 0; i < SIZE_WORD; i ++)
		{
			AS0[ i] = AS1[i];
			WR[ i] = 0;
		}
	}else
	{
		for ( i = 0; i < SIZE_WORD; i ++)
		{
			AS1[i] = AS0[i];
			WR[ i] = 1;
		}
	}
}
void InitData( void)
{
	memset(AS0, 0,sizeof( int ) * SIZE_WORD);
	memset(AS1, 0,sizeof( int ) * SIZE_WORD);
	memset(WR, 0,SIZE_WORD);
	cur = 0;
	lock = 2;	
}
int HandleRead(int index)
{
	return 1 == WR[index]? AS1[index]: AS0[index];
}
int HandleWrite(int index,int newValue)
{
	if ( 0 == cur)
	{
		AS0[index] = newValue;
	}else
	{
		AS1[index] = newValue;
	}
	WR[index] = cur;
}
void WriteToStableStorage()
{
	int i = 0;
	int j;
	char file_name[30];
	FILE *cp_file;

	for (;;)
	{
		lock = !cur;
		PrepareForNextCheckpoint();
		sprintf(file_name,"./checkpoint/%d",i);
		if (NULL == (cp_file = fopen(file_name,"w")))
		{
			printf("cp file open error! %d\n",i);
			exit(1);
		}
		
		if (0 == cur)
		{
			fwrite(AS1,sizeof(int),SIZE_WORD,cp_file);
		}else
		{
			fwrite(AS0,sizeof(int),SIZE_WORD,cp_file);
		}
		cur = lock;
		lock = 2;
		fclose(cp_file);
		i++;			
		if ( 10 == i)
		{
			exit(0);
		}	
	}
}
void fc_app( void * arg)
{
	FILE *log;
	int value_rand;
	int index_rand;
	int wr;
	clock_t rawtime;
	char time_str[32];
	log = fopen("./me_log.txt", "w");
	
	for (; ;)
	{
		wr = rand() % 2;
		index_rand = rand()%SIZE_WORD;	
		value_rand = rand();
		HandleWrite(index_rand,value_rand);
		fprintf(log,"%d\n",clock());
		usleep(100);		
	}
}
int main( int argc, char *argv[])
{
	pthread_t fc_app_tid;
	InitData();

	if ( 0 != pthread_create( &fc_app_tid, NULL, fc_app,NULL))
	{
		printf("pthread create error!\n");
		exit(1);
	}
	
	WriteToStableStorage();
	exit (0);
}
