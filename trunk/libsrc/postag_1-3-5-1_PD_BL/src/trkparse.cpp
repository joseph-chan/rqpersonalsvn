/**
 * @file trkparse.cpp
 * @brief �Զ̴��������ɷ��������Դ��ﰴ��Ҫ�Խ��зּ���
 * @author David Dai
 * @date 2009/01/15
 * @version 1.0 
 */

#include "itrunk.h"
#include "iwordrank.h"
#include "tagdict.h"

#define MAX_BIGRAM_LENG 512
#define MIN_TRUNK_COUNT 1
#define MAX_RANK 3
#define RANK_THRES 0.05

/* �ִ�������غ� */
#define IS_ORGNAME(property)      ((property.m_hprop) & 0x00001000)
#define IS_LOC(property)          ((property.m_hprop) & 0x00000800)
#define IS_HUMAN_NAME(property)   ((property.m_hprop) & 0x00000040) 
#define IS_BOOKNAME(property)     ((property.m_hprop) & 0x08000000)
#define IS_UNIVERSITY(property)   ((property.m_lprop) & 0x00000001)
#define IS_SOFTNAME(property)     ((property.m_lprop) & 0x00000002)
#define IS_FMNAME(property)       ((property.m_lprop) & 0x00000004)
#define IS_BRAND(property)        ((property.m_lprop) & 0x00000010)
#define IS_ASCIIWORD(property)    ((property.m_hprop)& 0x00000010) 

#define IS_HZ(property)           ((property.m_hprop) & 0x01000000)
#define IS_MARK(property)         ((property.m_hprop) & 0x00000002)  
#define IS_SPACE(property)        ((property.m_lprop) & 0x00000040)

/* ר�������غ� */
#define TYPE_NAME(type)  (type & 0x00000001)
#define TYPE_LOC(type)   (type & 0x00000002)
#define TYPE_ORG(type)   (type & 0x00000004)
#define TYPE_STOP(type)  (type & 0x80000000)

/* ר�����Ժ� */
#define PROP_PUBLIC(prop) (prop & 0x80000000)  // ͨ�����ԣ�����Ӧ����ʵ�����

/* �����ϵģʽ */
#define PATT_LOC(patt)   (patt & 0x00000002)
#define PATT_DESC_ORDER(patt)  (patt & 0x80000000)

/* ����Term��� */
#define PROP_ORG_SUFFIX(lprop,hprop)	((lprop & 0x00000004) || (hprop & 0x00000004))

/*! type of trunk dict */
typedef struct trunk_dict_t
{ 
  odict_t*  type_dict;   // ���������Ϣ�ʵ�
  odict_t*  prop_dict;   // ����������Դʵ�
  odict_t*  patt_dict;   // ����������ʵ� 
} trunk_dict_t;


//-------------------------------------------------------------------------------------------//
static void _dict_free(trunk_dict_t* p)
{
  if( !p )
    return; 

  if( p->type_dict )
  {
    odb_destroy_search(p->type_dict);
    p->type_dict = 0;
  }

  if( p->prop_dict )
  {
    odb_destroy_search(p->prop_dict); 
    p->prop_dict = 0;
  }

  if( p->patt_dict )
  {
    odb_destroy_search(p->patt_dict);
    p->patt_dict = 0;
  } 

  delete p;
}
static trunk_dict_t* _dict_load(char* dictpath)
{
  char filename[MAX_PATH_LENG] = {0};
  trunk_dict_t* p = new trunk_dict_t();
  if( !p )
  {
    LOG_ERROR("error: creating dictionary object failed");
    return NULL;
  }
  memset(p,0,sizeof(trunk_dict_t));    

  strncpy(filename,TERM_TYPE_DICT,strlen(TERM_TYPE_DICT)+1);
  p->type_dict = odb_load_search(dictpath, filename); 
  if( !p->type_dict || p->type_dict==(void*)ODB_LOAD_NOT_EXISTS )
  {
    LOG_ERROR("error: loading binary term type dictionary failed");    
    goto over;
  }
  
  strncpy(filename,TERM_PROP_DICT,strlen(TERM_PROP_DICT)+1);
  p->prop_dict = odb_load_search(dictpath, filename); 
  if( !p->prop_dict || p->prop_dict==(void*)ODB_LOAD_NOT_EXISTS )
  {
    LOG_ERROR("error: loading binary term property dictionary failed");    
    goto over;
  }

  strncpy(filename,TERM_PATT_DICT,strlen(TERM_PATT_DICT)+1);
  p->patt_dict = odb_load_search(dictpath, filename); 
  if( !p->patt_dict || p->patt_dict==(void*)ODB_LOAD_NOT_EXISTS )
  {
    LOG_ERROR("error: loading binary term pattern dictionary failed");   
    goto over;
  } 

  return p;

over:
  _dict_free(p);
  return NULL;
}

/// ��ȡ����Ԥ���������Ϣ��
inline unsigned _get_type(trunk_dict_t* pdict, char* key)
{
  if( !pdict->type_dict || !key )
    return 0;

  sodict_snode_t dictNode;  
  creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
  if( ODB_SEEK_OK == odb_seek_search(pdict->type_dict, &dictNode) )
    return dictNode.cuint1;

  return 0;
}

