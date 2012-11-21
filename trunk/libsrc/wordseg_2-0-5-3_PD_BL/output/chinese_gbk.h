#ifndef CHINESE_GBK_H
#define CHINESE_GBK_H

#include "scwdef.h"
#include "isegment.h"
#include "property.h"

#define SCW_WD_CH 1
#define DATA_CNAME 0

#define MAX_FNAME_LEN_GBK 14

#define TRUE 1
#define FALSE 0

#define CHN_CHAR_NUM  6768

#define SYM_HIMIN  0xA1  /* GB symbol : minmum value of first byte */
#define SYM_HIMAX  0xF7  /* GB symbol : maxmum value of first byte */
#define SYM_LOMIN  0xA1  /* GB symbol : minmum value of second byte */
#define SYM_LOMAX  0xFE  /* GB symbol : maxmum value of second byte */

#define CHAR_HIMIN  0xB0  /* GB char   : min - 1st byte */
#define CHAR_HIMAX  0xF7  /* GB char   : max - 1st byte */
#define CHAR_LOMIN  0xA1  /* GB char   : min - 2nd byte */
#define CHAR_LOMAX  0xFE  /* GB char   : max - 2nd byte */

#define CHAR_HINUM      0x48  /* (0xF7-0xB0+1) */
#define CHAR_LONUM      0x5E  /* (0xFE-0xA1+1) */

#define MARK_HIMIN  0xA1  /* GB mark   : min - 1st byte */
#define MARK_HIMAX  0xAF  /* GB mark   : max - 1st byte */
#define MARK_LOMIN  0xA1  /* GB mark   : min - 2nd byte */
#define MARK_LOMAX  0xFE  /* GB mark   : max - 2nd byte */

#define SMARK_HIMIN      0xA3    /* simple GB mark   : min - 1st byte */
#define SMARK_HIMAX      0xA3    /* simple GB mark   : max - 1st byte */
#define SMARK_LOMIN      0xA1    /* simple GB mark   : min - 2nd byte */
#define SMARK_LOMAX      0xFE    /* simple GB mark   : max - 2nd byte */

#define DIGIT_HIMIN     0xa3    /* GB 0---9 */
#define DIGIT_HIMAX     0xa3
#define DIGIT_LOMIN     0xb0
#define DIGIT_LOMAX     0xb9
 
#define HI_HIMIN        0xa3    /* GB A---Z */   
#define HI_HIMAX        0xa3
#define HI_LOMIN        0xc1
#define HI_LOMAX        0xda
                        
#define LOW_HIMIN       0xa3    /* GB a---z */ 
#define LOW_HIMAX       0xa3
#define LOW_LOMIN       0xe1
#define LOW_LOMAX       0xfa

#define GBK_HIMIN       0x81
#define GBK_HIMAX       0xfe
#define GBK_LOMIN       0x40
#define GBK_LOMAX       0xfe

/* define the range of gb */
#define IN_RANGE(ch, min, max) ( (((unsigned char)(ch))>=(min)) && (((unsigned char)(ch))<=(max)) )

#define IS_GB(cst)  ( IN_RANGE((cst)[0], SYM_HIMIN, SYM_HIMAX) && IN_RANGE((cst)[1], SYM_LOMIN, SYM_LOMAX) )

#define IS_GBK(cst)      ( IN_RANGE((cst)[0], GBK_HIMIN, GBK_HIMAX) && IN_RANGE((cst)[1], GBK_LOMIN, GBK_LOMAX) )

#define IS_GB_CODE(cst)  ( IN_RANGE((cst)[0], CHAR_HIMIN, CHAR_HIMAX) && IN_RANGE((cst)[1], CHAR_LOMIN, CHAR_LOMAX) )

#define IS_GB_MARK(cst)  ( IN_RANGE((cst)[0], MARK_HIMIN, MARK_HIMAX) && IN_RANGE((cst)[1], MARK_LOMIN, MARK_LOMAX) )

#define IS_GB_SMAEK(cst) ( IN_RANGE((cst)[0], SMARK_HIMIN, SMARK_HIMAX) && IN_RANGE((cst)[1], SMARK_LOMIN, SMARK_LOMAX) )

#define IS_GB_SPACE(cst) ( (((unsigned char)((cst)[0]))==SYM_HIMIN) && (((unsigned char)((cst)[1]))==SYM_LOMIN) )

