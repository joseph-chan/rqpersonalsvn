/**
 * @brief 生成标注系统所需的二进制辞典。相关词典二进制格式也请参考本程序文件相关注释以及代码。
 * @author David Dai  
 * @date 2007/9/25
 */

#include "tagdict.h"

#include <ul_file.h>

#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define VERSION_STRING "Postag Dictionary Building Tool 1.0"

enum arg_opt
{
  OPT_HELP,
  OPT_TEST,
  OPT_VERSION,
  OPT_NEW_TAG,  
  OPT_ADD_TAG
};

// 生成词语标记信息辞典
int build_term_dict(char* srcFile, char* dstFilePath);

// 在已有辞典中添加新的词语重新构建辞典
int rebuild_term_dict(char* srcFilePath, char* dstFilePath);

// 检验已构建的词语标记信息辞典
int check_term_dict(char* dictPath);

void usage( const char *msg ) 
{
  fprintf(stderr,"Dictionary building tool for postag module.\n" );     
  fprintf(stderr,"Usage: %s  [-?] [-V] [-o outdir] [-c filename] [-a dictpath] [-t dictpath]\n",msg);
  fprintf(stderr,"Summary of parameters:\n");             
  fprintf(stderr,"  -?\t print this help message.\n");
  fprintf(stderr,"  -V\t print Version number and exit.\n");
  fprintf(stderr,"  -t\t Test the dict whether a term is in it.\n");
  fprintf(stderr,"  -a\t Add new data to the named dictionary.\n"); 
  fprintf(stderr,"  -c\t Create binary dictionaries by the filename of term tag text dictionary.\n");  
  fprintf(stderr,"  -o\t Output dictionaries to the directory for output.\n");
  fprintf(stderr,"e.g.:\n");
  fprintf(stderr,"%s -c term_tag_dict.txt -o dict/bin/ \n",msg);
  fprintf(stderr,"%s -a dict/old-bin/ -o dict/new-bin/ < addData.txt \n",msg);
}

// Main Entry
int main(int argc , char *argv[])
{
  if(argc == 1)
  {
    usage(argv[0]);
    return 1;
  }  

  char* tmp  = NULL;
  char* filename = NULL;
  char outdir[300] = {0};  
  arg_opt opt = OPT_HELP;

  while (1)
  {
    char ch = getopt(argc, argv, "V?a:c:o:t:");
    if (ch == -1) break;
    switch (ch)
    {
    case '?':      
      usage(argv[0]);
      return 0;  
    case 'V':      
      printf("%s\n",VERSION_STRING);
      return 0;
    case 'a':        
      opt = OPT_ADD_TAG;
      filename = strdup(optarg);
      break;            
    case 'c':
      opt = OPT_NEW_TAG;
      filename = strdup(optarg);
      break;   
    case 't':
      opt = OPT_TEST;
      filename = strdup(optarg);
      break;   
    case 'o':
      tmp = strdup(optarg);
      strncpy(outdir, tmp, strlen(tmp)+1);
      strncat(outdir,"/",1);
      free(tmp);
      break;        
    default:      
      usage(argv[0]);
      return 1;
    }
  }
  if( (optind == argc-1) || !filename )
  {
    usage(argv[0]);
    free(filename);
    return 1;
  }
  if( !outdir[0] )
  {
    strncpy(outdir,"./",3); // 默认在当前目录
  }
  
  switch( opt )
  {  
  case OPT_NEW_TAG: 
    // build a new dictionary
    fprintf(stderr,"building pos tag dictionary for postag ...\n");
    if( build_term_dict(filename,outdir) != 0 )
    {
      LOG_ERROR("error: building binary dictionary failed");
      return -1;
    }    
    break;
  case OPT_ADD_TAG: 
    // append new items to dictionary  
    fprintf(stderr,"rebuilding pos tag dictionary for postag ...\n");
    if( rebuild_term_dict(filename,outdir) != 0 )
    {
      LOG_ERROR("error: rebuilding pos tag dictionary failed");
      return -1;
    }
    break;
  case OPT_TEST:
    // test the dictionary
    fprintf(stderr,"checking the pos tag dictionary for postag ...\n");  
    if( check_term_dict(filename) != 0 )
    {
      LOG_ERROR("error: checking pos tag dictionary failed");
      return -1;
    }  
    break; 
  default:
    usage(argv[0]);
  }  

  free(filename);
  return 0;
}

