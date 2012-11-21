#include <stdio.h>
#include <string>
#include <set>
#include <ul_log.h>
#include <math.h>
#include <ctype.h>
#include "scwdef.h"
#include "international.h"
#include "property.h"

using namespace std;

extern int is_gb_pijm(char * p);
extern int is_gb_pajm(char * p);
//extern int is_gbk_hz(u_char * p);
//extern int is_mid_mark(char * p);
//extern int is_oneword_gb(char * p);

extern int scw_build_ambfrag(const char * ambfragfile,scw_worddict_t* pwdict);
extern int scw_add_extra_property(char * property_file,scw_worddict_t* pwdict);
//extern int scw_build_nameinfo(const char* namefile,int type,scw_worddict_t* pwdict);
static int scw_build_dict(char * dictfile,scw_worddict_t* pwdict);
static int add_ascii(scw_worddict_t* pwdict);
static int add_gbk_words(scw_worddict_t* pwdict);
static int set_oneword_property(char * word,scw_property_t& prop,int wdtype);
 
/// help.
void show_help(char* progname)
{
  fprintf(stderr, "usage: %s\n" 
      "\t\t-d: totalraw_path\n"
      "\t\t-a: ambfrag_path\n"
      "\t\t-p: property_path\n"
      "\t\t-h: chnameinfo_path\n"
      "\t\t-f: fnameinfo_path\n"
      "\t\t-b: binarydict_path\n"
      "\t\t-l: dicttype\n", progname);
}

int main(int argc,char** argv)
{
  // *** check the argument
  if( argc == 1 )
  {
    show_help(argv[0]);
    exit(-1);
  }
  
  char arg = 0;
  char* totalraw_path = NULL;
  char* ambfrag_path = NULL;
  char* property_path = NULL;
  char* chname_path = NULL;
  char* fname_path = NULL;
  char* binary_path = NULL;
  int wdtype = SCW_WD_CH;
  
  while((arg = getopt(argc,argv,"d:a:p:h:l:b:f:")) != -1)
  {
    switch (arg)
    {
      case 'd':
        totalraw_path = optarg;
        break;
      case 'a':
        ambfrag_path = optarg;
        break;
      case 'p':
        property_path = optarg;
        break;
      case 'h': 
        chname_path = optarg;
        break;
      case 'l':
        wdtype = atoi(optarg);
        break;
      case 'b':
        binary_path = optarg;
        break;
      case 'f':
        fname_path = optarg;
        break;
      default :
        show_help(argv[0]);
        exit(0);
    }
  }
  
  init_function(SCW_GBK,SCW_CHINESE);
  ul_logstat_t  va_stat;
  va_stat.events = UL_LOG_NOTICE;
  va_stat.to_syslog = UL_LOG_NONE;
  va_stat.spec = UL_LOG_NONE;
  ul_openlog("./", "/log/build_dict.", &va_stat,1000 );
    
  // init the worddict
  scw_worddict_t* pwdict = NULL; 
  if((pwdict=scw_create_worddict())==NULL)
  {
    fprintf(stderr,"can't creat dict, the memory is not enough!\n");
    return -1;
  }

  scw_set_wdtype(pwdict,wdtype);
  
  if(scw_build_dict(totalraw_path,pwdict) < 0)
  {
    fprintf(stderr, "build dict failed!\n");
     return -1;
  }

  if(wdtype & SCW_WD_CH)
  {
    if(scw_build_ambfrag(ambfrag_path,pwdict) < 0)
    {
      fprintf(stderr, "build ambfrag dict failed!\n"); 
      return -1;
    }
    
    if(scw_add_extra_property(property_path,pwdict) < 0)
    {
      fprintf(stderr, "add extra property failed!\n");
      return -1;
    }
    
    if(scw_build_nameinfo(chname_path,DATA_CNAME,pwdict) < 0)
    {
      fprintf(stderr, "build chinese nameinfo failed!\n");  
      return -1;
    }
    
    if(scw_build_nameinfo(fname_path,DATA_FNAME,pwdict) < 0)
    {
      fprintf(stderr, "build foreign nameinfo failed!\n");
      return -1;
    }
  }
  else if(wdtype & SCW_WD_JP)
  {
    if(scw_build_ambfrag(ambfrag_path,pwdict) < 0)
    {
      fprintf(stderr, "build ambfrag dict failed!\n"); 
      return -1;
    }
    if(scw_add_extra_property(property_path,pwdict) < 0)
    {
      fprintf(stderr, "add extra property failed!\n");
      return -1;
    }
    if(scw_build_nameinfo(chname_path,DATA_JNAME,pwdict) < 0)
    {
      fprintf(stderr, "build japanese nameinfo failed!\n");
      return -1;
    }    
  }

  if(scw_save_worddict(pwdict,binary_path)!= 1)
  {
    fprintf(stderr,"Save worddict failed.\n");
    return -1;
  }   
         
  fprintf(stderr,"Total %d lemmas in worddict.\nTotal %d dictentry in wordict.\n",pwdict->m_lmpos,pwdict->m_depos);

  scw_destroy_worddict(pwdict);
  pwdict=NULL;
  return 0;
}
/// add all ascii symbol.
int add_ascii(scw_worddict_t* pwdict)
{
  char where[20]="scw_add_ascii";
  char word[2];
  int ret;
  scw_inlemma_t inlemma;
    
  inlemma.m_type  = LEMMA_TYPE_SBASIC;
  inlemma.m_weight=1;
  inlemma.m_bwcount  = 1;
  inlemma.m_sphcount  = 0;
  
  for(int i = 1; i<256 ;i++)
  {
    word[0]=i;
    word[1]=0;

    INIT_PROPERTY(inlemma.m_property);
      SET_ONEWORD(inlemma.m_property);
    SET_ASCIIWORD(inlemma.m_property);
  
    if(ispunct(char(i)))
    SET_MARK(inlemma.m_property);
        
    if(isdigit(i))
      SET_ASCIINUM(inlemma.m_property);
    
    if(isalnum(i))
      SET_ALNUM(inlemma.m_property);
    
    if(isspace(i))
      SET_SPACE(inlemma.m_property);
    
    strcpy(inlemma.m_lmstr, word);

	ret = scw_add_lemma(pwdict,NULL,inlemma);
    if(ret<0)
    {
      fprintf(stderr, "add lemma error in %s\n", where);
      return -1;
    }
  }

  return 0;
}
/// word should be oneword lemma.
int set_oneword_property(char * word,scw_property_t& prop,int wdtype)
{
  int len = strlen(word);
  if(len == 1)
  {
    SET_ONEWORD(prop);
    SET_ASCIIWORD(prop);
    if(ispunct(word[0]))
             SET_MARK(prop);
     if(isdigit(word[0]))
             SET_ASCIINUM(prop);
    if(isalnum(word[0]))
      SET_ALNUM(prop);
  }
  else if(len == 2)
  {
    SET_ONEWORD(prop);
    if(is_gbk_hz((u_char*)word))
    {//hz
      SET_HZ(prop);
    }
    if(IS_GB_MARK(word))
    {
      SET_MARK(prop);
      if( IS_GB_1234(word) || IS_GB_ABCD(word) || is_mid_mark(word))
      {
        SET_ASCIIWORD(prop);
      }
      if(wdtype & SCW_WD_JP)
      {
        if(is_gb_pijm(word))
        {
          SET_PINGJM(prop);
        }
        if(is_gb_pajm(word))
        {
          SET_PIANJM(prop);
        }
      }
    }
  }
  else if(len == 4)
  {
    SET_ONEWORD(prop);
  }
  
  return 0;
}
/// add all one hanzi term.
int add_gbk_words(scw_worddict_t* pwdict)
{
  char * where = "scw_add_gbk_words";
  char word[3];
  scw_inlemma_t inlemma;
  
  inlemma.m_type  = LEMMA_TYPE_SBASIC;
  inlemma.m_weight=1;
  inlemma.m_bwcount  = 1;
  inlemma.m_sphcount  = 0;
  
  word[2] = 0;
  
  for(u_char i=0x81;i<0xFF;i++)
  {
    word[0]=(char)i;
    for(u_char j=0x40;j<0xFF;j++)
    {
      word[1]=j;
      
      INIT_PROPERTY(inlemma.m_property);
      strcpy(inlemma.m_lmstr, word);
      set_oneword_property(word,inlemma.m_property,pwdict->m_wdtype);
      
      if(is_gbk_hz((u_char*)word))
      {//hz
        if(pwdict->m_wdtype & SCW_WD_CH)
        {
          SET_NAME_STICK(inlemma.m_property);
          SET_NAMEMID(inlemma.m_property);
          SET_NAMEEND(inlemma.m_property);
        }
      }
        
      inlemma.m_bwcount   = 1;
      inlemma.m_sphcount  = 0;

      if(scw_add_lemma(pwdict,NULL,inlemma)<0)
      {
        fprintf(stderr, "add lemma error in %s\n", where);
        return -1;
      }
    }
  }

  return 0;
}

