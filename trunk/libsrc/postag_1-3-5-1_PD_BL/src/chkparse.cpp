/**
 * @file chkparse.cpp
 * @author David Dai, Shanow Lin, HaiLei Zhang 
 * @date Mon Oct  6 10:08:15 CST 2008
 * @version 1.0.1 
 * @brief 对输入的文本信息进行组块分析，对term 进行属性标注，通过组块信息表述term 间的关系。
 */

#include "iparse.h"
#include "tagdict.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ul_string.h>

/* 相关宏定义 */
#define GET_CHUNK_STATE(value) ( value & 0xFF)
#define GET_CHUNK_HEAD(value)  ((value & 0xFF00) >> 8 )
#define GET_CHUNK_PRIOR(value) ((value & 0xFF0000) >> 16 )

#define GET_TERM_TF(value)     ((value) & 0x00FFFFFF )
#define GET_TERM_IDF(value)    ( value >> 24 )
#define GET_TERM_ATTRIBUTE(value) ((value & 0x00FF0000) >> 16 )

#define BUFFER_SIZE MAX_PARSED_COUNT*4
#define MAX_BIGRAM_LENG 512
#define MAX_TRANS_PROB 16 // 词性标记转移概率的最大值
#define MAX_IDF 16 

/*专名属性*/
#define SNG 42
#define NVL 43
#define SFT 45
#define GME 46 
#define LOC 47
#define STE 48
#define ORGNAME 49
#define PER 50
#define VDO_MVE 51
#define VDO_TLP 52

#define FTR_NVL_SN 53
#define FTR_VDO_SN 54
#define FTR_MP3 55
#define FTR_NVL 56		
#define FTR_VDO 57		
#define FTR_PDT 58
#define FTR_IMG 59
#define FTR_DOC 60
#define FTR_SFT 61
#define FTR_GME 62
#define FTR_VN 63
#define FTR_STE 64
#define NOR_Q 65
#define NOR_T 66
#define NOR_N 67

/*相关属性标记定义*/
#define VERB 1
#define OBJECT 2
#define PRODUCT 3
#define ILL 4
#define NUMS 5
#define NZ 6
#define ORG 7
#define BASE_NP 8 //定义基本名词性元素；在没有命中属性时提供 
#define BASE_VP 9

/*相关词性标记定义*/
#define VN 36
#define V 34
#define N 21
#define NR 22
#define NT 24
#define ZN 26
#define Vg 5
#define Ng 3
#define Ag 1
#define A 6
#define Dg 2
#define G 14
#define K 18
#define W 37
#define C 10 //连词
#define P 28
#define U 33
#define M 20 //数词
#define Q 29//量词
#define J 17 //简略语
#define NT 24//机构团体

/*粘接关系的范围33-63*/
#define CONJ_N 33 //粘接后的名词元素
#define CONJ_A 34 //粘接后的形容词元素
#define CONJ_PRO 35 //粘接后的实体属性

/*定义中心成分类型*/
#define VP_CENTER 1
#define NP_CENTER 2
#define BASE_PARSE 3
#define ORG_CENTER 4

/*定义上下位类型*/
#define OBJECT_OTHER 1 //商品与型号/品牌
#define NZ_OTHER 2 //品牌与商品/型号
#define NUM_OTHER 3 // 型号与商品/ 品牌
#define NER_OTHER 4 //专名上下位
#define REQUEST_OTHER 5 //需求上下位

/*定义上下位关系:范围9-15*/
#define UP_DOWN 9
#define DOWN_UP 10
#define UP_PRODUCT 11
#define DOWN_PRODUCT 12
#define NVL_SN 13 //小说章节
#define MVE_SN 14 //电影
#define TLP_SN 15 //电视
#define SFT_VN 16 //软件
#define GME_VN 18 //游戏

/*定义修饰关系和动宾关系*/
#define BASENP_CENTER 5 //修饰性关系范围:5-7
#define REQUEST_RELATE 8 //专名与需求词间的需求关系
#define BASEVP_CENTER 17 //动宾关系范围:17-31
#define DEFALT_RELATE 3 //普通组块

#define MAX_VP_IDF_BASE 0 //定义最小的baseVp 的idf
#define MAX_NP_IDF_BASE 2

/*定义weight 值 */
#define VN_CHUNK_W 9
#define UP_CHUNK_W 8
#define XS_CHUNK_W 8
#define QT_CHUNK_W 5 //chunk_depth
#define ZJ_CHUNK_W 4
#define ILL_W 9
#define DEFALT_W 2
#define BASE_NP_W 8
#define BASE_VP_W 4 //除了意图属性意外其他的baseVp
#define NZ_W 8
#define ATTRI_W 6
#define ZHUANMING_W 6 //专名组块权重

#define CHUNK_W_STATE 1
#define TERM_W_STATE 2

/*
  Chunk parsing所需辞典结构
  为了提高效率，二元状态转化是根据两个状态作为下标直接寻址；
  而三元状态转化是把三个状态拼合成一个整数在数组中进行查找。
 */
typedef struct chunk_dict_t
{  
  unsigned int state_size;    // 二元三元状态转化表缓冲大小
  unsigned int* state_buffer; // 状态转移表信息记录在这个缓冲中，对应文法规则词典，是必需的

  dict_t*  unigram_dict; // 词语本身的信息，主要是idf,tf等
  dict_t*  bigram_dict;  // 词语二元关系信息，主要是两个词语的共现cooccurrence
} chunk_dict_t;

/*
  用于存储baseVp 和baseNp 的信息
*/
typedef struct base_info_t
{
	unsigned int baseNp_num;//基本名词成分的个数
	unsigned int baseVp_num;//基本动词成分个数

	int baseNp[MAX_PARSED_COUNT];//用于记录baseVp 的token 索引，动宾关系
	int baseVp[MAX_PARSED_COUNT];//用于记录baseNp 的token 索引
	int baseOp;	//用于记录baseOp 的token 索引，机构关系;针对拍卖词;默认一个拍卖词只描述一个机构类型
}base_info_t;

/* 全局Chunk词典定义 */
static chunk_dict_t* gChkDict = NULL;

/**
* @brief 根据token 信息获取term 的专名属性
* @param prop - 存放在token.prop.m_hprop 中的专名属性
* @return 专名属性值，如果没有专名属性，则返回0
*/
inline int _token_prop2state( uint32_t prop )
{	
	if (prop == 0x00000002)
		return SNG;
	if (prop == 0x00000004)
		return NVL; 
	if (prop == 0x00000008)
		return NZ;
	if (prop == 0x00000010) 
		return SFT;
	if (prop == 0x00000020) 
		return GME;
	if (prop == 0x00000040)
		return LOC;
	if (prop == 0x00000080) 
		return STE;
	if (prop == 0x00000100) 
		return PRODUCT;
	if (prop == 0x00000200) 
		return ORGNAME;
	if (prop == 0x00000400) 
		return PER;
	if (prop == 0x00000800) //TAG_NE_VDO_MVE
		return VDO_MVE;
	if (prop == 0x00001000)
		return VDO_TLP;
	if (prop == 0x00002000) 
		return ORG;
	if (prop == 0x00004000) 
		return ILL;
	if (prop == 0x00008000) 
		return VERB;	
	if (prop == 0x00010000) 
		return FTR_MP3; 	
	if (prop == 0x00020000) 
		return FTR_NVL; 	
	if (prop == 0x00040000) 
		return FTR_VDO; 	
	if (prop == 0x00080000) 
		return FTR_PDT;
	if (prop == 0x00100000) 
		return FTR_IMG;
	if (prop == 0x00200000) 
		return FTR_DOC;
	if (prop == 0x00400000) 
		return FTR_SFT;
	if (prop == 0x00800000) 
		return FTR_GME;
	if (prop == 0x01000000) 
		return FTR_NVL_SN;	
	if (prop == 0x02000000) 
		return FTR_VDO_SN;
	if (prop == 0x04000000)
		return NUMS;
	if (prop == 0x08000000)
		return FTR_VN;
	if (prop == 0x10000000)
		return FTR_STE;
	if (prop == 0x20000000) 	
		return NOR_Q;
	if (prop == 0x40000000)
		return NOR_T;
	if (prop == 0x80000000)
		return NOR_N;
			
	return 0;	
}

/**
* @brief get the bi-trasition state
* @param state1 - the pos state of chunk1
* @param state2 - the pos state of chunk2
* @return The bi-trasition state by two chunks, if miss hit, return 0
*/
inline int _state_transit(int state1, int state2) 
{
  if( state1 < 1 || state1 > USED_TAG_COUNT ||
      state2 < 1 || state2 > USED_TAG_COUNT )
      return 0;

  return gChkDict->state_buffer[state1*gChkDict->state_size+state2];
}

/**
* @brief get the tri-trasition state
* @param state1 - the pos state of chunk1
* @param state2 - the pos state of chunk2
* @param state3 - the pos state of chunk3
* @return The tri-trasition state by three chunks, if miss hit, return 0
*/
inline int _state_transit(int state1, int state2, int state3) 
{
  if( state1 < 1 || state1 > USED_TAG_COUNT ||
      state2 < 1 || state2 > USED_TAG_COUNT ||
      state3 < 1 || state3 > USED_TAG_COUNT )
      return 0;

  int idx = state1*gChkDict->state_size*gChkDict->state_size + \
            state2*gChkDict->state_size + state3;
  return gChkDict->state_buffer[idx];
}

/**
* @brief 获得单term 的idf 信息
* @param key - string value of the term
* @return The unigram term info - idf, if miss match, return -1
*/
inline int _unigram_get(char* key)
{
  if( !gChkDict->unigram_dict || !key )
    return -1;

  Sdict_snode dictNode;  
  creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
  if( ds_op1(gChkDict->unigram_dict, &dictNode, SEEK) == 1 )
    return dictNode.code;

  return -1;
}

/**
* @brief 获得bigram 的信息，若为一个term 则取idf，否则取搭配强度
* @param key - string value of the conjection of two terms
* @return The bigram term info - word integrative intensity, if miss match, return 0
*/
inline int _bigram_get(char* key)
{
  if( !key )
    return 0;

  Sdict_snode dictNode;
  if( gChkDict->unigram_dict )
  {
    creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
    if( ds_op1(gChkDict->unigram_dict, &dictNode, SEEK) == 1 )
      return GET_TERM_IDF(dictNode.code);
  }

  if( !gChkDict->bigram_dict )
    return 0;
  
  creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
  if( ds_op1(gChkDict->bigram_dict, &dictNode, SEEK) == 1 )
    return dictNode.code;

  return 0;
}

