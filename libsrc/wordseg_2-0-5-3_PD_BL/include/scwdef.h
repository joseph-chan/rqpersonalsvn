/*
 * filename    : scwdef.h
 * author      : Paul Yang
 * version     : 
 * date        : 2004.12.1
 * description : interface and definition file of segment module.
 */

#ifndef SCWDEF_H
#define SCWDEF_H

#include <stdio.h>
#include <ul_dict.h>
#include <ul_sign.h>
#include <ul_log.h>
#include <assert.h>
#include <stdint.h>
#include "hinshi_map.h"
#include "property.h"
#include "lib_crf.h"
#include "ul_dictmatch.h"
#include "odict.h"
#include "ul_ccode.h"

//¹ú¼Ê»¯±àÂëÏà¹Ø
#define SCW_UTF8 0
#define SCW_GBK 1
#define SCW_CHINESE 0
#define SCW_JAPANESE 1

// micros defined here
//--------------------------------------------------------------------------------//
#define u_int unsigned int
#define u_char unsigned char
#define MAX_WORD_LEN 256
#define DEFAULT_AMB_NUM 256
#define SCW_MAX_VERSION_LEN 100
#define SCW_MAX_INNER_TERM 256
#define MAX_EXTEND_PHINFO_SIZE 770
#define MAX_POT_LEMMA_SIZE 100//Ã¿¸öÇĞ·ÖÆ¬¶ÎÓĞ¿ÉÄÜÔö¼ÓµÄlemma¸öÊı£¬Èç¹û³¬¹ı£¬Ôò²»Ìí¼Ó¸Ãterm
#define MAX_EXTEND_ENTRY_SIZE 100000
#define LEMMA_ADD_SIZE 10000//Ã¿´ÎlemmalistÔö¼Ó¸öÊı
#define MAX_NBEST 1 //crf nbest

#define SCW_OUT_PROP 1
#define SCW_OUT_BASIC  2
#define SCW_OUT_WPCOMP 4
#define SCW_OUT_SUBPH   8
#define SCW_OUT_HUMANNAME 16
#define SCW_OUT_BOOKNAME 32
#define SCW_OUT_NEWWORD 64
#define SCW_OUT_ALL 126

#define SCW_STATIC 0
#define SCW_DYNAMIC 1
#define SCW_DICTNAME "worddict.scw"
#define SCW_NEWWORDDICT "newword"
#define SCW_LSN_RULEFILE "leftrule"
#define SCW_RSN_RULEFILE "rightrule"
#define SCW_VERINFONAME "version.scw"
#define SCW_DICTVER_TAG "DictVer"
#define SCW_VERMAP_TAG "VerMap"
#define SCW_MANNAME "worddict.man"  //ÈË¹¤¿ØÖÆÎÄ¼şÃû³Æ
#define CRF_MODEL_NAME "crf_model"
#define SCW_MULTIDICT "multi_dict"
#define SCW_CONFIGURE "scw.conf"
#define SCW_WD_CH 1
#define SCW_WD_JP 2
#define SPLIT ' '

#define MAX_VERSION_NUM 1024

#define DATA_CNAME 0
#define DATA_FNAME 1
#define DATA_JNAME 2


#define GET_TERM_POS(property)  ((property) & 0x00FFFFFF)
#define GET_TERM_LEN(property)  (((unsigned)property) >> 24)

/* ÓÃÓÚ¼ì²â·Ö´Ê¿âºÍ´Êµä¿â°æ±¾Æ¥ÅäÖ®ÓÃ */
typedef struct _SCW_VERSION_MAP
{  
  Sdict_build * m_sver_dict;
  Sdict_build * m_dver_dict;
  
  char * m_strbuf;
  u_int m_bufsize;
  u_int m_bufpos;

  u_int m_sver_count;
  u_int m_dver_count;
  
  char m_ver_table[MAX_VERSION_NUM][MAX_VERSION_NUM];
}scw_ver_map;

  
// constant defined here
//--------------------------------------------------------------------------------//
const u_int DEFAULT_LEMMA_COUNT = 1000;
const u_int DEFAULT_INFO_SIZE = 4000;
const u_int DEFAULT_DE_SIZE = 2000;
const u_int DEFAULT_WORDBUF_SIZE = 20000;
const u_int DEFAULT_HASH_SIZE = 1;
const u_int DENTRY_NULL = 0xFFFFFFFF;
const u_int DENTRY_FIRST = 0xFFFFFFFE;
const u_int SUFENTRY_NULL = 0xFFFFFFFF;
const u_int LEMMA_NULL = 0xFFFFFFFF;
const u_int COMMON_NULL = 0xFFFFFFFF;

