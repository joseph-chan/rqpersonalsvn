/**
 * @brief 用于处理和生成chunk parsing所需要的词类转换规则等二进制辞典。
 * @author David Dai
 * @date 2007/12/4
 */

#include "tagdict.h"

#include <ul_file.h>

#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

static int TAG_TRAN_TABLE[USED_TAG_COUNT+1][USED_TAG_COUNT+1][USED_TAG_COUNT+1];

int make_tran_rule(const char* srcFile, const char* dstFilePath);
int make_unigram(char* srcFile, char* dstFilePath);
int make_bigram(char* srcFile, char* dstFilePath);

void test_dict(char* binPath, char* fileName); // test the built dict files

void usage( const char *msg ) 
{
  fprintf(stderr,"Building rule data and other data for chunk parsing module.\n");       
  fprintf(stderr,"Usage: %s -r srcFileName [destFilePath]\n",msg);
  fprintf(stderr,"Usage: %s -1 srcFileName [destFilePath]\n",msg);
  fprintf(stderr,"Usage: %s -2 srcFileName [destFilePath]\n",msg);  
  //fprintf(stderr,"Usage: %s -t dictPath\n",msg); // for internal usage!
}

int main(int argc , char *argv[])
{
  if( argc < 3 )
  {    
    usage(argv[0]);
    return 1;
  }  

  char path[MAX_PATH_LENG] = "./";
  if( argc > 3 )
    strncpy(path,argv[3],strlen(argv[3])+1);

  string opt = argv[1];
  if( opt == "-r" )
    make_tran_rule(argv[2],path); 
  else if( opt == "-1" )
    make_unigram(argv[2],path);
  else if( opt == "-2" )
    make_bigram(argv[2],path);
  else if( opt == "-t" && argc > 3 )
    test_dict(argv[2],argv[3]);
  else
    usage(argv[0]);
  
  return 0;
}

/*
  状态转化表中的元素为一整型数，从低位到高位，第一个字节表示转移到
  的新状态；第二个字节表示哪一个标记更重要；第三个字节表示优先级别,
  第四个字节预留使用。
*/
/// Create transition rule binary dictionary.
int make_tran_rule(const char* srcFile, const char* dstFilePath)
{
  const int MIN_COLS = 2;
  const int MAX_TAGS = 3;

  ifstream in(srcFile);
  if( !in )
  {
    cerr << "error: can't open file " << srcFile << endl;
    return -1;
  }

  string line;
  int linenum = 0;
	while(getline(in,line))
	{	
    linenum++;
    if( line.size() < 8 || line[0] == '#' )
      continue;	
		
		istringstream ls(line);
    string tags[MIN_COLS];

    int i = 0;
    for( ; i<MIN_COLS; i++ )
    {
      ls >> tags[i];
      if( !tags[i].size() )
        break;
    }

    if( i < MIN_COLS )
    {
      cerr << "error: invalid row " << linenum << ", skip: " << line << endl;
      continue;
    }

    unsigned value = get_pos_val(tags[0].c_str());
    int cnt = atoi(tags[1].c_str());

    unsigned tagn[MAX_TAGS] = {0};
    for( i=0; i<cnt; i++ )
    {      
      string tag;
      ls >> tag;
      tagn[i] = get_pos_val(tag.c_str());
      if( !tagn[i] )
        break;       
    }
    
    if( i < cnt )
    {
      cerr << "error: invalid row " << linenum << ", skip: " << line << endl;
      continue;
    }    

    unsigned sign1 = 0;
    unsigned sign2 = 0;
    ls >> sign1 >> sign2;    
    if( sign1 )
    {
      value |= sign1 << 8;
    }    

    if( sign2 )
    {
      value |= sign2 << 16;
    }

    if( cnt == 2 )
      TAG_TRAN_TABLE[0][tagn[0]][tagn[1]]= value;
    else if( cnt == 3 )
      TAG_TRAN_TABLE[tagn[0]][tagn[1]][tagn[2]]= value;
    else
      cerr << "error: invalid row " << linenum << ", skip: " << line << endl;
  }

  // write data out
  string path = string(dstFilePath) + "/" + 
                string(CHUNK_RULE_DICT) + string(DATA_DICT_POSTFIX);
  FILE* fp = fopen(path.c_str(),"wb");
  if( !fp )
  {
    cerr << "error: can't open file " << path << " to save" << endl;
    return -1;
  }  
  int size = USED_TAG_COUNT + 1;
  fwrite(&size,sizeof(int),1,fp);
  fwrite(TAG_TRAN_TABLE,sizeof(int),size*size*size,fp);  
  fclose(fp);

  // for debug
  /*for( int i=1; i<=USED_TAG_COUNT; i++ )
  {
    for( int j=1; j<=USED_TAG_COUNT; j++ )
    {
      int value = TAG_TRAN_TABLE[0][i][j];
      cout << value << "(" << int(value&0xff) << " " << int((value&0xff00)>>8) << " " << int((value&0xff0000)>>16) << ")";
      cout << "\t";
    }    
    cout << endl;
  }*/

  return 0;
}

