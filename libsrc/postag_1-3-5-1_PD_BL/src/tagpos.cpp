/**
 * @file tagpos.cpp
 * @brief Implementation of part of speech tagging automaticly for Chinese segmented text.
 * @author  David Dai
 * @version 1.0
 * @date 2007/09/27 
 */

#include "ipostag.h"
#include "tagdict.h"

#define MIN_PROB_SUM -1000000.0f

#define BUFFER_SIZE 1024

/* 属性相关宏 */
#define IS_SPACE(property)        ((property.m_lprop) & 0x00000040)
#define IS_ORGNAME(property)      ((property.m_hprop) & 0x00001000)
#define IS_LOC(property)          ((property.m_hprop) & 0x00000800)
#define IS_HUMAN_NAME(property)   ((property.m_hprop) & 0x00000040) 
#define IS_ALNUM(property)        ((property.m_hprop) & 0x00000004) 
#define IS_ASCIINUM(property)     ((property.m_hprop) & 0x00000020) 
#define IS_CNUMBER(property)      ((property.m_hprop) & 0x00000100)

/* 全局词典定义 */
static tag_dict_t* gDict = NULL;

//------------------------------------------------------------------------------------------------------------------//
// <inner>
/* 快速判断是否为ASCII分隔符 */
bool ASCII_DELIM_MAP[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1};
inline bool _is_delim( char c )
{
  return c & 0x80 ? false : ASCII_DELIM_MAP[(unsigned)c];  
}

/* 
   预处理分词序列，以确定某些固定的词性标记。
   token_t的type成员用来存储最终的词性标记值，0表示无效。
   toke_t的index成员在计算时用来存储指向标记信息内存位置的偏移量，负数表示无效。
 */
static void _pretreat(tag_dict_t* pdict, token_t tokens[], uint32_t size )
{
  for( unsigned i=0; i<size; i++ )
  {
    // 如果是具有特殊属性的词，其词性标记自然可以提前确定
    if( (tokens[i].length == 1) && _is_delim(tokens[i].buffer[0]) )
    {
      tokens[i].type = POS_DELIM;
      continue;
    }

    if( IS_SPACE(tokens[i].prop) )
    {
      tokens[i].type = POS_DELIM;
      continue;
    }

    if( IS_ORGNAME(tokens[i].prop) )
    {
      tokens[i].type = POS_ORG;
      continue;
    }

    if( IS_HUMAN_NAME(tokens[i].prop) )
    {
      tokens[i].type = POS_PER;
      continue;
    }

    if( IS_LOC(tokens[i].prop) )
    {
      tokens[i].type = POS_LOC;
      continue;
    }

    if( IS_ALNUM(tokens[i].prop) || IS_ASCIINUM(tokens[i].prop) || IS_CNUMBER(tokens[i].prop)  )
    { 
      tokens[i].type = POS_NUMBER;
      continue;
    }    

    // 如果index小于0，则其词性标记将被提前设定为基本名词
    tokens[i].index = tag_dict_find(pdict,tokens[i].buffer); 
    if( tokens[i].index < 0 )
    {    
      tokens[i].type = POS_DEFAULT; 
      tokens[i].weight |= OUT_OF_TAG; // 不在标注词典中的词特殊标记：最高位为1!
    }
    else
    {
      tokens[i].type = 0;
    }
  }
}