/// ��ȡĳ��������Ƿ�ΪĳЩ�������Դʣ����ط�0��ʾ������ĳ��������ԡ�
int _get_type_prop(trunk_dict_t* pdict, char* key, unsigned& lprop, unsigned& hprop)
{
  if( !pdict->prop_dict || !key )
    return 0;

  sodict_snode_t dictNode;  
  creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
  if( ODB_SEEK_OK == odb_seek_search(pdict->prop_dict, &dictNode) )
  {
    lprop = dictNode.cuint1; // ���ص�32λ����-��������
    hprop = dictNode.cuint2; // ���ظ�32λ����-��������  
  }

  return lprop|hprop;
}

/// ��ȡ�����Ԫ������Ϣ������1��ʾĬ�ϵĹ�ϵ������-1��ʾ�����ϵ��type�������ؾ����ϵ���͡�
/// �磺��������λ���Է���1��ʾǰһ��term�Ǻ�һ������λ��-1��֮��
inline int _get_pattern(trunk_dict_t* pdict, char* term1, char* term2, unsigned& type) 
{
  if( !term1 || !term2 )
    return 0;
  if( !pdict->patt_dict )
    return 0;

  int len1 = strlen(term1);
  int len2 = strlen(term2);
  char key[MAX_BIGRAM_LENG] = {0};

  if( len1 > 255 )
    len1 = 255;
  if( len2 > 255 )
    len2 = 255;

  // ��˳����Ϻ��ѯ
  int ret = 1;
  if( strcmp(term1,term2) > 0 )
  {
    memcpy(key,term2,len2);
    memcpy(key+len2,term1,len1);
    ret = -1;
  }
  else
  {
    memcpy(key,term1,len1);
    memcpy(key+len1,term2,len2);
  }
   
  sodict_snode_t dictNode;
  creat_sign_f64(key, strlen(key), &dictNode.sign1, &dictNode.sign2);
  if( ODB_SEEK_OK == odb_seek_search(pdict->patt_dict, &dictNode) )
  {
    type = dictNode.cuint1;
    if( PATT_DESC_ORDER(type) )      
      ret *= -1;
    return ret;    
  }

  return 0;
}

/// ���ݴʷ������жϣ�����true��ʾ���߹�ϵ���ܣ����ܲ��ɷָ���
inline bool _judge_by_grammar(token_t& tok1, token_t& tok2)
{
  // V + V/Vg
  if( tok1.type==POS_VERB && (tok2.type==POS_VERB||tok2.type==POS_VG) && IS_HZ(tok2.prop) )
    return true;

  // N/ADJ + Ng
  if( (tok1.type==POS_DEFAULT||tok1.type==POS_ADJ) && tok2.type==POS_NG && IS_HZ(tok2.prop) )
    return true;
    
  return false;
}

/// get the subscript of the maxinum among 3 numbers.
inline int _get_max3(int a, int b, int c)
{  
  if( c>=a && c>=b )
    return 2;  
  if( b>=a && b>=c )
    return 1;
  if( a>b && a>c )
    return 0;
  return 0;
}

/// �Ƿ�Ϊר��
inline int _is_proper_noun( token_t& tok )
{
  if( tok.type==POS_PER || tok.type==POS_ORG || tok.type==POS_NZ )
    return 1;

  if( IS_ORGNAME(tok.prop) || IS_HUMAN_NAME(tok.prop) || IS_BOOKNAME(tok.prop) ||
      IS_UNIVERSITY(tok.prop) || IS_FMNAME(tok.prop) || IS_BRAND(tok.prop) ) // || IS_SOFTNAME(tok.prop) // �������׼ȷ
    return 1;

  return 0;
}

// �����ʴʷ���1 ����0
int is_interrogative(token_t& tok, Sdict_build* pword_tag_dict)
{
    Sdict_snode snode;
    char term_sign[256];
    //���Ա��+word
    snprintf(term_sign, sizeof(term_sign), "%d_%s", 1, tok.buffer);
    creat_sign_f64(term_sign, strlen(term_sign), &snode.sign1, &snode.sign2);
    if(db_op1 (pword_tag_dict, &snode, SEEK) == 1)
    {
        if(1==snode.other && 1==snode.code)
        {//˵�������ʴ�
            //printf("%s is interrogative\n", tok.buffer);
            return 1;
        }
    }

    return 0;
}

/// �Ƿ�Ϊ�����Գɷ�
inline int _is_modifier( token_t& tok, Sdict_build* pword_tag_dict)
{
  // �����ʣ���λ�����м��
  if( tok.type==POS_F || tok.type==POS_S )
    return 1; 

/*  // �������м�㣬��������ǿ�޶�
  if( tok.type==POS_LOC || IS_LOC(tok.prop) )
    return 3;    */

  // ʱ�䣬���֣�������Ϊǿ�޶�
  if( tok.type==POS_TIME || tok.type==POS_NUMBER ) 
    return 3;

  if( is_interrogative(tok, pword_tag_dict) )
  {//���ʴʻ����м��
     return 1;
  }

  // ��ʶ��汾�ţ�������Ϊǿ�޶�
  if( (tok.length>1) && (tok.buffer[0]=='V'||tok.buffer[0]=='v') && (tok.buffer[1]>=48&&tok.buffer[1]<=57) )
    return 3;

  // ʶ���������֣�������Ϊǿ�޶�
  if( (tok.length>0)  )
  {//������ĸ�� ���Ʒ�� Ӧ�ù鵽����
      unsigned int i = 0;
      for(i=0; i < tok.length; ++i)
      {
          if( !(tok.buffer[i]>=48&&tok.buffer[i]<=57 || '.'==tok.buffer[i]) )
          {
              return 0;
          }
      }
    return 3;
  }

  return 0;
}
inline bool _is_non_sense( token_t& tok )
{
  if( tok.type==POS_DELIM || tok.type==POS_CONJ || tok.type==POS_PREP || 
      tok.type==POS_AUX || tok.type==POS_ONO || tok.type==POS_MOOD )
    return true;

  if( IS_MARK(tok.prop) || IS_SPACE(tok.prop) )
    return true;

  return false;
}

