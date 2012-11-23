/**
 * @file tagdict.h
 * @brief Some definitions relative with POS tag dictionary. 
 * @author David Dai
 * @version 1.0
 * @date 2007/09/24
 */

#ifndef TAG_DICT_H
#define TAG_DICT_H

#include <ul_log.h>
#include <ul_sign.h>
#include <ul_dict.h>
#include <odict.h>

#include "ipostag.h"

/* �ʵ���غ궨�� */
#define TERM_TYPE_DICT "termtype"  /// ���������Ϣ
#define TERM_PROP_DICT "property"  /// ���������Ϣ
#define TERM_PATT_DICT "pattern"   /// ����λ��Ϣ
#define TERM_TAG_DICT  "termtag"   /// ���Ա����Ϣ
#define TERM_UNIGRAM_DICT "unigram"
#define TERM_BIGRAM_DICT "bigram"
#define CHUNK_RULE_DICT "chkrule"
#define DATA_DICT_POSTFIX ".dat"

#define MAX_PATH_LENG 512

/* ���Ա����ض��� */
#define POS_TAG_COUNT 39
#define USED_TAG_COUNT POS_TAG_COUNT-3  // ���3�����chunk parsing���ã�

#define POS_DEFAULT  21  // default is noun "n"
#define POS_PER      22  // "nr"
#define POS_LOC      23  // "ns"
#define POS_ORG      24  // "nt"
#define POS_NX       25  // "nx"
#define POS_NZ       26  // "nz", other proper noun
#define POS_DELIM    37  // delimiter is "w"
#define POS_TIME     32  // time is "t"
#define POS_NUMBER   20  // number is "m"
#define POS_CONJ     10  // conj.
#define POS_PREP     28  // prep.
#define POS_AUX      33  // aux.
#define POS_VERB     34  // verb.
#define POS_PRON     30  // pron.
#define POS_ADJ      6   // adj.
#define POS_ADV      11  // adv.
#define POS_ONO      27  // onomatopoeia
#define POS_MOOD     38  // mood
#define POS_NG       3   //
#define POS_VG       5   //
#define POS_B        9   // �����
#define POS_F        13  // ��λ��
#define POS_S        31  // �����ʣ��硰�ڵأ����ߡ�
#define POS_QUAN     29  //
#define POS_EXCL     12  //̾��
#define POS_ZHUANG   39  //״̬��

/* �����ʵ����Ͷ��� */
typedef Sdict_search dict_t;
typedef sodict_search_t odict_t;

/*! 
   �ǵ�ṹ���� 
   ÿ������ı����Ϣ����¼��һ���������ֽڻ��������С�
   ������ͷҲ���ǵ�һ���ֽ�Ϊ�����Ϣ����Ŀ������Ϊ�����ı����Ϣ���� 
   �����Ϣ: ��һ���ֽ�Ϊ���Ա��id������0С��256������������������sizeof(double)
   ���ֽ�������¼���Ա�ǵ�Ƶ����Ϣ��ʵ������һ��double����ȡ��log����
   �������δ洢��������Ϣ��  
*/
typedef struct tag_dict_t // �����������Ѿ������ӿ��ļ���
{
  dict_t*  dict;         // ����ǵ�  
  unsigned int size;     // buffer��С
  unsigned char* buffer; // ÿ���ʵı����Ϣ��¼����������У������¼��������˵��
} tag_dict_t;

/* ����������� */
extern const double POS_TAG_PROB[POS_TAG_COUNT];
extern const double TRANS_PROB_MATRIX[POS_TAG_COUNT][POS_TAG_COUNT];


// <log>
#define LOG_ERROR(message)  ul_writelog(UL_LOG_FATAL, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
// </log>

// <api>
/*! 
 * Load binary dictionary, create a tag_dict_t instance. 
 */
tag_dict_t* tag_dict_load( char* dictpath );
/*! 
 * Dispose a tag_dict_t pointer. 
 */
void tag_dict_free( tag_dict_t* pdict );
/*! 
 * Seek a key in dictionary, return the index of tag info. 
 * If fails, return -1. 
 */
int tag_dict_find( tag_dict_t* pdict, char* key );
// <api/>

// <misc>
/*! 
 * Bisearch in sorted string array.
 * If succeeds, return the suffix (start with 0), else return -1.
 */
int bisearch( const char* str , const char** array , int size );
// </misc>

/*! Get the string of POS tag. If fails, return NULL. */
const char* get_pos_str( unsigned int nPOS );
/*! Get the number of one POS tag. If fails, return 0. */
unsigned int get_pos_val( const char* strPOS );


#endif /* TAG_DICT_H */