// Viterbi算法求解最优标注序列。
// 程序中涉及频率的地方均已经预先取过以e为底的对数。
static int _viterbi(tag_dict_t* pdict, token_t tokens[], uint32_t size, bool alloc)
{  
  unsigned char* tagInfo = pdict->buffer;
  int lastTagCount, tagCount, tagIdx; 

  // 如果只有一个词语需要标注
  if( size == 1 )
  {
    // 如果词性已经确定
    if( tokens[0].type ) 
      return 1;

    // 否则挑概率最大的标记
    tagIdx = tokens[0].index;    
    tagCount = tagInfo[tagIdx++];
    double max = MIN_PROB_SUM;
    while( tagCount-- > 0 )
    {      
      unsigned tag = tagInfo[tagIdx++];
      double prob = *(double*)(tagInfo+tagIdx);
      tagIdx += sizeof(double);
      //if( max < prob )
      if( prob - max > 0.000001f )
      {
        max = prob;
        tokens[0].type = tag;
      }
    }
    return 1;
  }

  // 预定义缓冲，存放动态规划过程中的中间结果
  double* bufBestScore[BUFFER_SIZE] = {0};
  int bufBackPath[BUFFER_SIZE+1] = {0};
  double** bestScore = bufBestScore;
  int* backPath = bufBackPath;
  if( alloc )
  {    
    bestScore = new double*[size];
    backPath = new int[size+1];

    // meet error
    if( !bestScore || !backPath )
    {
      LOG_ERROR("error: allocating memory failed");
      size = 0;
      goto end;
    }    
  }

  // 对于序列中的首词，假定其前面的词为标点，初始化发射概率
  tagCount = 1;  
  tagIdx = tokens[0].index;
  if( !tokens[0].type )
    tagCount = tagInfo[tagIdx++];  
  bestScore[0] = new double[tagCount];

  //meet error
  if( !bestScore[0] )
  {
    LOG_ERROR("error: allocating memory failed");
    size = 0;
    goto end;
  }

  for( int j=0; j<tagCount; j++ )
  {    
    double curTagProb = 0.0;            
    unsigned curTag = tokens[0].type;
    if( !tokens[0].type ) 
    {
      curTag = tagInfo[tagIdx++];
      curTagProb = *(double*)(tagInfo+tagIdx);
      tagIdx += sizeof(double);
    }

    // lnP(ti|ri)
    double relativeCost = curTagProb - POS_TAG_PROB[curTag-1];
    // lnP(ri|r(i-1))
    double transProb = TRANS_PROB_MATRIX[POS_DELIM-1][curTag-1] - POS_TAG_PROB[POS_DELIM-1];          
    // lnP(ti|ri)+lnP(ri|r(i-1))
    bestScore[0][j] = relativeCost + transProb;  
  }

  lastTagCount = tagCount;

  // 对于序列中非首尾词，进行迭代计算
  for( unsigned i=1; i<size; i++ )
  {  
    tagCount = 1;
    tagIdx = tokens[i].index;
    if( !tokens[i].type )
      tagCount = tagInfo[tagIdx++];
    bestScore[i] = new double[tagCount];

    // meet error
    if( !bestScore[i] )
    {
      LOG_ERROR("error: allocating memory failed");
      for( unsigned si=0; si<i; si++ )
        delete[] bestScore[si];  
      size = 0;
      goto end;
    }

    for( int j=0; j<tagCount; j++ ) 
    { 
      double curTagProb = 0.0;            
      unsigned curTag = tokens[i].type;
      if( !tokens[i].type ) 
      {
        curTag = tagInfo[tagIdx++];
        curTagProb = *(double*)(tagInfo+tagIdx);
        tagIdx += sizeof(double);
      }

      // lnP(ti|ri)
      double relativeCost = curTagProb - POS_TAG_PROB[curTag-1];

      // 计算当前词语的前一个词语的最优词性标记
      double max = MIN_PROB_SUM;
      int lastTagIdx = tokens[i-1].index + 1;
      for( int k=0; k<lastTagCount; k++ ) 
      {        
        double lastTagProb = 0.0;
        unsigned lastTag = tokens[i-1].type;        
        if( !tokens[i-1].type ) 
        {
          lastTag = tagInfo[lastTagIdx++];
          lastTagProb = *(double*)(tagInfo+lastTagIdx);
          lastTagIdx += sizeof(double);
        }

        // ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))]
        double transProb= TRANS_PROB_MATRIX[lastTag-1][curTag-1] - POS_TAG_PROB[lastTag-1];
        double score = bestScore[i-1][k] + relativeCost + transProb; // 当前累计概率

        //if( max < score )
        if( score - max > 0.000001f )
        {
          max = score;
          backPath[i] = lastTag; // 存储前一个词语的最优词性标记值          
        }          
      }

      // bestScore(i,j) = max{ ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))] }
      bestScore[i][j] = max;      
    }

    lastTagCount = tagCount;
  } 

  // 如果最后一个词词性不确定，则虚拟一个结尾标点，计算最后一个词的词性标记
  if( !tokens[size-1].type )
  {    
    // P(ti|ri)
    double relativeCost = 1 - POS_TAG_PROB[POS_DELIM-1]; // 假定标点频率为e
    
    // 计算当前词语的前一个词语的最优词性标记
    double max = MIN_PROB_SUM;
    int lastTagIdx = tokens[size-1].index + 1;
    for( int k=0; k<lastTagCount; k++ ) 
    {        
      double lastTagProb = 0.0;
      unsigned lastTag = tokens[size-1].type;        
      if( !tokens[size-1].type ) 
      {
        lastTag = tagInfo[lastTagIdx++];
        lastTagProb = *(double*)(tagInfo+lastTagIdx);
        lastTagIdx += sizeof(double);
      }

      // ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))]
      double transProb= TRANS_PROB_MATRIX[lastTag-1][POS_DELIM-1] - POS_TAG_PROB[lastTag-1];
      double score = bestScore[size-1][k] + relativeCost + transProb;

      //if( max < score )
      if( score - max > 0.000001f )
      {
        max = score;
        backPath[size] = lastTag; // 存储前一个词语的最优词性标记值          
      }          
    }
  } // 否则，最后一个词性标记不需要计算
  

  // 回溯获取词语序列的最优词性标记
  for( int i=size ;i>0; i-- )
  {
    if( !tokens[i-1].type )
      tokens[i-1].type = backPath[i];    
  }

  // 释放临时资源
  for( unsigned i=0; i<size; i++ )
    delete[] bestScore[i];

 end: 
  if( alloc )
  {
    delete[] bestScore;
    delete[] backPath;
  }
 
  return size;
}
// 未对内存申请进行检查的版本，效率高！
/*static int _viterbi( token_t tokens[], uint32_t size ) 
{  
  unsigned char* tagInfo = gDict->buffer;
  int lastTagCount, tagCount, tagIdx;  

  // 如果只有一个词语需要标注
  if( size == 1 )
  {
    // 如果词性已经确定
    if( tokens[0].type ) 
      return 1;

    // 否则挑概率最大的标记
    tagIdx = tokens[0].index;    
    tagCount = tagInfo[tagIdx++];
    double max = MIN_PROB_SUM;
    while( tagCount-- > 0 )
    {      
      unsigned tag = tagInfo[tagIdx++];
      double prob = *(double*)(tagInfo+tagIdx);
      tagIdx += sizeof(double);
      //if( max < prob )
      if( prob - max > 0.000001f )
      {
        max = prob;
        tokens[0].type = tag;
      }
    }
    return 1;
  }

  // 存放动态规划过程中的中间结果
  double** bestScore = new double*[size];
  int* backPath = new int[size+1];

  // 对于序列中的首词，假定其前面的词为标点，初始化发射概率
  tagCount = 1;  
  tagIdx = tokens[0].index;
  if( !tokens[0].type )
    tagCount = tagInfo[tagIdx++];  
  bestScore[0] = new double[tagCount]; 

  for( int j=0; j<tagCount; j++ )
  {    
    double curTagProb = 0.0;            
    unsigned curTag = tokens[0].type;
    if( !tokens[0].type ) 
    {
      curTag = tagInfo[tagIdx++];
      curTagProb = *(double*)(tagInfo+tagIdx);
      tagIdx += sizeof(double);
    }

    // lnP(ti|ri)
    double relativeCost = curTagProb - POS_TAG_PROB[curTag-1];
    // lnP(ri|r(i-1))
    double transProb = TRANS_PROB_MATRIX[POS_DELIM-1][curTag-1] - POS_TAG_PROB[POS_DELIM-1];          
    // lnP(ti|ri)+lnP(ri|r(i-1))
    bestScore[0][j] = relativeCost + transProb;  
  }

  lastTagCount = tagCount;

  // 对于序列中非首尾词，进行迭代计算
  for( unsigned i=1; i<size; i++ )
  {  
    tagCount = 1;
    tagIdx = tokens[i].index;
    if( !tokens[i].type )
      tagCount = tagInfo[tagIdx++];
    bestScore[i] = new double[tagCount];

    for( int j=0; j<tagCount; j++ ) 
    { 
      double curTagProb = 0.0;            
      unsigned curTag = tokens[i].type;
      if( !tokens[i].type ) 
      {
        curTag = tagInfo[tagIdx++];
        curTagProb = *(double*)(tagInfo+tagIdx);
        tagIdx += sizeof(double);
      }

      // lnP(ti|ri)
      double relativeCost = curTagProb - POS_TAG_PROB[curTag-1];

      // 计算当前词语的前一个词语的最优词性标记
      double max = MIN_PROB_SUM;
      int lastTagIdx = tokens[i-1].index + 1;
      for( int k=0; k<lastTagCount; k++ ) 
      {        
        double lastTagProb = 0.0;
        unsigned lastTag = tokens[i-1].type;        
        if( !tokens[i-1].type ) 
        {
          lastTag = tagInfo[lastTagIdx++];
          lastTagProb = *(double*)(tagInfo+lastTagIdx);
          lastTagIdx += sizeof(double);
        }

        // ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))]
        double transProb= TRANS_PROB_MATRIX[lastTag-1][curTag-1] - POS_TAG_PROB[lastTag-1];
        double score = bestScore[i-1][k] + relativeCost + transProb; // 当前累计概率

        //if( max < score )
        if( score - max > 0.000001f )
        {
          max = score;
          backPath[i] = lastTag; // 存储前一个词语的最优词性标记值          
        }          
      }

      // bestScore(i,j) = max{ ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))] }
      bestScore[i][j] = max;      
    }

    lastTagCount = tagCount;
  } 

  // 如果最后一个词词性不确定，则虚拟一个结尾标点，计算最后一个词的词性标记
  if( !tokens[size-1].type )
  {    
    // P(ti|ri)
    double relativeCost = 1 - POS_TAG_PROB[POS_DELIM-1]; // 假定标点频率为e
    
    // 计算当前词语的前一个词语的最优词性标记
    double max = MIN_PROB_SUM;
    int lastTagIdx = tokens[size-1].index + 1;
    for( int k=0; k<lastTagCount; k++ ) 
    {        
      double lastTagProb = 0.0;
      unsigned lastTag = tokens[size-1].type;        
      if( !tokens[size-1].type ) 
      {
        lastTag = tagInfo[lastTagIdx++];
        lastTagProb = *(double*)(tagInfo+lastTagIdx);
        lastTagIdx += sizeof(double);
      }

      // ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))]
      double transProb= TRANS_PROB_MATRIX[lastTag-1][POS_DELIM-1] - POS_TAG_PROB[lastTag-1];
      double score = bestScore[size-1][k] + relativeCost + transProb;

      //if( max < score )
      if( score - max > 0.000001f )
      {
        max = score;
        backPath[size] = lastTag; // 存储前一个词语的最优词性标记值          
      }          
    }
  } // 否则，最后一个词性标记不需要计算
  

  // 回溯获取词语序列的最优词性标记
  for( int i=size ;i>0; i-- )
  {
    if( !tokens[i-1].type )
      tokens[i-1].type = backPath[i];    
  }

  // 释放临时资源
  for( unsigned i=0; i<size; i++ )
    delete[] bestScore[i];
  delete[] bestScore;
  delete[] backPath;

  return size;
}*/
// </inner>