/*!
 * ����������������ɷ�����
 * 1)����������������MIN_TRUNK_COUNT��
 * 2)�зֽ����Ϊ���֣�
 * 3)ȫ��Ϊascii���ַ�query��
 */
bool _is_filtered(token_t tokens[], int size)
{
  if(size <= MIN_TRUNK_COUNT)
    return true;

  bool filtered1 = true;  
  for(int i=0; i<size; i++)
  {
    if( !IS_HZ(tokens[i].prop) )
    {
      filtered1 = false;
      break;
    }
  }

  bool filtered2 = true;
  for(int i=0; i<size; i++)
  {
    if( !IS_ASCIIWORD(tokens[i].prop) )
    {
      filtered2 = false;
      break;
    }

    // ��ֹprop�ֶ�û�и�ֵ�����prop��ֵ��������������ࡣ
    for(unsigned j=0; j<tokens[i].length; j++)
    {
      if( tokens[i].buffer[j] & 0x80 )
      {
        filtered2 = false;
        break;
      }
    }
    if( !filtered2 )
      break;
  }

  return filtered1|filtered2;
}

/// ���ݷִʴʵ����ԣ����Ա�ע��Ϣ����Ԥ����Ĵ���������Դʵ���зֲ�ͶƱ
void _rank_by_property(trunk_dict_t* pdict, Sdict_build* pword_tag_dict, token_t tokens[],
        int size, unsigned char rankSet[][MAX_RANK], trunk_t trunks[])
{ 
   if(size < 1) 
       return;
  for(int i=0; i<size; i++)
  {    
    unsigned type = _get_type(pdict,tokens[i].buffer);
    
    // ר�����
    if( _is_proper_noun(tokens[i]) )
    {
      rankSet[i][TERM_RANK_TRUNK] += 2;
#ifdef _DEBUG
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: proper noun\n",tokens[i].buffer,TERM_RANK_TRUNK,2);
#endif
      continue; // ר������
    }    
    else if( TYPE_LOC(type) || tokens[i].type==POS_LOC || IS_LOC(tokens[i].prop)) // �������Ϊǿ�޶� PATCH:����ҲͶƱ������
    {
      trunks[i].type = TERM_MODI_HIGH;
    }  
    else if( type && !TYPE_STOP(type) ) // stopwords�������������ר��
    {
      rankSet[i][TERM_RANK_TRUNK] += 2;
#ifdef _DEBUG
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: term type\n",tokens[i].buffer,TERM_RANK_TRUNK,2);
#endif
      continue; // ר������
    }    

    // ���δ�
    int ret = _is_modifier(tokens[i], pword_tag_dict);
    if( ret )
    {
      rankSet[i][TERM_RANK_MODIFY] += 2;

      if( ret == 3 ) // ���Ϊǿ�޶�
        trunks[i].type = TERM_MODI_HIGH;

      if(i+1 < size)
      {
          if(tokens[i].type==POS_NUMBER && tokens[i+1].type==POS_QUAN)
          {//����+����
              rankSet[i][TERM_RANK_MODIFY] += 1;
          }
      }
#ifdef _DEBUG
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: postag/type\n",tokens[i].buffer,TERM_RANK_MODIFY,2);
#endif
    }
    else if(tokens[i].type==POS_LOC || IS_LOC(tokens[i].prop) )
    {//��Ҫ�Բ��Ǻܸߵĵ��������м��
        if(trunks[i].weight <= 2*1.0/size)
        {
            rankSet[i][TERM_RANK_MODIFY] += 2;
        }
        else
        {
            if(i+1<size && POS_DEFAULT<=tokens[i+1].type && tokens[i+1].type<=POS_NZ )
            {//���������ʣ�˵����ǰ���޶�
                rankSet[i][TERM_RANK_MODIFY] += 2;
            }
        }
        trunks[i].type = TERM_MODI_HIGH;
    }
    
    // �������
    if( _is_non_sense(tokens[i]) )
    {
      rankSet[i][TERM_RANK_OMIT] += 2;

#ifdef _DEBUG
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: postag\n",tokens[i].buffer,TERM_RANK_OMIT,2);
#endif
    }

    // �����������
    unsigned lprop=0, hprop=0;
    ret = _get_type_prop(pdict,tokens[i].buffer,lprop,hprop);    
    if( ret ) // ������Դ�ȫ��ͶƱ���м�㣬���Ǳ��ϸ�ֲ��
    {
      if (PROP_ORG_SUFFIX(lprop,hprop))
	    {
		    //No-Vote For these terms
	    }
	    else
	    {
		    rankSet[i][TERM_RANK_MODIFY] += 2;
  #ifdef _DEBUG
		    fprintf(stderr,"term: %-16s vote: %d-%d\twhys: type prop\n",tokens[i].buffer,TERM_RANK_MODIFY,2);
  #endif
	    }

      if( lprop ) // ���ǿ���������м��޶�
        trunks[i].type = TERM_MODI_MID;
      else if( hprop ) // ��������������޶� 
        trunks[i].type = TERM_MODI_LOW;
    }   

  }

}