/**
* @brief 获得两个term 的搭配强度信息
* @param term1 - string value of term1
* @param term2 - string value of term2
* @return The bigram term info - word integrative intensity, if miss match, return 0
*/
inline int _bigram_get(char* term1, char* term2)
{
  if( !term1 || !term2 )
    return 0;

  int len1 = strlen(term1);
  int len2 = strlen(term2);
  char key[MAX_BIGRAM_LENG] = {0};

  if( len1 > 255 )
    len1 = 255;
  if( len2 > 255 )
    len2 = 255;

  if( strcmp(term1,term2) > 0 )
  {
    memcpy(key,term2,len2);
    memcpy(key+len2,term1,len1);
  }
  else
  {
    memcpy(key,term1,len1);
    memcpy(key+len1,term2,len2);
  }
  //assert( len1 + len2 < MAX_BIGRAM_LENG ); // assert!

  Sdict_snode dictNode;
  if( gChkDict->unigram_dict )
  {
    creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
    if( ds_op1(gChkDict->unigram_dict, &dictNode, SEEK) == 1 )
      return GET_TERM_IDF(dictNode.code);
  }  

  if( !gChkDict->bigram_dict )
    return 0;
   
  creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
  if( ds_op1(gChkDict->bigram_dict, &dictNode, SEEK) == 1 )
    return dictNode.code;

  return 0;
}

/**
* @brief 合并两个子chunk 到一个新的组块
* @param p - new chunk
* @param chunk1 - chunk to be merged
* @param chunk2 - the other chunk to be merged
* @return
*/
static void _head_merge(chunk_t* p, chunk_t* chunk1, chunk_t* chunk2)
{    
  memcpy(p->head_buffer,chunk1->head_buffer,chunk1->head_count*sizeof(int)); 
  p->head_count = chunk1->head_count;
  int last = p->head_buffer[p->head_count-1];
  for( int i=0; i<chunk2->head_count; i++ )
  {
    if( p->head_count == MAX_HEAD_COUNT )
      break;
    if( chunk2->head_buffer[i] > last )
      p->head_buffer[p->head_count++] = chunk2->head_buffer[i];
  }
  //assert(p->head_count < MAX_HEAD_COUNT); // assert!
}

/**
* @brief 合并三个子chunk 到一个新的组块
* @param p - new chunk
* @param chunk1 - chunk to be merged
* @param chunk2 - chunk to be merged
* @param chunk3 - chunk to be merged
* @return
*/
static void _head_merge(chunk_t* p, chunk_t* chunk1, chunk_t* chunk2, chunk_t* chunk3)
{    
  memcpy(p->head_buffer,chunk1->head_buffer,chunk1->head_count*sizeof(int)); 
  p->head_count = chunk1->head_count;
  int last = p->head_buffer[p->head_count-1];
  for( int i=0; i<chunk2->head_count; i++ )
  {
    if( p->head_count == MAX_HEAD_COUNT )
      break;
    if( chunk2->head_buffer[i] > last )
      p->head_buffer[p->head_count++] = chunk2->head_buffer[i];
  }
  last = p->head_buffer[p->head_count-1];
  for( int i=0; i<chunk3->head_count; i++ )
  {
    if( p->head_count == MAX_HEAD_COUNT )
      break;
    if( chunk3->head_buffer[i] > last )
      p->head_buffer[p->head_count++] = chunk3->head_buffer[i];
  }
  //assert(p->head_count < MAX_HEAD_COUNT); // assert!
}

/**
* @brief 获得组块的字符串信息
* @param p - chunk to be analyze
* @param tokens - tokens info of input
* @param buffer - the space to store the string of chunk
* @param buflen - the size of the buffer space 
* @return the length of chunk's string value
*/
static int _chunk_getstr(chunk_t* p, token_t* tokens, char buffer[], int buflen)
{
  if( !p ) 
    return 0;

  int leng = 0;
  for( int j=0; j<p->head_count; j++ )
  {
    int k = p->head_buffer[j];
    if( (leng + (int)tokens[k].length) >= buflen )
      break;
    strncpy(buffer+leng,tokens[k].buffer,tokens[k].length+1);
    leng += tokens[k].length;
  }
  return leng;
}

/**
* @brief 新组块的权重，该权重由子组块的搭配强度及词性转移概率决定
* @param p1 - sub-chunk1
* @param t1 - string value of sub-chunk1
* @param p2 - sub-chunk2
* @param t2 - string value of sub-chunk2
* @return the weight decided by two sub-chunks
*/
static double _bigram_weight(chunk_t* p1, char* t1, chunk_t* p2, char* t2)
{
	// 如果无共现信息，则采用句法规则概率，否则加入共现概率
	double weight = TRANS_PROB_MATRIX[p1->head_state-1][p2->head_state-1];
	int rank = _bigram_get(t1,t2);    
	weight += rank; 
	return weight;
}

/**
* @brief 新组块的权重，该权重由识别的组块关系及term 属性所决定
* @param p1 - chunk info
* @param flag - to tag the chunk is a real chunk, or a base term
* @return the weight decided by chunk/term state
*/
static unsigned int _chunk_weight(chunk_t* p1, int flag)
{
  if (flag == CHUNK_W_STATE)
  {
	  if (p1->chunk_state == BASEVP_CENTER)
	  	return VN_CHUNK_W;
	  if (p1->chunk_state == UP_DOWN || p1->chunk_state == DOWN_UP || p1->chunk_state == UP_PRODUCT
	  		|| p1->chunk_state == DOWN_PRODUCT)
	  	return UP_CHUNK_W;
	  if (p1->chunk_state == BASENP_CENTER)
	  	return XS_CHUNK_W;
  }
  //term weight
  if (flag == TERM_W_STATE)
  {
	if (p1->weight == BASE_NP_W)
		return BASE_NP_W;
  	if (p1->chunk_state == BASE_NP)
		return BASE_NP_W;
	if (p1->chunk_state == BASE_VP || p1->chunk_state == VERB)
		return BASE_VP_W;
	if (p1->chunk_state == ILL)
		return ILL_W;
	if (p1->chunk_state == OBJECT ||
			p1->chunk_state == PRODUCT)
		return ATTRI_W;
	if (p1->chunk_state == NZ || p1->head_state == NT)
		return NZ_W;
  }
  
  return DEFALT_W;
}

/**
* @brief 获得term 的权重，针对单字的实体概念进行降权
* @param tokens - the tokens info of input
* @param size - size of tokens array
* @param chkbuf - chunks array
* @return 
*/
static void _term_weight_new(token_t tokens[], uint32_t size, chunk_t chkbuf[])
{
	for (int i=0; i<(int)size; i++)
	{
		if (tokens[i].length <= 2 && (chkbuf[i].chunk_state == OBJECT || chkbuf[i].chunk_state == PRODUCT))
		{
			chkbuf[i].chunk_state = 0;
			chkbuf[i].weight = DEFALT_W;
		}
		chkbuf[i].weight = _chunk_weight(&chkbuf[i], TERM_W_STATE);
	}
}

/**
* @brief 当词类为标点，连词，介词或助词时，不参与粘接
* @param token - token info to be judged
* @return 1 if the token is not to be conjected , else return 0
*/
static int _unvalid_single_conj(token_t token)
{
	if (token.type == W || token.type == C || token.type == P || token.type == U || token.type == Q)
		return 1;
	return 0;
}

/**
* @brief 判断是否为粘接后缀，依赖term 词性和属性
* @param token - token info to be judged
* @param chunks - chunks info to be judged
* @return 1 if the token is conjection-suffix , else return 0
*/
static int _isvalid_single_conj(token_t token, chunk_t chunks)
{
	if (token.length <= 2 && (token.type == Ng || token.type == Ag || token.type == Vg
					|| token.type == Dg || token.type == G
					|| token.type == K || chunks.chunk_state == PRODUCT))
		return 1;
	return 0;
}

/**
* @brief 添加粘接组块chunk
* @param tokens - input token information
* @param chkbuf -space to restore the chunk info
* @param chk_index - current index of stored chunk
* @param in_buffer - 输入buffer
* @param index - 输入token 的索引
* @param end - 输入token 的右边界
* @return 0
*/
static void _add_conj_chunk(token_t tokens[], chunk_t chkbuf[], int &chk_index, chunk_t** in_buffer, int index, int end)
{
	chunk_t* p = NULL;
	p = &chkbuf[chk_index++];
	p->head_count = 0;
	p->head_buffer[p->head_count++] = index;
	p->head_buffer[p->head_count++] = index+1;
	p->chunk_depth = 1;
	p->left = &chkbuf[index];
	if (index < end - 2 && _isvalid_single_conj(tokens[index+2], chkbuf[index+2]))
	{
		p->head_buffer[p->head_count++] = index+2;
		p->right = &chkbuf[index+2];
		p->left->next = &chkbuf[index+1];
		p->left->next->next = p->right;
		p->right->next = NULL;
		//修改输入buffer
		in_buffer[index+2] = p;
		in_buffer[index+2]->weight = ZJ_CHUNK_W;
		in_buffer[index+1] = NULL;
		in_buffer[index] = NULL;
		index += 2;
	}
	else
	{
		p->right = &chkbuf[index+1];
		p->left->next = p->right;
		p->right->next = NULL;
		//修改输入buffer
		in_buffer[index+1] = p;
		in_buffer[index+1]->weight = ZJ_CHUNK_W;
		in_buffer[index] = NULL;
		index++;
	}
	if ( chkbuf[index].chunk_state == PRODUCT )
	{
		p->chunk_state = CONJ_PRO;//区分属性类
		p->head_state = N;
	}
	else if (tokens[index].type == N 
				||(tokens[index].type == Ng && tokens[index-1].type != A))
	{
		p->chunk_state = CONJ_N;//区分属性类
		p->head_state = N;
	}
	else
	{
		p->chunk_state = CONJ_A;//区分属性类
		p->head_state = A;
	}	
}