const u_int LEMMA_TYPE_SBASIC = 0;
const u_int LEMMA_TYPE_DBASIC = 1;
const u_int LEMMA_TYPE_SPHRASE = 2;
const u_int LEMMA_TYPE_DPHRASE = 3;
const u_int LEMMA_TYPE_QYPD = 4;
const u_int LEMMA_TYPE_POLY = 5;
const u_int LEMMA_TYPE_MAN = 6;//ÈË¹¤¿ØÖÆÆ¬¶Î

const u_int MAX_LEMMA_LENGTH = 256;
const u_int MAX_AMB_NUM = 256;
const u_int MAX_IL_LEN = 2408;
const u_int MAX_LEMMA_POS = 0x80000000;
const u_int SCW_INFO_FACT = 8;

const int AUX_WEIGHT_INC = 1000;
const int WORDELE_WEIGHT_INC = -200;

const u_int SCW_MAXWORDLEN = 256;

const double LEMMA_SMOOTH_PROB = 1e-11;
const double LEMMA_DEFAULT_PROB = 0.0333;
const int NORMALIZE_INTEGER = 10000;

const double LEMMA_SMOOTH_PROB_JP = 2e-11;
const double LEMMA_DEFAULT_PROB_JP = 1e-11;

const u_int NAME_PROP_COUNT = 4;
const u_int MAX_SPLIT_NUM = 256;
const int MAX_TERM = 10000000;
//const u_int CRF_BUF_SIZE = 512;
const u_int TYPE_NEWWORD = 0;
const u_int TYPE_MULTITERM = 1;
const u_int TYPE_MULTITERM_NEWWORD = 2;
enum NAME_POS{NAME_BGN,NAME_MID,NAME_END,NAME_OTH};


/*-------------------crf for wordseg preprpocessing -----------------*/

typedef struct scw_crf_term_t
{
    u_int wordtotallen;          // µ±Ç°±»ÇĞ·ÖÎÄ±¾³¤¶È£¨²»º¬ÓÃÓÚ·Ö¸ôÇĞ·ÖÆ¬¶ÎµÄ'\t'£©	
    u_int crftermcount;          // ÇĞ·ÖÆ¬¶ÎÊıÄ¿
  //  u_int* crftermoffsets;        // »ù±¾´Ê¼ÆÊı£¬Ò²¾ÍÊÇµ±Ç°ÇĞ·ÖÆ¬¶ÎÊÇ´ÓµÚ¼¸¸ö»ù±¾´Ê¿ªÊ¼µÄ
    u_int* crftermpos;           // µ±Ç°ÇĞ·ÖÆ¬¶ÎµÄ×Ö½ÚÆ«ÒÆÒÔ¼°³¤¶È£¬ÕûÊıµÄµÍ24bitÎªÆ«ÒÆ£¬¸ß8bitÎª³¤¶È
}scw_crf_term;

typedef struct scw_crf_out_t
{
    int nbest;                   // ·µ»ØµÄnbestÖµ
    scw_crf_term term_buf[MAX_NBEST];
}scw_crf_out;


// data structure defined here
//--------------------------------------------------------------------------------//
/* ´ÊÓïÊôĞÔ¼ÇÂ¼ */
typedef struct SCW_PROPERTY_T
{
  int m_hprop;
  int m_lprop;
}scw_property_t;

