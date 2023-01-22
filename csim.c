#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define bool short
#define true 1
#define false 0 

int hits=0,miss=0,eviction=0;
int tag_len=-1,group_idx_len=-1,block_bias_len=-1;
int E,S;
bool need_trace_info=false;

char trace_file_name[100];

typedef struct
{
    int time_stamp;
    bool valid;
    int tag;
}cache_line,*cache_group,**cache;

cache _cache=NULL;

//global error handle
void program_error(char *msg)
{
    fprintf(stderr,"program error:%s\n",msg);
    exit(0);
}

void printUsage()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Print this help message.\n"
            "  -v         Optional verbose flag.\n"
            "  -s <num>   Number of set index bits.\n"
            "  -E <num>   Number of lines per set.\n"
            "  -b <num>   Number of block offset bits.\n"
            "  -t <file>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void argHandle(int argc,char *argv[])
{
    memset(trace_file_name,0,sizeof(trace_file_name));
    int opt=0;
    while((opt=getopt(argc,argv,"hvs:E:b:t:"))!=-1)
    {
        switch(opt)
        {
            case 'h':
                printUsage();
                break;
            case 'v':
                need_trace_info=1;
                break;
            case 's':
                group_idx_len=atoi(optarg);
                break;
            case 'E':
                E=atoi(optarg);
                break;
            case 'b':
                block_bias_len=atoi(optarg);
                break;
            case 't':
                strcpy(trace_file_name,optarg);
                break;
            default:
                printUsage();
                break;
        }
        
    }
    if(block_bias_len<0||group_idx_len<0||E<0||strlen(trace_file_name)==0)
        program_error("wrong argment");
    S=1<<group_idx_len;
    tag_len=64-block_bias_len-group_idx_len;
}

void initCache()
{
    _cache=(cache)malloc(sizeof(cache_group)*S);
    for(int i=0;i<S;i++)
    {
        _cache[i]=(cache_group)malloc(sizeof(cache_line)*E);
        for(int j=0;j<E;j++)
        {
            _cache[i][j].time_stamp=0;
            _cache[i][j].valid=false;
            _cache[i][j].tag=0;
        }
    }
}

void readTraceFile()
{
    FILE *fp=fopen(trace_file_name,"r");
    if(fp==NULL)
        program_error("file open error");
    int address,size;
    char operation;
    while(fscanf(fp," %c %x,%d",&operation,&address,&size)!=EOF)
    {
        printf("%c %x %d\n",operation,address,size);
        
    }
        

}

int main(int argc,char *argv[])
{
    argHandle(argc,argv);
    initCache();
    readTraceFile();
    return 0;
}