/**
* @brief 添加组块chunk
* @param chkbuf -space to restore the chunk info
* @param chk_index - current index of stored chunk
* @param in_buffer - 输入buffer
* @param index - 做孩子的组块索引 
* @param inner -右孩子的组块索引
* @param type - 组块类型
* @param state - 组块的属性/ 关系标示
* @return 0
*/
static int _add_felt_chunk(chunk_t chkbuf[], int &chk_index, chunk_t** in_buffer, int index, int inner, int type, int state )
{
	chunk_t* p = NULL;
	p = &chkbuf[chk_index++];
	p->head_count = 0;
	p->chunk_depth = 1;
	p->left = &chkbuf[index];
	p->right = &chkbuf[inner];
	p->left->next = p->right;	
	p->right->next = NULL;	
	
	//修改输入buffer
	in_buffer[inner] = p;
	if (type == OBJECT_OTHER)
	{
		p->left->chunk_state = BASE_NP;
		p->head_state = N;
		p->chunk_state = UP_PRODUCT;//标注上下位UP_DONW
		in_buffer[inner]->weight = UP_CHUNK_W;
		p->head_buffer[p->head_count++] = index;
	}
	else if (type == NZ_OTHER)
	{
		if (chkbuf[inner].chunk_state == NUMS)
		{
			p->head_buffer[p->head_count++] = index;
			p->head_state = A;
			p->chunk_state = UP_DOWN;
		}
		else
		{
			p->head_state = N;
			p->chunk_state = DOWN_PRODUCT;//标注上下位DOWN_UP
			chkbuf[inner].chunk_state = BASE_NP;//0812
		}
		p->head_buffer[p->head_count++] = inner;
		in_buffer[inner]->weight = UP_CHUNK_W;
	}
	else if (type == NUM_OTHER)
	{
		if (chkbuf[inner].chunk_state == NZ)
		{
			p->head_buffer[p->head_count++] = index;
			p->head_state = A;
			p->chunk_state = DOWN_UP;//标注上下位
		}
		else
		{
			p->head_state = N;	
			p->chunk_state = DOWN_PRODUCT;//标注上下位
			chkbuf[inner].chunk_state = BASE_NP;//0812
		}
		p->head_buffer[p->head_count++] = inner;
		in_buffer[inner]->weight = UP_CHUNK_W;
	}
	else
	{
		in_buffer[inner]->weight = ZHUANMING_W;
		if (state == UP_DOWN) //专名相关的上下位关系，下位词是head，默认左边，当UP_DOWN时，右节点的head是head
		{
			memcpy(p->head_buffer, p->right->head_buffer, p->right->head_count*sizeof(int));
			p->head_count = p->right->head_count;
		}
		else if (state == REQUEST_RELATE)//专名需求关系，两个儿子节点的头都是头
		{
			memcpy(p->head_buffer, p->left->head_buffer, p->left->head_count*sizeof(int));
			memcpy(p->head_buffer + p->left->head_count, p->right->head_buffer, p->right->head_count*sizeof(int));
			p->head_count = p->left->head_count + p->right->head_count;
		}
		else
		{			
			memcpy(p->head_buffer, p->left->head_buffer, p->left->head_count*sizeof(int));
			p->head_count = p->left->head_count;
		}			

		p->chunk_state = state;
		p->head_state = N;
	}

	if (type != REQUEST_OTHER)
		in_buffer[index] = NULL;			
	return 0;
}

/**
* @brief 预处理: 进行新概念的粘接和上下位关系term 的粘接,合并后缀为Ng 的term，
若存在Verb，则标示为a; 若为N，则标示为N; 粘接单字商品后缀构成的商品名
合并上下位关系:object->nz object->num nz->num,被合并的term 在输入缓冲中标示为NULL
* @param tokens - tokens info to be judged
* @param count - num of tokens
* @param chkbuf -buffer to restore the chunk results
* @param leng -size of chkbuf
* @param chk_index -current chunk index
* @return the pointer of altered input chunk--预处理后的输入buffer 指针
*/
static chunk_t** _preprocess_merge(token_t tokens[], int count, chunk_t chkbuf[], int leng, int &chk_index)
{
	chunk_t** in_buffer = NULL;
	in_buffer = (chunk_t**)calloc(count, sizeof(chunk_t*));
	if (in_buffer == NULL)
	{
		LOG_ERROR("error: calloc chunk_t** failed");
		return NULL;
	}
	int index = 0;
	int inner = 0;
	int up_down = 0;
	int end = count;
	int type = 0;
	int state = 0;
	char line[MAX_PARSED_COUNT];

	//从后往前看，对于被merge 的输入buffer 赋予Null
	for (index = 0; index < end; index++)
	{		
		if (in_buffer[index] != NULL)
		{
			continue;
		}
		memset(line, 0, sizeof(line));
		//跳过空格 和标点
		if (tokens[index].type == W || (tokens[index].prop.m_lprop)& 0x00000040)
		{
			in_buffer[index] = NULL;
			continue;
		}
		if ((tokens[index].type == M && index < end - 1 && tokens[index+1].type == Q) 
			||(!_unvalid_single_conj(tokens[index]) && index < end - 1 && (_isvalid_single_conj(tokens[index+1], chkbuf[index+1]) 
			||(tokens[index].length <= 2) && (tokens[index+1].length <= 2) && !_unvalid_single_conj(tokens[index+1]) 
				&&index < end - 2 && _isvalid_single_conj(tokens[index+2], chkbuf[index+2]))))
		{
			_add_conj_chunk(tokens, chkbuf, chk_index, in_buffer, index, end);
			continue;
		}
		//规则上下位
		else if ((index < count -1) && 
					(chkbuf[index].chunk_state == OBJECT || chkbuf[index].chunk_state == NZ
				    || chkbuf[index].chunk_state == NUMS || chkbuf[index].chunk_state == PRODUCT))
		{
			if (in_buffer[index+1] != NULL)
					continue;			
			for (inner=index+1; inner<count; inner++)
			{
				//标示被关联过
				if (in_buffer[inner] != NULL)
					continue;
				if ((chkbuf[index].chunk_state == OBJECT || chkbuf[index].chunk_state == PRODUCT) 
						&&(chkbuf[inner].chunk_state == NZ || chkbuf[inner].chunk_state == NUMS))
				{	
					type = OBJECT_OTHER;
					_add_felt_chunk(chkbuf, chk_index, in_buffer, index, inner, type, state);
					up_down = 1;
					break;
				}
				else if (chkbuf[index].chunk_state == NZ &&
					(chkbuf[inner].chunk_state == OBJECT|| chkbuf[inner].chunk_state == NUMS
						|| chkbuf[inner].chunk_state == PRODUCT))
				{
					type = NZ_OTHER;
					_add_felt_chunk(chkbuf, chk_index, in_buffer, index, inner, type, state);	
					up_down = 1;
					break;
				}
				else if (chkbuf[index].chunk_state == NUMS &&
					(chkbuf[inner].chunk_state == OBJECT|| chkbuf[inner].chunk_state == NZ
						|| chkbuf[inner].chunk_state == PRODUCT))
				{
					type = NUM_OTHER;
					_add_felt_chunk(chkbuf, chk_index, in_buffer, index, inner, type, state);	
					up_down = 1;
					break;
				}
			}
		}		
		else if (chkbuf[index].chunk_state == SNG || 
			       chkbuf[index].chunk_state == NVL || 
			       chkbuf[index].chunk_state == SFT || 
			  		 chkbuf[index].chunk_state == GME || 
			       chkbuf[index].chunk_state == VDO_MVE || 
			       chkbuf[index].chunk_state == VDO_TLP ||
			       chkbuf[index].chunk_state == PRODUCT ||
			       chkbuf[index].chunk_state == NZ ||
			       (strcmp( tokens[index].buffer, "电影" ) == 0 && index == 0) ||
			       (strcmp( tokens[index].buffer, "电视剧" ) == 0 && index == 0) ||
			       ((ul_strncasecmp( tokens[inner].buffer, "mp3", 3 ) == 0 || 
			         strcmp( tokens[inner].buffer, "歌曲" ) == 0 || 
			         strcmp( tokens[inner].buffer, "歌" ) == 0) && index == 0) ||
			       (strcmp( tokens[index].buffer, "小说" ) == 0 && index == 0) ||
			       chkbuf[index].chunk_state == FTR_VDO ||
			       chkbuf[index].chunk_state == FTR_NVL ||
			       chkbuf[index].chunk_state == FTR_GME ||
			       chkbuf[index].chunk_state == FTR_SFT ||
			       chkbuf[index].chunk_state == FTR_MP3 ||
			       chkbuf[index].chunk_state == FTR_PDT) //影视剧或小说
		{
			int state = 0;
			type = NER_OTHER;
			for (inner=index+1; index < count - 1 && inner<count; inner++) //backward
			{
				if (in_buffer[inner] != NULL)//被关联过
					continue;
								
				if (chkbuf[index].chunk_state == VDO_MVE && chkbuf[inner].chunk_state == FTR_VDO_SN)
					state = MVE_SN;
				else if (chkbuf[index].chunk_state == VDO_TLP && chkbuf[inner].chunk_state == FTR_VDO_SN)
					state = TLP_SN;
				else if (chkbuf[index].chunk_state == NVL && chkbuf[inner].chunk_state == FTR_NVL_SN)
					state = NVL_SN;
				else if (chkbuf[index].chunk_state == SFT && chkbuf[inner].chunk_state == FTR_VN)
					state = SFT_VN;
				else if (chkbuf[index].chunk_state == GME && chkbuf[inner].chunk_state == FTR_VN)
					state = GME_VN;		
				else if (chkbuf[index].chunk_state == VDO_MVE && strcmp( tokens[inner].buffer, "电影" ) == 0 && inner == count - 1)
					state = DOWN_UP;
				else if (chkbuf[inner].chunk_state == VDO_MVE && strcmp( tokens[index].buffer, "电影" ) == 0 && index == 0)
					state = UP_DOWN;
				else if (chkbuf[index].chunk_state == VDO_TLP && strcmp( tokens[inner].buffer, "电视剧" ) == 0 && inner == count - 1)
					state = DOWN_UP;
				else if (chkbuf[inner].chunk_state == VDO_TLP && strcmp( tokens[index].buffer, "电视剧" ) == 0 && index == 0)
					state = UP_DOWN;	
				else if (chkbuf[index].chunk_state == NVL && strcmp( tokens[inner].buffer, "小说" ) == 0 && inner == count - 1)
					state = DOWN_UP;
				else if (chkbuf[inner].chunk_state == NVL && strcmp( tokens[index].buffer, "小说" ) == 0 && index == count - 1)
					state = UP_DOWN;	
				else if (chkbuf[index].chunk_state == SNG && (ul_strncasecmp( tokens[inner].buffer, "mp3", 3 ) == 0 || strcmp( tokens[inner].buffer, "歌曲" ) == 0 || strcmp( tokens[inner].buffer, "歌" ) == 0) && inner == count - 1)
					state = DOWN_UP;
				else if (chkbuf[inner].chunk_state == SNG && (ul_strncasecmp( tokens[index].buffer, "mp3", 3 ) == 0 || strcmp( tokens[index].buffer, "歌曲" ) == 0 || strcmp( tokens[index].buffer, "歌" ) == 0) && index == 0)
					state = UP_DOWN;	
				else if (chkbuf[index].chunk_state == VDO_MVE && chkbuf[inner].chunk_state == FTR_VDO)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == VDO_TLP && chkbuf[inner].chunk_state == FTR_VDO)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == NVL && chkbuf[inner].chunk_state == FTR_NVL)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == SFT && chkbuf[inner].chunk_state == FTR_SFT)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == GME && chkbuf[inner].chunk_state == FTR_GME)
					state = REQUEST_RELATE;				
				else if (chkbuf[index].chunk_state == SNG && chkbuf[inner].chunk_state == FTR_MP3)
					state = REQUEST_RELATE;		
				else if ((chkbuf[index].chunk_state == PRODUCT || chkbuf[index].chunk_state == NZ) && chkbuf[inner].chunk_state == FTR_PDT)
					state = REQUEST_RELATE;				
				else if (chkbuf[index].chunk_state == FTR_VDO && chkbuf[inner].chunk_state == VDO_MVE)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == FTR_VDO && chkbuf[inner].chunk_state == VDO_TLP)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == FTR_NVL && chkbuf[inner].chunk_state == NVL)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == FTR_SFT && chkbuf[inner].chunk_state == SFT)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == FTR_GME && chkbuf[inner].chunk_state == GME)
					state = REQUEST_RELATE;				
				else if (chkbuf[index].chunk_state == FTR_MP3 && chkbuf[inner].chunk_state == SNG)
					state = REQUEST_RELATE;		
				else if ((chkbuf[inner].chunk_state == PRODUCT || chkbuf[index].chunk_state == NZ) && chkbuf[index].chunk_state == FTR_PDT)
					state = REQUEST_RELATE;

				if (state > 0)
				{
					_add_felt_chunk(chkbuf, chk_index, in_buffer, index, inner, type, state);					
					up_down = 1;
					break;
				}
			}						
		}	

		if (up_down == 0)
		{
			//不需要合并的term 直接复制原先的混排chk 信息
			in_buffer[index] = &chkbuf[index];
			in_buffer[index]->weight = tokens[index].weight;//record idf
		}
		else
		{
			in_buffer[index] = NULL;
		}
		up_down = 0;
	}
	
	int chk_end = chk_index;
	type = REQUEST_OTHER;  
	for (index = end; index < chk_end; index++)
	{
		if (chkbuf[index].chunk_state == NVL_SN || 
			  chkbuf[index].chunk_state == SFT_VN || 
			  chkbuf[index].chunk_state == GME_VN || 
			  chkbuf[index].chunk_state == MVE_SN || 
			  chkbuf[index].chunk_state == TLP_SN) //影视剧或小说
		{
			int state = 0;
			for (inner=chkbuf[index].head_buffer[0] + 1; inner<count; inner++) //backward
			{
				if (in_buffer[inner] == NULL)//
					continue;
								
				if (chkbuf[index].chunk_state == MVE_SN && in_buffer[inner]->chunk_state == FTR_VDO)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == TLP_SN && in_buffer[inner]->chunk_state == FTR_VDO)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == NVL_SN && in_buffer[inner]->chunk_state == FTR_NVL)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == SFT_VN && in_buffer[inner]->chunk_state == FTR_SFT)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == GME_VN && in_buffer[inner]->chunk_state == FTR_GME)
					state = REQUEST_RELATE;								
				
				if (state > 0)
				{
					_add_felt_chunk(chkbuf, chk_index, in_buffer, index, inner, type, state);					
					break;
				}
			}
			
			for (inner=chkbuf[index].head_buffer[0] - 1; state == 0 && inner >= 0; inner--) //forward
			{
				if (in_buffer[inner] == NULL)//
					continue;
												
				if (chkbuf[index].chunk_state == MVE_SN && in_buffer[inner]->chunk_state == FTR_VDO)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == TLP_SN && in_buffer[inner]->chunk_state == FTR_VDO)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == NVL_SN && in_buffer[inner]->chunk_state == FTR_NVL)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == SFT_VN && in_buffer[inner]->chunk_state == FTR_SFT)
					state = REQUEST_RELATE;
				else if (chkbuf[index].chunk_state == GME_VN && in_buffer[inner]->chunk_state == FTR_GME)
					state = REQUEST_RELATE;				
				
				if (state > 0)
				{
					_add_felt_chunk(chkbuf, chk_index, in_buffer, index, inner, type, state);
					break;
				}
			}
		}				
	}
	return in_buffer;
}

