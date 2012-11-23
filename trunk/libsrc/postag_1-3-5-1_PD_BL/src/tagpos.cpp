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

/* ������غ� */
#define IS_SPACE(property)        ((property.m_lprop) & 0x00000040)
#define IS_ORGNAME(property)      ((property.m_hprop) & 0x00001000)
#define IS_LOC(property)          ((property.m_hprop) & 0x00000800)
#define IS_HUMAN_NAME(property)   ((property.m_hprop) & 0x00000040) 
#define IS_ALNUM(property)        ((property.m_hprop) & 0x00000004) 
#define IS_ASCIINUM(property)     ((property.m_hprop) & 0x00000020) 
#define IS_CNUMBER(property)      ((property.m_hprop) & 0x00000100)

/* ȫ�ִʵ䶨�� */
static tag_dict_t* gDict = NULL;

//------------------------------------------------------------------------------------------------------------------//
// <inner>
/* �����ж��Ƿ�ΪASCII�ָ��� */
bool ASCII_DELIM_MAP[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1};
inline bool _is_delim( char c )
{
  return c & 0x80 ? false : ASCII_DELIM_MAP[(unsigned)c];  
}

/* 
   Ԥ����ִ����У���ȷ��ĳЩ�̶��Ĵ��Ա�ǡ�
   token_t��type��Ա�����洢���յĴ��Ա��ֵ��0��ʾ��Ч��
   toke_t��index��Ա�ڼ���ʱ�����洢ָ������Ϣ�ڴ�λ�õ�ƫ������������ʾ��Ч��
 */
static void _pretreat(tag_dict_t* pdict, token_t tokens[], uint32_t size )
{
  for( unsigned i=0; i<size; i++ )
  {
    // ����Ǿ����������ԵĴʣ�����Ա����Ȼ������ǰȷ��
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

    // ���indexС��0��������Ա�ǽ�����ǰ�趨Ϊ��������
    tokens[i].index = tag_dict_find(pdict,tokens[i].buffer); 
    if( tokens[i].index < 0 )
    {    
      tokens[i].type = POS_DEFAULT; 
      tokens[i].weight |= OUT_OF_TAG; // ���ڱ�ע�ʵ��еĴ������ǣ����λΪ1!
    }
    else
    {
      tokens[i].type = 0;
    }
  }
}

// Viterbi�㷨������ű�ע���С�
// �������漰Ƶ�ʵĵط����Ѿ�Ԥ��ȡ����eΪ�׵Ķ�����
static int _viterbi(tag_dict_t* pdict, token_t tokens[], uint32_t size, bool alloc)
{  
  unsigned char* tagInfo = pdict->buffer;
  int lastTagCount, tagCount, tagIdx; 

  // ���ֻ��һ��������Ҫ��ע
  if( size == 1 )
  {
    // ��������Ѿ�ȷ��
    if( tokens[0].type ) 
      return 1;

    // �������������ı��
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

  // Ԥ���建�壬��Ŷ�̬�滮�����е��м���
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

  // ���������е��״ʣ��ٶ���ǰ��Ĵ�Ϊ��㣬��ʼ���������
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

  // ���������з���β�ʣ����е�������
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

      // ���㵱ǰ�����ǰһ����������Ŵ��Ա��
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
        double score = bestScore[i-1][k] + relativeCost + transProb; // ��ǰ�ۼƸ���

        //if( max < score )
        if( score - max > 0.000001f )
        {
          max = score;
          backPath[i] = lastTag; // �洢ǰһ����������Ŵ��Ա��ֵ          
        }          
      }

      // bestScore(i,j) = max{ ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))] }
      bestScore[i][j] = max;      
    }

    lastTagCount = tagCount;
  } 

  // ������һ���ʴ��Բ�ȷ����������һ����β��㣬�������һ���ʵĴ��Ա��
  if( !tokens[size-1].type )
  {    
    // P(ti|ri)
    double relativeCost = 1 - POS_TAG_PROB[POS_DELIM-1]; // �ٶ����Ƶ��Ϊe
    
    // ���㵱ǰ�����ǰһ����������Ŵ��Ա��
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
        backPath[size] = lastTag; // �洢ǰһ����������Ŵ��Ա��ֵ          
      }          
    }
  } // �������һ�����Ա�ǲ���Ҫ����
  

  // ���ݻ�ȡ�������е����Ŵ��Ա��
  for( int i=size ;i>0; i-- )
  {
    if( !tokens[i-1].type )
      tokens[i-1].type = backPath[i];    
  }

  // �ͷ���ʱ��Դ
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
// δ���ڴ�������м��İ汾��Ч�ʸߣ�
/*static int _viterbi( token_t tokens[], uint32_t size ) 
{  
  unsigned char* tagInfo = gDict->buffer;
  int lastTagCount, tagCount, tagIdx;  

  // ���ֻ��һ��������Ҫ��ע
  if( size == 1 )
  {
    // ��������Ѿ�ȷ��
    if( tokens[0].type ) 
      return 1;

    // �������������ı��
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

  // ��Ŷ�̬�滮�����е��м���
  double** bestScore = new double*[size];
  int* backPath = new int[size+1];

  // ���������е��״ʣ��ٶ���ǰ��Ĵ�Ϊ��㣬��ʼ���������
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

  // ���������з���β�ʣ����е�������
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

      // ���㵱ǰ�����ǰһ����������Ŵ��Ա��
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
        double score = bestScore[i-1][k] + relativeCost + transProb; // ��ǰ�ۼƸ���

        //if( max < score )
        if( score - max > 0.000001f )
        {
          max = score;
          backPath[i] = lastTag; // �洢ǰһ����������Ŵ��Ա��ֵ          
        }          
      }

      // bestScore(i,j) = max{ ln[bestScore(i-1,k)*P(ti|rj)*P(rj|r(j-1,k))] }
      bestScore[i][j] = max;      
    }

    lastTagCount = tagCount;
  } 

  // ������һ���ʴ��Բ�ȷ����������һ����β��㣬�������һ���ʵĴ��Ա��
  if( !tokens[size-1].type )
  {    
    // P(ti|ri)
    double relativeCost = 1 - POS_TAG_PROB[POS_DELIM-1]; // �ٶ����Ƶ��Ϊe
    
    // ���㵱ǰ�����ǰһ����������Ŵ��Ա��
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
        backPath[size] = lastTag; // �洢ǰһ����������Ŵ��Ա��ֵ          
      }          
    }
  } // �������һ�����Ա�ǲ���Ҫ����
  

  // ���ݻ�ȡ�������е����Ŵ��Ա��
  for( int i=size ;i>0; i-- )
  {
    if( !tokens[i-1].type )
      tokens[i-1].type = backPath[i];    
  }

  // �ͷ���ʱ��Դ
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
  // ��������  
  for( unsigned i=0; i<size; i++ )
    pbuf[i] = tokens[i].index;

  _pretreat(gDict,tokens,size);
  ret = _viterbi(gDict,tokens,size,alloc);

  // �ָ�����
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
  // ��������  
  for( unsigned i=0; i<size; i++ )
    pbuf[i] = tokens[i].index;

  _pretreat(pdict,tokens,size);
  ret = _viterbi(pdict,tokens,size,alloc);

  // �ָ�����
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