/// ���ݴ����ϵģʽ���зֲ�ͶƱ����ǰƥ�䷽���Ƚϼ򵥣��д�ϸ����
/// ����λ���Կ�����Ϊ����������Ȩ����1������term�����Ĳ���Ӱ���ʧȥ���ã�
void _rank_by_pattern( trunk_dict_t* pdict, token_t tokens[], int size, unsigned char rankSet[][MAX_RANK], trunk_t trunks[] )
{
  for(int i=0; i<size-1; i++)
  {
    unsigned type = 0;
    int ret = _get_pattern(pdict,tokens[i].buffer,tokens[i+1].buffer,type);
    if( !ret )
      continue;

    if( ret == 1 )
    {
      if( PATT_LOC(type) )  // ������λ����ʡ�Բ�
      {
        rankSet[i][TERM_RANK_OMIT] += 4;
        trunks[i].type = TERM_MODI_UNESCAPE;
      }
      else
        rankSet[i][TERM_RANK_MODIFY] += 3;
      rankSet[i+1][TERM_RANK_TRUNK] += 3;
#ifdef _DEBUG
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: pattern punished\n",tokens[i].buffer,TERM_RANK_MODIFY,3);
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: pattern weighted\n",tokens[i+1].buffer,TERM_RANK_TRUNK,3);
#endif
    }
    else if( ret == -1 )
    {
      if( PATT_LOC(type) )  // ������λ����ʡ�Բ�
      {
        rankSet[i+1][TERM_RANK_OMIT] += 4;
        trunks[i+1].type = TERM_MODI_UNESCAPE;
      }
      else
        rankSet[i+1][TERM_RANK_MODIFY] += 3;
      rankSet[i][TERM_RANK_TRUNK] += 3;

#ifdef _DEBUG
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: pattern punished\n",tokens[i+1].buffer,TERM_RANK_MODIFY,3);
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: pattern weighted\n",tokens[i].buffer,TERM_RANK_TRUNK,3);
#endif
    }
  }
}

//��֤�Ӵ�С����
static int cmp_trunk_wgt(const void *pair1,  const void *pair2)
{
    cmp_trunk_t* pt1 = (cmp_trunk_t*) (pair1);
    cmp_trunk_t* pt2 = (cmp_trunk_t*) (pair2);
    if(pt1->weight > pt2->weight)
    {
        return -1;
    }

    return 1;
}

//stopword return 1 or 0
int is_stopword(token_t& tok, Sdict_build *pword_tag_dict)
{
    char term_sign[256];
    snprintf(term_sign, sizeof(term_sign), "%d_%s", 3, tok.buffer);
    Sdict_snode snode;
    creat_sign_f64 (term_sign, strlen(term_sign), &snode.sign1, &snode.sign2);
    if(1 == db_op1 (pword_tag_dict, &snode, SEEK))
    {//����ܵ͵Ĵ�
        return 1;
    }
    snprintf(term_sign, sizeof(term_sign), "%d_%s", 2, tok.buffer);
    creat_sign_f64 (term_sign, strlen(term_sign), &snode.sign1, &snode.sign2);
    if(1 == db_op1 (pword_tag_dict, &snode, SEEK))
    {//������
        return 0;
    }
    if(1 == tok.length)
    {
        if(tok.buffer[0]>='A'&&tok.buffer[0]<='Z' || tok.buffer[0]>='a'&&tok.buffer[0]<='z' ||
                tok.buffer[0]>='0'&&tok.buffer[0]<='9' )
        {
            return 0;
        }
        return 1;
    }   

    if( IS_MARK(tok.prop) || IS_SPACE(tok.prop) )
        return 1;
    if( tok.type==POS_DELIM )
        return 1;

    return 0;
}

