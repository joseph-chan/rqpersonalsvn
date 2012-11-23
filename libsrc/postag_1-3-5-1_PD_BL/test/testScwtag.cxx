/*
 * 测试程序：使用最古老的分词接口，也是PS使用最广泛的接口。
 * 本测试程序中使用了新加入的获取token的接口scw_get_token_1()，
 * 通过此接口获取分词结果然后直接传给postag模块进行标注。
 * 本测试程序支持多线程模式下执行。
 * David Dai
 * 2007.11.6
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>  
#include <unistd.h> 

#include "scwdef.h"
#include "isegment.h"
#include "ipostag.h"

#define MAX_TEXT_LENGTH 4096
#define MAX_TERM_COUNT 4096

void* run(void* data); // run thread instance.
void print_result(token_t tokens[], int count);
extern const char* get_pos_str( unsigned int nPOS ); // from libpostag.a
extern int scw_get_token_1( scw_out_t* pout, int type, token_t result[], unsigned max ); // from libsegment.a

// global vars
scw_worddict_t * pwdict = NULL;
pthread_mutex_t mutex; // for lock


int main(int argc,char** argv)
{
  char filename[300] = {0};
      
  if( argc < 4 )
  {
    fprintf(stderr, "usage: %s  segDictPath tagDictPath threadCount\n", argv[0]);
    exit(-1);
  }

  strcpy(filename,argv[1]);
  strcat(filename,"/");
  if( (pwdict=scw_load_worddict(filename)) == NULL )
  {
    fprintf(stderr,"error: loading worddict failed: %s\n",argv[1]);
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
    pthread_create(&pids[i], 0, pf, NULL);  
    fprintf(stderr,"start thread: [%ld]\n",pids[i]);  
  }
  for(int i=0; i<N; i++)
  {
    pthread_join(pids[i], 0);
  }
  
  tag_close();  
  scw_destroy_worddict(pwdict);

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
  scw_out_t *pout = NULL;

  u_int scw_out_flag = SCW_OUT_ALL | SCW_OUT_PROP;
  if( (pout=scw_create_out(10000, scw_out_flag)) == NULL )
  {
    fprintf(stderr,"error: initializing the output buffer error.\n");
    return 0;
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
    if( scw_segment_words(pwdict,pout,line,len) < 0 )
    {
      fprintf(stderr, "error: when segmenting, jump over: %s\n",line);
      continue;
    }

    // get mixed term tokens    
    int count = scw_get_token_1(pout,SCW_OUT_WPCOMP,tokens,MAX_TERM_COUNT);
    // postag
    int ret = tag_postag(tokens,count);
    if( !ret )
    {
      fprintf(stderr, "error: when pos tagging, jump over: %s\n",line);
      continue;
    }     
    // output  mixed terms
    //printf("mixed term result:\t");
    pthread_mutex_lock(&mutex);
    print_result(tokens,count);
    pthread_mutex_unlock(&mutex);

    // get basic term tokens    
    count = scw_get_token_1(pout,SCW_OUT_BASIC,tokens,MAX_TERM_COUNT);
    // postag
    ret = tag_postag(tokens,count);
    if( !ret )
    {
      fprintf(stderr, "error: when pos tagging, jump over:\n%s\n",line);
      continue;
    }     
    // output basic terms      
    //printf("basic term result:\t");
    pthread_mutex_lock(&mutex);
    print_result(tokens,count); 
    pthread_mutex_unlock(&mutex);
  }

  scw_destroy_out(pout);

  return 0;
}

