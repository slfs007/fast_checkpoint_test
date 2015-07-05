#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define SIZE_WORD	1024 * 1024 * 10

int AS0[SIZE_WORD];
int AS1[SIZE_WORD];
unsigned char MW[SIZE_WORD];
unsigned char MR[SIZE_WORD];

void PrepareForNextCheckpoint( void)
{
	int i;

	for (i = 0;i < SIZE_WORD; i++)
	{
		MW[i] = !MR[i];
	}
}
void InitData( void)
{
	memset(AS0, 0,sizeof( int ) * SIZE_WORD);
	memset(AS1, 0,sizeof( int ) * SIZE_WORD);
	memset(MR, 0,SIZE_WORD);
	memset(MW, 1,SIZE_WORD);
}
int HandleRead(int index)
{
	return MR[index] == 1? AS1[index]: AS0[index];
}
int HandleWrite(int index,int newValue)
{
	if ( 1 == MW[index])
	{
		AS1[index] = newValue;
	}else
	{
		AS0[index] = newValue;
	}
	MR[index] = MW[index];
}
void WriteToStableStorage()
{
	int i = 0;
	int j;
	char file_name[30];
	FILE *cp_file;

	for (;;)
	{
		PrepareForNextCheckpoint();
		sprintf(file_name,"./checkpoint/%d",i);
		if (NULL == (cp_file = fopen(file_name,"w")))
		{
			printf("cp file open error! %d\n",i);
			exit(1);
		}
		for (j = 0; j < SIZE_WORD; j++)
		{
			if ( 0 == MW[j])
			{
				fwrite(( void *) &(AS1[j]),sizeof(int),1,cp_file);
			}else
			{
				fwrite(( void *) &(AS0[j]),sizeof(int),1,cp_file);
				
			}
		}	
		fclose(cp_file);
		i++;	
		if ( 10 == i)
		{
			exit (0);
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
	log = fopen("./log.txt", "w");

	for (;;)
	{
		wr = rand() % 2;
		index_rand = rand()%SIZE_WORD;	
		value_rand = rand();
		HandleWrite(index_rand,value_rand);
		fprintf(log,"%d\n", clock());
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
