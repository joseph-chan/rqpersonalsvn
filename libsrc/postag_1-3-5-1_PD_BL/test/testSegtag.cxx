
/*
 * 采用分词最新接口配合对标注模块进行测试。
 * isegment.h中的分词接口和标注模块提供的接口配合使用最方便。
 * David Dai
 * 2007.11.6
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "isegment.h"
#include "ipostag.h"


#define MAX_TEXT_LENGTH 4096
#define MAX_TERM_COUNT 4096

void print_result(token_t tokens[], int count);

extern const char* get_pos_str( unsigned int nPOS ); // from libpostag.a


int main(int argc,char** argv)
{
  token_t tokens[MAX_TERM_COUNT];
  token_t subtokens[MAX_TERM_COUNT];
  char line[MAX_TEXT_LENGTH] = {0};  

  if(argc < 3)
  {
    fprintf(stderr,"usage: %s  segDictPath tagDictPath \n",argv[0]);
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

  // start ...
  handle_t handle = seg_open(dictPtr,MAX_TERM_COUNT);
  if( !handle )
  {
    fprintf(stderr,"error: can't create handle for segment.\n");
    exit(-1);
  }
  
  // start timing
  clock_t start = clock();    
  
  while(fgets(line,sizeof(line),stdin))
  {    
    int len=strlen(line);
    while((line[len-1]=='\r') ||(line[len-1]=='\n'))
    {
      line[--len]=0;
    }

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
    // output mixed terms        
    print_result(tokens,count);     

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
    print_result(subtokens,cnt);  

  }    

  // end timing
  double time = (double)( clock() - start ) / CLOCKS_PER_SEC;
  fprintf(stderr,"The elapsed time: %f seconds.\n", time);

  // end ...
  seg_close(handle);
  tag_close();

  // close dict
  seg_dict_close(dictPtr);

  return 0;
}

void print_result(token_t tokens[], int count)
{  
  for(int i=0; i<count; i++)
  {    
    const char* stag = get_pos_str(tokens[i].type);
    if( stag )
      printf("%s/%s",tokens[i].buffer,stag);    
    else
      printf("%s",tokens[i].buffer);  
    if( tokens[i].weight & OUT_OF_TAG )
      printf("(OOT)");
    printf("\t");
  }  
  printf("\n");
}