/*
  根据给定文本辞典创建二进制辞典。
  1. 文本辞典格式（行）：孤寂  a 19 ad 1 an 7  
  2. 二进制辞典格式：词典索引文件单独存储在两个二进制文件中（因为Sdict_build设计上的问题）。
  3. 标记信息单独存储在一个二进制数据文件中。

  srcFile: 文本词典文件名（含路径）。
  dstFilePath: 输出二进制词典路径。
 */
static unsigned char g_buffer[10*1024*1024] = {0};  // global memory usage
int build_term_dict(char* srcFile, char* dstFilePath)
{
  char delims[16] = " \t\n\r";
  char errInfo[512] = {0};  
  
  Sdict_build *sdict;  
  FILE* fp = NULL;
    
  // get directory name and file name of file
  char* name = strrchr(srcFile, '/');
  if( name == NULL )
  {
    name = srcFile;
    strncpy(errInfo, "./",3);
  }  
  else
  {    
    *name = '\0';
    strncpy(errInfo,srcFile,strlen(srcFile)+1);
    *name = '/';
    name++;
  }

  // create dict structure
  int nline, nword, nchar, nsize;
  ul_finfo(errInfo, name, &nline, &nword, &nchar);
  nsize = (int)(nline*1.25);
  if( (sdict = db_creat(nsize, 0)) == NULL ) 
  {
    LOG_ERROR("error: dictionary creation failed");
    return -1;
  }

  // building ...
  fp = fopen(srcFile, "rt");
  if( !fp )
  {
    snprintf(errInfo,512,"error: can't open file %s",srcFile);
    LOG_ERROR(errInfo);
    db_del(sdict);
    return -1;
  }  
  
  int size = 0;
  int count = 0;  
  char line[512] = {0};
  char term[256] = {0};
  while( fgets(line, sizeof(line), fp) )
  {
    int len = strlen(line);
    while( (line[len-1]=='\r') || (line[len-1]=='\n') )
    {
      line[--len] = 0;
    }

    if( !len ) continue;
    
    int cnt = 0, tagcnt = 0;
    int curpos = size + 1;
    char*  token = strtok( line, delims );
    while( token != NULL )
    {        
      if( cnt == 0 )
      {
        strncpy(term,token,strlen(token)+1);
      }
      else if( cnt % 2 == 1 ) // meet POS tag
      {
        int tagid = get_pos_val(token);        
        *(g_buffer+curpos) = tagid;
        curpos++;        
        tagcnt++;
      }
      else // meet frequency
      {
        double prob = atoi(token);
        prob = log(prob + 1); // 加1平滑        
        *((double*)(g_buffer+curpos)) = prob;
        curpos += sizeof(double);
      }
      
      token = strtok( NULL, delims );      
      cnt++;
    }  
    g_buffer[size] = tagcnt;    
    
    Sdict_snode dnode;  
    creat_sign_fs64(term, strlen(term), &dnode.sign1, &dnode.sign2);
    dnode.other = size;
    dnode.code = size;
    db_op1(sdict, &dnode, ADD);

    size += tagcnt*(1+sizeof(double)) + 1;
    count++;    
  }
  fprintf(stderr,"build: %d rows have been processed\n", count); // for debug

  fclose(fp);  

  // write the index data out
  snprintf(line,512,"%s",TERM_TAG_DICT);
  if( db_save(sdict, dstFilePath, line) < 0 )
  {
    LOG_ERROR("error: saving indices of dictionary failed");
    db_del(sdict);
    return -1;
  }

  // dispose dict
  db_del(sdict);

  // write the tag info out
  snprintf(line,512,"%s/%s.dat",dstFilePath,TERM_TAG_DICT);
  fp = fopen(line,"wb");
  if( !fp )
  {
    snprintf(errInfo,512,"error: can't open file %s to save tag info",line);
    LOG_ERROR(errInfo);
    return -1;
  }  
  fwrite(&size,sizeof(int),1,fp);  // size of buffer
  fwrite(g_buffer,sizeof(char),size,fp); // byte buffer  
  fclose(fp);
  
  return 0;
}

/*
  把给定文本加入已有的二进制辞典。
  已有文本格式必须和已有辞典格式一致，行样例：孤寂  a 19 ad 1 an 7

  srcFilePath: 已有二进制词典路径。
  dstFilePath: 输出二进制词典路径。
  需要添加的文本数据通过标准输入进行输入。
 */
