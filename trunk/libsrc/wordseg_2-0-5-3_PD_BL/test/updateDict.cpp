
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ul_dict.h>
#include <ul_sign.h>
#include "scwdef.h"

#include <iostream>
#include <string>
#include <set>
using namespace std;

class CItem
{
public:
  string key;
  
public:
  int operator< (const CItem &t) const
  {
    if(key.size() < t.key.size())
      return 1;
    else if(key.size() > t.key.size())
      return 0;
    else
      return (key < t.key);
  }  
};

extern u_int 
scw_seek_lemma(scw_worddict_t * pwdict,const char* term,int len);

int main(int argc,char** argv)
{
  scw_out_t* pout;
  scw_item_t* pitem;
  scw_worddict_t* pwdict;
  Sdict_build* sdict_build;
  Sdict_snode dict_node;
  scw_inlemma_t inlemma;

  char line[102400];
  char word[1024];
  char type[1024];

  FILE* fp;
  CItem item;
  set<CItem> set_dict;
  set<CItem>::iterator it;
  pair<set<CItem>::iterator, bool> itpair;
  
  if(argc != 4)
  {
    fprintf(stderr, "usage: %s olddictdir newwordfile newdictfile\n", argv[0]);
    exit(-1);
  }

  if((pwdict = scw_load_worddict(argv[1]) ) == NULL)
  {
    fprintf(stderr,"error: load worddict failed.Filename=%s\n",argv[1]);
    return -1;
  }

  if((pout = scw_create_out(40000, SCW_OUT_ALL)) == NULL)
  {
    fprintf(stderr,"error: init the output buffer error.\n");
    return -1;
  }
  
  if((pitem = scw_create_item(40000)) == NULL)
  {
    fprintf(stderr, "error: init pitem failed\n");
    return -1;
  }
  
  if((sdict_build = db_creat(400000, 0)) == NULL)
  {
    fprintf(stderr,  "error: db_creat error!");
    return -1;
  }  
  
  if((fp=fopen(argv[2], "r")) == NULL)
  {
    fprintf(stderr, "error: open file %s failed\n", argv[2]);
    return -1;
  }

  //int linenum=0;
  while(fgets(line,sizeof(line),fp))
  {
    //if(++linenum%1000==0)
    //fprintf(stderr, "%d\n", linenum);
      
    int len=strlen(line);
    while((line[len-1]=='\r') ||(line[len-1]=='\n'))
      line[--len]=0;
    
    if(sscanf(line, "%s%s", word, type) != 2)
    {
      fprintf(stderr, "sscanf line [%s] error\n", line);
      continue;
    }
    
    // add the basic terms into dict
    if(strcmp(type, "-b") == 0) // basic term
    {
      strcpy(inlemma.m_lmstr, word);
      inlemma.m_type = LEMMA_TYPE_SBASIC;
      inlemma.m_weight = 0;
      inlemma.m_bwcount = 1;
      inlemma.m_sphcount = 0;
      memset(inlemma.m_wordlist, 0, sizeof(char)*512);
      memset(inlemma.m_subphlist, 0, sizeof(char)*512);
      
      creat_sign_f64(word, strlen(word), &dict_node.sign1, &dict_node.sign2);
      /*if(db_op1(sdict_build, &dict_node, ADD) < 0)
      {
        fprintf(stderr, "error: db_op1(ADD) error");
        return -1;
      }*/
      
      scw_add_lemma(pwdict, sdict_build, inlemma); // add basic term
    }
    else if(strcmp(type, "-p") == 0) // phrase
    {
      item.key = word;
      itpair = set_dict.insert(item);
    }
    else
    {
      fprintf(stderr, "error: line tag [%s] error\n", line);
      continue;
    }
  }
  
  // segment the phrases, then add them into dict.
  it = set_dict.begin();
  for(; it!= set_dict.end(); it++)
  {
    u_int pos;
    char basic_word[256];
    bool has_dyn_word = false;
    
    memset(inlemma.m_wordlist, 0, sizeof(char)*512);
    memset(inlemma.m_subphlist, 0, sizeof(char)*512);
    
    strcpy(word,(*it).key.c_str());
    int len = strlen(word);
    scw_segment_words(pwdict, pout, word, len);
    
    strcpy(inlemma.m_lmstr, word);
    inlemma.m_type = LEMMA_TYPE_SPHRASE;
    inlemma.m_weight = 0;
    inlemma.m_bwcount = pout->wsbtermcount;
    inlemma.m_sphcount = pout->spbtermcount;
    
    if(scw_get_result(pitem, pwdict, pout->m_pir, SCW_OUT_BASIC) < 0)
    {
      fprintf(stderr, "error: get basic seg result error!\n");
      exit(-1);
    }    
    for( u_int i=0; i<pout->wsbtermcount; i++ )
    {
      scw_gi_tmstr(pitem,i,basic_word,sizeof(basic_word));
      inlemma.m_wordlist[i*2] = pout->wsbtermoffsets[i];

      pos = scw_seek_lemma(pwdict,basic_word,strlen(basic_word));
      if(pos == LEMMA_NULL) // meet dynamic term
      {
        fprintf(stderr, "error: has dynamic word [%s]!\n", word);
        has_dyn_word = true;
        break;
      }

      inlemma.m_wordlist[i*2+1] = pos;
    }
    if(has_dyn_word)
    {
      continue;
    }

    if(scw_get_result(pitem, pwdict, pout->m_pir, SCW_OUT_SUBPH) < 0)
    {
      fprintf(stderr, "error: get basic seg result error!\n");
      exit(-1);
    }
    for(u_int i = 0; i<pout->spbtermcount; i++)
    {
      scw_gi_tmstr(pitem,i,basic_word,sizeof(basic_word));
      inlemma.m_subphlist[i*2] = pout->spbtermoffsets[i];

      pos = scw_seek_lemma(pwdict,basic_word,strlen(basic_word));
      if(pos == LEMMA_NULL) // meet dynamic term
      {
        fprintf(stderr, "error; has dynamic word [%s]!\n", word);
        has_dyn_word = true;
        break;
      }

      inlemma.m_subphlist[i*2+1] = pos;
    }
    if(has_dyn_word)
    {
      continue;
    }

    creat_sign_f64(word, strlen(word), &dict_node.sign1, &dict_node.sign2);
    /*if(db_op1(sdict_build, &dict_node, ADD) < 0)
    {
      fprintf(stderr, "error: db_op1(ADD) error");
      return -1;;
    }*/

    scw_add_lemma(pwdict, sdict_build, inlemma); // add phrase
  }
  
  if(scw_save_worddict(pwdict, argv[3]) < 0)
  {
    fprintf(stderr, "error: save worddict [%s] failed\n", argv[3]);
  }
  
  db_del(sdict_build);
  scw_destroy_out(pout);
  scw_destroy_worddict(pwdict);
  
  fclose(fp);
  return 0;
}
