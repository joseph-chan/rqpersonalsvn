#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "scwdef.h"

static void scw_dump_out2(scw_out_t* pout,int flag,int outtype);
static void dump_item(scw_item_t& item);
static void dump_item1(scw_item_t& item,int wdtype);
extern void write_prop_to_str(scw_property_t& property, char* buffer,int wdtype);

int main(int argc,char** argv)
{
  scw_worddict_t * pwdict;
  scw_out_t *pout;
  char line[1024000];
  u_int scw_out_flag;
  int flag = 0;

  if(argc!= 3 )
  {
    fprintf(stderr, "usage: %s worddict_dir outtype\n", argv[0]);
    exit(-1);
  }

  if((pwdict=scw_load_worddict(argv[1]))==NULL)
  {
    fprintf(stderr,"Load worddict failed.Filename=worddict/bin/");
    return 1;
  }

  flag = atoi(argv[0]);

  scw_out_flag = SCW_OUT_ALL | SCW_OUT_PROP;
  if((pout=scw_create_out(80000, scw_out_flag))==NULL)
  {
    fprintf(stderr,"Init the output buffer error.\n");
    return -1;
  }

  while(fgets(line,sizeof(line),stdin))
  {    
    int len=strlen(line);
    while((line[len-1]=='\r') ||(line[len-1]=='\n'))
      line[--len]=0;
    
    if(scw_segment_words(pwdict,pout,line,len)<0)
    {
      fprintf(stderr, "query %s error\n", line);
      scw_destroy_out(pout);
      return -1;
    }
    scw_dump_out2(pout,flag,pwdict->m_wdtype);
  }

  return 0;
}

void scw_dump_out2(scw_out_t* pout,int flag,int wdtype)
{
  scw_item_t item;
  memset(&item,0,sizeof(item));

    // view basic word sep result
  if(scw_get_item(&item, pout, SCW_OUT_BASIC | SCW_OUT_PROP)>=0){
      printf("=================== Basic Word Sep Result =====================\n");  
    if(flag == 0)
      dump_item(item);
    else if(flag == 1)
      dump_item1(item,wdtype);
    printf("\n\n");
  }

  if(scw_get_item(&item, pout, SCW_OUT_WPCOMP | SCW_OUT_PROP)>=0){
      printf("=================== Word Phrase Result =====================\n");
    if(flag == 0)
      dump_item(item);
         else if(flag == 1)
      dump_item1(item,wdtype);
    printf("\n\n");
  }

  if(scw_get_item(&item, pout, SCW_OUT_SUBPH | SCW_OUT_PROP)>=0){
    printf("=================== Sub Phrase Result =====================\n");
    if(flag == 0)
      dump_item(item);
    else if(flag == 1)
      dump_item1(item,wdtype);
    printf("\n\n");
  }
  
  if(scw_get_item(&item, pout, SCW_OUT_NEWWORD | SCW_OUT_PROP)>=0){
    printf("=================== NEW Word Result =====================\n");
    if(flag == 0)
      dump_item(item);
    else if(flag == 1)
      dump_item1(item,wdtype);
    printf("\n\n");
  }


  if(scw_get_item(&item, pout, SCW_OUT_HUMANNAME | SCW_OUT_PROP)>=0){
    printf("=================== Humanname Result =====================\n");
    if(flag == 0)
      dump_item(item);
    else if(flag == 1)
      dump_item1(item,wdtype);
    printf("\n\n");
  }
  if(scw_get_item(&item, pout, SCW_OUT_BOOKNAME | SCW_OUT_PROP)>=0){
    printf("================== Bookname Result =====================\n");
    if(flag == 0)
      dump_item(item);
    else if(flag == 1)
      dump_item1(item,wdtype);
    printf("\n\n");
  }
}

void dump_item(scw_item_t& item)
{
  char word[256];
  int offset;

  for(int i=0;i<(int)item.m_tmcnt; i++){
    scw_gi_tmstr(&item,i, word, sizeof(word));
    offset = scw_gi_tmoff(&item,i);
    printf("[%d] %s  ", offset, word);
  }
}

void dump_item1(scw_item_t& item,int wdtype)
{
  char word[256];
  char prop[256];
  int offset;
  scw_property_t * ppt;
  
  for(int i=0;i<(int)item.m_tmcnt; i++){
    scw_gi_tmstr(&item,i, word, sizeof(word));
    offset = scw_gi_tmoff(&item,i);
    ppt = scw_gi_tmprop(&item,i);
    write_prop_to_str(*ppt, prop,wdtype);
    printf("[%d] %s(%s)  ", offset, word,prop);
  }
}