//ͨ��term���ֲ�
int _rank_by_term_num_wgt(token_t tokens[], int size, unsigned char rankSet[][MAX_RANK],
        trunk_t trunks[], Sdict_build *pword_tag_dict)
{
    if(size < 1)
        return 0;
    cmp_trunk_t cmp_trk[MAX_TOKEN_COUNT];
    char is_stop_word[MAX_TOKEN_COUNT];
    int num = size;
    if(num > MAX_TOKEN_COUNT)
        num = MAX_TOKEN_COUNT;
    int i = 0, non_stopword_num = 0;
    for(i=0; i < num; ++i)
    {
        is_stop_word[i] = is_stopword( tokens[i], pword_tag_dict);
        if(is_stop_word[i])
        {
            //printf("stopword:%s\n", tokens[i].buffer);
            cmp_trk[i].weight = 0;
        }
        else
        {
            cmp_trk[i].weight = trunks[i].weight;
            ++non_stopword_num;
        }
        cmp_trk[i].ind = i;
    }

    //printf("non_stopword_num=%d all_term_num=%d\n", non_stopword_num, size);

    qsort(cmp_trk, num, sizeof(cmp_trunk_t), cmp_trunk_wgt );
    if(non_stopword_num < 2)
    {
        int ind = cmp_trk[0].ind;
        rankSet[ind][TERM_RANK_TRUNK] += 2;
    }
    else if(non_stopword_num < 3)
    {
        int big_ind = cmp_trk[0].ind;
        int small_ind = cmp_trk[1].ind;
        if(cmp_trk[1].weight>0)
        {
            if(cmp_trk[0].weight/cmp_trk[1].weight >= 3)
            {
                rankSet[big_ind][TERM_RANK_TRUNK] += 1;
                rankSet[small_ind][TERM_RANK_MODIFY] += 1;
            }
            else
            {
                rankSet[big_ind][TERM_RANK_TRUNK] += 1;
                rankSet[small_ind][TERM_RANK_TRUNK] += 1;
            }
        }
        else
        {
            rankSet[big_ind][TERM_RANK_TRUNK] += 1;
            rankSet[small_ind][TERM_RANK_TRUNK] += 1;
        }
    }
    else if(non_stopword_num < 4)
    {
        int k = 0;
        for(; k < non_stopword_num-1; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_TRUNK] += 1;
        }
        int ind = cmp_trk[non_stopword_num-1].ind;
        rankSet[ind][TERM_RANK_MODIFY] += 1;
    }
    else if(non_stopword_num < 5)
    {
        int k = 0;
        for(; k < non_stopword_num-2; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_TRUNK] += 1;
        }
        for(; k < non_stopword_num; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_MODIFY] += 1;
        }
    }
    else if(non_stopword_num < 6)
    {
        int k = 0;
        for(; k < 3; ++k)
        {//����Ҫ������Ҫ���ٵ���
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_TRUNK] += 1;
        }
        for(; k < non_stopword_num; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_MODIFY] += 1;
        }
        if(cmp_trk[1].weight>=1.0/5.0 && cmp_trk[1].weight>=cmp_trk[2].weight*1.5)
        {
            int ind = cmp_trk[2].ind;
            rankSet[ind][TERM_RANK_TRUNK] -= 1;
            rankSet[ind][TERM_RANK_MODIFY] += 1;
        }
    }
    else if(non_stopword_num < 7)
    {
        int k = 0;
        for(; k < 3; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_TRUNK] += 1;
        }
        for(; k < non_stopword_num; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_MODIFY] += 1;
        }
        //������Ҫ�Կɿ��Ǽ��ٺܲ���Ҫ��term�����β�
        //if(cmp_trk[2].weight>=1.0/6.0 && cmp_trk[2].weight>=cmp_trk[3].weight*1.5)
        //{
        //    int ind = cmp_trk[3].ind;
        //    rankSet[ind][TERM_RANK_TRUNK] -= 1;
        //    rankSet[ind][TERM_RANK_MODIFY] += 1;
        //}

    }
    else if(non_stopword_num < 8)
    {
        int k = 0;
        for(; k < 3; ++k)
        {
            int ind = cmp_trk[k].ind;
             rankSet[ind][TERM_RANK_TRUNK] += 1;
        }
        for(; k < non_stopword_num; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_MODIFY] += 1;
        }
        //7��term��ʡ����Ҫ����
        if( cmp_trk[2].weight<=cmp_trk[3].weight*1.1)
        {
            int ind = cmp_trk[3].ind;
            rankSet[ind][TERM_RANK_TRUNK] += 1;
            rankSet[ind][TERM_RANK_MODIFY] -= 1;
        }
    }
    else
    {
        int k = 0;
        for(; k < 4; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_TRUNK] += 1;
        }   
        for(; k < non_stopword_num; ++k)
        {
            int ind = cmp_trk[k].ind;
            rankSet[ind][TERM_RANK_MODIFY] += 1;
        }
    }

    int k = non_stopword_num;
    for( ; k < num; ++k)
    {
        int ind = cmp_trk[k].ind;
        rankSet[ind][TERM_RANK_OMIT] += 1;
    }

    return 0;
}

/// ���ݶ�̬term��Ҫ�Խ��зֲ㡣
void _rank_by_weight(rank_dict_t* pdict, token_t tokens[], int size, unsigned char rankSet[][MAX_RANK],
        trunk_t trunks[], Sdict_build *pword_tag_dict)
{
  // ����term�����Ҫ��
  float ranks[MAX_TOKEN_COUNT] = {0}; 
  const char* terms[MAX_TOKEN_COUNT] = {0};    
  for(int i=0; i<size; i++)
    terms[i] = tokens[i].buffer;
  if( !wdr_evaluate(pdict,terms,size,ranks) )
  {
    // term��Ҫ�Լ��������ȫ��ͶƱ������
    for(int i=0; i<size; i++)
      rankSet[i][TERM_RANK_TRUNK] += 2;
    LOG_ERROR("error: term evaluating failed, ignore it");
    return;
  }

  int cnt = 0;
  for(int i=0; i<size; i++)
  {
    // �м��ϸ������Ĭ��û��ϸ��
    trunks[i].type = 0;     

    // ����û�м����term��Ҫ�Ե�term
    float rank = ranks[i];
    if( rank > 0.0001 )    
      cnt++;
    trunks[i].weight = rank;
  }

  // �趨��̬��ֵ
  float limen = 0.0f; 
  if( cnt ) 
    limen = (1.0f/cnt)*0.618;   // �ƽ�ָ�  
  if( limen < RANK_THRES )      // ��֤�����ֵ
    limen = RANK_THRES;

  // ���ݶ�̬��ֵ�ֵ�
  for(int i=0; i<size; i++)
  {
    float rank = ranks[i];
    if( (RANK_THRES - rank > 0.0001) && 
        (tokens[i].length <= 2) ) // С��RANK_THRES���֣�ͶƱ��ʡ�Բ�
    {      
      rankSet[i][TERM_RANK_OMIT] += 1;
#ifdef _DEBUG
      fprintf(stderr,"term: %-16s vote: %d-%d\twhys: %f\n",tokens[i].buffer,TERM_RANK_OMIT,1,rank);
#endif
    }
  }

  _rank_by_term_num_wgt(tokens, size, rankSet, trunks, pword_tag_dict);

  //for(int i=0; i<size; i++)
  //{
  //    printf("%s:rank=%d mod=%d omit=%d\n", tokens[i].buffer, rankSet[i][2], rankSet[i][1], rankSet[i][0]);
  //}

}

