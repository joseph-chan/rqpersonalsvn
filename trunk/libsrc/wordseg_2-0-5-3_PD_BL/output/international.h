
#ifndef INTERNATIONAL_H
#define INTERNATIONAL_H

#include "scwdef.h"
#include "chinese_gbk.h"
//#include "chinese_utf8.h"

/*
#define UTF8 0
#define GBK 1
#define CHINESE 0
#define JAPANESE 1
*/
        
extern int (*scw_judge_backtrack)(scw_worddict_t * pwdict,u_int flmpos,u_int ftlmpos,u_int slmpos,u_int stlmpos);
extern int (*get_lgt_scw_seg)(scw_worddict_t * pwdict,scw_inner_t * pir);
extern int (*humanname_recg)(scw_worddict_t* pwdict, scw_inner_t* pir);		
extern int (*judge_fname)(scw_worddict_t * pwdict,scw_inner_t* pir,u_int bpos,int lmcnt);
extern int (*fhumanname_recg)(scw_worddict_t* pwdict, scw_inner_t* pir);		
extern int (*canbe_long_name)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
extern int (*is_3word_cname)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
extern int (*judge_3cname)(scw_worddict_t* pwdict,scw_lemma_t* plm1,scw_lemma_t* plm2,scw_lemma_t* plm3,int flag);
extern int (*is_2word_cname)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
extern int (*is_real_surname)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
extern int *(*scw_get_nameprob)(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);
extern int (*is_my_alnum)(char * p);
extern int (*is_my_num)(char * p);
extern int (*is_my_alpha)(char * p);
extern void (*ch_set_prop_by_str)(char*pstr,scw_property_t& property);
extern void (*ch_write_prop_to_str)(scw_property_t property, char* buffer);
extern int (*is_gbk_hz)(unsigned char* pstr);
extern int (*is_my_ascii)(char * pstr);
extern int (*is_mid_mark)(char * pstr);
extern int (*is_split_headmark)(char * pstr);
extern int (*is_split_tailmark)(char * pstr);
extern int (*is_radix_point)(char * pstr);
extern int (*is_fourbit_point)(const char * word);
extern u_int (*get_value_gb)(const char * buf, int& pos, const int len);
extern int (*is_oneword_gb)(char * word);
extern int (*is_3human_need_reseg)(scw_worddict_t* pwdict,scw_inlemma_t* pilemma,char* src);
extern int (*reseg_3human_phrase)(scw_worddict_t* pwdict,scw_inlemma_t *pilemma);
extern int (*update_multichr_lmprob)(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);
extern void (*scw_init_utilinfo)(scw_utilinfo_t * puti,int wdtype);
extern int (*scw_seek_ambfrag)(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos);
extern int (*scw_add_lmprob)(scw_worddict_t * pwdict,scw_lemma_t * plm,int * prob,int pbsize,int type);
extern int (*scw_build_nameinfo)(const char * namefile,int type,scw_worddict_t * pwdict);
//extern int (*seg_split)( Sdict_search* phrasedict, scw_out_t* pout, token_t subtokens[], int tcnt );
//extern int (*seg_split_tokenize)( Sdict_search* phrasedict, handle_t handle, token_t token, token_t subtokens[], int tcnt);
	
int init_function(int code, int language);

#endif	