/*
// Only process bi-transition rule.
int make_tran_rule_bi(const char* srcFile, const char* dstFilePath)
{
  const int MIN_COLS = 3;

  ifstream in(srcFile);
  if( !in )
  {
    cerr << "error: can't open file " << srcFile << endl;
    return -1;
  }

  string line;
  int linenum = 0;
	while(getline(in,line))
	{	
    linenum++;
    if( line.size() < 3 || line[0] == '#' )
      continue;	
		
		istringstream ls(line);
    string tags[MIN_COLS];

    int i = 0;
    for( ; i<MIN_COLS; i++ )
    {
      ls >> tags[i];
      if( !tags[i].size() )
        break;
    }

    if( i < MIN_COLS )
    {
      cerr << "error: invalid row " << linenum << ", jump over: " << line << endl;
      continue;
    }

    unsigned t1 = get_pos_val(tags[0].c_str());
    unsigned t2 = get_pos_val(tags[1].c_str());
    unsigned value = get_pos_val(tags[2].c_str());
    if( !t1 || !t2 )
    {
      cerr << "error: invalid row " << linenum << ", jump over: " << line << endl;
      continue;
    }    

    unsigned sign1 = 0;
    unsigned sign2 = 0;
    ls >> sign1 >> sign2;    
    if( sign1 )
    {
      value |= sign1 << 8;
    }    

    if( sign2 )
    {
      value |= sign2 << 16;
    }

    TAG_TRAN_TABLE[t1][t2]= value;
  }

  // write data out
  string path = string(dstFilePath) + "/" + 
                string(CHUNK_RULE_DICT) + string(DATA_DICT_POSTFIX);
  FILE* fp = fopen(path.c_str(),"wb");
  if( !fp )
  {
    cerr << "error: can't open file " << path << " to save" << endl;
    return -1;
  }  
  int size = USED_TAG_COUNT + 1;
  fwrite(&size,sizeof(int),1,fp);
  fwrite(TAG_TRAN_TABLE,sizeof(int),size*size,fp);  
  fclose(fp);

  // for debug
  for( int i=1; i<=USED_TAG_COUNT; i++ )
  {
    for( int j=1; j<=USED_TAG_COUNT; j++ )
    {
      int value = TAG_TRAN_TABLE[i][j];
      cout << value << "(" << int(value&0xff) << " " << int((value&0xff00)>>8) << " " << int((value&0xff0000)>>16) << ")";
      cout << "\t";
    }    
    cout << endl;
  }

  return 0;
}*/

/*
  辞典节点的code域的高8位保存了idf信息。
  idf范围:[0-16]
 */