int is_define_word(token_t tok, Sdict_build * pword_tag_dict)
{
    char term_sign[256];
    snprintf(term_sign, sizeof(term_sign), "%d_%s", 6, tok.buffer);
    Sdict_snode snode;
    creat_sign_f64(term_sign, strlen (term_sign), &snode.sign1, &snode.sign2);
    if(1 == db_op1(pword_tag_dict, &snode, SEEK))
    {
        return 1;
    }

    return 0;
}

/// �����ض��ķ��������Ż��ֲ㡣
void _tune_by_rule( trunk_dict_t* pdict, token_t tokens[], int size, unsigned char rankSet[][MAX_RANK],
        trunk_t trunks[], Sdict_build * pword_tag_dict)
{  
  for(int i=0; i<size; i++)
  {
    // �����ʽ��Ҫ�ر���
    if( i+2 < size ) // NS + P + NS (�磺���ݵ��Ϻ�����·37����ô��)
    {
      if( tokens[i].type==POS_LOC && tokens[i+1].type==POS_PREP && tokens[i+2].type==POS_LOC ) 
      {
        // ��������Ĳ�
        rankSet[i][TERM_RANK_TRUNK] += 3;
        rankSet[i+1][TERM_RANK_TRUNK] += 3;
        rankSet[i+2][TERM_RANK_TRUNK] += 3;

#ifdef _DEBUG
        fprintf(stderr,"termid: %d to %d vote: %d-%d\twhys: recall by loc rule\n",i,i+2,TERM_RANK_TRUNK,3);
#endif
        i += 2;
        continue;
      }
    }

    if( i+1 < size ) // NS + NS (�磺�������ɽ����� �ⷿ)
    {
      if( tokens[i].type==POS_LOC && tokens[i+1].type==POS_LOC && rankSet[i+1][TERM_RANK_TRUNK]<rankSet[i+1][TERM_RANK_MODIFY])
      {        
        rankSet[i+1][TERM_RANK_TRUNK] = rankSet[i+1][TERM_RANK_MODIFY];        

#ifdef _DEBUG
        fprintf(stderr,"term: %-16s vote: %d-%d\twhys: recall by loc rule\n",tokens[i+1].buffer,TERM_RANK_TRUNK,rankSet[i+1][TERM_RANK_TRUNK]);
#endif
        i += 1;
        continue;        
      }
    }

    if(i > 0)
    {
        if( is_define_word( tokens[i], pword_tag_dict) )
        {
            if(POS_TIME==tokens[i-1].type || POS_NUMBER==tokens[i-1].type)
            {
                rankSet[i-1][TERM_RANK_TRUNK] += 2;
                //printf("%s define time_number trunk=%d", tokens[i-1].buffer, rankSet[i-1][TERM_RANK_TRUNK]);
            }
            else
            {
                int j = i-1;
                for(; j >= 0; --j)
                {
                    if(POS_DEFAULT<=tokens[j].type && tokens[j].type<=POS_NZ)
                        break;
                }
                for(; j >= 0; --j)
                {
                    if(POS_DEFAULT<=tokens[j].type && tokens[j].type<=POS_NZ)
                    {
                        rankSet[j][TERM_RANK_TRUNK] += 2;
                        //printf("%s define add trunk=%d", tokens[j].buffer, rankSet[j][TERM_RANK_TRUNK]);
                    }
                    else
                        break;
                }
            }
        }
    }

  }  
}

/// ��֤����Ҫ��һ��termΪ���ġ�
void _check_trunk( trunk_t trunks[], int size, unsigned char rankSet[][MAX_RANK] )
{   
  float maxw = -1.0;
  int trkcnt = 0, maxi = -1;   
  for(int i=0; i<size; i++)
  {
    trunks[i].rank = _get_max3(rankSet[i][TERM_RANK_OMIT],rankSet[i][TERM_RANK_MODIFY],rankSet[i][TERM_RANK_TRUNK]);
    if( trunks[i].weight - maxw > 0.0001 )
    {
      maxw = trunks[i].weight;
      maxi = i;
    }       
    if( trunks[i].rank == TERM_RANK_TRUNK )
    {
      trkcnt++;
    }
  }

  // ���û������term�����ٻ�Ȩ������term
  if( !trkcnt )
  {
    rankSet[maxi][TERM_RANK_TRUNK] = 10; // ���������һ���ܴ�ķ֣�ǿ�ƽ������ɲ㣡
    trunks[maxi].rank = TERM_RANK_TRUNK;    

#ifdef _DEBUG
    fprintf(stderr,"termid: %-14d vote: %d-%d\twhys: recall by max weight\n",maxi,TERM_RANK_TRUNK,10);
#endif
  } 
}

