
/**
 * @brief 测试主干分层分析模块，调用了wordner模块作为预处理。
 * @author David Dai
 * @date 2009/5
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "iwordrank.h"
#include "isegment.h"
#include "ipostag.h"
#include "itrunk.h"
#include "iwordner.h"

#define MAX_TEXT_LENGTH 4096
#define MAX_TERM_COUNT 4096
#define DEFAULT_DMPACK_SIZE  1024

void print_result(token_t tokens[], int count);
extern const char* get_pos_str( unsigned int nPOS ); // from libpostag.a

int main(int argc,char** argv)
{
  token_t tokens[MAX_TERM_COUNT];
  //token_t subtokens[MAX_TERM_COUNT];
  trunk_t trunks[MAX_TERM_COUNT];
  char line[MAX_TEXT_LENGTH] = {0};
  tag_t tags[256];

  tag_dict_t* gTagDict = NULL;
  trunk_dict_t* gTrkDict = NULL;
  rank_dict_t* gRankDict = NULL;
  ner_dict_t *gNerDict = NULL;
  dm_pack_t* pdmpack = NULL;

  if(argc < 6)
  {
    fprintf(stderr,"usage: %s  segDictPath rankDictPath tagDictPath trkDictPath nerDictPath \n",argv[0]);
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
  if((gNerDict = ner_create(argv[5], NER_OUT_NE|NER_OUT_PER|NER_OUT_ORG|NER_OUT_OOV)) == NULL)
  {
    fprintf( stderr, "error: wordner open failed.\n" );
    exit(-1);
  }
  if((pdmpack = dm_pack_create( DEFAULT_DMPACK_SIZE )) == NULL)
  {
    fprintf( stderr, "error: create dm_pack failed.\n" );
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
  
  int queryCount = 0;
  float rankCounts[3] = {0};
  while(fgets(line,sizeof(line),stdin))
  {    
    int len=strlen(line);
    while((line[len-1]=='\r') ||(line[len-1]=='\n'))
    {
      line[--len]=0;
    }

    if( !len )
      continue;
    
    // segment ...
    int count = seg_segment(handle,line,len,tokens,MAX_TERM_COUNT);
    if( !count )
    {
      fprintf(stderr, "error: when segmenting, jump over:\n%s\n",line);
      continue;
    } 

    // postag ...
    int ret = tag_postag(gTagDict,tokens,count);
    if( !ret )
    {
      fprintf(stderr, "error: when pos tagging, jump over:\n%s\n",line);
      continue;
    }      
    // output mixed terms        
    print_result(tokens,count);  

    // word ner
    int tagcnt = -1;
    if((tagcnt = ner_tag( gNerDict, pdmpack, tokens, count, tags, 256 )) < 0)
    {
      fprintf(stderr, "error: ner %s failed.\n", line);
      continue;
    }

    // trunk parse ...
    ret = trk_parse(gTrkDict,gRankDict,NULL,tokens,count,trunks,count);
    if( !ret )
    {
      fprintf(stderr, "error: when trunk parsing, jump over:\n%s\n",line);
      continue;
    }   

    
    printf("%s\t",line);
    for(int i=0; i<ret; i++)
      printf("%s(%d)  ",tokens[i].buffer,trunks[i].rank);

    // 利用专名识别调整分层结果
    bool bNer = false;
    for(int i=0; i<tagcnt; i++)
    {
      if( tags[i].length < 2 )
        continue;
      for(unsigned j=0; j<tags[i].length; j++)
        trunks[tags[i].offset+j].rank = TERM_RANK_TRUNK;
      bNer = true;
    }

    if( bNer ) // for debug
    {
      printf("\t");

      for(int i=0; i<tagcnt; i++)
      {
        if( tags[i].length < 2 )
          continue;
        for(unsigned j=0; j<tags[i].length; j++)
        {
          printf("%s",tokens[tags[i].offset+j].buffer);
        }       
        printf("  ");
      } 
      printf("\t");

      for(int i=0; i<ret; i++)
        printf("%s(%d)  ",tokens[i].buffer,trunks[i].rank);        
    }
    printf("\n");  

    // for debug
    queryCount++;
    for(int i=0; i<ret; i++)
      rankCounts[trunks[i].rank]++;  
  }    

  // for debug
  fprintf(stderr,"%f\t%f\t%f\n",rankCounts[0]/queryCount,rankCounts[1]/queryCount,rankCounts[2]/queryCount);

  // end timing
  double time = (double)( clock() - start ) / CLOCKS_PER_SEC;
  fprintf(stderr,"The elapsed time: %f seconds.\n", time);

  // end ...
  seg_close(handle); 
  
  // close dict
  wdr_destroy(gRankDict);
  tag_destroy(gTagDict);
  trk_destroy(gTrkDict);
  ner_destroy( gNerDict );
  seg_dict_close(dictPtr);  

  return 0;
}

void print_result(token_t tokens[], int count)
{ 
  for(int i=0; i<count; i++)
  {    
    const char* stag = get_pos_str(tokens[i].type);
    if( stag )
      fprintf(stderr,"%s/%s\t",tokens[i].buffer,stag);    
    else
      fprintf(stderr,"%s\t",tokens[i].buffer);    
  }  
  fprintf(stderr,"\n");
}