/// Create unigram term information binary dictionary.
int make_unigram(char* srcFile, char* dstFilePath)
{
  Sdict_build *sdict;  
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
  if( (sdict = db_creat(nsize, 0)) == NULL ) 
  {
    LOG_ERROR("error: dict struct creation failed");
    return -1;
  }

  // building ...
  const int MIN_COLS = 2;
  ifstream in(srcFile);
  if( !in )
  {
    cerr << "error: can't open file " << srcFile << endl;
    return -1;
  }

  string line;	
  int linenum = 0;
	while(getline(in,line))
	{	
    linenum++;
    if( line.size() < 3 )
      continue;	
		
		istringstream ls(line);
    string cols[MIN_COLS];

    int i = 0;
    for( ; i<MIN_COLS; i++ )
    {
      ls >> cols[i];
      if( !cols[i].size() )
        break;
    }

    if( i < MIN_COLS )
    {
      cerr << "error: invalid row " << linenum << ", skip: " << line << endl;
      continue;
    }

    strncpy(strbuf,cols[0].c_str(),cols[0].size()+1);
    int idf = atoi(cols[1].c_str()); // idf
    
    Sdict_snode dnode;  
    creat_sign_fs64(strbuf, strlen(strbuf), &dnode.sign1, &dnode.sign2);
    //dnode.other = idf;
    dnode.code = idf << 24;  
    db_op1(sdict, &dnode, ADD);    
  }

  // write the index data out
  snprintf(strbuf,512,"%s",TERM_UNIGRAM_DICT);
  if( db_save(sdict, dstFilePath, strbuf) < 0 )
  {
    LOG_ERROR("error: saving indices of term info dictionary failed");
    db_del(sdict);
    return -1;
  }

  // dispose dict
  db_del(sdict);
  
  return 0;
}

/*
  辞典节点的code域保存了词语bigram信息。
  bigram权重范围:[1-16]
 */
/// Create bigram term information binary dictionary.
int make_bigram(char* srcFile, char* dstFilePath)
{
  Sdict_build *sdict;  
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
  if( (sdict = db_creat(nsize, 0)) == NULL ) 
  {
    LOG_ERROR("error: dict struct creation failed");
    return -1;
  }

  // building ...
  const int MIN_COLS = 3;
  ifstream in(srcFile);
  if( !in )
  {
    cerr << "error: can't open file " << srcFile << endl;
    return -1;
  }

  string line;	
  int linenum = 0;
	while(getline(in,line))
	{	
    linenum++;
    if( line.size() < 3 )
      continue;	
		
		istringstream ls(line);
    string cols[MIN_COLS];

    int i = 0;
    for( ; i<MIN_COLS; i++ )
    {
      ls >> cols[i];
      if( !cols[i].size() )
        break;
    }

    if( i < MIN_COLS )
    {
      cerr << "error: invalid row " << linenum << ", skip: " << line << endl;
      continue;
    }

    string term = cols[0] + cols[1]; //"_" + cols[1];
    strncpy(strbuf,term.c_str(),term.size()+1);
    int val = atoi(cols[2].c_str());

    Sdict_snode dnode;  
    creat_sign_fs64(strbuf, strlen(strbuf), &dnode.sign1, &dnode.sign2);
    //dnode.other = val;
    dnode.code = val;
    db_op1(sdict, &dnode, ADD);    
  }

  // write the index data out
  snprintf(strbuf,512,"%s",TERM_BIGRAM_DICT);
  if( db_save(sdict, dstFilePath, strbuf) < 0 )
  {
    LOG_ERROR("error: saving indices of bigram dictionary failed");
    db_del(sdict);
    return -1;
  }

  // dispose dict
  db_del(sdict);
  
  return 0;
}
/// Only for test the built dict files.
void test_dict(char* binPath, char* fileName)
{  
  dict_t* p = ds_load(binPath, fileName);
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

    Sdict_snode dictNode;  
    creat_sign_f64(line, strlen(line), &dictNode.sign1, &dictNode.sign2);
    if( ds_op1(p, &dictNode, SEEK) == 1 )
      cerr << line << "\t" << dictNode.code << endl;
  }

  ds_del(p);
}


