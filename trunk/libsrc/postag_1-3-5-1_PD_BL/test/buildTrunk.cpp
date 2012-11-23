/**
 * @brief �������ɷ�������Ҫ�Ķ����ƴʵ䡣
 * @author David Dai
 * @date 2009/2/3
 */

#include "tagdict.h"
#include <ul_file.h>

#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

/// build dicts
int make_category(char* srcFile, char* dstFilePath);
int make_property(char* srcFile, char* dstFilePath);
int make_pattern(char* srcFile, char* dstFilePath);
/// test the built dict files
void test_dict(char* binPath, char* fileName);     


void usage( const char *msg ) 
{
  fprintf(stderr,"Build trunk dict.\n" );      
  fprintf(stderr,"Usage: %s -type srcFileName [destFilePath]\n",msg);
  fprintf(stderr,"Usage: %s -patt srcFileName [destFilePath]\n",msg);
  fprintf(stderr,"Usage: %s -prop srcFileName [destFilePath]\n",msg);  
  fprintf(stderr,"Usage: %s -t dictPath filename\n",msg);
}

int main(int argc , char *argv[])
{
  if( argc <= 2 )
  {    
    usage(argv[0]);
    return 1;
  }  

  string opt = argv[1];
  char path[MAX_PATH_LENG] = "./";    
  if( opt == "-type" && argc >= 3 )
  {
    if( argc > 3 )
      strncpy(path,argv[3],strlen(argv[3])+1);
    make_category(argv[2],path);
  }
  else if( opt == "-prop" && argc >= 3 )
  {
    if( argc > 3 )
      strncpy(path,argv[3],strlen(argv[3])+1);
    make_property(argv[2],path);
  }
  else if( opt == "-patt" && argc >=3 )
  {
    if( argc > 3 )
      strncpy(path,argv[3],strlen(argv[3])+1);
    make_pattern(argv[2],path);
  }
  else if( opt == "-t" && argc > 3 )
  {
    test_dict(argv[2],argv[3]);
  }
  else
    usage(argv[0]);
  
  return 0;
}

/*! 
 * ���������Ϣ�ʵ䡣
 * ���������Ϣ�洢��Sdict_snode.other���С�
 * ���������Ϣ��ǰ��ʹ����32λ������λ�洢��
 */
int make_category(char* srcFile, char* dstFilePath)
{
  sodict_build_t *sdict;  
  char strbuf[MAX_PATH_LENG] = {0};
    
  // get directory name and file name of file
  char* name = strrchr(srcFile, '/');
  if( name == NULL )
  {
    name = srcFile;
    strncpy(strbuf, "./",3);
  }  
  else
  {    
    *name = '\0';
    strncpy(strbuf,srcFile,strlen(srcFile)+1);
    *name = '/';
    name++;
  }

  // create dict structure
  int nline, nword, nchar, nsize;
  ul_finfo(strbuf, name, &nline, &nword, &nchar);
  nsize = (int)(nline*1.25);
  if( (sdict = odb_creat(nsize)) == NULL ) 
  {
    LOG_ERROR("error: dict struct creation failed");
    return -1;
  }

  // building ...  
  ifstream in(srcFile);
  if( !in )
  {
    cerr << "error: can't open file " << srcFile << endl;
    return -1;
  }

  string line;	
  int linenum = 0;
  char termbuf[256] = {0};
  char propbuf[256] = {0};
	while(getline(in,line))
	{	
    linenum++;
    if( !line.size() )
      continue;	
		
		// �ԡ�\t���ָ�������term�а����ո�
    if(sscanf(line.c_str(),"%[^\t]\t%[^\n]\n",termbuf,propbuf) != 2)
    {
      cerr << "warning: invalid format, jump over line " << linenum << endl;
      continue;
    }
#ifdef _DEBUG
    cerr << termbuf << "\t" << propbuf << "\t";
#endif

    unsigned prop = 0;
    string sprop = propbuf;
    for(unsigned i=0; i<sprop.size(); i++)
    {
      if( sprop[i] == '|' )
        sprop[i] = '\t';
    }
    istringstream ls(sprop);
    while( ls )
    {
      int type = 0;
      ls >> type;  
#ifdef _DEBUG
      cerr << type << "\t";
#endif
      if( type >= 1 && type <=32 )
      {
        unsigned sign = 1;
        sign <<= type - 1;
        prop |= sign;
      }
    }
#ifdef _DEBUG
    cerr << "\n";
#endif   

    sodict_snode_t dnode;  
    creat_sign_fs64(termbuf, strlen(termbuf), &dnode.sign1, &dnode.sign2);
    dnode.cuint1 = prop;
    dnode.cuint2 = 0;
    odb_add(sdict, &dnode, 1);
  } 

  // write the index data out
  snprintf(strbuf,512,"%s",TERM_TYPE_DICT);
  if( odb_save(sdict, dstFilePath, strbuf) != ODB_SAVE_OK )
  {
    LOG_ERROR("error: saving indices of term info dictionary failed");
    odb_destroy(sdict);
    return -1;
  }

  // dispose dict
  odb_destroy(sdict);
  
  return 0;
}