/* ´ÊÌõ½á¹¹ */
typedef struct SCW_LEMMA_T
{
  unsigned m_type:3;
  unsigned m_weight:21;        // the lemma's weight
  unsigned m_length:8;

  scw_property_t m_property;   // some property of the lemma

  union
  {
    u_int m_word_bpos;         // »ù±¾´Ê´ÊÌõ¿ªÊ¼Î»ÖÃ
    u_int m_phinfo_bpos;       // ¶ÌÓï´ÊÌõ¿ªÊ¼Î»ÖÃ£¬Ö¸ÏòÒ»¸öÕûĞÍÊı×éµÄ¿ªÊ¼Î»ÖÃ£¬¸ÃÊı×éµÚÒ»¸öÔªËØ´æ´¢´Ë´ÊÌõËùº¬»ù±¾´ÊµÄ¸öÊı£¬ºóÃæÒÀ´ÎÎªÃ¿Ò»¸ö»ù±¾´ÊµÄ´ÊÌõÖ¸Õë
  };
  
  u_int m_subphinfo_bpos;      // ×Ó¶ÌÓï´ÊÌõ¿ªÊ¼Î»ÖÃ£¬Ö¸ÏòÒ»¸öÕûĞÍÊı×éµÄ¿ªÊ¼Î»ÖÃ£¬Êı×éµÄµÚÒ»Î»´æ´¢´Ë´ÊÌõ×Ó¶ÌÓïµÄ¸öÊı£¬¼ÙÉèÎªM¸ö£¬Ôò½ÓÏÂÀ´µÄ2*MµÄ¿Õ¼äÖĞ£¬ÒÀ´Î´æ´¢µÚi¸ö×Ó¶ÌÓïµÄÄÚ²¿Æ«ÒÆÁ¿(ÒÔ»ù±¾´Ê¸öÊıÀ´¼ÆËã)£¬ÒÔ¼°µÚi¸ö×Ö¶ÌÓïµÄlemmaÖ¸Õë
  u_int m_prelm_pos;           // the position of the prelemma
  u_int m_utilinfo_pos;
}scw_lemma_t;

typedef struct SCW_UTILITYINFO_T
{
  u_int m_ambsuffix_pos;
  union
  {
    u_int m_cnameprob_pos;
    u_int m_jnameprob_pos;
  };
  u_int m_fnameprob_pos; 
}scw_utilinfo_t;

/* ½öÔÚÉú³É´ÊµäÊ±Ê¹ÓÃ */
typedef struct SCW_INPUTLEMMA_T
{
  char m_lmstr[SCW_MAXWORDLEN];
  u_int m_type; // ´ÊÌõÀàĞÍ£¬Èç¾²Ì¬»ù±¾´ÊµÈ
  u_int m_weight;
  scw_property_t m_property;

  int m_bwcount;
  int m_sphcount;
  u_int  m_wordlist[512];  // Á½¸öÕûÊıÒ»×é£¬Ç°Ò»¸öÕûÊı±íÊ¾»ù±¾´ÊÔÚµ±Ç°×Ö·û´®ÖĞĞòºÅ£¬ºóÒ»¸ö±íÊ¾Æä´æ´¢Î»ÖÃ
  u_int  m_subphlist[512]; // Á½¸öÕûÊıÒ»×é£¬Ç°Ò»¸öÕûÊı±íÊ¾¶ÌÓïÔÚµ±Ç°×Ö·û´®ÖĞĞòºÅ£¬ºóÒ»¸ö±íÊ¾Æä´æ´¢Î»ÖÃ
  
  u_int   m_mcount;//ÈË¹¤¿ØÖÆ²¿·ÖÇĞ·Ö¸öÊı
  u_int  m_manuallist[512];//ÈË¹¤¿ØÖÆÆ¬¶Î½á¹û£¬Èç¹ûÊÇ¾²Ì¬´Ê£¬´æ´¢¸ÃlemmaÔÚlemmalistÖĞµÄÆ«ÒÆÁ¿£¬Èç¹ûÊÇ¶¯Ì¬´Ê£¬´æ´¢¸ÃlemmaÔÚpwdict->pir->m_dynlmÖĞµÄÆ«ÒÆÁ¿¡£
}scw_inlemma_t;


typedef struct SCW_NEWWORD_T
{
	char*  newwordbuf;
	u_int  newwordbsize;
	u_int  newwordb_curpos;
	u_int  newwordbtermcount;
	u_int* newwordbtermoffsets;//Í¬ne buffer£¬Ã¿¸ötermµÄoffsetÒª´æÁ½Î»£¬Ò»Î»´æÆğÊ¼µÄ»ìÅÅµÄtermÎ»ÖÃ£¬ÁíÒ»Î»´æ´¢°üº¬µÄ»ìÅÅµÄterm¸öÊı£¬ÔÚ³õÊ¼»¯µÄÊ±ºòoffset bufferµÄsizeÒª³Ë2
	u_int* newwordbtermpos;
	neprop_t* newwordneprop;           // wordnerµÄÏà¹ØĞÅÏ
	scw_property_t* newwordbtermprop;
}scw_newword_t;


