/**
 * @brief 采用分词最新接口配合对主干分析模块进行测试(多线程版本)。
 * @author David Dai
 * @date 2009/2/25
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <pthread.h>  
#include <unistd.h> 

#include "iwordrank.h"
#include "isegment.h"
#include "ipostag.h"
#include "itrunk.h"

#define MAX_TEXT_LENGTH 4096
#define MAX_TERM_COUNT 4096

void* run(void* data); // run thread instance.
void print_result(token_t tokens[], int count);
extern const char* get_pos_str( unsigned int nPOS ); // from libpostag.a

// global vars
pthread_mutex_t mutex; // for lock

tag_dict_t* gTagDict = NULL;
trunk_dict_t* gTrkDict = NULL;
rank_dict_t* gRankDict = NULL;

int main(int argc,char** argv)
{      
  if( argc < 6 )
  {
    fprintf(stderr, "usage: %s  segDictPath rankDictPath tagDictPath trkDictPath threadCount\n", argv[0]);
    exit(-1);
  }

  // open dict
  dict_ptr_t dictPtr = seg_dict_open(argv[1]);
  if( !dictPtr )
  {
    fprintf(stderr,"error: can't load the segment dictionary %s.\n",argv[1]);
    exit(-1);
  }
  gRankDict = wdr_create(argv[2]);
  if( !gRankDict )
  {
    fprintf(stderr,"error: can't load the word ranking dictionary %s.\n",argv[2]);
    exit(-1);
  }
  gTagDict = tag_create(argv[3]);
  if( !gTagDict )
  {
    fprintf(stderr,"error: can't load the pos tagging dictionary %s.\n",argv[3]);
    exit(-1);
  }
  gTrkDict = trk_create(argv[4]);
  if( !gTrkDict )
  {
    fprintf(stderr,"error: can't load the trunk parsing dictionary %s.\n",argv[4]);
    exit(-1);
  }

  // start up threads  
  pthread_t pids[256] = {0};   
  void* (*pf)( void* ) = run;  
  int N = atoi(argv[5]);
  if( !N ) N = 1;

  pthread_mutex_init(&mutex,NULL);
  for(int i=0; i<N; i++)
  {
    pthread_create(&pids[i], 0, pf, &dictPtr);  
    fprintf(stderr,"start thread: [%ld]\n",pids[i]);  
  }
  for(int i=0; i<N; i++)
  {
    pthread_join(pids[i], 0);
  }
  
  wdr_destroy(gRankDict);
  tag_destroy(gTagDict);
  trk_destroy(gTrkDict); 
  seg_dict_close(dictPtr);

  return 0;
}

void print_result(token_t tokens[], int count)
{  
  for(int i=0; i<count; i++)
  {    
    const char* stag = get_pos_str(tokens[i].type);
    if( stag )
      printf("%s/%s\t",tokens[i].buffer,stag);    
    else
      printf("%s\t",tokens[i].buffer);    
  }  
  printf("\n");
}

void* run(void* data)
{  
  char line[MAX_TEXT_LENGTH] = {0};
  token_t tokens[MAX_TERM_COUNT];
  trunk_t trunks[MAX_TERM_COUNT];
 
  // start ...
  dict_ptr_t* p = (dict_ptr_t*)data;
  handle_t handle = seg_open(*p,MAX_TERM_COUNT);
  if( !handle )
  {
    fprintf(stderr,"error: can't create handle for segment.\n");
    exit(-1);
  }  
  
  while( 1 )
  {  
    pthread_mutex_lock(&mutex);
    char* tmp = fgets(line,sizeof(line),stdin);    
    pthread_mutex_unlock(&mutex);
    
    if( !tmp )
      break;
    int len = strlen(line);
    if( !len )
      break;

    while( (line[len-1]=='\r') ||(line[len-1]=='\n') )
      line[--len]=0;

    if( !len )
      continue;

    // segment ...
    int count = seg_segment(handle,line,len,tokens,MAX_TERM_COUNT);
    if( !count )
    {
      fprintf(stderr, "error: when segmenting, jump over:\n%s\n",line);
      continue;
    }     

    // trunk parse ...
    int ret = 0;
    //trk_parse(gTrkDict,gRankDict,gTagDict,tokens,count,trunks,count);
    if( !ret )
    {
      fprintf(stderr, "error: when trunk parsing, jump over:\n%s\n",line);
      continue;
    }
   
    // output term class           
    pthread_mutex_lock(&mutex);
    printf("%s\t",line);
    for(int i=0; i<ret; i++)
      printf("%s(%d,%d)\t",tokens[i].buffer,trunks[i].rank,trunks[i].type);
    printf("\n");
    pthread_mutex_unlock(&mutex);
  }

  // end ...
	seg_close(handle);

  return 0;
}

