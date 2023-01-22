#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define bool short
#define true 1
#define false 0 
#define uint unsigned int

int hits=0,miss=0,evictions=0;
int tag_len=-1,group_idx_len=-1,block_bias_len=-1;
int E,S,now=0;
bool need_trace_info=false;

char trace_file_name[100];

typedef struct
{
    int time_stamp;
    bool valid;
    __uint64_t tag;
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

void freeCache()
{
    for(int i=0;i<S;i++)
        free(_cache[i]);
    free(_cache);
}

void checkCache(__uint64_t address,int size)
{
    //get group
    __uint64_t group_idx_mask=((1<<(group_idx_len))-1)<<block_bias_len;
    printf(",,\n");
    uint group_idx=((address&group_idx_mask)>>block_bias_len);
    uint empty_line,last_use_line,hit_line;
    bool is_hit=false,have_empty=false;
    int last_use_time=(1<<15)-1;
    __uint64_t tag=address>>(group_idx_len+block_bias_len);
    printf("??\n");
    for(int i=0;i<E;i++)
    {
        if(_cache[group_idx][i].valid&&_cache[group_idx][i].tag==tag)
        {
            is_hit=true;
            hit_line=i;
        }
        if(!_cache[group_idx][i].valid)
        {
            have_empty=true;
            empty_line=i;
        }
        else{
            if(_cache[group_idx][i].time_stamp<last_use_time)
            {
                last_use_time=_cache[group_idx][i].time_stamp;
                last_use_line=i;
            }
        }
    }
    if(is_hit)
    {
        if(need_trace_info)
            printf(" hit");
        _cache[group_idx][hit_line].time_stamp=now;
        hits++;
        return;
    }
    if(need_trace_info)
    {
        printf(" miss");
    }
    miss++;
    if(have_empty)
    {
        _cache[group_idx][empty_line].time_stamp=now;
        _cache[group_idx][empty_line].tag=tag;
        _cache[group_idx][empty_line].valid=true;
        return;
    }
    _cache[group_idx][last_use_line].time_stamp=now;
    _cache[group_idx][last_use_line].tag=tag;
    evictions++;
    if(need_trace_info)
        printf(" eviction");
    return;
}

void readTraceFile()
{
    FILE *fp=fopen(trace_file_name,"r");
    if(fp==NULL)
        program_error("file open error");
    int size;
    __uint64_t address;
    char operation;
    while(fscanf(fp," %c %lx,%d",&operation,&address,&size)!=EOF)
    {
        if(need_trace_info)
            printf("%c %lx,%d",operation,address,size);
        switch(operation)
        {
            case 'M'://need update Cache twice
                checkCache(address,size);
            case 'L':
            case 'S':
                checkCache(address,size);
        }
        now++;
        if(need_trace_info)
            printf("\n");
    }
    fclose(fp);
    freeCache();
}

int main(int argc,char *argv[])
{
    argHandle(argc,argv);
    initCache();
    readTraceFile();
    printSummary(hits,miss,evictions);
    return 0;
}