typedef struct SCW_DICTENTRY_T
{
  u_int m_value;      // ½áµãºº×Ö»òÕß±ğµÄ×Ö·û¶ÔÓ¦µÄÕûĞÍÖµ
  u_int m_lemma_pos;  // Ö¸Ïòµ±Ç°entryËù¶ÔÓ¦µÄlemmaÎ»ÖÃ£¬Õâ¸ölemmaÊÇÖ¸TrieÊ÷´Ó¸ù½Úµãµ½´Ë½ÚµãÂ·¾¶ÉÏÄÜÆ¥ÅäµÄ´ÊÓï¡£
  u_int m_suffix_pos; // Ö¸ÏòºóĞø½áµãµÄ¿ªÊ¼Î»ÖÃ
}scw_dictentry_t;

/* ·Ö´Ê´Êµä½á¹¹ */
typedef struct SCW_WORDDICT_T
{
  /// store the strings of terms.
  char* m_wordbuf;              // buffer for store word  result;
  u_int m_wbsize;
  u_int m_wbpos;                // the next position in word buf

  /// store the phrase info.
  u_int* m_phinfo;              // the buffer for phrase information and sub phrase information
  u_int m_phisize;
  u_int m_phipos;               // the unused pos in ph info list

  //store the dynamic phrase and basic info
  char* 	 m_dynbuf;         //the buffer for dynamic basic infomation
  u_int		 m_dynb_size; 
  u_int		 m_dynb_curpos;
	
  u_int*	 m_dynphinfo;  //the buffer for dynamic phrase infomation
  u_int		 m_dynph_size;
  u_int		 m_dynph_curpos;

  /// store the node nexus.
  u_int* m_seinfo;              // the node nexus list
  u_int m_seisize;
  u_int m_seipos;
  
  /// store the node contents.
  scw_dictentry_t* m_dictentry; // the dict entry list
  u_int m_desize;
  u_int m_depos;                // the first unused pos in node list

  /// store the lemma structure of dictionary.
  scw_lemma_t* m_lemmalist;     // the lemma list
  u_int m_lmsize;
  u_int m_lmpos;                // the first unused pos in lemma list, ending position

  /// root node of trie tree.
  u_int m_entrance; 

  Sdict_search* pdict_lsn_rule;
  Sdict_search* pdict_rsn_rule;
  
  int* m_probbuf;
  u_int m_pbsize;
  u_int m_pbpos;

  u_int* m_ambinfo;
  u_int m_ambsize;
  u_int m_ambpos;

  scw_utilinfo_t* m_utilinfo;
  u_int m_utilisize;
  u_int m_utilipos;

  u_int m_wdtype;
  CrfModel *crf_model; 
  sodict_search_t* newworddict;//ĞÂ´Ê¿ØÖÆ´Êµä
  dm_dict_t* multitermdict;//ĞÂ´Ê¿ØÖÆ´Êµä
  
  char version[SCW_MAX_VERSION_LEN];
}scw_worddict_t;