#define IS_GB_NOBREAK(cst) ( (unsigned char)(cst)[0]==0xa3 && ((unsigned char)(cst)[1]==0xa6 ||(unsigned char)(cst)[1]==0xad ||(unsigned char)(cst)[1]==0xdf) )

#define IS_GB_1234(cst) (IN_RANGE((cst)[0],DIGIT_HIMIN,DIGIT_HIMAX) && IN_RANGE((cst)[1],DIGIT_LOMIN,DIGIT_LOMAX) )

#define IS_GB_ABCD(cst) ( (IN_RANGE((cst)[0],HI_HIMIN,HI_HIMAX) && IN_RANGE((cst)[1],HI_LOMIN,HI_LOMAX) ) || (IN_RANGE((cst)[0],LOW_HIMIN,LOW_HIMAX) && IN_RANGE((cst)[1],LOW_LOMIN,LOW_LOMAX) ) ) 

//Other gb characters can not displayed properly:
//A9: F5--F6
//A8: EA--F6
//    96--A0
//A7: F2--F6
//    C2--D0
//A6: B9--C0
//    D9--DF
//    F6--FE
//A4: F3--F7
//A2: FE
//A2: FD--FE
#define IS_GB_UNDEFINED(cst) ( (IN_RANGE((cst)[0], 0xaa, 0xaf) && IN_RANGE((cst)[1], 0xa1, 0xfe) ) || (IN_RANGE((cst)[0], 0xf8, 0xfe) && IN_RANGE((cst)[1], 0xa1, 0xfe) ) || (IN_RANGE((cst)[0], 0xa1, 0xa7) && IN_RANGE((cst)[1], 0x40, 0xa0) ) || ( IN_RANGE((cst)[0], 0xa4, 0xa9) && IN_RANGE((cst)[1], 0xf7, 0xfe)) ) 

int scw_judge_backtrack_gbk_chinese(scw_worddict_t * pwdict,u_int flmpos,u_int ftlmpos,u_int slmpos,u_int stlmpos);
int get_lgt_scw_seg_gbk_chinese(scw_worddict_t * pwdict,scw_inner_t * pir);
int humanname_recg_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir);
int judge_fname_gbk_chinese(scw_worddict_t * pwdict,scw_inner_t* pir,u_int bpos,int lmcnt);
int fhumanname_recg_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir);
int canbe_long_name_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
int is_3word_cname_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
int judge_3cname_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t* plm1,scw_lemma_t* plm2,scw_lemma_t* plm3,int flag=0x1);
int is_2word_cname_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
int is_real_surname_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
int * scw_get_nameprob_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);
int is_my_alnum_gbk_chinese(char * p);
int is_my_num_gbk_chinese(char * p);
int is_my_alpha_gbk_chinese(char * p);
void ch_set_prop_by_str_gbk_chinese(char*pstr,scw_property_t& property);
void ch_write_prop_to_str_gbk_chinese(scw_property_t property, char* buffer);
int is_gbk_hz_gbk_chinese(unsigned char* pstr);
int is_my_ascii_gbk_chinese(char * pstr);
int is_mid_mark_gbk_chinese(char * pstr);
int is_split_headmark_gbk_chinese(char * pstr);
int is_split_tailmark_gbk_chinese(char * pstr);
int is_radix_point_gbk_chinese(char * pstr);
int is_fourbit_point_gbk_chinese(const char * word);
u_int get_value_gb_gbk_chinese(const char * buf, int& pos, const int len);
int is_oneword_gb_gbk_chinese(char * word);
int is_3human_need_reseg_gbk_chinese(scw_worddict_t* pwdict,scw_inlemma_t* pilemma,char* src);
int reseg_3human_phrase_gbk_chinese(scw_worddict_t* pwdict,scw_inlemma_t *pilemma);
int update_multichr_lmprob_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);
void scw_init_utilinfo_gbk_chinese(scw_utilinfo_t * puti,int wdtype);
int scw_seek_ambfrag_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos);
int scw_add_lmprob_gbk_chinese(scw_worddict_t * pwdict,scw_lemma_t * plm,int * prob,int pbsize,int type);
int scw_build_nameinfo_gbk_chinese(const char * namefile,int type,scw_worddict_t * pwdict);
int seg_split_gbk_chinese( Sdict_search* phrasedict, scw_out_t* pout, token_t subtokens[], int tcnt );
int seg_split_tokenize_gbk_chinese( Sdict_search* phrasedict, handle_t handle, token_t token, token_t subtokens[], int tcnt);

#endif