int rebuild_term_dict(char* srcFilePath, char* dstFilePath)
{
  char delims[] = " \t\n\r";
  char errInfo[512] = {0};  
  
  Sdict_build *sdict;  
  FILE* fp = NULL;

  // get directory name and file name of file
  char filename[128] = {0};
  snprintf(filename,128,"%s",TERM_TAG_DICT);
  if( (sdict = db_load(srcFilePath, filename, 0)) == NULL ) 
  {
    LOG_ERROR("error: loading index dictionary failed");
    return -1;
  }
  
  int size = 0;
  int count = 0;  
  char line[512] = {0};
  char term[256] = {0};  

  snprintf(filename,128,"%s/%s.dat",srcFilePath,TERM_TAG_DICT);
  fp = fopen(filename,"rb");
  if( !fp )
  {
    snprintf(errInfo,512,"error: can't open file %s to save tag info",filename);
    LOG_ERROR(errInfo);
    return -1;
  }  
  fread(&size,sizeof(int),1,fp);  // size of buffer
  fread(g_buffer,sizeof(char),size,fp); // byte buffer  
  fclose(fp);

  while( fgets(line, sizeof(line), stdin) )
  {
    int len = strlen(line);
    while( (line[len-1]=='\r') || (line[len-1]=='\n') )
    {
      line[--len] = 0;
    }

    if( !len ) continue;
    if( !strcmp(line,"quit") ) break;
    
    int cnt = 0, tagcnt = 0;
    int curpos = size + 1;
    char*  token = strtok( line, delims );
    while( token != NULL )
    {        
      if( cnt == 0 )
      {
        strncpy(term,token,strlen(token)+1);
      }
      else if( cnt % 2 == 1 ) // meet POS tag
      {
        int tagid = get_pos_val(token);        
        *(g_buffer+curpos) = tagid;
        curpos++;        
        tagcnt++;
      }
      else // meet frequency
      {
        double prob = atoi(token);
        prob = log(prob + 1); // 加1平滑        
        *((double*)(g_buffer+curpos)) = prob;
        curpos += sizeof(double);
      }
      
      token = strtok( NULL, delims );      
      cnt++;
    }  
    g_buffer[size] = tagcnt;    
    
    Sdict_snode dnode;  
    creat_sign_fs64(term, strlen(term), &dnode.sign1, &dnode.sign2);
    dnode.other = size;
    dnode.code = size;
    db_op1(sdict, &dnode, ADD);

    size += tagcnt*(1+sizeof(double)) + 1;
    count++;    
  }
  fprintf(stderr,"build: %d rows have been processed\n", count); // for debug

  // write the index data out
  snprintf(filename,512,"%s",TERM_TAG_DICT);
  if( db_save(sdict, dstFilePath, filename) < 0 )
  {
    LOG_ERROR("error: saving indices of dictionary failed");
    db_del(sdict);
    return -1;
  }

  db_del(sdict);

  // write the tag info out
  snprintf(line,512,"%s/%s.dat",dstFilePath,TERM_TAG_DICT);
  fp = fopen(line,"wb");
  if( !fp )
  {
    snprintf(errInfo,512,"error: can't open file %s to save tag info",line);
    LOG_ERROR(errInfo);
    return -1;
  }  
  fwrite(&size,sizeof(int),1,fp);  // size of buffer
  fwrite(g_buffer,sizeof(char),size,fp); // byte buffer  
  fclose(fp);
  
  return 0;
}

int check_term_dict(char* dictPath)
{
  char filename[300] = {0};
  strncpy(filename,dictPath,strlen(dictPath)+1);
  strncat(filename,"/",1);
  tag_dict_t* dict = tag_dict_load(filename);
  if( !dict )
  {
    LOG_ERROR("error: opening binary dictionary failed");
    return -1;
  }

  char line[1024] = {0};
  while( fgets(line, sizeof(line), stdin) )
  {
    int len = strlen(line);
    while( (line[len-1]=='\r') || (line[len-1]=='\n') )
    {
      line[--len] = 0;
    }

    if( !len ) continue;
    if( !strcmp(line,"quit") ) break;

    //char str[32] = "戴帅湘";
    char* str = line;
    int idx = tag_dict_find(dict,str);

    if( idx >= 0 )
    {
      fprintf(stderr,"check: %s is found: ",str);
      unsigned cnt = dict->buffer[idx];
      int pos = idx + 1;
      fprintf(stderr,"%s ", str);
      while( cnt-- > 0)
      {
        const char* stag = get_pos_str( dict->buffer[pos] );
        double prob = *(double*)(dict->buffer+pos+1);
        fprintf(stderr,"%s %f ", stag,prob);
        pos += 1 + sizeof(double);
      }
      fprintf(stderr,"\n");
    }
    else
    {
      fprintf(stderr,"check: %s is NOT found\n",str);
    }
  }
  
  tag_dict_free(dict);
  return 0;
}