/* ·Ö´Ê½á¹û´æ´¢½á¹¹II£¬¶ÔÓ¦½Ó¿ÚII */
typedef struct _SCW_INNER_T
{
  u_int    m_maxterm;       // ·Ö´Ê½á¹ûËùÄÜÈİÄÉ×î´ó»ù±¾´ÊÊı
  u_int    m_flag;          // ·Ö´Ê½á¹ûÊı¾İ¿ØÖÆ¿ª¹Ø
  u_int    m_merged_flag;   // ÎªÕ³½Ó·şÎñ
  u_int    m_offset;        // ·Ö´Ê½á¹û»ù±¾´ÊÆ«ÒÆÁ¿

  /// store segmented lemmas.
  scw_lemma_t**  m_ppseg;   // ¼ÇÂ¼ÇĞ·Ö½á¹û(»ìÅÅ)ÖĞ´ÊÌõµØÖ·ĞòÁĞ£¬Èç¹û´ÊÌõÊÇ¶¯Ì¬Éú³ÉµÄ£¬ÔòÖ¸Ïò<m_dynlm>ÖĞÔªËØµÄµØÖ·
  u_int  m_ppseg_cnt;
  int *m_ppseg_flag;
  
  scw_lemma_t** tmp_ppseg;// ¼ÇÂ¼×îÔ­Ê¼querywordsegµÄÇĞ·Ö½á¹û£»ĞèÒª´Óm_ppsegÖĞcopy¹ıÀ´
  u_int tmp_ppseg_cnt;
  

  /// store dynamic terms.
  char*  m_dynbuf;
  u_int  m_dynb_size;
  u_int  m_dynb_curpos;

  uintptr_t* m_dynphinfo;   // ¼ÇÂ¼¶¯Ì¬¶ÌÓïĞÅÏ¢
  u_int      m_dynph_size;
  u_int      m_dynph_curpos;

  uintptr_t* m_dynphinfo_tmp;   // ĞÂÔöÒ»¸ö¶¯Ì¬¶ÌÓïĞÅÏ¢£¬ÓÃÓÚasciiÂëÊ¶±ğ¿¼ÂÇ¡£
  u_int      m_dynph_size_tmp;
  u_int      m_dynph_curpos_tmp;


  scw_lemma_t*  m_dynlm;    // ÓÃÀ´¼ÇÂ¼ÇĞ·Ö¹ı³ÌÖĞÉú³ÉµÄ¶¯Ì¬´ÊÌõ
  u_int    m_dynlm_size;
  u_int    m_dynlm_cnt;

  u_int *m_mandictword_prop;	//¼ÇÂ¼queryÖĞ°üº¬worddict.manÖĞ´ÊÌõµÄĞÅÏ¢£¬Ã¿¸öµ¥ÔªµÄ¸ß°ËÎ»ÊÇ³¤¶ÈĞÅÏ¢£¬µÍ24Î»ÊÇÆ«ÒÆĞÅÏ¢
  u_int m_mandictword_size;
  u_int m_mandictword_cnt;

  scw_newword_t* pnewword;
  //ner_out_t *pnerout;  //wordnerÊ¶±ğ½á¹û£¬²»Ò»¶¨ÓĞ±ØÒª£¬Õâ¸öÒª¿¼ÂÇ
  
  crf_out_t* crf_out;    //crfµÄÇĞ´Ê½á¹ûµÄÊä³ö
  scw_crf_out* crf_res;
  SegCrfTag *crf_tag; //crf_tag£¬

  char* m_tmpbuf;//ÓÃÀ´×°ÔØ¼Ó±£»¤ºóµÄ°ëÇĞ·Ö½á¹û£¬ËÍÓëwordsegÇĞ·Ö¡£
  u_int m_tmpbuf_len;
  u_int* m_protect; //ÓÃÀ´¼ÇÂ¼±£»¤crf´Êµä´ÊµÄ±êÇ©µÄÎ»ÖÃ¡£
  u_int m_protect_cnt;
  bool needremove; //ÓÃÀ´±ê×¢ÊÇ·ñĞèÒªremovetag

  dm_pack_t* multiterm_pack;
  
}scw_inner_t;

