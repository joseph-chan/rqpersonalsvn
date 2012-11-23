
/**
 * @brief 测试主干分层分析模块。
 * @author David Dai
 * @date 2007/11/6
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <ul_dict.h>
#include <ul_sign.h>

#include "iwordrank.h"
#include "isegment.h"
#include "ipostag.h"
#include "itrunk.h"

#define SPLIT_DICT_NAME "splitphrasedict"

#define MAX_TEXT_LENGTH 4096
#define MAX_TERM_COUNT 4096

void print_result(token_t tokens[], int count);
extern const char* get_pos_str( unsigned int nPOS ); // from libpostag.a

Sdict_build* test_load_word_tag_dict(const char *conf_path, const char *filename)
{
    if(!conf_path || !filename)
    {
        printf("%s:Wrong Parameter", __func__);
        return NULL;
    }

    char line_buf[512], term[256];
    int ntag = 0, nval = 0;
    Sdict_snode snode;
    char full_file_name[512];
    Sdict_build * word_tag_dict = NULL;
    snprintf (full_file_name, sizeof (full_file_name), "%s/%s", conf_path, filename);
    FILE * pofile = fopen (full_file_name, "r");
    if(NULL == pofile)
    {
        printf("File [%s] cannot be opened for reading", full_file_name);
        return NULL;
    }

    word_tag_dict = db_creat (1000, 0);
    if(NULL == word_tag_dict)
    {
        fclose(pofile);
        pofile = NULL;
        printf("db_creat (1000, 0) for word_tag_dict fail");
        return NULL;
    }

    int line_num = 0;
    while(fgets (line_buf, sizeof (line_buf), pofile))
    {
        int nscanf_num = sscanf (line_buf, "%s\t%d\t%d", term, &ntag, &nval);
        if(3 != nscanf_num)
        {
            printf("sscanf error, nscanf_num=%d", nscanf_num);
            continue;
        }
        char term_sign[256];
        snprintf(term_sign, sizeof(term_sign), "%d_%s", ntag, term);
        creat_sign_f64 (term_sign, strlen (term_sign), &snode.sign1, &snode.sign2);
        snode.other = ntag;
        snode.code = nval;
        if(db_op1 (word_tag_dict, &snode, ADD) > 0)
        {
            printf("123456879 %s\t%d\t%d\n", term_sign, ntag, nval);
        }
        else
        {
            printf("%s fail to db_op1 add\n", term_sign);
        }
        ++line_num;
    }
    printf("word_tag_dict read line_num=%d\n", line_num);

    fclose(pofile);

    return word_tag_dict;
}

int main(int argc,char** argv)
{
  token_t tokens[MAX_TERM_COUNT];
  token_t subtokens[MAX_TERM_COUNT];
  trunk_t trunks[MAX_TERM_COUNT];
  char line[MAX_TEXT_LENGTH] = {0};

  tag_dict_t* gTagDict = NULL;
  trunk_dict_t* gTrkDict = NULL;
  rank_dict_t* gRankDict = NULL;

  if(argc < 7)
  {
    fprintf(stderr,"usage: %s  segDictPath rankDictPath tagDictPath trkDictPath splitDictPath word_tag_path\n",argv[0]);
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
  Sdict_search* phrasedict = ds_load(argv[5], SPLIT_DICT_NAME);
	if (phrasedict == NULL)
	{
		fprintf(stderr, "error: load phrasedict at %s/%s fail", argv[5], SPLIT_DICT_NAME);
		return -1;
	}

  // start ...
  handle_t handle = seg_open(dictPtr,MAX_TERM_COUNT);
  if( !handle )
  {
    fprintf(stderr,"error: can't create handle for segment.\n");
    exit(-1);
  }
 
  char word_tag_path[256];
  sprintf(word_tag_path, "%s", argv[6]);
  Sdict_build* pword_dict = test_load_word_tag_dict(word_tag_path, "word_tag.txt");
  if(NULL == pword_dict)
  {
      printf("test_load_word_tag_dict %s/%s error\n", word_tag_path, "word_tag.txt");
      return -1;
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

#ifdef _DEBUG
    // output mixed terms        
    print_result(tokens,count);
#endif

    // split ...
    int offset = 0;
    for(int i=0; i<count; i++)
    {
      int cnt = seg_split_tokenize( phrasedict, handle, tokens[i], subtokens+offset, MAX_TERM_COUNT );
      offset += cnt;
    }
    count = offset;

    // trunk parse ...
    ret = trk_parse(gTrkDict,gRankDict,NULL, pword_dict, subtokens,count,trunks,count);
    if( !ret )
    {
      fprintf(stderr, "error: when trunk parsing, jump over:\n%s\n",line);
      continue;
    }   
    
    // for debug
    int trkcnt = 0;
    queryCount++;
    for(int i=0; i<ret; i++)
    {
      rankCounts[trunks[i].rank]++;
      if( trunks[i].rank == TERM_RANK_TRUNK )
        trkcnt++;
    }    
    
    printf("%s\t",line);
    for(int i=0; i<ret; i++)
      //printf("%s(%d,%d)\t",subtokens[i].buffer,trunks[i].rank,trunks[i].type);
      printf("%s(%d,%d,%d)(%f)\t",subtokens[i].buffer,trunks[i].rank,trunks[i].type, trunks[i].reduce_type, trunks[i].weight);      
    printf("\n");    
    
  }    

#ifdef _DEBUG
  fprintf(stderr,"%f\t%f\t%f\n",rankCounts[0]/queryCount,rankCounts[1]/queryCount,rankCounts[2]/queryCount);
#endif

  // end timing
  double time = (double)( clock() - start ) / CLOCKS_PER_SEC;
  fprintf(stderr,"The elapsed time: %f seconds.\n", time);

  // end ...
  seg_close(handle); 
  
  // close dict
  wdr_destroy(gRankDict);
  tag_destroy(gTagDict);
  trk_destroy(gTrkDict);
  seg_dict_close(dictPtr);
  ds_del(phrasedict);

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