/**
* @brief 获得基本的名词中心和动词中心;0.资源命中baseNp 和baseVp 优先;
1.baseVp优先，选取idf大的v或者vn；如果存在多个idf相同的v或者vn，则选取多个baseVp
2.如果不存在baseVp，则识别baseNp;选取idf大的n；
如果存在多个idf相同的n，则根据位置关系，选取位置靠后的term作为baseNp，认为在中文的表达中；
多个n并存时，前面的n多为最后一个n的修饰成分。新增输入buffer: 用于存储合并的单字和合并的动名词
* @param base_info - buffer to restore base_info
* @param count - nums of input tokens
* @param chunks -chunk info for terms
* @param tokens -input tokens info
* @return 0 if the process success, else return -1
*/
static int _get_base_info(base_info_t *base_info, int count, chunk_t** chunks, token_t tokens[])
{
	if (chunks == NULL || base_info == NULL || count <= 0)
	{
		LOG_ERROR("error: get_base_info NULL param");
		return -1;
	}

	int max_bv_idf = MAX_VP_IDF_BASE;//baseVp max_idf ;smaller than 2 is recarded
	int i = 0;
	int bv_index = 0;//baseVp_index

	//finding baseVp and baseNp......
	//基于属性发现
	for (i=0; i<count; i++)
	{
		if (chunks[i] == NULL)
			continue;

		if (chunks[i]->chunk_state == VERB)
		{
			base_info->baseVp[base_info->baseVp_num++] = i;
		}
		//记录被资源命中的baseNp 
		else if (chunks[i]->chunk_state == OBJECT || chunks[i]->chunk_state == PRODUCT 
					|| chunks[i]->chunk_state == ILL		
					|| chunks[i]->chunk_state == DOWN_PRODUCT || chunks[i]->chunk_state == UP_PRODUCT
					|| chunks[i]->chunk_state == CONJ_PRO || chunks[i]->chunk_state == NZ
					|| chunks[i]->chunk_state == SNG
					|| chunks[i]->chunk_state == NVL
					|| chunks[i]->chunk_state == SFT
					|| chunks[i]->chunk_state == GME
					|| chunks[i]->chunk_state == VDO_TLP
					|| chunks[i]->chunk_state == VDO_MVE
					|| chunks[i]->chunk_state == NVL_SN
					|| chunks[i]->chunk_state == TLP_SN
					|| chunks[i]->chunk_state == MVE_SN
					|| chunks[i]->chunk_state == SFT_VN
					|| chunks[i]->chunk_state == GME_VN
					|| chunks[i]->chunk_state == TLP_SN)
		{
			base_info->baseNp[base_info->baseNp_num++] = i;
		}
		else if (chunks[i]->chunk_state == ORG)
		{
			base_info->baseOp = i;
		}
	}
	
	//deal baseVp info
	if (base_info->baseVp_num == 0)
	{
		for (i=0; i<count; i++)
		{
			//被合并的term， 跳过
			if (chunks[i] == NULL)
				continue;
			if (chunks[i]->chunk_state == 0 && i>0 && i==count-1 && chunks[i]->head_state == VN && chunks[i-1]!= NULL &&
					chunks[i-1]->head_state == N)
			{
				//单字忽略
				if (tokens[i].length <= 2)
					continue;	
				if ((int)chunks[i]->weight < MAX_VP_IDF_BASE)
					continue;				
				if ((int)chunks[i]->weight > max_bv_idf)
				{
					max_bv_idf = (int)chunks[i]->weight;
					bv_index = 0;
					base_info->baseVp[bv_index] = i;
					base_info->baseVp_num = 1;
				}
				//record the baseVp with the same idf
				else if ((int)chunks[i]->weight == max_bv_idf)
				{
					base_info->baseVp[++bv_index] = i;
					base_info->baseVp_num ++;
				}
			}
		}
	}
	
	if (base_info->baseNp_num == 0)
	{
		for (i=0; i<count; i++)
		{
		    if (chunks[i] == NULL)
				continue;
			//非属性的名词或者动名词；或者粘接后的名词性词语0813新加专名
			if ((chunks[i]->chunk_state == 0 && 
				(chunks[i]->head_state == N || chunks[i]->head_state == ZN || chunks[i]->head_state == J
				|| (chunks[i]->head_state == VN && (base_info->baseVp_num == 0 || i!=base_info->baseVp[0]))
				|| chunks[i]->head_state == NT))
				|| chunks[i]->chunk_state == CONJ_N)
			{	
				base_info->baseNp[base_info->baseNp_num++] = i;
			}
		}
	}	
	return 0;
}

/**
* @brief 根据idf 和位置共同确定baseNp的选择；如果原始idf 大于默认值3，则不处理
否则，取前大于默认值的一个
* @param chunks - input chunks to be analyzed
* @param base_info - base_info of baseNp and baseVps
* @param begin -选取chunks 数组中待处理的区间,begin index
* @param end -end index
* @return the index of baseNp be chosen
*/
static int _get_baseNp_index(chunk_t** chunks, base_info_t *base_info, int begin, int end)
{
	int np_index = end;	
	int vp_object = base_info->baseNp[np_index];
	// 如果idf <= 3， 则取前面一个idf 大的np
	while ((unsigned int)chunks[vp_object]->weight < MAX_NP_IDF_BASE && np_index >= begin+1)
	{				
		int tmp = base_info->baseNp[np_index-1];
		if (chunks[vp_object]->weight < chunks[tmp]->weight)
			vp_object = tmp;
		np_index--;
	}
	return vp_object;
}

