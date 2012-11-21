/*
 * 测试程序：使用最古老的分词接口，也是PS使用最广泛的接口。
 * 本测试程序中使用了新加入的获取token的接口scw_get_token_1()，
 * 通过此接口获取的分词结果可以直接传给postag模块进行标注。
 * David Dai
 * 2007.10.10
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "scwdef.h"
#include "isegment.h"

static void scw_dump_out(scw_out_t* pout, char * line);
static void scw_dump_out_old(scw_out_t* pout, char* line, int wdtype);

extern void write_prop_to_str(scw_property_t& property, char* buffer,int wdtype);
extern int scw_get_token_1( scw_out_t* pout, int type, token_t result[], unsigned max );

int main(int argc,char** argv)
{
  scw_worddict_t * pwdict = NULL;
  scw_out_t *pout = NULL;
  char line[1024000];
  u_int scw_out_flag;
  int flag = 0;
  
  if( argc < 2 )
  {
    fprintf(stderr, "usage: %s  dictpath  [outtype]\n", argv[0]);
    exit(-1);
  }

  if((pwdict=scw_load_worddict(argv[1]))==NULL)
  {
    fprintf(stderr,"error: loading worddict failed: %s\n",argv[1]);
    return -1;
  }

  if( argc > 2 )
    flag = atoi(argv[2]);

  scw_out_flag = SCW_OUT_ALL | SCW_OUT_PROP;
  if((pout=scw_create_out(10000, scw_out_flag))==NULL)
  {
    fprintf(stderr,"error: initializing the output buffer error.\n");
    return -1;
  }

  // start timing
  clock_t start = clock();

  while(fgets(line,sizeof(line),stdin))
  {  
    int len = strlen(line);
    while( (line[len-1]=='\r') ||(line[len-1]=='\n') )
      line[--len]=0;

    if( !len )
      continue;

    scw_segment_words(pwdict,pout,line,len);

    if(flag == 0)
      scw_dump_out(pout,line);
    else
      scw_dump_out_old(pout,line,pwdict->m_wdtype);
  }

  // end timing
  double time = (double)( clock() - start ) / CLOCKS_PER_SEC;
  fprintf(stderr,"The elapsed time: %f seconds.\n", time);

  scw_destroy_out(pout);
  scw_destroy_worddict(pwdict);

  return 0;
}

// 新接口方式输出，用于和Postag系统配合使用
void scw_dump_out(scw_out_t* pout, char* line)
{
  int i;
  token_t tokens[1024];

  int count = scw_get_token_1(pout,SCW_OUT_WPCOMP,tokens,1024);
  // view word phrase compond result
  printf("============== Segment Result =============\n");
  for( i=0; i<count; i++)
  {
    //printf("%s  ", tokens[i].buffer);
    printf("%s(%d)  ", tokens[i].buffer, tokens[i].offset);
    //printf("%s[%d,%d]  ", tokens[i].buffer, tokens[i].prop.m_lprop,tokens[i].prop.m_hprop); // output property
  }
  printf("\n");

  count = scw_get_token_1(pout,SCW_OUT_BASIC,tokens,1024);
  // view basic word sep result
  printf("============== Basic Word Result =============\n");
  for( i=0; i<count; i++)
  {
    //printf("%s  ", tokens[i].buffer);
    printf("%s(%d)  ", tokens[i].buffer, tokens[i].offset);
  }
  printf("\n");  

  count = scw_get_token_1(pout,SCW_OUT_SUBPH,tokens,1024); //fprintf(stderr,"count of sub phrase = %d\n",count);
  // view sub phrase result
  printf("============== Sub Phrase Result =============\n");
  for( i=0; i<count; i++)
  {
    //printf("%s  ", tokens[i].buffer);
    printf("%s(%d)  ", tokens[i].buffer, tokens[i].offset);
  }
  printf("\n");

  count = scw_get_token_1(pout,SCW_OUT_NEWWORD,tokens,1024);
  // view new word result
  printf("============== NEW Word Result =============\n");
  for( i=0; i<count; i++)
  {
    //printf("%s  ", tokens[i].buffer);
    printf("%s(%d)  ", tokens[i].buffer, tokens[i].offset);
  }
  printf("\n");  
  count = scw_get_token_1(pout,SCW_OUT_HUMANNAME,tokens,1024);
  // view NER result
  printf("============== Human Name Result =============\n");
  for( i=0; i<count; i++)
  {
    //printf("%s  ", tokens[i].buffer);
    printf("%s(%d)  ", tokens[i].buffer, tokens[i].offset);
  }
  printf("\n");

  count = scw_get_token_1(pout,SCW_OUT_BOOKNAME,tokens,1024);
  // view the book name result
  printf("============== Book Name Result =============\n");
  for( i=0; i<count; i++)
  {
    //printf("%s  ", tokens[i].buffer);
    printf("%s(%d)  ", tokens[i].buffer, tokens[i].offset);
  }
  printf("\n");
}

// 老接口方式输出
void scw_dump_out_old(scw_out_t* pout, char* line, int wdtype)
{
  u_int i, pos, len;
  char word[256];
  char prop[256];

  // view basic word sep result
  printf("=================== Basic Word Sep Result =====================\n");
  for(i=0;i<pout->wsbtermcount; i++)
  {
      printf("[%d] ", pout->wsbtermoffsets[i]);
      pos = GET_TERM_POS(pout->wsbtermpos[i]);
      len = GET_TERM_LEN(pout->wsbtermpos[i]);
      strncpy(word, pout->wordsepbuf+pos, len);
      word[len]= 0;
      write_prop_to_str(pout->wsbtermprop[i], prop,wdtype);
      printf("%s(%s)  ", word,prop);
  }
  printf("\n\n");

  // view word phrase compond result
  printf("====================  Word Phrase Result  ======================\n");
  for( i = 0; i<pout->wpbtermcount; i++)
  {
      printf("[%d] ", pout->wpbtermoffsets[i]);
      pos = GET_TERM_POS(pout->wpbtermpos[i]);
      len = GET_TERM_LEN(pout->wpbtermpos[i]);
      strncpy(word, pout->wpcompbuf + pos, len);
      word[len] = 0;
      write_prop_to_str(pout->wpbtermprop[i], prop,wdtype);
      printf("%s(%s)  ", word,prop);
  }
  printf("\n\n");

  // view sub phrase result
  printf("====================  Sub Phrase Result   ======================\n");
  for(i=0; i<pout->spbtermcount; i++)
  {
      printf("[%d] ", pout->spbtermoffsets[i]);
      pos = GET_TERM_POS(pout->spbtermpos[i]);
      len = GET_TERM_LEN(pout->spbtermpos[i]);
      strncpy(word, pout->subphrbuf + pos, len);
      word[len] = 0;
      write_prop_to_str(pout->spbtermprop[i], prop,wdtype);
      printf("%s(%s)  ", word,prop);
  }
  printf("\n\n");

  // view NER result
  printf("===================  Human Name Result   ======================\n");
  for(i=0; i<pout->namebtermcount; i++)
  {
      printf("[%d] ", pout->namebtermoffsets[i]);
      pos = GET_TERM_POS(pout->namebtermpos[i]);
      len = GET_TERM_LEN(pout->namebtermpos[i]);
      strncpy(word, pout->namebuf+pos, len);
      word[len] = 0;
      write_prop_to_str(pout->namebtermprop[i], prop,wdtype);
      printf("%s(%s)  ", word,prop);
  }
  printf("\n\n");

  // view the book name result
  printf("=======================  book names   =========================\n");
  for(i=0; i<pout->bnbtermcount; i++)
  {
      printf("[%d] ", pout->bnbtermoffsets[i]);
      pos = GET_TERM_POS(pout->bnbtermpos[i]);
      len = GET_TERM_LEN(pout->bnbtermpos[i]);
      strncpy(word, pout->booknamebuf+pos, len);
      word[len] = 0;
      write_prop_to_str(pout->bnbtermprop[i], prop,wdtype);
      printf("%s(%s)  ", word,prop);
  }
  printf("\n\n");
}