//-------------------------------------------------------------------------------------------//

trunk_dict_t* trk_create( const char* dictpath )
{
  if( strlen(dictpath) >= MAX_PATH_LENG - 1 )
  {
    LOG_ERROR("error: length of path is too long");
    return 0;
  }

  char path[MAX_PATH_LENG] = {0};  
  strncpy(path,dictpath,strlen(dictpath)+1);
  strncat(path,"/",1); 
  
  return _dict_load( path );
}

void trk_destroy( trunk_dict_t* p )
{
  if( p != NULL )
  {
    _dict_free(p);  
    p = NULL;
  }
}

//����term�������������ٵ�������
int _check_trunk_by_term_num(token_t tokens[], uint32_t size, unsigned char rankSet[][MAX_RANK],
        trunk_t trunks[], Sdict_build * pword_tag_dict)
{
    if(size < 3)
    {
        return 0;
    }
    unsigned i=0, trunk_num = 0, non_stopword_num = 0;
    int cur_rank[MAX_TOKEN_COUNT] = {0};
    for(i=0; i < size; ++i)
    {
        cur_rank[i] = _get_max3(rankSet[i][TERM_RANK_OMIT],rankSet[i][TERM_RANK_MODIFY],rankSet[i][TERM_RANK_TRUNK]);
        if(TERM_RANK_TRUNK == cur_rank[i])
        {
            ++trunk_num;
        }
    }
    cmp_trunk_t cmp_trk[MAX_TOKEN_COUNT];
    for(i=0; i < size; ++i)
    {
        cmp_trk[i].ind = i;
        int is_stop_word = is_stopword( tokens[i], pword_tag_dict);
        cmp_trk[i].weight = trunks[i].weight;
        if(rankSet[i][TERM_RANK_OMIT] || is_stop_word)
        {
            cmp_trk[i].weight = 0;
        }
        if(TERM_RANK_TRUNK == cur_rank[i])
        {//��֤������ǰ
            cmp_trk[i].weight += 1.0;
        }
        if(!is_stop_word)
        {
            ++non_stopword_num;
        }
    }
    qsort(cmp_trk, size, sizeof(cmp_trunk_t), cmp_trunk_wgt );

    if(3<=non_stopword_num && non_stopword_num<=5 && trunk_num < 2)
    {
        for(i=0; i <= 1; ++i)
        {
            int ind = cmp_trk[i].ind;
            if(TERM_RANK_TRUNK == cur_rank[ind])
                continue;
            rankSet[ind][TERM_RANK_TRUNK] = rankSet[ind][ cur_rank[ind] ] + 1;
        }
    }
    if(6 <= non_stopword_num && trunk_num < 3)
    {
        for(i=0; i < 3; ++i)
        {
            int ind = cmp_trk[i].ind;
            if(TERM_RANK_TRUNK == cur_rank[ind])
                continue;
            rankSet[ind][TERM_RANK_TRUNK] = rankSet[ind][ cur_rank[ind] ] + 1;
        }
    }

    if(3<=non_stopword_num && non_stopword_num<=5 && trunk_num>=non_stopword_num)
    {
        int ind = cmp_trk[non_stopword_num-1].ind;
        rankSet[ind][TERM_RANK_MODIFY] = rankSet[ind][ TERM_RANK_TRUNK ] + 1;
    }
    if(6 <= non_stopword_num && trunk_num > 5)
    {
        for(i=5; i < trunk_num; ++i)
        {
            int ind = cmp_trk[i].ind;
            rankSet[ind][TERM_RANK_MODIFY] = rankSet[ind][ TERM_RANK_TRUNK ] + 1;
        }
    }


    return 0;
}

int get_reduce_type(token_t tokens[], uint32_t size, trunk_t trunks[], Sdict_build* pword_tag_dict)
{
    if(!tokens || !trunks)
    {
        return -1;
    }
    unsigned int i = 0;
    for(i=0; i < size; ++i)
    {
        trunks[i].reduce_type = DEFAULT_REDUCE_TYPE;
        if(POS_DELIM==tokens[i].type ||  IS_MARK(tokens[i].prop) || IS_SPACE(tokens[i].prop) )
        {//����ʧ���彵Ȩ
            trunks[i].reduce_type = 0;
            continue;
        }
        if(TERM_MODI_UNESCAPE == trunks[i].type)
        {   
            trunks[i].reduce_type = 0;
            continue;
        }
        if(POS_EXCL==tokens[i].type || POS_MOOD==tokens[i].type)
        {
            trunks[i].reduce_type = 1;
            continue;
        }

        if(POS_PREP==tokens[i].type || POS_CONJ==tokens[i].type || POS_ONO==tokens[i].type ||
                POS_AUX==tokens[i].type || POS_ZHUANG==tokens[i].type)
        {
            trunks[i].reduce_type = 2;
            continue;
        }
        if( is_interrogative(tokens[i], pword_tag_dict) )
        {
            trunks[i].reduce_type = 2;
            continue;
        }

        if(TERM_MODI_HIGH == trunks[i].type)
        {
            trunks[i].reduce_type = 3;
        }
    }

    return 0;
}

