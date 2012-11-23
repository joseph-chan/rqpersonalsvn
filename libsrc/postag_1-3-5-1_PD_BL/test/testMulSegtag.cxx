/*
 * 采用分词最新接口配合对标注模块进行测试。(多线程版本)
 * isegment.h中的分词接口和标注模块提供的接口配合使用最方便。
 * David Dai
 * 2007.11.28
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>  
#include <unistd.h> 

#include "isegment.h"
#include "ipostag.h"

#define MAX_TEXT_LENGTH 4096
#define MAX_TERM_COUNT 4096

void* run(void* data); // run thread instance.
void print_result(token_t tokens[], int count);
extern const char* get_pos_str( unsigned int nPOS ); // from libpostag.a

// global vars
pthread_mutex_t mutex; // for lock


int main(int argc,char** argv)
{      
  if( argc < 4 )
  {
    fprintf(stderr, "usage: %s  segDictPath tagDictPath threadCount\n", argv[0]);
    exit(-1);
  }

  // open dict
  dict_ptr_t dictPtr = seg_dict_open(argv[1]);
  if( !dictPtr )
  {
    fprintf(stderr,"error: can't load the segment dictionary %s.\n",argv[1]);
    exit(-1);
  }
  if( tag_open( argv[2]) )
  {
    fprintf(stderr,"error: can't load the postag dictionary %s.\n",argv[2]);
    exit(-1);
  }  

  // start up threads  
  pthread_t pids[256] = {0};   
  void* (*pf)( void* ) = run;  
  int N = atoi(argv[3]);
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
  
  tag_close();  
  seg_dict_close(dictPtr);

  return 0;
}

void print_result(token_t tokens[], int count)
{  
  for(int i=0; i<count; i++)
  {    
    const char* stag = get_pos_str(tokens[i].type);
    if( stag )
      printf("%s/%s  ",tokens[i].buffer,stag);    
    else
      printf("%s  ",tokens[i].buffer);    
  }  
  printf("\n");
}

void* run(void* data)
{  
  char line[MAX_TEXT_LENGTH] = {0};
  token_t tokens[MAX_TERM_COUNT];
  token_t subtokens[MAX_TERM_COUNT];
 
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

    // segment
    int count = seg_segment(handle,line,len,tokens,MAX_TERM_COUNT);
    if( !count )
    {
      fprintf(stderr, "error: when segmenting, jump over:\n%s\n",line);
      continue;
    } 
    // postag
    int ret = tag_postag(tokens,count);
    if( !ret )
    {
      fprintf(stderr, "error: when pos tagging, jump over:\n%s\n",line);
      continue;
    }      
    // output  mixed terms    
    pthread_mutex_lock(&mutex);
    print_result(tokens,count);
    pthread_mutex_unlock(&mutex);

    // get basic terms
    int cnt = 0;
    for(int i=0; i<count; i++)
		{			
			cnt += seg_tokenize(handle,tokens[i],TOK_BASIC,subtokens+cnt,32);	
		}
    // postag
    ret = tag_postag(subtokens,cnt);
    if( !ret )
    {
      fprintf(stderr, "error: when pos tagging, jump over:\n%s\n",line);
      continue;
    }      
    // output basic terms          
    pthread_mutex_lock(&mutex);
    print_result(subtokens,cnt); 
    pthread_mutex_unlock(&mutex);
  }

  // end ...
	seg_close(handle);

  return 0;
}