/**
* @brief 寻找组块的中心概念baseVp 优先
1.如果只有一个baseVp, 且存在多个 baseNp ，则按以下规则关联
  资源优先--> 位置-->共现
  位置:  如果baseNp 均在baseVp 的一侧，则取最靠近的关联
  		  如果baseNp 在baseVp 的两侧，则取最靠近的关联，其中cooc大的优先
2.如果存在多个baseVp
  则返回，对选择的baseVp 和baseNp 进行parse
3.如果不存在baseVp，则处理baseNp
  存在多个baseNp，取位置最靠后的一个作为中心成分
  此时不取中心概念
  认为中文表达中，多名词性成分并列时，重心在最后
4.将处理过的baseVp的token_weight (原先记录idf)标记为1，运算器不再处理
  记录中心位置及中心概念chunk 信息;利用baseVp 的空间，当
  baseVp处理完毕，替换为其作用对象baseNp的索引；按升序排列。
* @param tokens - input tokens to be analyzed
* @param count - num of input tokens
* @param chkbuf -buffer to restore the chunk result
* @param leng - size of chkbuf
* @param base_info -infos of baseNp and baseVp
* @param chk_index -the current index of chkbuf
* @param chunks -input chunks to be analyzed
* @return the type of the input string
	BASE_PARSE: more than one baseVp
	BASEVP_CENTER: single baseVp-baseNp relation
	NP_CENTER: the center of string is a concept, with other information to decorate it
	ORG_CENTER: the input string got the organization attribute
	default return 0;
*/
static int _base_parse(token_t tokens[], int count, chunk_t chkbuf[], int leng,  
								base_info_t *base_info, int &chk_index, chunk_t** chunks)
{
	int index;	
	//set weight 0
	for (index = 0; index < count; index++)
	{
		tokens[index].weight = 0;
	}
	//多于一个baseVp，则对名词和动词性成分进行小型chunk parse处理
	if (base_info->baseVp_num > 1)
		return BASE_PARSE;
	
	//prior to baseVp: only one baseVp
	if (base_info->baseVp_num == 1)
	{
		int vp_object = 0;//记录baseVp 作用对象base_Np 索引				
		int vp_index = base_info->baseVp[0];//寻找baseVp 的作用对象

		//不存在baseNp，按原先流程处理
		if (base_info->baseOp >= 0 && (base_info->baseNp_num <= 0 ||
				(base_info->baseOp < vp_index && base_info->baseNp[base_info->baseNp_num-1] < base_info->baseOp)))			
		{
			//如果机构名在动词前，则按照机构名分段处理
			base_info->baseNp_num = 0;
			base_info->baseNp[base_info->baseNp_num++] = base_info->baseOp;
			goto BASENP;			
		}
		else if (base_info->baseOp >= 0 && base_info->baseOp < base_info->baseNp[base_info->baseNp_num-1] 
			&& base_info->baseNp[base_info->baseNp_num-1] < vp_index)
		{
			int b_np = base_info->baseNp_num - 1;
			base_info->baseNp_num++;
			while (b_np >= 0)
			{
				if (base_info->baseNp[b_np] > base_info->baseOp)
				{
					base_info->baseNp[b_np+1] = base_info->baseNp[b_np];
					b_np--;
				}
				else
				{
					base_info->baseNp[b_np+1] = base_info->baseOp;
					break;
				}
			}
			if (b_np < 0)
				base_info->baseNp[0] = base_info->baseOp;
			goto BASENP;
		}
		else if (base_info->baseNp_num <= 0)
		{
			chunks[base_info->baseVp[0]]->chunk_state = BASE_VP;
			return 0;
		}
 BASENP:		
		chunk_t* p = NULL;
		//左侧
		if (base_info->baseNp[base_info->baseNp_num-1] < vp_index)
		{
			vp_object = _get_baseNp_index(chunks, base_info, 0, base_info->baseNp_num-1);
			//新加chunk
			p = &chkbuf[chk_index++]; 
   			memset(p,0,sizeof(*p));
			//weight 权重待定			
			p->left = chunks[vp_object];
			p->right = chunks[vp_index];
			//标记处理过vp:token.weight = 1
			tokens[vp_index].weight = 1;
			memcpy(p->head_buffer,chunks[vp_object]->head_buffer,chunks[vp_object]->head_count*sizeof(int));
			p->head_count = chunks[vp_object]->head_count;
		}
		//右侧
		else if (base_info->baseNp[0] > vp_index)
		{
			vp_object = _get_baseNp_index(chunks, base_info, 0, base_info->baseNp_num-1);
			//新加chunk
			p = &chkbuf[chk_index++]; 
   			memset(p,0,sizeof(*p));
			//weight 权重待定
			memcpy(p->head_buffer,chunks[vp_object]->head_buffer,chunks[vp_object]->head_count*sizeof(int));
			p->head_count = chunks[vp_object]->head_count;
			p->right = chunks[vp_object];
			p->left = chunks[vp_index];
			tokens[vp_index].weight = 1;
		}
		else
		{
			//否则，二分查找左右测的相邻baseNp  索引,以low 为基准
			int low = 0;
			int mid = 0;
			int high = base_info->baseNp_num;

			while (high > low)
			{
				mid = (high + low)/2;
				if (base_info->baseNp[mid] > vp_index)
				{
					high = mid -1;
				}
				else
				{
					low = mid + 1;
				}
			}	
			//low 取最近的左边那个，high 取最高位置的那个
			if (low > high)
			{
				int tmp_index = high;
				high = low;
				low = tmp_index;
			}
			int low_np = _get_baseNp_index(chunks, base_info, 0, low);
			int high_np = _get_baseNp_index(chunks, base_info, high, base_info->baseNp_num-1);
			
			int cooc_low = _bigram_get(tokens[low_np].buffer, tokens[vp_index].buffer);
			int cooc_high =  _bigram_get(tokens[vp_index].buffer, tokens[high_np].buffer);
			vp_object = (cooc_low > cooc_high) ? low_np: high_np;

			if (cooc_high == cooc_low)
				vp_object = (low_np < high_np) ? low_np: high_np;
			
			//新加chunk 
			p = &chkbuf[chk_index++]; 
	  		memset(p,0,sizeof(*p));

			//weight 权重待定
			if (vp_object == low_np)
			{
				tokens[vp_index].weight = 1;//标记baseVp 为已处理状态
				p->left = chunks[vp_object];
				p->right = chunks[vp_index];
				memcpy(p->head_buffer,chunks[vp_object]->head_buffer,chunks[vp_object]->head_count*sizeof(int));
				p->head_count = chunks[vp_object]->head_count;
			}
			else
			{
				tokens[vp_index].weight = 1;
				p->right = chunks[vp_object];
				p->left = chunks[vp_index];
				memcpy(p->head_buffer,chunks[vp_object]->head_buffer,chunks[vp_object]->head_count*sizeof(int));
				p->head_count = chunks[vp_object]->head_count;
			}
		}
		p->head_state = VN;
		p->chunk_state = BASEVP_CENTER;
		p->chunk_depth = 0;//最高级别深度设置为0；与其他chunk区别
		p->left->next = p->right;
		p->right->next = NULL;
		p->weight = VN_CHUNK_W;//0810
		chunks[vp_object]->weight = BASE_NP_W;//修改动宾关系的作用对象为baseNp 0810
		if (chunks[vp_index]->weight < BASE_VP_W) //0811---除了意图属性以外的baseVp 识别提权
			chunks[vp_index]->weight = BASE_VP_W;
		
		//debug
		char termLeft[MAX_BIGRAM_LENG] = {0};
		char termRight[MAX_BIGRAM_LENG] = {0};
		char vp[MAX_BIGRAM_LENG] = {0};
		char np[MAX_BIGRAM_LENG] = {0};
	
		_chunk_getstr(p->left,tokens,termLeft,MAX_BIGRAM_LENG);
		_chunk_getstr(p->right,tokens,termRight,MAX_BIGRAM_LENG);
		_chunk_getstr(chunks[vp_index],tokens,vp,MAX_BIGRAM_LENG);
		_chunk_getstr(chunks[vp_object],tokens,np,MAX_BIGRAM_LENG);
		//debug
		
		//修改chunk 指针
		chunks[vp_object] = p;	
		chunks[vp_index] = NULL;
		//用baseVp 的空间来记录中心成分baseNp;baseVp的信息存储于baseNp的下一个位置
		base_info->baseVp[base_info->baseVp_num++] = base_info->baseVp[0];
		base_info->baseVp[0] = vp_object;

		if (base_info->baseOp >= 0)
		{
			return ORG_CENTER; 
		}
		return VP_CENTER;
	}		

	//only baseNp	
	if (base_info->baseNp_num > 0)
	{
		if (base_info->baseOp >= 0)
		{
			if (base_info->baseOp < base_info->baseNp[0])
				return NP_CENTER;
			return ORG_CENTER;
		}
		return NP_CENTER;
	}
	return 0;
}