/* ·Ö´Ê½á¹û´æ´¢½á¹¹I£¬¶ÔÓ¦½Ó¿ÚI */
typedef struct _SCW_OUT_T
{  
  //0 basic word sep result
  char* wordsepbuf;            // ×Ö·û»º³å£¨ĞèÔ¤ÏÈ³õÊ¼»¯£©£¬ÓÃÓÚ´æ´¢·Ö´Ê½á¹û
  u_int wsbsize;               // ×Ö·û»º³å´óĞ¡£¨³õÊ¼»¯»º³åÇøÊ±¸ø¶¨£©
  u_int wsb_curpos;            // µ±Ç°¿ÉÓÃ»º³åÇø¿ªÊ¼ÎªÎ»ÖÃ
  u_int wordtotallen;          // µ±Ç°±»ÇĞ·ÖÎÄ±¾³¤¶È£¨²»º¬ÓÃÓÚ·Ö¸ôÇĞ·ÖÆ¬¶ÎµÄ'\t'£©
  u_int wsbtermcount;          // ÇĞ·ÖÆ¬¶ÎÊıÄ¿
  u_int* wsbtermoffsets;       // »ù±¾´Ê¼ÆÊı£¬Ò²¾ÍÊÇµ±Ç°ÇĞ·ÖÆ¬¶ÎÊÇ´ÓµÚ¼¸¸ö»ù±¾´Ê¿ªÊ¼µÄ
  u_int* wsbtermpos;           // µ±Ç°ÇĞ·ÖÆ¬¶ÎµÄ×Ö½ÚÆ«ÒÆÒÔ¼°³¤¶È£¬ÕûÊıµÄµÍ24bitÎªÆ«ÒÆ£¬¸ß8bitÎª³¤¶È
  scw_property_t* wsbtermprop; // ¸÷ÇĞ·ÖÆ¬¶ÎµÄÊôĞÔĞÅÏ¢

  //1 word phrase sep result
  char* wpcompbuf;
  u_int wpbsize;
  u_int wpb_curpos;
  u_int wpbtermcount;
  u_int* wpbtermoffsets;
  u_int* wpbtermpos;
  scw_property_t* wpbtermprop;

  //2 sub-phrase result
  char*  subphrbuf;
  u_int  spbsize;
  u_int  spb_curpos;
  u_int  spbtermcount;
  u_int*  spbtermoffsets;
  u_int*  spbtermpos;
  scw_property_t * spbtermprop;

  //3 human name result
  char*  namebuf;
  u_int  namebsize;
  u_int  nameb_curpos;
  u_int  namebtermcount;
  u_int* namebtermoffsets;
  u_int* namebtermpos;
  scw_property_t * namebtermprop;
  
  //4 book name result
  char*  booknamebuf;
  u_int   bnbsize;
  u_int   bnb_curpos;
  u_int   bnbtermcount;
  u_int*  bnbtermoffsets;
  u_int*  bnbtermpos;
  scw_property_t * bnbtermprop;
    
  //5 internal buffer for term merging
  int  phrase_merged;
  char*  mergebuf;
  u_int   mbsize;
  u_int   mb_curpos;
  u_int   mbtermcount;
  u_int*  mbtermoffsets;
  u_int*  mbtermpos;

  scw_inner_t * m_pir; // ·Ö´Ê½á¹ûÖĞ¼ä´æ´¢½á¹¹
  scw_newword_t* pnewword;

}scw_out_t; 

/* ÓÃÓÚÊä³öÄ³ÖÖ¾ßÌå·Ö´Ê½á¹û(Èç£º»ù±¾´Ê)Ö®ÓÃ */
typedef struct _SCW_ITEM_T
{
  char*  m_buff;
  u_int  m_size;
  u_int  m_curpos;
  u_int  m_tmcnt:31;
  u_int  m_allocated:1;
  u_int* m_tmpos;
  u_int* m_tmoff;
  u_int m_off_width; //ÔÚnewwordÖĞ£¬offsetºÍÆäËûoffset²»Ò»Ñù£¬Õ¼2Î»
  scw_property_t* m_tmprop;
}scw_item_t;
//


// dict API
//--------------------------------------------------------------------------------//
/*!
 * Create a worddict structure
 */
scw_worddict_t* scw_create_worddict(void);

/*!
 * Save worddict to files
 */
int scw_save_worddict(scw_worddict_t * pwdict,const char* fullname);

/*!
 * Free the the scw_worddict_t
 * @param pwdict - the pointer to  dict
 */
void scw_destroy_worddict(scw_worddict_t * pwdict);

/*!
 * Load worddict from the file
 * @param fullpath
 * @return the pointer to worddict if success, NULL if failed.
 */
//scw_worddict_t* scw_load_worddict(const char* fullpath);
scw_worddict_t* scw_load_worddict(const char* dictpath, int code = SCW_GBK, int language = SCW_CHINESE);

/*!
 * Search if the term exist in the dictionary 
 * @param <pwdict> - the worddict
 * @param <term> - the string searched
 * @param <len> - the length of <term>
 * @return 1 ,find ,0 cant not find;
 */
int scw_search(scw_worddict_t * pwdict,const char* term,int len);


int update_lemma_with_line(scw_worddict_t* pwdict,Sdict_build * pdynwdict,scw_inlemma_t * pilemma,char * line);

/*!
 * Add a lemma into a worddict
 * @param <pwdict> - the worddict
 * @param <pinlemma> - the input lemma
 * @return 1 if success,0 if exist, < 0 if failed.
 */