//------------------------------------------------------------------------------------------------------------------//
// <api i>
int tag_open( const char* dictpath )
{
  if( gDict != NULL )
    tag_dict_free(gDict);

  if( strlen(dictpath) >= MAX_PATH_LENG - 1 )
  {
    LOG_ERROR("error: length of path is too long");
    return -1;
  }

  char path[MAX_PATH_LENG] = {0};  
  strncpy(path,dictpath,strlen(dictpath)+1);
  strncat(path,"/",1);

  gDict = tag_dict_load( path );
  
  return (gDict == NULL);
}

int tag_postag( token_t tokens[], uint32_t size )
{
  /*if( !size )
    return 0;

  bool alloc = (size > BUFFER_SIZE);
  long buffer[BUFFER_SIZE] = {0};
  long* pbuf = buffer;
  int ret = 0;

  if( alloc )
  {    
    pbuf = new long[size];
    if( !pbuf )
    {
      LOG_ERROR("error: allocating memory failed");
      return 0;
    }
  }
  // 保存数据  
  for( unsigned i=0; i<size; i++ )
    pbuf[i] = tokens[i].index;

  _pretreat(gDict,tokens,size);
  ret = _viterbi(gDict,tokens,size,alloc);

  // 恢复数据
  for( unsigned i=0; i<size; i++ )
    tokens[i].index = pbuf[i];

  if( alloc )
  {
    delete[] pbuf;
  }

  return ret;*/

  return tag_postag(gDict,tokens,size);
}