// ����ʵ�����б�ע�ʵ��������ΪNULL�������ʵ�����Ѿ�װ�ء�
// 0��ʾ������㣬1��ʾ���β㣬2��ʾ���ɲ㣬����2����Ԥ��������ϸ�֡�
int trk_parse( trunk_dict_t* trkdict, rank_dict_t* wdrdict, tag_dict_t* tagdict, Sdict_build* pword_tag_dict, 
               token_t tokens[], uint32_t size, trunk_t trunks[], uint32_t trksize )
{
  if( !trkdict || !wdrdict || !tokens || !trunks || !size || !trksize || !pword_tag_dict)
    return 0;

  if( size >= MAX_TOKEN_COUNT )
    size = MAX_TOKEN_COUNT;
  if( size >= trksize )
    size = trksize;

  // ��ʼ��ͶƱ���ֱ�
  unsigned char rankSet[MAX_TOKEN_COUNT][MAX_RANK];
  memset(rankSet,0,sizeof(rankSet));  

  // ĳЩ����£�ȫ��term��Ӧ�������ɲ㣬ֱ�ӷ���  
  if( _is_filtered(tokens,size) )
  {
    for(unsigned i=0; i<size; i++)
    {
      trunks[i].rank = TERM_RANK_TRUNK;
      trunks[i].type = 0;
      trunks[i].reduce_type = DEFAULT_REDUCE_TYPE; 
      trunks[i].weight = 1; // ע�⣺���������û����term��Ҫ�ԣ�
    }
    return size;
  } 

  // �����ע�ʵ䲻Ϊ�գ������Ƚ��д��Ա�ע
  if( tagdict )  
  {
    if( !tag_postag(tagdict,tokens,size) )    
      LOG_ERROR("error: pos tagging failed, ignore it"); 

#ifdef _DEBUG
    for(unsigned i=0; i<size; i++)
    {    
      const char* stag = get_pos_str(tokens[i].type);
      if( stag )
        fprintf(stderr,"%s/%s\t",tokens[i].buffer,stag);    
      else
        fprintf(stderr,"%s\t",tokens[i].buffer);    
    }  
    fprintf(stderr,"\n");
#endif
  }

  // ���ȸ��ݶ�̬term��Ҫ�Էֲ�
  _rank_by_weight(wdrdict,tokens,size,rankSet,trunks, pword_tag_dict);

  // ���ݴ����������ǿ���ֲ�  
  _rank_by_property(trkdict, pword_tag_dict, tokens, size, rankSet, trunks);

  // ���ݴ���֮���ϵ�ֲ�
  _rank_by_pattern(trkdict,tokens,size,rankSet,trunks);

  // ��֤����Ҫ��һ��termΪ����
  _check_trunk(trunks,size,rankSet);

  // �����ķ�����΢��
  _tune_by_rule(trkdict,tokens,size,rankSet,trunks, pword_tag_dict); 
  //
  _check_trunk_by_term_num(tokens, size, rankSet, trunks, pword_tag_dict);
  //printf("\n\n\n");
  // ȷ������¼term�ķֲ�  
  for(unsigned i=0; i<size; i++)
  {
    trunks[i].rank = _get_max3(rankSet[i][TERM_RANK_OMIT],rankSet[i][TERM_RANK_MODIFY],rankSet[i][TERM_RANK_TRUNK]);
    if( trunks[i].rank == TERM_RANK_TRUNK )
    {
      trunks[i].type = 0;
    }
    //printf("%s:rank=%d mod=%d omit=%d type=%d reduce_type=%d\n", tokens[i].buffer, rankSet[i][TERM_RANK_TRUNK],
    //        rankSet[i][TERM_RANK_MODIFY], rankSet[i][TERM_RANK_OMIT], trunks[i].type, trunks[i].reduce_type);
  }
  //
  get_reduce_type(tokens, size, trunks, pword_tag_dict);
  //for(unsigned i=0; i<size; i++)
  //{
     //printf("%s:rank=%d mod=%d omit=%d type=%d reduce_type=%d\n", tokens[i].buffer, rankSet[i][TERM_RANK_TRUNK],
    //          rankSet[i][TERM_RANK_MODIFY], rankSet[i][TERM_RANK_OMIT], trunks[i].type, trunks[i].reduce_type);
  //}
  
#ifdef _DEBUG
  fprintf(stderr,"----------------------------------------------------\n");
  for(unsigned i=0; i<size; i++)
  {
    fprintf(stderr,"%-16s%10f%8d%8d%8d\n",tokens[i].buffer,trunks[i].weight,rankSet[i][0],rankSet[i][1],rankSet[i][2]);
  }
  fprintf(stderr,"----------------------------------------------------\n");

  for(unsigned i=0; i<size; i++)
  {
    fprintf(stderr,"%s(%d,%d)\t",tokens[i].buffer,trunks[i].rank,trunks[i].type);
  }
  fprintf(stderr,"\n\n");
#endif

  return size;
}