int scw_add_lemma(scw_worddict_t * pwdict,Sdict_build * pdynwdict,scw_inlemma_t& pinlemma);
int scw_add_lemma_man(scw_worddict_t * pwdict,Sdict_build * pdynwdict,scw_inlemma_t& pinlemma);


// segment API I
//--------------------------------------------------------------------------------//
/*!
 * Create the output struct
 * @param <tsize> - total word count
 * @return the pointer if success, NULL if failed.
 */
scw_out_t * scw_create_out(u_int tsize, int flag);

/*!
 * Destroy the out struct
 * @param <pout> - the pointer to the out structure
 */
void scw_destroy_out(scw_out_t* pout);

/*!
 * Segment the input buffer
 * @param <pwdict> - the worddict,
 * @param <pout> - the out struct
 * @param <inbuf> - the input buffer
 * @param <human_allow_blank> - true: allow jphuman recognition aross blank
 * @return 1 if success, <0 if failed.
 */
int scw_segment_words(scw_worddict_t * pwdict, scw_out_t * pout,const char* inbuf, const int inlen, bool human_allow_blank = true);

/*!
 * Get segmentation result represented by scw_item_t.
 * @param <item> - segmentation result.
 * @param <pout> - interim segmentation result.
 * @param <level> - which kind of segmentation result.
 * @return >=0, success; <0, faied.
 */
int scw_get_item(scw_item_t* item, scw_out_t* pout,int level);


// segment API II
//--------------------------------------------------------------------------------//
/*!
 * Create a scw_inner_t structure.
 */
scw_inner_t* scw_create_inner(int tsize, int flag);

/*!
 * Destroy the scw_inner_t struct.
 */
void scw_destroy_inner(scw_inner_t* pir);

/*!
 * Segment the input string, and store the result to a inner structure
 * @param <pwdict> - the worddict
 * @param <pir> - the inner seg result structure
 * @param <inbuf> - the input buffer
 * @param <human_allow_blank> - true: allow jphuman recognition aross blank.  **JP wordseg only**
 * @return 1 if success, <0 if failed.
 */
int scw_seg(scw_worddict_t * pwdict,scw_inner_t * pir,const char* inbuf,const int inlen, bool human_allow_blank = true);

/*!
 * Get a group of result from the scw_inner_t result structure.
 * @param <item> - segmentation result item.
 * @param <pir> - the interim segmentation result.
 * @param <level> - which kind of segmentation result.
 * @return >=0, success; <0, faied.
 */
int scw_get_result(scw_item_t* item, scw_worddict_t* pwdict, scw_inner_t* pir, int level);


// item operation
//--------------------------------------------------------------------------------//
/*!
 * Create a item structure.
 */
scw_item_t* scw_create_item(int tmcnt);

/*!
 * Reset the a item structure.
 */
void scw_reset_item(scw_item_t* pitem);

/*!
 * Delete a item structure.
 */
void scw_destroy_item(scw_item_t* pitem);

/*!
 * Get the ith lemma string.
 * @param <item> - the encapsulated seg result
 * @param <i> - which term you want to get
 * @param <term> - term buffer
 * @param <tlen> - buffer to hold the term string
 * @return If fails, return NULL.
 */
char* scw_gi_tmstr(scw_item_t* pitem, int i, char* term, int tlen);

/*!
 * Get the length of a term.
 */
int scw_gi_tmlen(scw_item_t* pitem, int i);

/*!
 * Get the offset of a term.
 */
int scw_gi_tmoff(scw_item_t* pitem, int i);

/*!
 * Get the string buffer of the seg result.
 */
char* scw_gi_buf(scw_item_t* pitem);

/*!
 * Get the property of the term.
 */
scw_property_t * scw_gi_tmprop(scw_item_t* pitem,int i);


// misc API
//--------------------------------------------------------------------------------//
/*!
 * Indicate blank in japan human name . **JP wordseg only**
 * @param <pwdict> - the worddict
 * @param <pout> - the out struct
 * @param <inbuf> - the input buffer
 * @param <inlen> - the len of input buffer
 * @param <blank_deleted> - the blank saved array
 * @param <blank_size> - the size of blank_deleted
 * @return >=0 then is the number of blank deleted, -1 if failed (==0 if in chinese segmentation)
 */