void tag_close()
{
  if( gDict != NULL )
  {
    tag_dict_free(gDict);
    gDict = NULL;
  }
}
// </api i>


//------------------------------------------------------------------------------------------------------------------//
// <api ii>

tag_dict_t* tag_create(const char* dictpath)
{
  if( strlen(dictpath) >= MAX_PATH_LENG - 1 )
  {
    LOG_ERROR("error: length of path is too long");
    return NULL;
  }

  char path[MAX_PATH_LENG] = {0};  
  strncpy(path,dictpath,strlen(dictpath)+1);
  strncat(path,"/",1);

  return tag_dict_load( path );
}

int tag_postag(tag_dict_t* pdict, token_t tokens[], uint32_t size )
{
  if( !size || !pdict )
    return 0;

  bool alloc = (size > BUFFER_SIZE);
  long buffer[BUFFER_SIZE] = {0};
  long* pbuf = buffer;
  int ret = 0;

  if( alloc )
  {    
    pbuf = new long[size];
    if( !pbuf )
    {
      LOG_ERROR("error: allocating memory failed");
      return 0;
    }
  }
  // 保存数据  
  for( unsigned i=0; i<size; i++ )
    pbuf[i] = tokens[i].index;

  _pretreat(pdict,tokens,size);
  ret = _viterbi(pdict,tokens,size,alloc);

  // 恢复数据
  for( unsigned i=0; i<size; i++ )
    tokens[i].index = pbuf[i];

  if( alloc )
  {
    delete[] pbuf;
  }

  return ret;
}

void tag_destroy( tag_dict_t* p )
{
  if( p != NULL )
  {
    tag_dict_free(p);
    p = NULL;
  }
}
// </api ii>