/**
* @brief 运算器处理:从左往右parsing
		1.如果命中三元规则，则三元规则优先;更新运算器节点信息
		2.如果两边规则均不满足，则将左节点进栈，更新运算器节点信息
		3.如果左右搭配均满足规则，则选取共现大的优先
		4.如果左规则不满足，则更新chunk 网络
		5.否则，将左规则结合为chunk，更新运算器节点信息
* @param tokens - input tokens to be analyzed
* @param count - parse 区间的右边界
* @param chkbuf -buffer to restore the output chunk info
* @param leng -size of chkbuf
* @param index - parse 区间的左边界
* @param ret - base_info of baseNp and baseVps
* @param chunks - input chunks to be analyzed
* @return
*/
static void _defalt_parse(token_t tokens[], int count, chunk_t chkbuf[], 
								int leng, int index, int &ret, chunk_t** chunks)
{
	int top = 0; // top index of statck
	
	chunk_t* pLeft = NULL;
	chunk_t* pRight = NULL;
	chunk_t* pCenter = NULL;
	chunk_t* pStack[MAX_PARSED_COUNT] = {0};

	//跳过动词
	while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		  index++;
	if (index >= count)
		  	return;
	pLeft = chunks[index++]; 
	//跳过动词
	while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		  index++;
	if (index >= count)
		  	return;
	pCenter = chunks[index++];
	//跳过动词
	while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		  index++;
	pRight = index < count ? chunks[index++] : NULL; 
	  
	// parsing ... 
	while( pCenter && (index <= count) && (ret < leng) )//?
	{
		char termLeft[MAX_BIGRAM_LENG] = {0};
		char termCenter[MAX_BIGRAM_LENG] = {0};
		char termRight[MAX_BIGRAM_LENG] = {0};
	
		_chunk_getstr(pLeft,tokens,termLeft,MAX_BIGRAM_LENG);
		_chunk_getstr(pCenter,tokens,termCenter,MAX_BIGRAM_LENG);
		_chunk_getstr(pRight,tokens,termRight,MAX_BIGRAM_LENG);
		
		int rule = 0;
		// 三元规则优先处理，直接合并
		if( pRight && 
		  ( rule = _state_transit(pLeft->head_state,pCenter->head_state,pRight->head_state) ) )
		{	   
		  // init a new chunk
		  chunk_t* p = &chkbuf[ret++]; 
		  memset(p,0,sizeof(*p));
		  _head_merge(p,pLeft,pCenter,pRight);
	
		  // fill a new chunk
		  p->head_state = GET_CHUNK_STATE(rule);
		  p->chunk_state = DEFALT_RELATE;
		  p->chunk_depth = pLeft->chunk_depth > pRight->chunk_depth ? pLeft->chunk_depth+1 : pRight->chunk_depth+1;
		  p->weight = _chunk_weight(p, CHUNK_W_STATE);
		  p->left = pLeft;
		  p->right = pRight;
		  p->left->next = pCenter;
		  p->left->next->next = pRight;
		  pRight->next = NULL;
	
		  // 更新chunk网络
		  if( top )
		  {
			pLeft = pStack[--top];
			pCenter = p;
			//skip verb
			while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		  		index++;
			pRight = index == count ? NULL : chunks[index++];		  
		  }
		  else
		  {
			pLeft = p;
			if( index == count )
			  break;
			//skip verb
			while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		 		 index++;
			pCenter = chunks[index++];
			//skip verb
			while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		 		 index++;
			if( index == count )
			  pRight = NULL;
			else
			  pRight = chunks[index++];
		  } 	 
		  continue;
		}
	
		int ruleL = _state_transit(pLeft->head_state,pCenter->head_state);
		int ruleR = pRight ? _state_transit(pCenter->head_state,pRight->head_state) : 0;
		// 左右搭配均不满足条件
		if( !ruleL && !ruleR )
		{	   
		  if( index == count )
			break;
		  // 更新chunk网络
		  pStack[top++] = pLeft;
		  pLeft = pCenter;
		  pCenter = pRight;
		  //skip verb
		  while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		  		index++;
		  pRight = chunks[index++];
		  continue;
		}
	
		double weight = -1.0f; 
		bool bLeft = false; // 左搭配是否成立?
		
		//1.左搭配是否可合成新chunk-------------------------------------------------------------------------
		if( ruleL && ruleR )
		{
		  // 如果规则优先级相同则比较权重
			double w1 = _bigram_weight(pLeft,termLeft,pCenter,termCenter);
			double w2 = _bigram_weight(pCenter,termCenter,pRight,termRight); 	   
			if( w2 - w1 < 0.000001f )
			{
			  weight = w1;
			  bLeft = true;
			}
		}
		else 
		{
		  bLeft = (ruleR == 0); 	 
		}
	
		// 如果左搭配不能合成chunk，则left节点进栈，开始新一轮运算
		if( !bLeft )
		{	   
		  // 更新chunk网络
		  pStack[top++] = pLeft;
		  pLeft = pCenter;
		  pCenter = pRight; 	
		  //skip verb
		  while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
		 	 index++;
		  pRight = index == count ? NULL : chunks[index++];
		  continue;
		}
		
		chunk_t* pSub1 = NULL;
		chunk_t* pSub2 = NULL;
		char* pTerm1 = NULL;
		char* pTerm2 = NULL;
	
		//2.选取新chunk中的head------------------------------------------------------------------------------
		pSub1 = pLeft;
		pSub2 = pCenter;
		pTerm1 = termLeft;
		pTerm2 = termCenter;
		rule = ruleL;  
	
		// init a new chunk
		chunk_t* p = &chkbuf[ret++]; 
		memset(p,0,sizeof(*p));
	
		if( weight < 0.000001f )
		  weight = _bigram_weight(pSub1,pTerm1,pSub2,pTerm2);
	
		if( weight - MAX_TRANS_PROB > 0.000001f || // 权重高的直接合并子节点中心词为新节点中心词
			(!pSub1->chunk_state&&pSub1->left) ||  // 有一个节点为“虚组块”的合并
			(!pSub2->chunk_state&&pSub2->left) ||
			strlen(pTerm1) <= 2 || // 有一个节点含单字的合并
			strlen(pTerm2) <= 2 ) 
		{
		  p->head_state = GET_CHUNK_STATE(rule); 
		  _head_merge(p,pSub1,pSub2);
	
		  // 某些嵌套的子chunk标记为“虚组块”
		  if( strlen(pTerm1)<=2 && pSub2->chunk_state  )
			pSub2->chunk_state = 0;
		  if( strlen(pTerm2)<=2 && pSub1->chunk_state  )
			pSub1->chunk_state = 0;
		} 
		else  
		{
		  chunk_t* pSub = pSub1; // 默认位置在后的term更重要
		  int hd = GET_CHUNK_HEAD(rule);
		 
		  // 根据规则词典数据从子head中选取新head
		  if( (hd == 1) || 
			(pSub2->head_count>1&&!pSub2->chunk_state) )
		  {
			pSub = pSub1;		 
		  }
		  else if( (hd == 2) || 
			(pSub1->head_count>1&&!pSub1->chunk_state) )
		  {
			pSub = pSub2;
		  }
		  // 如果子节点中心词是一个组块则优先，如果均为组块，则权重大的优先
		  else if( pSub1->weight != pSub2->weight )
		  {
			pSub = pSub1->weight - pSub2->weight > 0.000001f ? pSub1 : pSub2;
		  } 		  
		  // 如果上述策略不能确定则选idf大的
		  else 
		  { 	 
			int idf1 = _unigram_get(pTerm1);
			if( idf1 < 0 )
			  idf1 = MAX_IDF;
			int idf2 = _unigram_get(pTerm2);		
			if( idf2 < 0 )
			  idf2 = MAX_IDF;
	
			if( idf1 > idf2 )
			  pSub = pSub1;
			else
			  pSub = pSub2;
		  }
	
		  p->head_state = GET_CHUNK_STATE(rule); 
		  memcpy(p->head_buffer,pSub->head_buffer,pSub->head_count*sizeof(int));
		  p->head_count = pSub->head_count;
		}
	
		// fill a new chunk
		p->left = pSub1;
		p->right = pSub2;
		p->left->next = p->right;
		p->right->next = NULL;
		p->chunk_state = DEFALT_RELATE;
		p->chunk_depth = pSub1->chunk_depth > pSub2->chunk_depth ? pSub1->chunk_depth+1 : pSub2->chunk_depth+1;
		p->weight = _chunk_weight(p, CHUNK_W_STATE);//0810
		// 标记“虚组块”，过滤无效chunk
		if( (pSub1->head_state==POS_AUX||pSub2->head_state==POS_AUX) || // 含助词
			(strlen(pTerm1)<=2 && 
			(pSub1->head_state!=pSub2->head_state)&& 
			(pSub1->head_state==POS_VERB||pSub1->head_state==POS_PRON)) || // 单个动词和介词
			(strlen(pTerm2)<=2 && 
			(pSub1->head_state!=pSub2->head_state) && 
			(pSub2->head_state==POS_VERB||pSub2->head_state==POS_PRON)) )
		  p->chunk_state = 0; 
			
		//3.更新chunk网络------------------------------------------------------------------------------------
		if( top ) // 如果节点栈有前向节点，则从栈顶取一个节点作为左节点；
		{
		  if( !pRight && top > 1 ) // 如果前向节点多于一个，且运算器上有两个空节点
		  {
			pRight = p;
			pCenter = pStack[--top];
			pLeft = pStack[--top];
		  }
		  else
		  {
			pLeft = pStack[--top];
			pCenter = p;
		  }
		}
		else	  // 否则，从输入缓冲的当前节点位置取一个新的节点作为右节点。
		{
		  pLeft = p;	   
		  pCenter = pRight;
		  //skip verb
		  while (index < count && (chunks[index] == NULL || tokens[index].weight == 1))
			 index++;
		  pRight = index == count ? NULL : chunks[index++];
		}
	  } // while	
}