/// build binary dict data.
int scw_build_dict(char* dictfile,scw_worddict_t* pwdict)
{
  char line[MAX_IL_LEN];
  int linenum = 0;

  FILE * fp = NULL;
  scw_inlemma_t inlemma;
  set<string> lemmaset;
  Sdict_build * pdynwdict = NULL;

  if((pdynwdict = db_creat(10000, 0)) == NULL) 
  {
     ul_writelog(UL_LOG_NOTICE, "create db_dict failed!\n");
     return -1;
  }
  
  if(add_ascii(pwdict) < 0)
    return -1;
  if(add_gbk_words(pwdict) < 0)
    return -1;

  if((fp=fopen(dictfile,"r"))==NULL)
  {
    ul_writelog(UL_LOG_FATAL, "Cannot open dictfile %s\n", dictfile);
    return -1;
  }
  
  while(1)
  {
    if(fgets(line,sizeof(line),fp) == NULL)
       break;

    linenum++;
    
    if(update_lemma_with_line(pwdict,pdynwdict,&inlemma,line) < 0)
    {
      ul_writelog(UL_LOG_NOTICE, "bad format in %d\n",linenum);
      continue;
    }

    // judge whether the term has been added
    if(lemmaset.find(inlemma.m_lmstr) == lemmaset.end())
    {
      if(is_oneword_gb(inlemma.m_lmstr))
      {
        set_oneword_property(inlemma.m_lmstr,inlemma.m_property,pwdict->m_wdtype);
      }
      
      if(scw_add_lemma(pwdict,pdynwdict,inlemma) < 0)
      {
        ul_writelog(UL_LOG_FATAL, "Add lemma error.Line=[%s]\n",line);
        continue;
      }

      lemmaset.insert(inlemma.m_lmstr);
    }
  }
  
  db_del(pdynwdict);
  
  fclose(fp);
  return 0;
}