/*! 
 * ����������Ϣ�ʵ䡣
 * ����ľ������Ժͳ������Էֱ�洢��Sdict_snode.other���Sdict_snode.code���У�
 * ���о������Դ洢Ϊ��32λ���������Դ洢Ϊ��32λ��
 */
int make_property(char* srcFile, char* dstFilePath) 
{
  sodict_build_t *sdict;  
  char strbuf[MAX_PATH_LENG] = {0};
    
  // get directory name and file name of file
  char* name = strrchr(srcFile, '/');
  if( name == NULL )
  {
    name = srcFile;
    strncpy(strbuf, "./",3);
  }  
  else
  {    
    *name = '\0';
    strncpy(strbuf,srcFile,strlen(srcFile)+1);
    *name = '/';
    name++;
  }

  // create dict structure
  int nline, nword, nchar, nsize;
  ul_finfo(strbuf, name, &nline, &nword, &nchar);
  nsize = (int)(nline*1.25);
  if( (sdict = odb_creat(nsize)) == NULL ) 
  {
    LOG_ERROR("error: dict struct creation failed");
    return -1;
  }

  // building ...  
  ifstream in(srcFile);
  if( !in )
  {
    cerr << "error: can't open file " << srcFile << endl;
    return -1;
  }

  string line;	
  int linenum = 0;
  char termbuf[256] = {0};
  char propbuf[256] = {0};
	while(getline(in,line))
	{	
    linenum++;
    if( !line.size() )
      continue;	
		
		// �ԡ�\t���ָ�������term�а����ո�
    if(sscanf(line.c_str(),"%[^\t]\t%[^\n]\n",termbuf,propbuf) != 2)
    {
      cerr << "warning: invalid format, jump over line " << linenum << endl;
      continue;
    }
#ifdef _DEBUG
    cerr << termbuf << "\t" << propbuf << "\t";
#endif

    unsigned lprop = 0;
    unsigned hprop = 0;
    string sprop = propbuf;
    for(unsigned i=0; i<sprop.size(); i++)
    {
      if( sprop[i] == '|' )
        sprop[i] = '\t';
    }
    istringstream ls(sprop);
    while( ls )
    {
      int type = 0;
      ls >> type;  
#ifdef _DEBUG
      cerr << type << "\t";
#endif
      if( type >= 1 && type <= 32 )
      {
        unsigned sign = 1;
        sign <<= type - 1;
        lprop |= sign;
      }
      else if( type >= 33 && type <= 64)
      {
        type -= 32;
        unsigned sign = 1;
        sign <<= type - 1;
        hprop |= sign;
      }
    }
#ifdef _DEBUG
    cerr << "\n";
#endif 
    
    sodict_snode_t dnode;  
    creat_sign_fs64(termbuf, strlen(termbuf), &dnode.sign1, &dnode.sign2);
    dnode.cuint1 = lprop;
    dnode.cuint2 = hprop;
    odb_add(sdict, &dnode, 1);
  } 

  // write the index data out
  snprintf(strbuf,512,"%s",TERM_PROP_DICT);  
  if( odb_save(sdict, dstFilePath, strbuf) != ODB_SAVE_OK )
  {
    LOG_ERROR("error: saving indices of term info dictionary failed");
    odb_destroy(sdict);
    return -1;
  }

  // dispose dict
  odb_destroy(sdict);
  
  return 0;
}

/*
 * �����ϵ��Ϣ�ʵ䡣
 * ��ǰ�Ĵ����ϵ��Ҫ��ָ����λ��ϵ�������������������Ʒ�ȡ�
 * ��Ϊ���������࣬��ǰֱ�Ӱ�����termƴ�Ӻ���hash��������������˺���Ҫ������ش����ƵĴ洢�ṹ��
 * ע�⣺
 * 1���ڴ�����term�ǰ�˳��ƴ�Ӵ洢�ģ����ҵ�ʱ��Ҳ��Ҫ����
 * 2�����termֱ�������������ڲ�ѯ��û�в���ָ����š�
 * 3����31λ�д洢���ǹ�ϵ���ͣ����λ�洢����˳��
 */