/**
* @brief 运算器处理:保持右边节点不动
		1.如果命中三元规则，则三元规则优先
		2.如果左右规则都不命中，则将中间节点抛弃:是否合理
		3.如果左右两边规则均命中，则选取共现大的优先
		4.如果左规则相结合，则head选取同前；规则；
		5.如果右规则相结合，则head选取为baseNp
* @param count - parse 区间的右边界
* @param tokens - input tokens to be analyzed
* @param chkbuf -buffer to restore the output chunk info
* @param leng -size of chkbuf
* @param index - the index of current chunk 
* @param left_bound - parse 区间的左边界
* @param ret - base_info of baseNp and baseVps
* @param chunks - input chunks to be analyzed
* @return
*/
static void _right_center_parse(int count, token_t tokens[], chunk_t chkbuf[], int leng,
									int index, int left_bound, int &ret, chunk_t** chunks)
{
	//初始化节点
	chunk_t* pLeft = NULL;	
	chunk_t* pRight = NULL;
	chunk_t* pCenter = NULL;
	int org_flag = 0;//是否是关于org 的右结合
	
	pRight = chunks[index--]; 
	if (pRight->chunk_state == ORG || pRight->chunk_state == NT)
		org_flag = 1;
	//跳过动词
	while (index >= left_bound && 
				(chunks[index] == NULL || tokens[index].weight == 1))
		  index--;
	if (index < left_bound)
		  	return;
	pCenter = chunks[index--];
	//跳过动词
	while (index >= left_bound && 
				(chunks[index] == NULL || tokens[index].weight == 1))
		  index--;
	pLeft = index >= left_bound ? chunks[index--] : NULL;  
	
	// parsing ... 
	while (pCenter && (ret < leng))
	{
		char termLeft[MAX_BIGRAM_LENG] = {0};
		char termCenter[MAX_BIGRAM_LENG] = {0};
		char termRight[MAX_BIGRAM_LENG] = {0};
	
		_chunk_getstr(pLeft,tokens,termLeft,MAX_BIGRAM_LENG);
		_chunk_getstr(pCenter,tokens,termCenter,MAX_BIGRAM_LENG);
		_chunk_getstr(pRight,tokens,termRight,MAX_BIGRAM_LENG);
		
		int rule = 0;
		// 三元规则优先处理，直接合并
		//右节点不变，故存在左节点时才进行三元规则的判断
		if( pLeft && 
		  ( rule = _state_transit(pLeft->head_state,pCenter->head_state,pRight->head_state) ) )
		{	   
		  // init a new chunk
		  chunk_t* p = &chkbuf[ret++]; 
		  memset(p,0,sizeof(*p));
		  if (strlen(termLeft) <=2 || strlen(termCenter) <=2 || strlen(termRight) <=2)
		  	_head_merge(p,pLeft,pCenter,pRight);
		  else if (org_flag == 1)
		  {
			  //get the head info
			  memcpy(p->head_buffer,pLeft->head_buffer,pLeft->head_count*sizeof(int));
			  p->head_count = pLeft->head_count;
			  //0811对于机构，第一次构成修饰关系时，其中心为baseNp
			  if (pRight->head_count == 1 && pLeft->weight < BASE_NP_W)
			  {
				if (pLeft->head_count == 1)
					pLeft->chunk_state = BASE_NP;
				else
					pLeft->weight = BASE_NP_W;
			  }
		  }	
		  else
		  {
		  	  memcpy(p->head_buffer,pRight->head_buffer,pRight->head_count*sizeof(int));
			  p->head_count = pRight->head_count;
			  if (pRight->weight < BASE_NP_W)
			  {
			    if (pRight->chunk_state == CONJ_PRO)
			  		pRight->weight = BASE_NP_W;
				if (pRight->head_count == 1)//0811 token对应的单term chunk
					pRight->chunk_state = BASE_NP;
			  }
		  }
		  // fill a new chunk:head?
		  p->head_state = GET_CHUNK_STATE(rule);
		  p->chunk_state = BASENP_CENTER;
		  p->chunk_depth = pLeft->chunk_depth > pRight->chunk_depth ? pLeft->chunk_depth+1 : pRight->chunk_depth+1;
		  p->weight = _chunk_weight(p, CHUNK_W_STATE);
		  //与中心成分相结合时，仅保留中心成分信息
		  p->left = pLeft;
		  p->right = pRight;
		  p->left->next = pCenter;
		  p->left->next->next = pRight;
		  pRight->next = NULL;
	
		  // 更新chunk网络,没有进栈出栈处理,pRight 不动
		  if (index < left_bound)
		  	break;
		  //忽略动词
		  while (index >= left_bound && (chunks[index] == NULL || tokens[index].weight == 1))
		  	index--;
		  if (index < left_bound)
		  	break;
		  pCenter = &chkbuf[index--];
		  if (index < left_bound)
		  	break;
		  //忽略动词
		  while (index >= left_bound && (chunks[index] == NULL || tokens[index].weight == 1))
		  	index--;
		  pLeft = index >= left_bound ? chunks[index--] : NULL;  
		  pRight = p;
		  continue;
		}

		//右节点不动，需要判断左节点是否存在
		int ruleL = pLeft ? _state_transit(pLeft->head_state,pCenter->head_state) : 0;
		int ruleR = _state_transit(pCenter->head_state,pRight->head_state);
		// 左右搭配均不满足条件
		if( !ruleL && !ruleR )
		{	   		  
		  if( index < left_bound)
			break;
		  // 更新chunk网络,无进出栈处理，直接扔掉pCenter
		  //pRight 不动
		  pCenter = pLeft;
		  while (index >= left_bound && (chunks[index] == NULL || tokens[index].weight == 1))
		  	index--;
		  if (index < left_bound)
		  	break;
		  pLeft = chunks[index--];
		  continue;
		}
	
		double weight = -1.0f; 
		bool bLeft = false; // 左搭配是否成立?
		bool bRight = false;
		
		//1.左搭配是否可合成新chunk-------------------------------------------------------------------------
		if( ruleL && ruleR )
		{
		  // 如果规则优先级相同则比较权重
		    double w1 = _bigram_weight(pLeft,termLeft,pCenter,termCenter);
			double w2 = _bigram_weight(pCenter,termCenter,pRight,termRight); 
			if( w2 - w1 < 0.000001f )
			{
			  weight = w1;
			  bLeft = true;
			}
			//否则，选取右结合new
			else
			{
				weight = w2;
				bRight = true;
			}
		}
		else 
		{
		  if (ruleR == 0)
		  	bLeft = true;
		  else//处理右结合
		  	bRight = true;
		}
	
		// 如果左搭配
		if (bLeft)
		{
			chunk_t* pSub1 = NULL;
			chunk_t* pSub2 = NULL;
			char* pTerm1 = NULL;
			char* pTerm2 = NULL;
		
			//2.选取新chunk中的head------------------------------------------------------------------------------
			pSub1 = pLeft;
			pSub2 = pCenter;
			pTerm1 = termLeft;
			pTerm2 = termCenter;
			rule = ruleL;  
		
			// init a new chunk
			chunk_t* p = &chkbuf[ret++]; 
			memset(p,0,sizeof(*p));
		
			if( weight < 0.000001f )
			  weight = _bigram_weight(pSub1,pTerm1,pSub2,pTerm2);
		
			if( weight - MAX_TRANS_PROB > 0.000001f || // 权重高的直接合并子节点中心词为新节点中心词
				(!pSub1->chunk_state&&pSub1->left) ||  // 有一个节点为“虚组块”的合并
				(!pSub2->chunk_state&&pSub2->left) ||
				strlen(pTerm1) <= 2 || // 有一个节点含单字的合并
				strlen(pTerm2) <= 2 ) //0811--虚组块不提权
			{
			  p->head_state = GET_CHUNK_STATE(rule); 
			  _head_merge(p,pSub1,pSub2);
		
			  // 某些嵌套的子chunk标记为“虚组块”
			  if( strlen(pTerm1)<=2 && pSub2->chunk_state  )
				pSub2->chunk_state = 0;
			  if( strlen(pTerm2)<=2 && pSub1->chunk_state  )
				pSub1->chunk_state = 0;
			} 
			else  
			{
			  chunk_t* pSub = pSub1; // 默认位置在后的term更重要
			  int hd = GET_CHUNK_HEAD(rule);
			 
			  // 根据规则词典数据从子head中选取新head
			  if( (hd == 1) || 
				(pSub2->head_count>1&&!pSub2->chunk_state) )
			  {
				pSub = pSub1;		 
			  }
			  else if( (hd == 2) || 
				(pSub1->head_count>1&&!pSub1->chunk_state) )
			  {
				pSub = pSub2;
			  }
			  // 如果子节点中心词是一个组块则优先，如果均为组块，则权重大的优先
			  else if( pSub1->weight != pSub2->weight )
			  {
				pSub = pSub1->weight - pSub2->weight > 0.000001f ? pSub1 : pSub2;
			  } 		  
			  // 如果上述策略不能确定则选idf大的
			  else 
			  { 	 
				int idf1 = _unigram_get(pTerm1);
				if( idf1 < 0 )
				  idf1 = MAX_IDF;
				int idf2 = _unigram_get(pTerm2);		
				if( idf2 < 0 )
				  idf2 = MAX_IDF;
		
				if( idf1 > idf2 )
				  pSub = pSub1;
				else
				  pSub = pSub2;
			  }
		
			  p->head_state = GET_CHUNK_STATE(rule); 
			  memcpy(p->head_buffer,pSub->head_buffer,pSub->head_count*sizeof(int));
			  p->head_count = pSub->head_count;
			  //置psub的权重为5--0811
			}
		
			// fill a new chunk
			p->left = pSub1;
			p->right = pSub2;
			p->left->next = p->right;
			p->right->next = NULL;
			p->chunk_state = DEFALT_RELATE;
			p->chunk_depth = pSub1->chunk_depth > pSub2->chunk_depth ? pSub1->chunk_depth+1 : pSub2->chunk_depth+1;
			p->weight = _chunk_weight(p, CHUNK_W_STATE);
			// 标记“虚组块”，过滤无效chunk
			if( (pSub1->head_state==POS_AUX||pSub2->head_state==POS_AUX) || // 含助词
				(strlen(pTerm1)<=2 && 
				(pSub1->head_state!=pSub2->head_state)&& 
				(pSub1->head_state==POS_VERB||pSub1->head_state==POS_PRON)) || // 单个动词和介词
				(strlen(pTerm2)<=2 && 
				(pSub1->head_state!=pSub2->head_state) && 
				(pSub2->head_state==POS_VERB||pSub2->head_state==POS_PRON)) )
			  p->chunk_state = 0; 
				
			//3.更新chunk网络------------------------------------------------------------------------------------
			pCenter = p;
			while (index >= left_bound && (chunks[index] == NULL || tokens[index].weight == 1))
			  	index--;
			pLeft = index >= left_bound ? chunks[index--] : NULL;  
			continue;
		}
		//右搭配
		if (bRight)
		{
		  // init a new chunk
		  chunk_t* p = &chkbuf[ret++]; 
		  memset(p,0,sizeof(*p));
		  // fill a new chunk:head?
		  p->head_state = N;
		  p->chunk_state = BASENP_CENTER;
		  p->chunk_depth = pCenter->chunk_depth > pRight->chunk_depth ? pCenter->chunk_depth+1 : pRight->chunk_depth+1;
		  p->weight = _chunk_weight(p, CHUNK_W_STATE);//0810
		  //与中心成分相结合时，只保留中心成分
		  p->left = pCenter;
		  p->right = pRight;
		  p->left->next = pRight;
		  pRight->next = NULL;
		  if (strlen(termCenter) <=2 || strlen(termRight) <=2)
		  {
		  	_head_merge(p,pCenter,pRight);
			//修改pRight
			pRight = p;
		  }
		  else if (org_flag == 1)
		  {
			  //get the head info
			  memcpy(p->head_buffer,pCenter->head_buffer,pCenter->head_count*sizeof(int));
			  p->head_count = pCenter->head_count;
			  //0811对于机构，第一次构成修饰关系时，其中心为baseNp
			  if (pRight->head_count == 1 && pCenter->weight < BASE_NP_W)
			  {
				if (pCenter->head_count == 1)
					pCenter->chunk_state = BASE_NP;
				else
					pCenter->weight = BASE_NP_W;
			  }
		  }
		  else
		  {
			  _head_merge(p,pCenter,pRight);
			  if (pRight->weight < BASE_NP_W)
			  {
			    if (pRight->chunk_state == CONJ_PRO)
			  		pRight->weight = BASE_NP_W;
				if (pRight->head_count == 1)//0811 token对应的单term chunk
					pRight->chunk_state = BASE_NP;
			  }
		  }
		  //更新chunk 网络
		  if (pLeft == NULL)
		  	break;
		  pCenter = pLeft;
		  pRight = p;
		  while (index >= left_bound && (chunks[index] == NULL || tokens[index].weight == 1))
			  	index--;
		  pLeft = index >= left_bound ? chunks[index--] : NULL;  
		}
	  } // while		
}