int scw_get_blank_deleted(scw_worddict_t * pwdict, scw_out_t * pout, const char * inbuf, const int inlen, u_int * blank_deleted, const int blank_size);

/*!
 * get logistic result of scw_seg. "ÕæÌï ĞãÒ»" will be changed into "ÕæÌïĞãÒ»", etc. **JP wordseg only**
 * @param <pwdict> - the worddict
 * @param <pir> - the inner seg result structure
 * @return  1 if success and changed, 0 if successs and not changed , <0 if failed.
 */
//int get_lgt_scw_seg(scw_worddict_t * pwdict,scw_inner_t * pir);

/*!
 * get logistic result of word. "ÕæÌï ĞãÒ»" will be changed into "ÕæÌïĞãÒ»", etc. **JP wordseg only**
 * @param <str> - the input string
 * @return 0 if success and output to str, <0 if failed.
 */
int get_logic_word(char * str);

/*!
 * get logistic result of scw_segment_words. "ÕæÌï ĞãÒ»" will be changed into "ÕæÌïĞãÒ»", etc. **JP wordseg only**
 * @param <pwdict> - the worddict
 * @param <pout> - the out struct
 * @return 1 if success and changed, 0 if success and not changed, <0 if failed.
 */
int get_lgt_scw_segment_words(scw_worddict_t * pwdict, scw_out_t * pout);

/*!
 * Output the dynword info.
 */
int scw_output_dynwordinfo(scw_worddict_t* pwdict,scw_inner_t * pir,FILE * fp);

/*!
 * Get the dict type.
 */
u_int scw_get_wdtype(scw_worddict_t * pwdict);

/*!
 * Set the dict type.
 */
void scw_set_wdtype(scw_worddict_t * pwdict,u_int wdtype);

/*!
 * Show the  seg version.
 */
void scw_show_version();

/*
 * get wordseg version
 * success : length of version str;
 * fail : -1;
*/
int scw_get_wordseg_version(char *str, int len);


/*!
 * Get the seg version.
 * success:return the version in high 3 byte of u_int, each num with a corresponding byte
 * fatal:0
 */
u_int scw_get_version();

/*!
 * Get original form of verb and adj **Japan Only**
 * @param <dict> - the pre-built ime dict with prop
 * @param <src> - input term
 * @param <slen> - input term length
 * @param <des> - result term (same as input term if no original form)
 * @param <dlen> - result buffer length
 * @return 
 * 	>0  result term length
 * 	=0  input term has no original form (not verb or adj maybe)
 * 	<0  fatal
 */
int scw_get_goi(Sdict_search * dict, char * src, int slen, char * des, int dlen);

// µ÷ÓÃ´Ëº¯Êı´ò¿ª¶¯Ì¬ÊıÁ¿´ÊºÏ²¢¿ª¹Ø
void set_scw_tn();
//µ÷ÓÃ´Ëº¯Êı¹Ø±Õcrf
void set_close_crf();
//µ÷ÓÃ´Ëº¯Êı¹Ø±Õwordner
void set_close_ner();

/*
 * @brief:Ô¤´¦Àíºó½øĞĞcrfÇĞ´Ê
 * @param:
 *  crf_tag,  crf_outÎªcrfÇĞ·ÖĞèÒªµÄ²ÎÊı¡£
 * scw_crf_out* crf_res ×°ÔØÔ¤´¦ÀíºóµÄcrfµÄÇĞ·Ö½á¹û¡
 * const u_int max_count £ºcrf_res ÖĞ×î¶à×°ÔØµÄÇĞ·ÖÆ¬¶ÎÊı¡£
 * const char* inbuf: ÇĞ·ÖÆ¬¶Î£»
 * const int len: ÇĞ·ÖÆ¬¶Î³¤¶È£»
 * const int nbest£º Êä³önbestÇĞ·Ö½á¹û¡£
 * const vlevel£º Êä³ö¸ÅÂÊ¿ª¹Ø£»
 */
//int crf_segment_words_processed(SegCrfTag* crf_tag, crf_out_t* crf_out, 
//scw_crf_out* crf_res, const u_int max_count, const char* inbuf,
//const int inlen, const int nbest=1,const int vlevel=0);
int crf_segment_words_processed(scw_inner_t *pir, const char* inbuf, 
const int inlen, const int nbest=1,const int vlevel=0);

#endif