int make_pattern(char* srcFile, char* dstFilePath)
{
  sodict_build_t *sdict;  
  char strbuf[MAX_PATH_LENG] = {0};
    
  // get directory name and file name of file
  char* name = strrchr(srcFile, '/');
  if( name == NULL )
  {
    name = srcFile;
    strncpy(strbuf, "./",3);
  }  
  else
  {    
    *name = '\0';
    strncpy(strbuf,srcFile,strlen(srcFile)+1);
    *name = '/';
    name++;
  }

  // create dict structure
  int nline, nword, nchar, nsize;
  ul_finfo(strbuf, name, &nline, &nword, &nchar);
  nsize = (int)(nline*1.25);
  if( (sdict = odb_creat(nsize)) == NULL ) 
  {
    LOG_ERROR("error: dict struct creation failed");
    return -1;
  }

  // building ...  
  ifstream in(srcFile);
  if( !in )
  {
    cerr << "error: can't open file " << srcFile << endl;
    return -1;
  }

  string line;	
  int linenum = 0; 
  char propbuf[256] = {0};
  char termbuf1[256] = {0};
  char termbuf2[256] = {0};
	while(getline(in,line))
	{	
    linenum++;
    if( !line.size() )
      continue;	
		
		// �ԡ�\t���ָ�������term�а����ո�
    if(sscanf(line.c_str(),"%[^\t]\t%[^\t]\t%[^\n]\n",termbuf1,termbuf2,propbuf) != 3)
    {
      cerr << "warning: invalid format, jump over line " << linenum << endl;
      continue;
    }
#ifdef _DEBUG
    cerr << termbuf1 << "\t" << termbuf2 << "\t" << propbuf << "\t";
#endif

    string term1 = termbuf1;
    string term2 = termbuf2;

    unsigned prop = 0;
    string sprop = propbuf;
    for(unsigned i=0; i<sprop.size(); i++)
    {
      if( sprop[i] == '|' )
        sprop[i] = '\t';
    }
    istringstream ls(sprop);
    while( ls )
    {
      int type = 0;
      ls >> type;  
#ifdef _DEBUG
      cerr << type << "\t";
#endif
      if( type >= 1 && type <32 )
      {
        unsigned sign = 1;
        sign <<= type - 1;
        prop |= sign;
        if( term1 > term2 ) // ����
        {          
          prop |= 0x80000000;  // ���λ��1
        }
      }
    }
#ifdef _DEBUG
    cerr << "\n";
#endif
    
    string strbuf = term1 + term2;
    if( term1 > term2 )
      strbuf = term2 + term1;
    sodict_snode_t dnode;  
    creat_sign_fs64(const_cast<char*>(strbuf.c_str()), strbuf.size(), &dnode.sign1, &dnode.sign2);
    dnode.cuint1 = prop;
    dnode.cuint2 = 0;
    odb_add(sdict, &dnode, 1);   
  }

  // write the index data out
  snprintf(strbuf,512,"%s",TERM_PATT_DICT);
  if( odb_save(sdict, dstFilePath, strbuf) != ODB_SAVE_OK )
  {
    LOG_ERROR("error: saving indices of bigram dictionary failed");
    odb_destroy(sdict);
    return -1;
  }

  // dispose dict
  odb_destroy(sdict);
  
  return 0;
}
/// Only for test the built unigram and bigram dict files.
void test_dict(char* binPath, char* fileName)
{  
  odict_t* p = odb_load_search(binPath, fileName);
  if( !p )
  {
    LOG_ERROR("error: loading binary dict dictionary failed, skip");    
  }

  char line[10240] = {0};
  while(fgets(line,sizeof(line),stdin))
  {    
    int len = strlen(line);
    while( (line[len-1]=='\r') || (line[len-1]=='\n') )
      line[--len] = 0;

    if( !len )
      continue;

    sodict_snode_t dictNode;  
    creat_sign_f64(line, strlen(line), &dictNode.sign1, &dictNode.sign2);
    if( ODB_SEEK_OK == odb_seek_search(p, &dictNode) )
      cerr << line << "\t low=" << dictNode.cuint1 << ", high=" << dictNode.cuint2 << endl;
  }

  odb_destroy_search(p);
}