/**
* @brief 组块分析接口: 输出组块buffer中，前size个为term级别的组块信息
		之后的为分析生成的组块信息
   		head_state为term 的词性或组块的词类;
   		chunk_state为term 的属性或组块的关系标示
* @param tokens - input tokens to be analyzed
* @param size - the num of input tokens 
* @param chkbuf -buffer to restore the output chunk info
* @param leng -size of chkbuf
* @return 输出组块buffer 的实际大小
*/
static int _chunk_parse(token_t tokens[], uint32_t size, chunk_t chkbuf[], int leng)
{
  int ret = 0; // count of used chunks
  int count = 0; // count of leaf chunks
  int index = 0; // index of chunk buffer

  //记录baseVp 和baseNp 信息
  base_info_t base_info;
  base_info.baseNp_num = 0;
  base_info.baseVp_num = 0;
  base_info.baseOp = -1;
  
  memset(base_info.baseNp, 0, MAX_PARSED_COUNT);
  memset(base_info.baseVp, 0, MAX_PARSED_COUNT);
	
	count = (int)size;
	
  if( count < 2 )
    return count;
  //预处理，将中间切散概念和上下位概念先做成短语
  ret = count;
  chunk_t** chunks = _preprocess_merge(tokens, count, chkbuf, leng, ret);
  if (chunks == NULL)
  	return count;

  //寻找base_info
  if (_get_base_info(&base_info, count, chunks, tokens) == -1)
  {
	 return -1;
  }
  //根据base_info 获得中心概念, 并确定组块分析运算器的中心成分  
  int center_rst = _base_parse(tokens, count, chkbuf, leng, &base_info, ret, chunks);
  if (center_rst == VP_CENTER)
  {
	//动词作为分段的界限,在后面和在baseNp 前不同
  	int left_bound = (base_info.baseVp[1] < base_info.baseVp[0])?base_info.baseVp[1]:0;
	_right_center_parse(count, tokens, chkbuf, leng, base_info.baseVp[0], left_bound, ret, chunks);
	//首段两个term 才有意义parse
	if (left_bound > 1)
		_defalt_parse(tokens, left_bound, chkbuf, leng, 0, ret, chunks);
	else if ((base_info.baseVp[1] > base_info.baseVp[0]) && base_info.baseVp[1]+1 < count)
	{
		_defalt_parse(tokens, count, chkbuf, leng, left_bound+1, ret, chunks);
	}
  }
  else
  {
   if (center_rst == NP_CENTER || center_rst == ORG_CENTER)
   {
  //根据属性类别，同类属性的只取最后一个:根据case调整
	  if (base_info.baseNp_num > 1)
	  {
		for (int np_index = base_info.baseNp_num - 2; np_index >= 0; np_index--)
		{
			for (int inner = base_info.baseNp_num -1; inner > np_index; inner--)
			{
				if (base_info.baseNp[inner] == -1 || chunks[base_info.baseNp[inner]] == NULL 
					|| base_info.baseNp[np_index] == -1 || chunks[base_info.baseNp[np_index]] == NULL)
						continue;
				if ((chunks[base_info.baseNp[inner]]->chunk_state == chunks[base_info.baseNp[np_index]]->chunk_state)
					|| (chunks[base_info.baseNp[inner]]->chunk_state == N && chunks[base_info.baseNp[np_index]]->chunk_state == CONJ_N)
					|| (chunks[base_info.baseNp[inner]]->chunk_state == CONJ_N && chunks[base_info.baseNp[np_index]]->chunk_state == N))
				{
					base_info.baseNp[np_index] = -1;
				}
			}
		}
  	  }

	  for (int np_index = base_info.baseNp_num-1; np_index >= 0; np_index--)
	  {
		  if (base_info.baseNp[np_index] != -1 && chunks[base_info.baseNp[np_index]] != NULL
			  && chunks[base_info.baseNp[np_index]]->chunk_state == 0)
		  {
			  if (chkbuf[base_info.baseNp[np_index]].chunk_state != 0)
			    	LOG_ERROR("chkbuf!=chunks");
			  chkbuf[base_info.baseNp[np_index]].chunk_state = BASE_NP;
		  }
	  }
	  if (center_rst == ORG_CENTER)
	  {
		_right_center_parse(count, tokens, chkbuf, leng, base_info.baseOp, 0, ret, chunks);
		if (base_info.baseOp < count - 2)
			_defalt_parse(tokens, count, chkbuf, leng, base_info.baseOp+1, ret, chunks);
	  }
	  else if (center_rst == NP_CENTER)
	  {
		int left_bound = 0;
		for (int vi = 0; vi < (int)base_info.baseNp_num; vi++)
		{		
			if (base_info.baseNp[vi] == -1)
				continue;
			if (base_info.baseNp[vi] > 0)
			{
				_right_center_parse(count, tokens, chkbuf, leng, base_info.baseNp[vi], 
									left_bound, ret, chunks);
				left_bound = base_info.baseNp[vi]+1;
			}
		}
		if (left_bound < count-1)
			_defalt_parse(tokens, count, chkbuf, leng, left_bound, ret, chunks);
	  }
   }
   else if (center_rst == BASE_PARSE)
   {
	  	int np_i = 0;
		int vp_i = 0;
		//仅选取被baseNp 和baseVp 命中的处理以及机构成分处理
		for (int jj=0; jj<(int)count; jj++)
		{
			if (jj == base_info.baseOp)
				continue;
			if (vp_i >= (int)base_info.baseVp_num && np_i >= (int)base_info.baseNp_num)
				break;
			if (vp_i < (int)base_info.baseVp_num && jj == (int)base_info.baseVp[vp_i])
			{
				vp_i++;
				continue;
			}
			else if (np_i < (int)base_info.baseNp_num && jj == (int)base_info.baseNp[np_i])
			{
				np_i++;
				continue;
			}
			else
			{
				chunks[jj] = NULL;
			}
		}
		_defalt_parse(tokens, count, chkbuf, leng, index, ret, chunks);
	  }
	  else
	  {
		_defalt_parse(tokens, count, chkbuf, leng, index, ret, chunks);
	  } 
  }
  //free
  if (chunks != NULL)
  {
	free(chunks);
	chunks = NULL;
  }
  return ret;
}

/**
* @brief 装载字典信息
* @param dictpath 字典路径
* @return 组块字典结构指针
*/
static chunk_dict_t* _dict_load(char* dictpath)
{
  chunk_dict_t* p = new chunk_dict_t();
  if( !p )
  {
    LOG_ERROR("error: creating dictionary object failed");
    return NULL;
  }

  p->state_size = 0;
  p->state_buffer = 0;
 
  // load rule data, which is required
  char filename[MAX_PATH_LENG] = {0};
  snprintf(filename,MAX_PATH_LENG,"%s/%s%s",dictpath,CHUNK_RULE_DICT,DATA_DICT_POSTFIX);
  FILE* fp = fopen(filename,"rb");
  if( !fp )
  {
    LOG_ERROR("error: opening binary data dictionary failed");
    delete p;
    return NULL;
  }

  fread(&p->state_size,sizeof(int),1,fp);
  if( p->state_size )
  {
    int size = p->state_size*p->state_size*p->state_size;
    p->state_buffer = new unsigned int[size];
    if( !p->state_buffer )
    {      
      LOG_ERROR("error: allcating memory for chunk_dict_t::state_buffer failed");
      delete p;
      fclose(fp);
      return NULL;
    }
    else
    {
      fread(p->state_buffer,sizeof(int),size,fp);
    }
  }
  else
  {
    LOG_ERROR("error: loading binary rule dictionary failed");
    delete p;
    fclose(fp);
    return NULL;
  }
  
  // load hash dict data
  strncpy(filename,TERM_UNIGRAM_DICT,strlen(TERM_UNIGRAM_DICT)+1);
  p->unigram_dict = ds_load(dictpath, filename);
  if( !p->unigram_dict )
  {
    LOG_ERROR("error: loading binary term info dictionary failed, jump over");    
  }

  strncpy(filename,TERM_BIGRAM_DICT,strlen(TERM_BIGRAM_DICT)+1);
  p->bigram_dict = ds_load(dictpath, filename);
  if( !p->bigram_dict )
  {
    LOG_ERROR("error: loading binary bigram dictionary failed, jump over");    
  }

  fclose(fp);
  return p;
}

/**
* @brief 释放字典空间
* @param p  组块字典结构指针
* @return 
*/
static void _dict_free(chunk_dict_t* p)
{
  if( !p )
    return;

  if( p->state_buffer )
  {
    delete p->state_buffer;
    p->state_buffer = 0;
  }
  p->state_size =0;

  if( p->unigram_dict )
  {
    ds_del(p->unigram_dict);
    p->unigram_dict = 0;
  }

  if( p->bigram_dict )
  {
    ds_del(p->bigram_dict);
    p->bigram_dict = 0;
  }

  delete p;
}

// <api>
/**
* @brief 组块分析初始化，主要进行字典信息的装载
* @param dictpath 字典路径
* @return 
*/
int chk_open( const char* dictpath )
{
  if( gChkDict != NULL )
    _dict_free(gChkDict);

  if( strlen(dictpath) >= MAX_PATH_LENG - 1 )
  {
    LOG_ERROR("error: length of path is too long");
    return -1;
  }

  char path[MAX_PATH_LENG] = {0};  
  strncpy(path,dictpath,strlen(dictpath)+1);
  strncat(path,"/",1);

  gChkDict = _dict_load( path );
  
  return (gChkDict == NULL);
}
 
/**
* @brief 组块分析对外接口，进行组块分析和term的赋权
		可一次parsing的文本中包含token的最大数目限制为MAX_PARSED_COUNT
* @param tokens - input tokens to be analyzed
* @param size - num of input tokens
* @param chunks - buffer to restore output chunk info
* @return leng - size of output buffer
*/
int chk_parse( token_t tokens[], uint32_t size, chunk_t chunks[], uint32_t leng )
{
  if( size > MAX_PARSED_COUNT )
    size = MAX_PARSED_COUNT;

	// initing ...  
	int count = 0;
  for( int i=0; i<(int)size; i++ )
  {
  	if( count >= (int)leng )
      return count;

    chunk_t* p = &chunks[count++];
    memset(p,0,sizeof(*p));    
    p->head_count = 1;
		//record the attribute
		int attri = _unigram_get(tokens[i].buffer);	
		if (attri < 0)
		{
			attri = 0;		
			//将idf 信息记录到token的weight域中
			tokens[i].weight = tokens[i].type != POS_DELIM ? MAX_IDF : 0;
		}
		else
		{
			//record idf
			tokens[i].weight = GET_TERM_IDF(attri); 
			//get attribute		
			attri = GET_TERM_ATTRIBUTE(attri);			
		}
		
		p->chunk_state = attri; // invalid chunk
    p->head_buffer[0] = i;
    p->head_state = tokens[i].type;
		p->weight = 0;
  }
  
   // chunk parsing
  int cnt = _chunk_parse(tokens,size,chunks,leng);

  // weight computing
  _term_weight_new(tokens, size, chunks);
  return cnt;
}
/**
* @brief 组块分析对外接口，对经过专门标注的token序列进行组块分析和term的赋权
		可一次parsing的文本中包含token的最大数目限制为MAX_PARSED_COUNT
* @param tokens - input tokens to be analyzed
* @param size - num of input tokens
* @param chunks - buffer to restore output chunk info
* @return leng - size of output buffer
*/
int chk_parse2( token_t tokens[], uint32_t size, chunk_t chunks[], uint32_t leng )
{
  if( size > MAX_PARSED_COUNT )
    size = MAX_PARSED_COUNT;
	
	// initing ...
	int count = 0;
  for( int i=0; i<(int)size; i++ )
  {
  	if( count >= (int)leng )
      return count;

    chunk_t* p = &chunks[count++];
    memset(p,0,sizeof(*p));    
    p->head_count = 1;
		//record the attribute
		int attri = _unigram_get(tokens[i].buffer);	
		if (attri < 0)//将idf 信息记录到token的weight域中								
			tokens[i].weight = tokens[i].type != POS_DELIM ? MAX_IDF : 0;		
		else
			tokens[i].weight = GET_TERM_IDF(attri); 
			
		if (tokens[i].prop.m_hprop == 0x0)
			attri = 0;
		else
			attri = _token_prop2state( tokens[i].prop.m_hprop );
		
		p->chunk_state = attri; // invalid chunk
    p->head_buffer[0] = i;
    p->head_state = tokens[i].type;
		p->weight = 0;
  }
  
   // chunk parsing
  int cnt = _chunk_parse(tokens,size,chunks,leng);

  // weight computing
  _term_weight_new(tokens, size, chunks);
  return cnt;
}
/**
* @brief 组块分析结束时调用
		释放组块分析占用的字典内存空间
* @param
* @return 
*/
void chk_close()
{
  if( gChkDict != NULL )
  {
    _dict_free(gChkDict);
    gChkDict = NULL;
  }
}

// </api>
