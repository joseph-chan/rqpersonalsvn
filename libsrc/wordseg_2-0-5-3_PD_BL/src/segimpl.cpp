/*
 * Filename  : scword.cpp -> scwseg.cpp -> segimpl.cpp
 * Descript  : This is the implementation file for segmentation
 * Author  : Paul Yang, zhenhaoji@sohu.com
 * Time    : 2004-12-31
 */

/*
 * Revision 1.2.4.0  2007/08/22  David Dai
 * 1. Revised a interface bug when get the segmented result in get_lmstr(), added _get_token_str() method.
 * 2. Perfected the strategies when selecting the different segment ways to a string.
 * 3. Perfected the error log method.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "property.h"
#include "scwdef.h"
#include "international.h"
//------------------------------------------------------------------------------------------------------------------//
// Macro Definition
#define MAX_PAJMSTR_LENGTH 20
#define MIN_SPECSTR_LEN 3
#define MIN_LTR_LEN 1
//#define MAX_FNAME_LEN 14

//#define JPHUMAN_ACROSS_BLANK_STRICT

// <log>
#define LOG_ERROR(message)  ul_writelog(UL_LOG_FATAL, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
#define WARNING_LOG(message)  ul_writelog(UL_LOG_WARNING, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
// </log>

// <extern>
//------------------------------------------------------------------------------------------------------------------//
extern u_int scw_seek_lemma(scw_worddict_t * pwdict,const char* term,int len);
extern u_int scw_get_word(const char* inbuf, int &pos, const int slen, const char* charset);
extern u_int scw_seek_entry(scw_worddict_t* pwdict, u_int lde, u_int value);
extern scw_lemma_t* mm_seek_lemma(scw_worddict_t* pwdict, char* buff, int slen, int& curpos, int basiconly);
//extern int scw_seek_ambfrag(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos);
//extern int is_my_ascii(char * p);
//extern int is_mid_mark(char * p);
extern int is_gb_pajm(char * p);
//extern int is_my_alnum(char * p);
//extern int is_my_alpha(char * p);
//extern int is_my_num(char * p);
//extern int is_split_headmark(char * p);
//extern int is_split_tailmark(char * p);
//extern int is_radix_point(char * p);
//extern int is_fourbit_point(const char * word);
extern void set_prop_by_str(char* pstr,scw_property_t& property,int wdtype);
extern void write_prop_to_str(scw_property_t& property, char* buffer,int wdtype);
extern void set_scw_tn();
extern scw_crf_out *scw_crf_create(u_int tsize);
extern void scw_crf_des(scw_crf_out* pout);

// </extern>

// functions for manage the segged result
static bool all_asc(const char* str);
static inline int append_lemma( scw_worddict_t* pwdict, scw_inner_t *pir, u_int lmpos, int type);
static inline int append_lemma(scw_worddict_t* pwdict, scw_inner_t *pir, scw_lemma_t* plm);
static inline u_int scw_get_fmm_word(scw_worddict_t *pwdict,const char * inbuf,int inlen,int &pos);
//int * scw_get_nameprob(scw_worddict_t* pwdict,scw_lemma_t * plmi,int type);
int get_lmoff_step(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm);
int get_lmstr(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, char* dest, u_int destlen);

static int scw_seg_fmm(scw_worddict_t * pwdict,scw_inner_t * pir, const char* inbuf,const int inlen, bool human_allow_blank);
// functions for dynamic lemma operation
inline int create_dynword(scw_inner_t* pinner, char* str);
static inline int create_dphrase(scw_worddict_t* pwdict, scw_inner_t* pir, int bpos, int count);
//int poly_lemmas(scw_worddict_t* pwdict, scw_inner_t* pir, int bpos, int count,int bname = 0);

// functions for named entiry recgnition
static inline int big_phrase_recg(scw_worddict_t* pwdict, scw_inner_t* pir);
static inline int tn_recg(scw_worddict_t* pwdict, scw_inner_t* pir);
static inline int bookname_recg(scw_worddict_t* pwdict, scw_inner_t* pir);
//static inline int humanname_recg(scw_worddict_t* pwdict, scw_inner_t* pir);
//static inline int fhumanname_recg(scw_worddict_t* pwdict, scw_inner_t* pir);
static inline int jphumanname_recg(scw_worddict_t* pwdict, scw_inner_t* pir, bool human_allow_blank);
//static inline int canbe_long_name(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
//static inline int is_3word_cname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
//static inline int is_2word_cname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
//static inline int judge_3cname(scw_worddict_t* pwdict,scw_lemma_t* plm1,scw_lemma_t* plm2,scw_lemma_t* plm3,int flag=0x1);
//static inline int is_real_surname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
int poly_lemmas(scw_worddict_t* pwdict, scw_inner_t* pir, int bpos, int count,int bname = 0);
// functions for ascii and chinese number recgnition
static inline int proc_cnum(scw_worddict_t * pwdict,scw_inner_t * pir, 
		const char* inbuf, int inlen, u_int pos, int llmpos, int &newpos);
static inline int proc_ascii(scw_worddict_t * pwdict,scw_inner_t * pir, 
		const char * buf,int slen, int bpos, int lpos,scw_lemma_t * pllm, int& newpos);
//functions for adding ascii subphrase
int append_asciistr_subphrase(scw_worddict_t * pwdict,scw_inner_t* pir, scw_lemma_t * plm);
int append_asciistr_subphrase_crf(scw_worddict_t * pwdict,scw_inner_t* pir, scw_lemma_t * plm);
int create_ascii_dphrase(scw_worddict_t* pwdict, scw_inner_t* pir, int phrase_bpos, int bpos, int count);

static inline int proc_pajm(scw_worddict_t * pwdict,scw_inner_t * pir,
		const char * buf,int slen, int bpos, int lpos, int& newpos);
scw_lemma_t* get_oneword_lemma(scw_worddict_t* pwdict, int wordvalue);
extern int worddict2inner(scw_worddict_t* pwdict, scw_lemma_t *pwdictplm, scw_lemma_t *pirplm, scw_inner_t *pir);

// 动态数量词合并全局变量，默认false不合并，通过scwseg.h中的set_scw_tn()开关置true
bool g_scw_tn=false;
void set_scw_tn()
{
	g_scw_tn=true;
}

bool g_use_crf = true;
void set_close_crf()
{
	g_use_crf = false;
}
bool g_use_ner = true;
void set_close_ner()
{
	g_use_ner = false;
}

/*!
 * Search keyword from odict
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static int mysearch(sodict_search_t* sdb, char* key)
{
	if(key == NULL)
	{
		WARNING_LOG("search key is not available");
		return -1;
	}
	if(sdb == NULL)
	{
		LOG_ERROR("Cannot find odict dictionary");
		return -1;
	}

	u_int len = strlen(key);
	if(len > 12 || len < 4 ){
		return 0; 
	}
	//    printf("---%s\n",key);
	u_int sign1=0;
	u_int sign2 = 0 ;  
	sodict_snode_t snode;  
	creat_sign_fs64(key, len, &sign1, &sign2);
	snode.sign1 = sign1; 
	snode.sign2 = sign2;  
	if (ODB_SEEK_OK == odb_seek_search(sdb, &snode))
	{   
		return  snode.cuint1;   
	} 
	return 0;
}


/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
// scw_create_inner
scw_inner_t* scw_create_inner(int maxterm, int flag)
{  
	scw_inner_t* pir = NULL;
	scw_newword_t* pnewword = NULL;
	if(maxterm > MAX_TERM)
	{
		LOG_ERROR("maxterm is larger than MAX_TERM");
		goto failed;
	}

	if( (pir=(scw_inner_t*)calloc(1, sizeof(scw_inner_t))) == 0 ){
		goto failed;
	}

	pir->m_maxterm = maxterm;
	pir->m_flag = flag;


	if((pir->m_tmpbuf = (char*)calloc(maxterm*30, sizeof(char)) ) == NULL  )
	{
		goto failed;
	}

	pir->m_protect_cnt = 0;
	if((pir->m_protect = (u_int*)calloc(maxterm, sizeof(u_int)) ) == NULL  )
	{
		goto failed;
	}

	if( (pir->m_ppseg = (scw_lemma_t**)calloc(maxterm, sizeof(scw_lemma_t*))) == NULL ){
		goto failed;
	}
	pir->m_ppseg_flag = NULL;
	if( (pir->m_ppseg_flag = (int*)calloc(maxterm, sizeof(int))) == NULL ){
		goto failed;
	}
	if( (pir->tmp_ppseg = (scw_lemma_t**)calloc(maxterm, sizeof(scw_lemma_t*))) == NULL ){
		goto failed;
	}


	pir->m_dynbuf    = (char*)calloc(maxterm*20, sizeof(char));
	pir->m_dynphinfo  = (uintptr_t *)calloc(maxterm*20+MAX_EXTEND_PHINFO_SIZE, sizeof(uintptr_t));
	pir->m_dynphinfo_tmp  = (uintptr_t *)calloc(maxterm*20+MAX_EXTEND_PHINFO_SIZE, sizeof(uintptr_t));
	pir->m_dynlm    = (scw_lemma_t*)calloc(maxterm*2,sizeof(scw_lemma_t));
	if( !(pir->m_dynbuf && pir->m_dynphinfo && pir->m_dynlm) ){
		goto failed;
	}

	pir->m_dynb_size  = maxterm*20;
	pir->m_dynph_size  = maxterm*20 + MAX_EXTEND_PHINFO_SIZE;
	pir->m_dynph_size_tmp  = maxterm*20 + MAX_EXTEND_PHINFO_SIZE;
	pir->m_dynlm_size  = maxterm;

	//mandict_word_prop init, add by wanglijie
	if((pir->m_mandictword_prop = (u_int*)calloc(maxterm,sizeof(u_int))) == NULL)
	{
		goto failed;
	}
	pir->m_mandictword_size = maxterm;
	pir->m_mandictword_cnt = 0;

	pir->m_tmpbuf_len = 0;
	pir->crf_out=NULL;
	pir->crf_res=NULL;
	pir->crf_tag=NULL;
	pir->multiterm_pack=NULL;

	//-------------crf_out_t-------------------------------------------

	if(g_use_crf){

		if( (pir->crf_out = crf_create_out(CRF_MAX_WORD_LEN+1))  == NULL){
			goto failed;
		}

		if( (pir->crf_res = scw_crf_create(maxterm+1))  == NULL){
			goto failed;
		}

		if( (pir->crf_tag = new SegCrfTag(MAX_NBEST,CRF_MAX_WORD_LEN
						, CRF_MAX_WORD_LEN) )==NULL){
			goto failed;
		}

	}

	/*-----------newword pack create----------*/
	pir->multiterm_pack= dm_pack_create(maxterm);
	if(pir->multiterm_pack == NULL){
		goto failed;
	}

	//---------------newword-------------------------------------------
	if( (pir->pnewword = (scw_newword_t*)calloc(1, sizeof(scw_newword_t))) == NULL ) {
		goto failed;
	}
	pnewword = pir->pnewword;
	if( (pnewword->newwordbuf= (char*)calloc(maxterm*10, sizeof(char))) == NULL){
		goto failed;
	}
	pnewword->newwordbsize = maxterm*10*sizeof(char);
	pnewword->newwordb_curpos = 0;
	pnewword->newwordbtermcount = 0;

	if( (pnewword->newwordbtermoffsets= (u_int*)calloc(maxterm*2, sizeof(u_int))) == NULL){
		goto failed;
	}
	if( (pnewword->newwordbtermpos = (u_int*)calloc(maxterm, sizeof(u_int))) == NULL){
		goto failed;
	}

	if( (pnewword->newwordbtermprop = (scw_property_t*)calloc(maxterm, sizeof(scw_property_t))) == NULL){
		goto failed;
	}

	if((pnewword->newwordneprop = (neprop_t *)malloc(sizeof(neprop_t) * (maxterm + 1)) ) ==NULL){
		goto failed;
	}

	return pir;

failed:
	if(pir)
	{
		if(g_use_crf)
		{
			if(pir->crf_out != NULL )
			{
				crf_create_des(pir->crf_out);
				pir->crf_out = NULL;
			}
			if(pir->crf_res != NULL)
			{
				scw_crf_des(pir->crf_res);
				pir->crf_res = NULL;
			}
			if(pir->crf_tag)
			{
				delete(pir->crf_tag);
				pir->crf_tag=NULL;
			}


		}


		free(pir->m_tmpbuf);
		pir->m_tmpbuf=NULL;
		free(pir->m_protect);
		pir->m_protect=NULL;
		free(pir->m_dynbuf);
		pir->m_dynbuf=NULL;
		free(pir->m_dynphinfo);
		pir->m_dynphinfo=NULL;
		free(pir->m_dynlm);
		pir->m_dynlm=NULL;
		free(pir->m_dynphinfo_tmp);
		pir->m_dynphinfo_tmp=NULL;
		free(pir->m_ppseg);
		free(pir->tmp_ppseg);
		pir->tmp_ppseg=NULL;
		pir->m_ppseg=NULL;

		if(pir->m_ppseg_flag!=NULL)
		{
			free(pir->m_ppseg_flag);
			pir->m_ppseg_flag = NULL;
		}
		if(pir->multiterm_pack != NULL )
		{
			dm_pack_del(pir->multiterm_pack);
			pir->multiterm_pack=NULL;
		}
		if(pir->pnewword != NULL)
		{
			free(pir->pnewword->newwordbuf);
			free(pir->pnewword->newwordbtermoffsets);
			free(pir->pnewword->newwordbtermpos);
			free(pir->pnewword->newwordbtermprop);
			free(pir->pnewword->newwordneprop);

			pir->pnewword->newwordbtermpos = NULL;
			pir->pnewword->newwordbtermprop = NULL;
			pir->pnewword->newwordbtermoffsets = NULL;
			pir->pnewword->newwordneprop = NULL;
			pir->pnewword->newwordbuf = NULL;

			free(pir->pnewword);
			pir->pnewword=NULL;
		}

		if(pir->m_mandictword_prop != NULL)
		{
			free(pir->m_mandictword_prop);
			pir->m_mandictword_prop = NULL;
		}

		free(pir);
		pir=NULL;
	}

	LOG_ERROR("error: calloc error");

	return NULL;
}
// reset_inner
void reset_inner(scw_inner_t* pir)
{
	pir->m_dynbuf[0]  = '\t';
	pir->m_dynbuf[1]  = 0;
	pir->m_dynb_curpos  = 1;
	pir->m_dynph_curpos  = 0;
	pir->m_dynph_curpos_tmp  = 0;
	pir->m_dynlm_cnt  = 0;
	pir->m_dynph_curpos  = 0;

	pir->m_ppseg_cnt  = 0;
	pir->tmp_ppseg_cnt  = 0;
	pir->m_protect_cnt = 0;
	
	pir->m_mandictword_cnt = 0;

	pir->pnewword->newwordbuf[0]  = '\t';
	pir->pnewword->newwordbuf[1]  = 0;
	pir->pnewword->newwordb_curpos = 1;
	pir->pnewword->newwordbtermcount = 0;

}

/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
// destroy_inner
void scw_destroy_inner(scw_inner_t* pir)
{
	if(pir)
	{
		if(g_use_crf)
		{
			crf_create_des(pir->crf_out);
			pir->crf_out = NULL;
			scw_crf_des(pir->crf_res);
			pir->crf_res = NULL;

			delete(pir->crf_tag);
			pir->crf_tag = NULL;


		}


		free(pir->m_tmpbuf);
		free(pir->m_protect);
		free(pir->m_ppseg);
		free(pir->tmp_ppseg);
		free(pir->m_dynbuf);
		free(pir->m_dynphinfo);
		free(pir->m_dynphinfo_tmp);
		free(pir->m_dynlm);

		pir->m_dynphinfo_tmp=NULL;
		pir->m_dynlm=NULL;
		pir->m_dynphinfo=NULL;
		pir->m_dynbuf=NULL;
		pir->tmp_ppseg=NULL;
		pir->m_ppseg=NULL;
		pir->m_tmpbuf=NULL;
		pir->m_protect=NULL;

		if(pir->m_ppseg_flag != NULL)
		{
			free(pir->m_ppseg_flag);
			pir->m_ppseg_flag = NULL;
		}
		if(pir->m_mandictword_prop)
		{
			free(pir->m_mandictword_prop);
			pir->m_mandictword_prop = NULL;
		}
		if(pir->multiterm_pack != NULL)
		{
			dm_pack_del(pir->multiterm_pack);
			pir->multiterm_pack=NULL;
		}

		if(pir->pnewword != NULL)
		{
			free(pir->pnewword->newwordbuf);
			free(pir->pnewword->newwordbtermoffsets);
			free(pir->pnewword->newwordbtermpos);
			free(pir->pnewword->newwordbtermprop);
			free(pir->pnewword->newwordneprop);

			pir->pnewword->newwordbtermpos = NULL;
			pir->pnewword->newwordbtermprop = NULL;
			pir->pnewword->newwordbtermoffsets = NULL;
			pir->pnewword->newwordneprop = NULL;
			pir->pnewword->newwordbuf = NULL;

			free(pir->pnewword);
			pir->pnewword=NULL;
		}

		free(pir);
		pir=NULL;
	}
}


// append_lemma
/*========================================================================================
 * function : append a lemma to internal segged result
 * argu     : pout, the out struct
 *          : pwdict, the worddict
 *          : plemma, the lemma
 * return   : 1 if success
 *          : <0 if failed
 ========================================================================================*/
int append_lemma(scw_worddict_t* pwdict, scw_inner_t *pir, u_int lmpos, int type)    
{  
	if(pir->m_ppseg_cnt + 1 > pir->m_maxterm)
	{
		WARNING_LOG("error: m_ppseg_cnt out of range");
		return -1;
	}

	if(type==SCW_STATIC)
	{
		pir->m_ppseg_flag[pir->m_ppseg_cnt] = 0;
		pir->m_ppseg[pir->m_ppseg_cnt++] = &(pwdict->m_lemmalist[lmpos]);
	}
	else if(type==SCW_DYNAMIC)
	{
		pir->m_ppseg_flag[pir->m_ppseg_cnt] = 0;
		pir->m_ppseg[pir->m_ppseg_cnt++] = &(pir->m_dynlm[lmpos]);
	}
	else
	{
		WARNING_LOG("error: type undefined");
		return -1;
	}

	return 0;
}


// append_lemma
/*========================================================================================
 * function : append a lemma to internal segged result
 * argu     : pout, the out struct
 *          : pwdict, the worddict
 *          : plemma, the lemma
 * return   : 1 if success
 *          : <0 if failed
 ========================================================================================*/
int append_lemma(scw_worddict_t* pwdict,scw_inner_t *pir, scw_lemma_t* plm)    
{
	if(pir->m_ppseg_cnt + 1 > pir->m_maxterm)
	{
		WARNING_LOG("error: m_ppseg_cnt out of range");
		return -1;
	}

	scw_lemma_t *pirplm = NULL;
	scw_lemma_t *tmpplm = NULL;

	if(plm->m_type == LEMMA_TYPE_MAN)
	{
		int cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];//获取人工切分片段个数
		if(pir->m_ppseg_cnt + cnt >= pir->m_maxterm)
		{
			WARNING_LOG("error: m_ppseg_cnt out of range");
			return -1;
		}

		for(int i = 0; i < cnt; i++)
		{
			tmpplm = &(pwdict->m_lemmalist[pwdict->m_phinfo[plm->m_phinfo_bpos + i + 1]]);
			switch(tmpplm->m_type)
			{
				//如果是静态词，直接取出来做切分结果即可。
				case LEMMA_TYPE_SBASIC:
				case LEMMA_TYPE_SPHRASE:
					pir->m_ppseg_flag[pir->m_ppseg_cnt] = 1;
					pir->m_ppseg[pir->m_ppseg_cnt ++] = tmpplm;
					break;
				case LEMMA_TYPE_DBASIC:
				case LEMMA_TYPE_DPHRASE:
					if(pir->m_dynlm_cnt>=pir->m_dynlm_size)
					{
						WARNING_LOG(" m_dynlm_cnt out of range\n");
						return -1;
					}

					pirplm = &pir->m_dynlm[pir->m_dynlm_cnt];
					pir->m_dynlm_cnt++;
					if(worddict2inner(pwdict, tmpplm, pirplm, pir) < 0)
					{
						WARNING_LOG("error: when worddict2inner!\n");
						return -1;
					}
					pir->m_ppseg_flag[pir->m_ppseg_cnt] = 1;
					pir->m_ppseg[pir->m_ppseg_cnt ++] = pirplm;
					break;
				default:
					WARNING_LOG("this is not a normal type lemma!\n");
					return -1;
			}
		}
	}
	else
	{
		pir->m_ppseg_flag[pir->m_ppseg_cnt] = 0;
		pir->m_ppseg[pir->m_ppseg_cnt++] = plm;
	}
	return 0;
}

int append_manword(scw_worddict_t* pwdict, scw_lemma_t* plm, scw_inner_t* pir, u_int offset)
{
	const char* where="append_manword";
	char word[MAX_WORD_LEN] = {0};
	int len_all=0;
	int len = 0;
	int cnt = 0, i = 0, ph_cnt = 0;
	scw_lemma_t* tmplm = NULL;
	scw_lemma_t* tmplm2 = NULL;
	
	u_int pos = offset;
	u_int bpos = 0;

	if(plm==NULL)
		return 0;
	/*if(IS_MANWORD(plm->m_property))
		return 0;*/
	switch(plm->m_type)
	{
		case LEMMA_TYPE_SBASIC:
		case LEMMA_TYPE_DBASIC:
			if(pir->m_mandictword_cnt + 1 >= pir->m_mandictword_size)
			{
				WARNING_LOG("error: m_mandictword_cnt out of range");
				return -1;
			}
			len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
			if(len < 0)
			{
				return -1;
			}
			pir->m_mandictword_prop[pir->m_mandictword_cnt]=len<<24|pos;
			pir->m_mandictword_cnt++;
			pos += len;
			break;
		case LEMMA_TYPE_SPHRASE:
			bpos = plm->m_phinfo_bpos;
			ph_cnt = pwdict->m_phinfo[bpos];
			if(pir->m_mandictword_cnt + ph_cnt >= pir->m_mandictword_size)
			{
				WARNING_LOG("error: m_mandictword_cnt out of range");
				return -1;
			}
			for(int j=0; j<ph_cnt; j++)
			{
				tmplm2= &pwdict->m_lemmalist[pwdict->m_phinfo[bpos+j+1]];
				len = get_lmstr(pwdict, pir, tmplm2, word, sizeof(word));
				if(len < 0)
				{
					return -1;
				}
				pir->m_mandictword_prop[pir->m_mandictword_cnt]=len<<24|pos;
				pir->m_mandictword_cnt++;
				pos += len;
			}
			break;
		case LEMMA_TYPE_MAN:
			cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
			if(pir->m_mandictword_cnt + cnt >= pir->m_mandictword_size)
			{
				WARNING_LOG("error: m_mandictword_cnt out of range");
				return -1;
			}
			for(i=0; i<cnt; i++)
			{
				//tmplm = &(pwdict->m_lemmalist[pwdict->m_phinfo[plm->m_phinfo_bpos + i + 1]]);
				tmplm = pir->m_ppseg[pir->m_ppseg_cnt-cnt+i];
				//SET_WORDNOTDYN(tmplm->m_property);
				switch(tmplm->m_type){
					case LEMMA_TYPE_SBASIC:
					case LEMMA_TYPE_DBASIC:
						len = get_lmstr(pwdict, pir, tmplm, word, sizeof(word));
						if(len < 0)
						{
							return -1;
						}
						pir->m_mandictword_prop[pir->m_mandictword_cnt]=len<<24|pos;
						pir->m_mandictword_cnt++;
						pos += len;
						/*if(all_asc(word))
						{
							SET_MANWORD(tmplm->m_property);
						}*/
						break;
					case LEMMA_TYPE_SPHRASE:
						len_all = 0;
						bpos = tmplm->m_phinfo_bpos;
						ph_cnt = pwdict->m_phinfo[bpos];
						for(int j=0; j<ph_cnt; j++)
						{
							tmplm2= &pwdict->m_lemmalist[pwdict->m_phinfo[bpos+j+1]];
							len = get_lmstr(pwdict, pir, tmplm2, word, sizeof(word));
							if(len < 0)
							{
								return -1;
							}
							pir->m_mandictword_prop[pir->m_mandictword_cnt]=len<<24|pos;
							pir->m_mandictword_cnt++;
							pos += len;
							//len_all += len;
						}
						//pir->m_mandictword_prop[pir->m_mandictword_cnt++] = len_all<<24|pos;
						//pos += len_all;
						break;
					case LEMMA_TYPE_DPHRASE:
						len_all = 0;
						bpos = tmplm->m_phinfo_bpos;
						ph_cnt = pir->m_dynphinfo[bpos];
						for(int j=0; j<ph_cnt; j++)
						{
							tmplm2= (scw_lemma_t*)pir->m_dynphinfo[bpos+j+1];
							len = get_lmstr(pwdict, pir, tmplm2, word, sizeof(word));
							if(len < 0)
							{
								return -1;
							}
							pir->m_mandictword_prop[pir->m_mandictword_cnt]=len<<24|pos;
							pir->m_mandictword_cnt++;
							pos += len;
							//len_all += len;
						}		
						//pir->m_mandictword_prop[pir->m_mandictword_cnt++] = len_all<<24|pos;
						//pos += len_all;
						break;
					default:
						ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
						return -1;
				}
			}
			break;
		default:
			ul_writelog(UL_LOG_WARNING, "error in 2 %s\n", where);
			return -1;
	}
	return 0;
}
// scw_judge_backtrack
/*========================================================================================
 * function : process different meanings and select a segment result
 * argu     : pwdict, the worddict
 *          : inbuf,inlen, the buf and it's length
 *          : flemma,fpos, the first lemma and position
 *          : slemma,spos, the second lemma and position
 * return   : 1 if select the first position,
 *          : 0 if select the second position
 ========================================================================================*/
// 针对歧义片段的不同切分，根据各种策略选择具体的切分方式。
/*
   int scw_judge_backtrack(scw_worddict_t * pwdict,
   u_int flmpos,u_int ftlmpos,
   u_int slmpos,u_int stlmpos)
   {
   scw_lemma_t * lmlist[4];
   unsigned lenlist[4] = {0}; 

// judge the result
if(ftlmpos == LEMMA_NULL)
{
return 0;
}

lmlist[0]=pwdict->m_lemmalist+flmpos;
lenlist[0]=lmlist[0]->m_length;
lmlist[2]=pwdict->m_lemmalist+slmpos;
lenlist[2]=lmlist[2]->m_length;
lmlist[1]=pwdict->m_lemmalist+ftlmpos;
lenlist[1]=lmlist[1]->m_length;

if( lenlist[0] + lenlist[1] <= lenlist[2] )
{
return 0;
}

if(stlmpos == LEMMA_NULL )
{
return 1;
}

lmlist[3]=pwdict->m_lemmalist+stlmpos;
lenlist[3]=lmlist[3]->m_length;

// 优先根据歧义实例进行决策
if(scw_seek_ambfrag(pwdict,lmlist[2],lmlist[3],stlmpos))
return 0;
if(scw_seek_ambfrag(pwdict,lmlist[0],lmlist[1],ftlmpos))
return 1;

// 如果最长前缀词为短语，则选择当前最长切分作为此次切分结果。
// 这条规矩有点无聊，[经过调研，去掉此条策略效果明显变好！]
//if(IS_PHRASE(pwdict->m_lemmalist[flmpos].m_property))
{    
return 0;
}*/
/*
// 日文消岐策略
if(pwdict->m_wdtype & SCW_WD_JP )
{
int len01=lenlist[0]+lenlist[1];
int len23=lenlist[2]+lenlist[3];
// 如果较长切分的第一个单元不是单字，取长切分
// 主要解决"香|港の人|気"vs"香港|の|人気"这样的case
if( len01>len23 && lenlist[0]!=2 )
return 1;
else if( len01<len23 && lenlist[2]!=2 )
return 0;

double w[4];
int len=len01+len23;
// 否则进行权值比较
for( int i=0; i<4; i++ )
{
int my_len=i<2?len01:len23;
w[i] = (double)lmlist[i]->m_weight;
// 当长度>3字时考虑词长影响
// term权值=初始权值*自身长度*所在片段长度/所有片段长度和
if( len01>6 )
{
w[i] *= (double)lenlist[i]*my_len/len;
}
}
// 当切分片段01的第一个term是单字，并且长度大于切分片段23时
// 如果切分片段23的第二个term是单字，则权值给予1/3的惩罚
// 这是为了解决"新|大秦帝国"vs"新大|秦|帝国"这样的case
if( lenlist[0]==2&&lenlist[3]==2&&len23<len01 ) w[3]/=3;
//printf("%d:%f+%d:%f vs %d:%f+%d:%f\n",lenlist[0],w[0],lenlist[1],w[1],lenlist[2],w[2],lenlist[3],w[3]);
if( (w[0] + w[1]) - (w[2] + w[3]) > 0.00000001 )
{
return 1;
}
else
{
return 0;
}
} // 日文消歧策略完结
else
{

// 然后根据切分片段长度来进行比较
if(lenlist[0] + lenlist[1] < lenlist[2] + lenlist[3])
{
return 0;
}
else if(lenlist[0] + lenlist[1] > lenlist[2] + lenlist[3] )
{
return 1;
}

// 最后根据"词权重"来进行比较，权重用“归一化词频*长度系数”计算。
double w0 = lmlist[0]->m_weight; 
double w1 = lmlist[1]->m_weight;
double w2 = lmlist[2]->m_weight;
double w3 = lmlist[3]->m_weight;

unsigned len = lenlist[0] + lenlist[1];  
if( len > 6 ) // 如果大于3个汉字的歧义片段考虑词长影响
{
double freq = lmlist[0]->m_weight + lmlist[1]->m_weight + lmlist[2]->m_weight + lmlist[3]->m_weight; // 构建辞典的时候已经加1平滑了！
freq *= len;
// 归一化词频*长度系数
w0 = (lmlist[0]->m_weight*lenlist[0])/freq;
w1 = (lmlist[1]->m_weight*lenlist[1])/freq;
w2 = (lmlist[2]->m_weight*lenlist[2])/freq;
w3 = (lmlist[3]->m_weight*lenlist[3])/freq;

// [经过对比发现全局归一化比局部归一化好，所以下面局部归一化屏蔽掉！]
double freq01 = lmlist[0]->m_weight + lmlist[1]->m_weight; // 构建辞典的时候已经加1平滑了！
double freq23 = lmlist[2]->m_weight + lmlist[3]->m_weight;
w0 = (lmlist[0]->m_weight*lenlist[0])/(len*freq01);
w1 = (lmlist[1]->m_weight*lenlist[1])/(len*freq01);
w2 = (lmlist[2]->m_weight*lenlist[2])/(len*freq23);
w3 = (lmlist[3]->m_weight*lenlist[3])/(len*freq23);*/

// 如果有一个片段频率为0，则舍弃含此片段的切分，这样可以通过在辞典中设置0频率来灵活处理！
/*			if( (lmlist[0]->m_weight < 2) || (lmlist[1]->m_weight < 2) ) 
			{
			return 0;
			}  

			if( (lmlist[2]->m_weight < 2) || (lmlist[3]->m_weight < 2) ) 
			{ 
			return 1; 
			}    
			}


// 加入单字词后缀提权策略(add by liujianzhu, 2008.6)
if( (lenlist[0] == 4)  && (lenlist[1] == 2) && IS_1W_SUFFIX(pwdict->m_lemmalist[ftlmpos].m_property))
{
w1 += 1000;
}
if( (lenlist[2] == 4) && (lenlist[3] == 2) && IS_1W_SUFFIX(pwdict->m_lemmalist[stlmpos].m_property))
{
w3 += 1000;
}

if( (w0 + w1) - (w2 + w3) > 0.00000001 )
{
return 1;
}
else
{
return 0;
}
}// 因为加了日文消岐策略，以上这块儿代码只对中文奏效
}
*/

// 正向最大匹配：获取当前位置开始的最大匹配词。
u_int scw_get_fmm_word(scw_worddict_t *pwdict,const char * inbuf,int inlen,int &pos)
{
	u_int value = 0;
	u_int lde = 0;
	u_int nde = 0;
	u_int lmpos = LEMMA_NULL;
	int curpos = pos;
	value = scw_get_word(inbuf,pos,inlen,NULL);
	if(value == 0)
	{
		pos = curpos;
		return LEMMA_NULL;
	}
	curpos = pos;
	lde = DENTRY_FIRST;

	while( (value > 0) && 
			((nde = scw_seek_entry(pwdict,lde,value))!=DENTRY_NULL) )
	{
		if(pwdict->m_dictentry[nde].m_lemma_pos != LEMMA_NULL)
		{
			lmpos = pwdict->m_dictentry[nde].m_lemma_pos;
			curpos = pos;
		}
		value = scw_get_word(inbuf,pos,inlen,NULL);
		lde = nde;
	}

	pos = curpos;

	return lmpos;
}

// scw_backtrack
/*========================================================================================
 * function : when meet two different seperation, choose the better one.
 * argu     : pwdict, the worddict
 *          : inbuf,inlen, the buf and it's length
 *          : flemma,fpos, the first lemma and position
 *          : slemma,spos, the second lemma and position
 * return   : 1 if select the first position,
 *          : 0 if select the second position
 ========================================================================================*/
// 歧义决策，歧义的发现策略：
// 1. 记录当前最长匹配词以及此词的最长前缀词
// 2. 分别从这两个词的结束位置开始获取它们的下一个最长匹配词
// 3. 对这两种切分方式进行选择
inline int scw_backtrack(scw_worddict_t *pwdict,const char* inbuf,const int inlen,
		u_int &flmpos,int &fpos,u_int &slmpos,int &spos)
{
	int ret = 0;
	u_int ftlmpos = LEMMA_NULL;
	u_int stlmpos = LEMMA_NULL;

	// get the following lemma of the first lemma
	ftlmpos = scw_get_fmm_word(pwdict,inbuf,inlen,fpos);      
	// get the following lemma of the second lemma    
	stlmpos = scw_get_fmm_word(pwdict,inbuf,inlen,spos);

	ret = scw_judge_backtrack(pwdict,flmpos,ftlmpos,slmpos,stlmpos); 

	flmpos = ftlmpos;
	slmpos = stlmpos;

	return ret;  
}

// get_lgt_scw_seg
/*========================================================================================
 * function : get logistic result of scw_seg_fmm  **Jap wordseg only**
 * argu     : pwdict, the worddict
 *          : pir, the inner seg result structure
 * return   : 1 if success, <0 if failed.
 ========================================================================================*/
/*
   int get_lgt_scw_seg(scw_worddict_t * pwdict,scw_inner_t * pir)
   {
   scw_lemma_t *plm = NULL, *plm4=NULL;
   scw_lemma_t* plm3 = NULL;
   scw_property_t ppt;
   int len=0, phlen=0;
   int cnt, pos, bwcnt, cnt1;
   scw_lemma_t* plm2 = NULL;
   scw_lemma_t * ppm2 = NULL;
   u_int bpos = 0;
   char word[256];
   bool flag = false;
   int type = 0;
   int spcnt = 0, offset = 0, newoff = 0;

   for(int i = 0; i <(int)pir->m_ppseg_cnt; i++)
   {
   plm3 = pir->m_ppseg[i];
   if(plm3 == NULL)
   {
   continue;
   }
   if(IS_BIG_PHRASE(plm3->m_property))
   {
   phlen = 0;
   cnt = 0;
   pos = 0;
   bpos = 0;
   bwcnt = 0;
   spcnt = 0;
   offset = 0;
   newoff = 0;

   ppt.m_hprop = plm3->m_property.m_hprop;
   ppt.m_lprop = plm3->m_property.m_lprop;

   bpos = plm3->m_phinfo_bpos;
   cnt = pir->m_dynphinfo[bpos];

   if(pir->m_dynlm_cnt>=pir->m_dynlm_size){
   WARNING_LOG("error: m_dynlm_size not enough");
   return -1;
   }
   if(pir->m_dynph_curpos+MAX_EXTEND_PHINFO_SIZE>=pir->m_dynph_size){
   WARNING_LOG("error: m_dynph_size not enough");
   return -1;
   }

   plm=&(pir->m_dynlm[pir->m_dynlm_cnt]);
   plm->m_type    = LEMMA_TYPE_DPHRASE;
   plm->m_property.m_hprop = ppt.m_hprop;
   plm->m_property.m_lprop = ppt.m_lprop;
   plm->m_weight = 1;
   plm->m_subphinfo_bpos  = COMMON_NULL;
   plm->m_phinfo_bpos  = pir->m_dynph_curpos;
   plm->m_prelm_pos  = LEMMA_NULL;
   plm->m_utilinfo_pos  = COMMON_NULL;
   pir->m_dynph_curpos++;

   for(int j=0; j<cnt; j++){
   plm2= (scw_lemma_t*)pir->m_dynphinfo[bpos+j+1];
   len = get_lmstr(pwdict, pir, plm2, word, sizeof(word));
   if(IS_SPACE(plm2->m_property) || (len==4&&is_fourbit_point(word)))
   {
   continue;
   }
   phlen += len;

   type = plm2->m_type;
   switch(type){
case LEMMA_TYPE_SBASIC:
case LEMMA_TYPE_DBASIC:
pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm2;
bwcnt++;
break;
case LEMMA_TYPE_SPHRASE:
cnt1= pwdict->m_phinfo[plm2->m_phinfo_bpos];
for(int t=0;t<cnt1;t++){
	int lmpos = pwdict->m_phinfo[plm2->m_phinfo_bpos+t+1];
	plm4= &pwdict->m_lemmalist[lmpos];
	pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm4;
}
bwcnt+= cnt;
break;
case LEMMA_TYPE_DPHRASE:
cnt1= pir->m_dynphinfo[plm2->m_phinfo_bpos];
memcpy(pir->m_dynphinfo+pir->m_dynph_curpos, 
		pir->m_dynphinfo+plm2->m_phinfo_bpos+1, cnt1*sizeof(scw_lemma_t*));
pir->m_dynph_curpos += cnt1;
bwcnt += cnt1;
break;
}
}

plm->m_subphinfo_bpos = pir->m_dynph_curpos;
pir->m_dynph_curpos++;
offset=0;

type = plm3->m_type;    
switch(type){
	case LEMMA_TYPE_SBASIC:
	case LEMMA_TYPE_DBASIC:
		break;
	case LEMMA_TYPE_SPHRASE:
		if(plm3->m_subphinfo_bpos!=COMMON_NULL){
			cnt= pwdict->m_phinfo[plm3->m_subphinfo_bpos];
			for(int j=0;j<cnt;j++){
				newoff=offset+pwdict->m_phinfo[plm3->m_subphinfo_bpos+2*j+1];
				pos = pwdict->m_phinfo[plm3->m_subphinfo_bpos+2*j+2];
				ppm2= &pwdict->m_lemmalist[pos];
				pir->m_dynphinfo[pir->m_dynph_curpos++]=newoff;
				pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)ppm2;
				spcnt++;
			}
		}
		break;
	case LEMMA_TYPE_DPHRASE:

		if(plm3->m_subphinfo_bpos!=COMMON_NULL){
			cnt= pir->m_dynphinfo[plm3->m_subphinfo_bpos];
			for(int j=0;j<cnt;j++){
				newoff=offset+pir->m_dynphinfo[plm3->m_subphinfo_bpos+2*j+1];
				ppm2=(scw_lemma_t*)pir->m_dynphinfo[plm3->m_subphinfo_bpos+2*j+2];
				pir->m_dynphinfo[pir->m_dynph_curpos++]=newoff;
				pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)ppm2;
				spcnt++;
			}
		}
		break;
	default:
		return -1;
}
offset += get_lmoff_step(pwdict, pir, plm3);
if(spcnt>0)
{
	pir->m_dynphinfo[plm->m_subphinfo_bpos]= spcnt;
}
else
{
	plm->m_subphinfo_bpos=COMMON_NULL;
}

plm->m_length = phlen;
pir->m_dynphinfo[plm->m_phinfo_bpos] = bwcnt;
pir->m_ppseg[i] = &pir->m_dynlm[pir->m_dynlm_cnt];
pir->m_dynlm_cnt++;
flag = true;
}
}

if(flag)
{
	return 1;
}
else
{
	return 0;
}
}
*/

/*判断边界字符是不是特定的几类符号字符*/
/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static bool edge_asc(const char* str)
{
	if(str ==NULL)
	{
		WARNING_LOG("str is not available");
		return false;
	}
	u_int len = strlen(str);
	if(len > 1 || len==0){
		return false;
	}

	if( str[0] == '.'
			|| str[0] == '.'
			|| str[0] == '_'
			|| str[0] == '-'
			|| str[0] == '@'
			|| str[0] == '+'
	  ){
		return true;
	}
	return false;
}

/*判断字符串是不是全部都是数字串
 *
 * */
/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 */
static bool all_num(const char* str)
{
	if(str ==NULL)
	{
		WARNING_LOG("str is not available");
		return false;
	}

	u_int len = strlen(str);
	u_int i = 0 ;
	int ret = 0;

	while(i < len )
	{
		ret = is_my_num((char *)(str + i));
		if(ret == 0)
			return false;
		i += ret;
	}
	return true;
}
/*判断字符串是不是全部都是ascii串
 *
 * */
/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static bool all_asc(const char* str)
{
	if(str ==NULL)
	{
		WARNING_LOG("str is not available");
		return false;
	}

	u_int len = strlen(str);
	u_int i = 0 ;
	int ret = 0;

	while(i < len )
	{
		ret = is_my_ascii((char *)(str + i));
		if(ret != 0)
		{
			i += ret;
		}
		else
		{
			if(str[i] == '.' || str[i] == '_' || str[i] == '@')
			{
				i += 1;
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}


/*
 * check whether all crf results can be found from the dict
 * choosecrf = ture means found
 * @crf_res:	crf切分结果
 * @choosecrf:	crf切分结果是不是全是词典词。
 * */
static int check_crf_exist(scw_crf_out* crf_res, const char* inbuf,const int inlen, scw_worddict_t* pwdict, scw_inner_t* pir, bool& choosecrf)
{
	if(NULL==crf_res || NULL==inbuf)
	{
		LOG_ERROR("crf_res or inbuf is not available");
		return -1;
	}
	scw_lemma_t* plm = NULL;
	pir->m_ppseg_cnt = 0;
	u_int lemmapos = 0;

	scw_crf_term *pout = &crf_res->term_buf[0];

	int term_pos = 0;
	int term_len = 0;
	char term[MAX_WORD_LEN+10] = {} ;
	pir->m_tmpbuf[0] = 0;

	char sp[2]={27 , 0};//分隔符

	u_int buflen = 0;
	pir->m_tmpbuf_len = 0;

	u_int manword_cnt = 0;
	int man_flag = 0;
	pir->m_ppseg_cnt=0;

	u_int max_tmpbuf_len = pir->m_maxterm*30;

	pir->needremove = false;
	pir->m_protect_cnt = 0;
	//	if(max_tmpbuf_len-1 < tmpbuf_len)
	//	{
	//		memset(pir->m_protect, 0, sizeof(u_int)*(max_tmpbuf_len));
	//	}
	//	else memset(pir->m_protect, 0, sizeof(u_int)*(tmpbuf_len));


	for (u_int j = 0; j < pout->crftermcount; j++) 
	{
		term[0]=0;
		//memset(term,0,sizeof(term));
		term_pos = CRF_GET_TERM_POS(pout->crftermpos[j]);
		term_len = CRF_GET_TERM_LEN(pout->crftermpos[j]);
		strncpy(term, inbuf+term_pos, term_len);
		term[term_len] = 0;

		if(buflen + term_len + 9 > max_tmpbuf_len || pir->m_protect_cnt + 2 > pir->m_maxterm)
		{
			WARNING_LOG("scw_crf_check, buflen + 10 > max_tmpbuf_len || protect_cnt overflow ");
			goto failed;
		}
		man_flag = 0;
		if(manword_cnt < pir->m_mandictword_cnt)
		{
			if(pout->crftermpos[j] == pir->m_mandictword_prop[manword_cnt])
			{
				manword_cnt++;
				man_flag = 1;
			}
		}
		else if(manword_cnt > pir->m_mandictword_cnt)
		{
			WARNING_LOG("check_crf_exist:cnt > mandictword_cnt");
			goto failed;
		}

		if( (lemmapos =  scw_seek_lemma(pwdict,term,term_len)) == LEMMA_NULL && (!all_asc(term)) )
		{
			choosecrf = false;

			strncpy(pir->m_tmpbuf + buflen , term, term_len);
			buflen += term_len;
		}
		else
		{
			if(man_flag == 1) // is a mandictword, just copy to the buf
			{
				strncpy(pir->m_tmpbuf + buflen , term, term_len);
				buflen += term_len;
			}
			else
			{
				//pir->m_protect[buflen]=7;//就是个标记没啥别的；
				pir->m_protect[pir->m_protect_cnt]=buflen;
				pir->m_protect_cnt++;
				strncpy(pir->m_tmpbuf + buflen , sp, 1);
				buflen += 1;

				strncpy(pir->m_tmpbuf + buflen , term, term_len);
				buflen += term_len;

				//pir->m_protect[buflen]=7;
				pir->m_protect[pir->m_protect_cnt]=buflen;
				pir->m_protect_cnt++;
				strncpy(pir->m_tmpbuf + buflen , sp, 1);
				buflen += 1;
			}

			if(choosecrf)//将crf的切分结果copy到pir中。
			{
				if(all_asc(term))
				{
					int lmpos = -1;
					if((lmpos = create_dynword(pir, term )) < 0)
					{
						WARNING_LOG("error: create_dynword() failed");
						return -1;
					}
					pir->m_ppseg[pir->m_ppseg_cnt] = &(pir->m_dynlm[lmpos]);
					SET_ASCIIWORD(pir->m_ppseg[pir->m_ppseg_cnt]->m_property);
					if(all_num(term))
					{
						SET_ASCIINUM(pir->m_ppseg[pir->m_ppseg_cnt]->m_property);
					}
					if(man_flag == 1)
					{
						pir->m_ppseg_flag[pir->m_ppseg_cnt] = 1;
					}
					pir->m_ppseg_cnt++;
				}
				else
				{
					plm = pwdict->m_lemmalist + lemmapos;
					if(append_lemma(pwdict, pir,plm) < 0)
					{
						WARNING_LOG("error: append_lemma() failed ");
						return -1;
					}
				}
			}

		}
	}

	pir->m_tmpbuf[buflen] = 0 ;
	pir->m_tmpbuf_len = buflen;

	if(!choosecrf)
	{
		//reset_inner(pir);
		pir->m_ppseg_cnt = 0;
		pir->needremove=true;
	}

	return 0;

failed:
	pir->m_tmpbuf[0]=0;
	pir->m_tmpbuf_len = 0;
	pir->needremove=false;
	return -1;
}

/*
 * add newword to pir->pnewword;
 * @newwordbuf: newwordbuf string
 * @newwordbeg: the position of newword ( pir->m_ppseg )
 * @numoffset:	number of segments in newword
 * @return:		
 * */
static int scw_add_newword(scw_worddict_t* pwdict, scw_inner_t* pir, const char* newwordbuf,int newwordbeg, int numoffset, int weight, u_int type)
{
	if(NULL==pir->pnewword || NULL==newwordbuf)
	{
		LOG_ERROR("newword parameters are not available");
		return -1;
	}


	scw_newword_t* pnewword = pir->pnewword;

	u_int buflen = strlen(newwordbuf);

	//检查是否越界
	if(pnewword->newwordb_curpos + buflen + 1 > pnewword->newwordbsize)
	{
		WARNING_LOG("NEWWORD CURPOS OUT OF RANGE");
		return -1;
	}
	if(pnewword->newwordbuf[pnewword->newwordb_curpos]!=0)
	{
		WARNING_LOG("NEWWORD CURPOS OUT OF RANGE");
		return -1;
	}

	pnewword->newwordbtermpos[pnewword->newwordbtermcount] =  ((buflen)<<24 | pnewword->newwordb_curpos & 0x00FFFFFF);
	strncpy(pnewword->newwordbuf + pnewword->newwordb_curpos, newwordbuf,buflen);
	pnewword->newwordbtermoffsets[pnewword->newwordbtermcount*2] = newwordbeg ; //该新词在混排的位置
	pnewword->newwordbtermoffsets[pnewword->newwordbtermcount*2+1] = numoffset ;//此新词包含混排的个数

	pnewword->newwordb_curpos += buflen;
	pnewword->newwordbuf[pnewword->newwordb_curpos++] = '\t';
	pnewword->newwordbuf[pnewword->newwordb_curpos] = 0;


	pnewword->newwordneprop[pnewword->newwordbtermcount].iWeight = weight;	  

	scw_property_t tmpprop;
	INIT_PROPERTY(tmpprop);
	if(type == TYPE_NEWWORD)
	{
		SET_NEWWORD(tmpprop);
		snprintf(pnewword->newwordneprop[pnewword->newwordbtermcount].iNeName,
				sizeof(pnewword->newwordneprop[pnewword->newwordbtermcount].iNeName), "%s", "NEWWORD");
	}
	else if(type == TYPE_MULTITERM)
	{
		SET_MULTI_TERM(tmpprop);
		snprintf(pnewword->newwordneprop[pnewword->newwordbtermcount].iNeName,
				sizeof(pnewword->newwordneprop[pnewword->newwordbtermcount].iNeName), "%s", "MULTITERM");
	}
	else
	{

		SET_MULTI_TERM(tmpprop);
		SET_NEWWORD(tmpprop);
		snprintf(pnewword->newwordneprop[pnewword->newwordbtermcount].iNeName,
				sizeof(pnewword->newwordneprop[pnewword->newwordbtermcount].iNeName), "%s", "MULTI_NEWWORD");
	}

	memcpy(&pnewword->newwordbtermprop[pnewword->newwordbtermcount] ,&tmpprop,sizeof(scw_property_t));
	pnewword->newwordbtermcount++;

	return 1;
}

/*!
 * 对ascii的切分片段进行合并成一个phrase
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static int asc_merge(scw_worddict_t* pwdict, scw_inner_t* pir, const char* line)
{

	scw_lemma_t* plm = NULL;
	scw_lemma_t* plm_tmp = NULL;
	u_int i = 0 ;

	u_int ascbeg = 0; //m_ppseg中，需要合并的asc的起始pos；
	int ascnum = 0; // 在wordseg中需要合并的asc串的个数；
	int lmpos = 0;
	char word[MAX_WORD_LEN]={};

	bool edgeasc = false;
	int ploycount = 0;
	u_int lastokay = 0; //记录最后一个有效的asc的位置
	int manword_cnt = 0;

	while( i < pir->m_ppseg_cnt)
	{
		plm = pir->m_ppseg[i];

		if(plm != NULL)
		{
			//需要修改的英文结果在wordseg_fmm中只可能出现在动态的类型中，静态中的是词典中有的词组，不修改；
			//

			if(get_lmstr(pwdict, pir, plm, word, sizeof(word))<0)
				return 0;
			if(all_asc(word))
			{
				if(pir->m_ppseg_flag[i] == 1)
				{
					manword_cnt ++;
				}
				edgeasc = edge_asc(word);
				if(!edgeasc)
				{ 
					if(ascnum == 0)
					{
						ascbeg = i;
						ascnum = 1;
						lastokay = i;
					}
					else
					{
						lastokay = i;
						ascnum++;
					}
				}
			}
			else if(ascnum > 0)
			{
				if(ascnum == 1 || ascnum == manword_cnt)// single asc
				{
					ascnum = 0;
					manword_cnt = 0;
				}
				else
				{
					int j = 0;

					ploycount = i - ascbeg;
					if(edgeasc)
					{
						ploycount = lastokay - ascbeg + 1;
					}

					if(ploycount > 1)
					{
						//把dylm中的以 ph_beg_pos开始的以i-ascbeg个dynbic合并成一个dynphrase

						if( (lmpos =  poly_lemmas(pwdict, pir, ascbeg, ploycount )) < 0 )
						{
							WARNING_LOG("error:  create_dynword() failed");
							//	fprintf(stderr,"%s\n", line);
							return -1;
						}
						plm_tmp = &pir->m_dynlm[lmpos];
						SET_ASCIIWORD(plm_tmp->m_property);
						
						if(append_asciistr_subphrase_crf(pwdict,pir, plm_tmp) < 0)
						{
							WARNING_LOG("error: append_asciistr_subphrase() failed");
							return -1;
						}
						pir->m_ppseg[ascbeg] = &(pir->m_dynlm[lmpos]);

						for(j = 1; j < ploycount; j++)
						{
							pir->m_ppseg[j+ascbeg] = NULL;
						}
					}
					ascnum=0;
					manword_cnt = 0;
				}
			}
		}
		i++;
	}
	if((ascnum > 1) && (ascnum > manword_cnt))
	{
		int j = 0;
		ploycount = i - ascbeg;
		if(edgeasc)
		{
			ploycount = lastokay - ascbeg + 1;
		}


		if(ploycount > 1)
		{
			if( (lmpos =  poly_lemmas(pwdict, pir, ascbeg, ploycount)) < 0 )
			{
				WARNING_LOG("error:  create_dynword() failed");
				//	fprintf(stderr,"%s\n", line);
				return -1;
			}
			plm_tmp = &pir->m_dynlm[lmpos];
			SET_ASCIIWORD(plm_tmp->m_property);
						
			if(append_asciistr_subphrase_crf(pwdict,pir, plm_tmp) < 0)
			{
				WARNING_LOG("error: append_asciistr_subphrase() failed");
				return -1;
			}
			pir->m_ppseg[ascbeg] = &(pir->m_dynlm[lmpos]);

			for(j = 1; j < ploycount; j++)
			{
				pir->m_ppseg[j+ascbeg] = NULL;
			}
		}
		ascnum=0;

	} 

	return 0;
}

/*!
 * 根据dictmatch得到大粒度词
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static int generate_multiterm(scw_worddict_t* pwdict,scw_inner_t* pir, const char* inbuf, const int inlen)
{
	if(pwdict->multitermdict == NULL || pir->multiterm_pack==NULL)
	{
		LOG_ERROR("Parameter Error!");
		return -1;
	}


	if( dm_search(pwdict->multitermdict, pir->multiterm_pack, inbuf, inlen, DM_OUT_FMM)<0)
	{
		WARNING_LOG("Cannot do DictMatch");
		return -1;
	}

	//    char word[256]={};
	//    u_int multiterm_len = 0;
	//    printf("\n%s\n%d\n", inbuf, inlen);
	//    printf("DM Number: %d\n", pir->multiterm_pack->ppseg_cnt);
	//    
	//    for(u_int i = 0 ; i < pir->multiterm_pack->ppseg_cnt;i++)
	//    {
	//        multiterm_len = pir->multiterm_pack->ppseg[i]->len;
	//        memcpy(word,pir->multiterm_pack->ppseg[i]->pstr,multiterm_len);
	//        word[multiterm_len]=0;
	//        printf("[%d]%s\n",i, word);
	//    }
	//
	return pir->multiterm_pack->ppseg_cnt;
}

/*
 * 新词质量控制模块
 */
/*!
 * 
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static int check_newword(scw_worddict_t* pwdict,scw_inner_t* pir, const char* newwordbuf, const int buflen)
{
	if(buflen < 1)
		return 1;
	if(NULL==pwdict->newworddict)
	{
		LOG_ERROR("newworddict is not available");
		return 1;
	}
	//	return 1;
	char word[MAX_WORD_LEN]={};
	strncpy(word, newwordbuf, buflen);
	word[buflen] = 0;
	int ret = mysearch(pwdict->newworddict, word);
	return ret;
}

/*
 *获得multiterm的plm序号以及包含的pir的个数
 * */
/*!
 * 
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static int get_multi_pir(scw_worddict_t* pwdict, scw_inner_t* pir,int inlen, int pos, int len, int& pirpos, int hasnewword)
{
	int pirnum = 0;

	scw_lemma_t* plm = NULL;

	int plmslen = 0;
	int plmpos = 0;

	int offset = 0;

	pirpos = -1;

	for(u_int i = 0 ; i < pir->m_ppseg_cnt; i++)
	{
		plm = pir->m_ppseg[i];
		if(plm == NULL)
			continue;

		if(plmpos < pos)
		{
			plmpos += plm->m_length;

			//            offset +=  get_lmoff_step(pwdict,pir, plm);
			offset ++;
			continue;
		}
		else if(plmpos == pos)
		{
			if(pirpos < 0)
				pirpos = offset;

			plmslen += plm->m_length;
			pirnum++;

			if(plmslen > len)
				return 0;

			if(plmslen == len)
			{
				return pirnum;
			}
		}
		else if(plmpos > pos)
			return 0;

	}

	return pirnum;       

}

/*新词和大粒度词的生成过程；
 * @hasnewword:		是否有新词处理的流程是不一样的；
 * */
static int scw_combine(scw_worddict_t* pwdict, scw_inner_t* pir, const char* inbuf, const int inlen, int hasnewword)
{
	if(pwdict == NULL || pir==NULL)
	{
		LOG_ERROR("parameters error!");
		return -1;
	}
	if(pir->pnewword == NULL)
	{
		LOG_ERROR("NEWWROD BUFFER is NULL !");
		return -1;
	}

	char newwordbuf[MAX_WORD_LEN]={};

	int multinum = 0;
	int multiadded = 0;
	int multiterm_pos = 0;
	int multiterm_len = 0;
	char word[MAX_WORD_LEN]={};
	int dangerous_new = 0;
    int dangerous_multi = 0;

	/*---------multiterm--------*/
	multinum =  generate_multiterm(pwdict, pir, inbuf, inlen);
	if(multinum > 0)
	{
		multiterm_pos = pir->multiterm_pack->poff[0];
        dangerous_multi = multiterm_pos;
	}
	else if(multinum == -1)
	{
		WARNING_LOG("GET MULTI-TERM ERROR");
		return -1;
	}
    else
    {
        dangerous_multi = pir->m_maxterm*10;
    }

	//对于inbuf中inlen+1个位置；值为2的是对齐的边界，值为1的是wordseg的边界，值为-1的是crf的边界；

	//add newword
	int newwordlen = 0;

	if(hasnewword == 1)
	{
		int crfpos = 0;
		int crflen = 0;

		int crfalllen = 0;
		int plmalllen = 0;

		scw_crf_out *crf_res = pir->crf_res;
		scw_crf_term *pout = &crf_res->term_buf[0];
		u_int crftermcount =  pout->crftermcount;

		scw_lemma_t* plm = NULL;
		int plmnum = 0;
		int curroffset = 0;
		int newwordoffset = 0;

		bool share = true;

		u_int j = 0;
		for(u_int i = 0 ; i < crftermcount; i++)
		{
			crfpos = CRF_GET_TERM_POS(pout->crftermpos[i]);
			crflen = CRF_GET_TERM_LEN(pout->crftermpos[i]);

			crfalllen += crflen;

			plmnum = 0;
			newwordoffset = curroffset;
			if(crfalllen == plmalllen)
				share = true;
			while(j < pir->m_ppseg_cnt && crfalllen > plmalllen )
			{
				plm = pir->m_ppseg[j];
				if(plm == NULL)
				{
					j++;
					continue;
				}

				plmalllen += plm->m_length;
				plmnum++;

				if(plmalllen > crfalllen)
				{
					share = false;
				}
				else if(plmalllen == crfalllen  && plmnum > 1 && share)
				{
					strncpy(newwordbuf, inbuf+crfpos, crflen);
					newwordbuf[crflen] = 0;
					newwordlen = crflen;

					//                    printf("%s\n", newwordbuf);
					while((multiterm_pos <= crfpos) && (multiadded < multinum))
					{
						multiterm_len = pir->multiterm_pack->ppseg[multiadded]->len;
						memcpy(word,pir->multiterm_pack->ppseg[multiadded]->pstr,multiterm_len);
						word[multiterm_len]=0;

						int pirpos = 0;
						int pirnum =  get_multi_pir(pwdict, pir,inlen, multiterm_pos, multiterm_len, pirpos,hasnewword);

						if((multiterm_len == newwordlen) 
								&& (multiterm_pos == crfpos)
								&&  (check_newword(pwdict, pir , newwordbuf, newwordlen) < 1) 
								&& pirnum > 0)
						{

							if( scw_add_newword(pwdict, pir, word, pirpos,pirnum, 1000, TYPE_MULTITERM_NEWWORD) < 0)
							{
								WARNING_LOG("CANNOT APPEND MULTITERM NEWWORD");
								return -1;
							}
							newwordlen = 0;
						}
						else if(pirnum > 0)
						{
							if( scw_add_newword(pwdict, pir, word, pirpos, pirnum, 1000, TYPE_MULTITERM) < 0)
							{
								WARNING_LOG("CANNOT APPEND MULTITERM");
								return -1;
							}
						}
						dangerous_new = pirpos + pirnum;

						multiadded ++;
						if(multiadded < multinum)
						{
							multiterm_pos = pir->multiterm_pack->poff[multiadded];
                            dangerous_multi = multiterm_pos;
						}
                        else 
                        {
                            dangerous_multi = pir->m_maxterm*10;
                        }
					}


					//新词控制
                    
					if(check_newword(pwdict, pir , newwordbuf, newwordlen) < 1 
							&& (dangerous_new <= newwordoffset) 
                            && (dangerous_new+newwordlen <= dangerous_multi))
					{
						if( scw_add_newword(pwdict, pir, newwordbuf, newwordoffset,
									plmnum, 1000, TYPE_NEWWORD) < 0)
						{
							WARNING_LOG("ADD NEWWORD BUFFER ERROR!");
							return -1;
						}

					}
					newwordbuf[0] = 0;

				}
				else if (plmalllen == crfalllen)
					share = true;

				//                curroffset +=  get_lmoff_step(pwdict,pir, plm);
				curroffset += 1; 
				j++;
			}



		}
		crf_res=NULL;
		pout=NULL;
	}


	while(multiadded < multinum)
	{
		multiterm_len = pir->multiterm_pack->ppseg[multiadded]->len;
		memcpy(word,pir->multiterm_pack->ppseg[multiadded]->pstr,multiterm_len);
		word[multiterm_len]=0;

		int pirpos = 0;
		int pirnum =  get_multi_pir(pwdict, pir,inlen, multiterm_pos, multiterm_len, pirpos,hasnewword);
		if(pirnum > 0){
			if( scw_add_newword(pwdict, pir, word, pirpos, pirnum, 
						1000, TYPE_MULTITERM) < 0)
			{
				WARNING_LOG("CANNOT APPEND MULTITERM");
				return -1;
			}
		}
		multiadded ++;
		if(multiadded < multinum)
		{
			multiterm_pos = pir->multiterm_pack->poff[multiadded];
		}
	}

	return 0;

}
/*wordseg的后处理过程
 * */
static int fmm_postprocess(scw_worddict_t * pwdict,scw_inner_t * pir)
{
	if( g_scw_tn )
	{  
		if(tn_recg(pwdict, pir)<0){
			return -1;
		}
	}

	if(pwdict->m_wdtype & SCW_WD_CH)
	{
		if(pir->m_flag & SCW_OUT_HUMANNAME) 
		{
			if(humanname_recg(pwdict, pir) < 0){
				return -1;
			}
			if (fhumanname_recg(pwdict,pir) < 0){
				return -1;
			}
		}
	}

	if(pir->m_flag & SCW_OUT_BOOKNAME)
	{
		if(bookname_recg(pwdict, pir)<0){
			return -1;
		}
	}

	return 1;
}


/*static void print_crf_res(scw_crf_out* crf_res, const char *input_line)
{	
    int term_pos = 0;
    int term_len = 0;
    char term[MAX_WORD_LEN];

    scw_crf_term *pout = &crf_res->term_buf[0];
    for (u_int j = 0; j < pout->crftermcount; j++) {

        term_pos = CRF_GET_TERM_POS(pout->crftermpos[j]);
        term_len = CRF_GET_TERM_LEN(pout->crftermpos[j]);

        strncpy(term, input_line + term_pos , term_len);
        term[term_len] = 0;
        printf("%s|", term);
    }
    printf("\n");
}*/
//
/*static void print_pir(scw_worddict_t* pwdict, scw_inner_t* pir)
{
    scw_lemma_t* plm = NULL;
    char word[256]={};
    for(u_int i = 0 ; i < pir->m_ppseg_cnt; i++)
    {
        plm=pir->m_ppseg[i];
        if(plm == NULL)
        {
            continue;
        }
        get_lmstr(pwdict, pir, plm, word, sizeof(word));
        printf("%s|", word);
    }
    printf("\n");
    
}*/
//
//

/*remove tag from pir
 * */
/*!
 * 
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static int remove_tag(scw_worddict_t* pwdict, scw_inner_t* pir)
{

	u_int len = 0;

	char sp[2]={27, 0};//分隔符
	scw_lemma_t* plm = NULL;
	char word[MAX_WORD_LEN]={};
	u_int tagpos = 0;

	u_int protect_cnt = 0;

	for(u_int i=0; i<pir->m_ppseg_cnt; )
	{
		plm = pir->m_ppseg[i];
		if(plm == NULL)
		{
			i++;
			continue;
		}
		len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
		if(len < 0)
		{
			continue;
		}
		if( (strncmp(word, sp,1) == 0) && (protect_cnt<pir->m_protect_cnt ) )
		{
			if(pir->m_protect[protect_cnt] == tagpos)
			{
				pir->m_ppseg[i] = NULL;
				protect_cnt++;
			}
		}
		tagpos += len;


		i++;
	}

	pir->m_protect_cnt=0;
	return 0;
}


/*将wordseg切分结果中的ppseg[from]覆盖掉crf_wordseg 切分结果中的ppseg[beg]
 * */
static int merge_pir(scw_worddict_t* pwdict, scw_inner_t* pir, int from, int beg, int len)
{
	//     char word[256]={};



	if(len == 1)
	{
		return 0;
	}
	if(IS_ASCIIWORD(pir->tmp_ppseg[from]->m_property))
	{
		return 0;
	}

	//    get_lmstr(pwdict,pir,pir->tmp_ppseg[from],word,sizeof(word));
	//    printf("from:%s\n",word);
	//    
	//
	//    get_lmstr(pwdict,pir,pir->m_ppseg[beg],word,sizeof(word));
	//    printf("to:%s\n",word);

	pir->m_ppseg[beg] = pir->tmp_ppseg[from];
	for(int i =  1; i < len; i++)
	{
		//        get_lmstr(pwdict,pir,pir->m_ppseg[beg+i],word,sizeof(word));
		//         printf("remove:%s\n",word);

		pir->m_ppseg[beg+i] = NULL;   
	}

	// printf("---------\n");

	return 0;   
}


/*大粒度词和新词生成模块
 * */
static int scw_merge(scw_worddict_t* pwdict, scw_inner_t* pir, const char* inbuf)
{
	scw_lemma_t* plm = 0;

	bool merging = true;

	u_int tmp_len = 0;
	u_int seg_len = 0;

	u_int seg_curr = 0;
	u_int seg_beg = 0;

	bool lastasc = false; 
	int merge_num = 0;

	//  char word[256]={};

	for(u_int i = 0 ; i < pir->tmp_ppseg_cnt; i++)
	{
		plm = pir->tmp_ppseg[i];
		if(plm == NULL )
		{
			continue;
		}
		tmp_len += plm->m_length;


		//        int dclen = get_lmstr(pwdict, pir, plm, word, sizeof(word));
		//        printf("%s--%d--%d\n", word, dclen, plm->m_length);


		merge_num=0;

		if(seg_len == tmp_len)
		{
			merging = true;
		}
		while(seg_curr < pir->m_ppseg_cnt  && tmp_len > seg_len )
		{
			plm=pir->m_ppseg[seg_curr];

			if(plm == NULL)
			{
				seg_curr++;   
				continue;
			}

			//    int clen = get_lmstr(pwdict, pir, plm, word, sizeof(word));

			seg_len += plm->m_length;

			//       printf("%d,%d,%s,%d,%d\n", tmp_len, seg_len, word, clen, plm->m_length);
			if(seg_len == tmp_len)
			{
				if(merging )
				{
					merge_pir(pwdict, pir, i, seg_beg, seg_curr - seg_beg + 1 );
				}

				seg_beg = seg_curr+1;                
				merging = true;
				merge_num = 0;
			}
			else if (tmp_len < seg_len)
			{
				merging = false;
				seg_beg = seg_curr+1;
				merge_num = 0;
			}
			seg_curr++;
			merge_num ++;
			lastasc = false;
		}
	}
	if(tmp_len != seg_len && !lastasc)
	{
		WARNING_LOG("MERGING FAULT");
		//		fprintf(stderr, "%s\n", inbuf);
		reset_inner(pir);
		pir->tmp_ppseg_cnt=0;
		return -1;
	}

	pir->tmp_ppseg_cnt=0;
	return 0;

}

/*!
 * c保存wordseg的切分结果 
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
static void scw_ppseg_cpy(scw_inner_t* pir)
{
	memcpy(pir->tmp_ppseg, pir->m_ppseg, sizeof(scw_lemma_t*)*pir->m_ppseg_cnt);
	pir->tmp_ppseg_cnt = pir->m_ppseg_cnt;
}

/*wordseg函数，是个总结构在这里面
 * */
int scw_seg(scw_worddict_t * pwdict,scw_inner_t * pir, const char* inbuf,const int inlen, bool human_allow_blank)
{
	if(pwdict == NULL || pir==NULL )
	{
		LOG_ERROR("Parameters Error");
		return -1;
	}
	if(inbuf == NULL)
	{
		LOG_ERROR("inbuf is broken");
		return -1;
	}

	reset_inner(pir);
	if(inlen < 1)
	{
		//		WARNING_LOG("inbuf len < 1");
		return 0;
	}


	int ret = 0;
	pir->needremove=false;

	bool choosecrf = true;

	if(g_use_crf)//使用crf
	{
		if(scw_seg_fmm(pwdict, pir, inbuf,inlen, human_allow_blank)<0)
		{
			return -1;
		}
		/*printf("scw_seg_fmm over\n");
		for(int m=0; m<pir->m_ppseg_cnt;m++)
		{
			scw_lemma_t *tmp_lm = pir->m_ppseg[m];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",m);
			}
		}
		printf("\n");
		for(int n=0; n<pir->m_dynlm_cnt;n++)
		{
			scw_lemma_t *tmp_lm = &pir->m_dynlm[n];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",n);
			}
		}
		printf("\n");*/
		if(fmm_postprocess(pwdict, pir)<0)
		{
			return -1;
		}
		/*printf("fmm_postprocess\n");
		for(int m=0; m<pir->m_ppseg_cnt;m++)
		{
			scw_lemma_t *tmp_lm = pir->m_ppseg[m];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",m);
			}
		}
		printf("\n");
		for(int n=0; n<pir->m_dynlm_cnt;n++)
		{
			scw_lemma_t *tmp_lm = &pir->m_dynlm[n];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",n);
			}
		}
		printf("\n");*/

		scw_ppseg_cpy(pir);


		//检查crf_tag是否加载了crf_model;
		if(pir->crf_tag->get_tag_init_stat()!=TAG_INIT_FINISH)
		{
			pir->crf_tag->seg_init_by_model(pwdict->crf_model);
		}


		//调用crf进行切分；
		//ret = crf_segment_words_processed(pir->crf_tag, pir->crf_out, pir->crf_res, 
		//		pir->m_maxterm, inbuf, inlen, 1,0);
		ret = crf_segment_words_processed(pir,inbuf,inlen,1,0);

		if( ret < 0)
		{
			WARNING_LOG("CRF SEG ERROR");
			goto failed;
		}

		//printf("this is crf result\n");
		//print_crf_res(pir->crf_res, inbuf);//打印crf输出结果

		choosecrf = true;


		if(check_crf_exist(pir->crf_res,inbuf,inlen, pwdict, pir, choosecrf) < 0)
		{
			WARNING_LOG("check_crf_exist error");
			goto failed;
		}

		/*--如果crf所有切分结果都是词典词--*/
		if(choosecrf)
		{
			/*--为了保证新切词的粒度和原wordseg的保持一致，这一步尝试着做粒度上的合并--*/
			if( scw_merge(pwdict, pir, inbuf) < 0)
			{
				WARNING_LOG("MERGING ERROR!");
				goto failed;
			}

			//print_pir(pwdict, pir);
				
			/*for(u_int m=0; m<pir->m_ppseg_cnt;m++)
			{
				scw_lemma_t *tmp_lm = pir->m_ppseg[m];
				char word_tmp[256] = {0};
				if(tmp_lm != NULL)
				{
					get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
					printf("%s(%d)|", word_tmp,tmp_lm->m_type);
				}
				else
				{
					printf("%d=null|",m);
				}
			}
			printf("\n");
		for(int n=0; n<pir->m_dynlm_cnt;n++)
		{
			scw_lemma_t *tmp_lm = &pir->m_dynlm[n];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",n);
			}
		}
		printf("\n");*/
			/*--把crf切分后的ascii合并成一个动态的phrases--*/
			asc_merge(pwdict, pir,inbuf);

	/*for(u_int m=0; m<pir->m_ppseg_cnt;m++)
	{
		scw_lemma_t *tmp_lm = pir->m_ppseg[m];
		char word_tmp[256] = {0};
		if(tmp_lm != NULL)
		{
			get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
			printf("%s(%d)|", word_tmp,tmp_lm->m_type);
		}
		else
		{
			printf("%d=null|",m);
		}
	}
	printf("\n");
		for(int n=0; n<pir->m_dynlm_cnt;n++)
		{
			scw_lemma_t *tmp_lm = &pir->m_dynlm[n];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",n);
			}
		}
		printf("\n");*/
			/*--生成新词和大粒度词--*/
			if( scw_combine(pwdict,pir,inbuf, inlen, 0)<0)
			{
				WARNING_LOG("Nultiterm Generating error!");
				goto failed;
			}
		}
		else/*--如果CRF中存在新词，则先用wordseg_fmm对tmpbuf进行切分--*/
		{
			//调用wordseg_fmm进行切分。
			if( scw_seg_fmm( pwdict, pir,pir->m_tmpbuf, pir->m_tmpbuf_len ,human_allow_blank) < 0)
			{
				WARNING_LOG("WORDSEG TMPBUF SEG FAULT!");
				goto failed;
			}

			fmm_postprocess(pwdict, pir);

		//	print_pir(pwdict, pir);//这个函数打印任何pir情况下切分结果
/*	for(u_int m=0; m<pir->m_ppseg_cnt;m++)
	{
		scw_lemma_t *tmp_lm = pir->m_ppseg[m];
		char word_tmp[256] = {0};
		if(tmp_lm != NULL)
		{
			get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
			printf("%s(%d)|", word_tmp,tmp_lm->m_type);
		}
		else
		{
			printf("%d=null|",m);
		}
	}
	printf("\n");
		for(int n=0; n<pir->m_dynlm_cnt;n++)
		{
			scw_lemma_t *tmp_lm = &pir->m_dynlm[n];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",n);
			}
		}
		printf("\n");*/


			if( scw_merge(pwdict, pir,inbuf) < 0)
			{
				WARNING_LOG("MERGING FAULT!");
				goto failed;
			}
			asc_merge(pwdict, pir, inbuf);

	//		          print_pir(pwdict, pir);
	/*for(u_int m=0; m<pir->m_ppseg_cnt;m++)
	{
		scw_lemma_t *tmp_lm = pir->m_ppseg[m];
		char word_tmp[256] = {0};
		if(tmp_lm != NULL)
		{
			get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
			printf("%s(%d)|", word_tmp,tmp_lm->m_type);
		}
		else
		{
			printf("%d=null|",m);
		}
	}
	printf("\n");
		for(int n=0; n<pir->m_dynlm_cnt;n++)
		{
			scw_lemma_t *tmp_lm = &pir->m_dynlm[n];
			char word_tmp[256] = {0};
			if(tmp_lm != NULL)
			{
				get_lmstr(pwdict, pir, tmp_lm, word_tmp, sizeof(word_tmp));
				printf("%s(%d)|", word_tmp,tmp_lm->m_type);
			}
			else
			{
				printf("%d=null|",n);
			}
		}
		printf("\n");*/
			if(	scw_combine(pwdict,pir,inbuf, inlen,1)<0)
			{
				WARNING_LOG("Multiterm of Newword Generating error!");
				goto failed;
			}

		}
	}
	else
	{
		goto failed;
	}

	return 0;



failed:
	reset_inner(pir);
	if(scw_seg_fmm(pwdict, pir, inbuf,inlen, human_allow_blank)<0)
	{
		return -1;
	}
	fmm_postprocess(pwdict, pir);
	/*--生成新词和大粒度词--*/
	if( scw_combine(pwdict,pir,inbuf, inlen, 0)<0)
	{
		WARNING_LOG("Nultiterm Generating error!");
		return -1;
	}
	return 0;


}


//  segment
/*========================================================================================
 * function : segment the input buffer
 * argu     : pwdict, the worddict,
 *          : pir, the internal segment structure
 *          : inbuf, the input buffer
 * return   : 1 if success, <0 if failed.
 ========================================================================================*/
int scw_seg_fmm(scw_worddict_t * pwdict,scw_inner_t * pir, const char* inbuf,const int inlen, bool human_allow_blank)
{
	int pos = 0;
	int ret = 0;
	int nextpos = 0;
	int is_end = false;
	char dynword[MAX_LEMMA_LENGTH]={};
	/*
	 * 1 : not meet the first lemma
	 * 2 : after meet first lemma, can not meet the second
	 * 3 : after two lemma, not meet the next
	 */
	int state = 1;

	int fpos = 0;
	int spos = 0;
	int newspos = 0;
	int newfpos = 0;
	u_int flemma = COMMON_NULL;
	u_int slemma = COMMON_NULL;
	u_int ftlemma = COMMON_NULL;
	u_int stlemma = COMMON_NULL;
	u_int cur_lmpos = LEMMA_NULL;
	scw_lemma_t * fplm = NULL,*splm = NULL;

	//reset_inner(pir);

	while(!is_end)
	{  
		switch(state)
		{
			case 1:
				if(pos >= inlen)
				{
					is_end = true;
					break;
				}

				spos = pos;
				cur_lmpos = scw_get_fmm_word(pwdict,inbuf,inlen,spos);
				if(cur_lmpos != LEMMA_NULL)
				{
					slemma = cur_lmpos;
					splm = pwdict->m_lemmalist + cur_lmpos;
					if((flemma = splm->m_prelm_pos) == LEMMA_NULL)
					{
						state = 2;
					}
					else
					{
						fplm = pwdict->m_lemmalist + flemma;
						fpos = pos + fplm->m_length;
						state = 3;
					}
					break;
				}
				else//find a char not in entrance
				{
					if(spos <= pos)
					{
						is_end = 1;
						break;
					}//meet end

					memcpy(dynword,inbuf+pos,spos-pos);
					dynword[spos-pos] = 0;  
					flemma = create_dynword(pir,dynword);

					if((int)flemma < 0)
					{
						WARNING_LOG("error: create_dynword() failed");
						return -1;
					}

					if(append_lemma(pwdict,pir,flemma,SCW_DYNAMIC)<0)
					{
						WARNING_LOG("error: append_lemma() failed");
						return -1;
					}

					pos = spos;
					state = 1;
				}      
				break;

			case 2:
				if(IS_ASCIIWORD(splm->m_property))
				{
					// process ascii
					ret = proc_ascii(pwdict,pir,inbuf,inlen,pos,spos,splm,nextpos);

					if(ret < 0)
					{
						WARNING_LOG("error: proc_ascii() failed");
						return -1;
					}

					if(ret == 1)
					{
						// process ascii successfully
						state = 1;
						pos = nextpos;
						break;
					}
				}
				else if(IS_CNUMBER(splm->m_property))
				{
					ret = proc_cnum(pwdict,pir,inbuf,inlen,pos,spos,nextpos);

					if(ret < 0)
					{
						WARNING_LOG("error: proc_cnum() failed");
						return -1;
					}

					if(ret == 1)
					{
						state = 1;
						pos = nextpos;
						break;
					}
				}
				else 
				{
					if(pwdict->m_wdtype & SCW_WD_JP) 
					{
						if(IS_PAJMSTR(splm->m_property))
						{
							ret=proc_pajm(pwdict,pir,inbuf,inlen,pos,spos,nextpos);

							if(ret<0)
							{
								WARNING_LOG("error: proc_pajm() failed");
								return -1;
							}
							if(ret == 1)
							{
								state = 1;
								pos = nextpos;
								break;
							}
						}
					}
				}

				if(append_lemma(pwdict, pir,splm) < 0)
				{
					WARNING_LOG("error: append_lemma() failed");
					return -1;
				}
				if(IS_MANWORD(splm->m_property))
				{
					//fprintf(stderr,"this is a manword\n");
					/*if(pir->m_mandictword_cnt + 1 > pir->m_mandictword_cnt)
					{
						WARNING_LOG("error: mandictword add more");
						return -1;
					}
					pir->m_mandictword_prop[pir->m_mandictword_cnt]=splm->m_length<<24|pos;*/
					if(append_manword(pwdict, splm, pir, pos) < 0)
					{
						WARNING_LOG("error:mandictword add error");
						return -1;
					}
				}
				pos = spos;
				state = 1;
				break;

			case 3:
				// max match
				if(IS_ASCIIWORD(splm->m_property))
				{
					// process ascii
					ret = proc_ascii(pwdict,pir,inbuf,inlen,pos,spos,splm,nextpos);

					if(ret < 0)
					{
						WARNING_LOG("error: proc_ascii() failed");
						return -1;
					}

					if(ret == 1)
					{
						// process ascii successfully
						state = 1;
						pos = nextpos;
						break;
					}
				}
				else if(IS_CNUMBER(splm->m_property))
				{
					ret = proc_cnum(pwdict,pir,inbuf,inlen, pos, spos, nextpos);

					if(ret<0)
					{
						WARNING_LOG("error: proc_cnum() failed");
						return -1;
					}

					if(ret == 1)
					{
						state = 1;
						pos = nextpos;
						break;
					}
				}
				else 
				{
					if(pwdict->m_wdtype & SCW_WD_JP)
					{
						if(IS_PAJMSTR(splm->m_property))
						{
							ret = proc_pajm(pwdict,pir,inbuf,inlen,pos,spos,nextpos);

							if(ret < 0)
							{       
								WARNING_LOG("error: proc_pajm() failed");      
								return -1;
							}                               
							if(ret == 1)
							{
								state = 1;
								pos = nextpos;
								break;
							}
						}
					}                
				}

				newfpos = fpos;
				newspos = spos;
				ftlemma = flemma;
				stlemma = slemma;
				// 日文消歧策略：递归比较所有前缀
				if(pwdict->m_wdtype & SCW_WD_JP )
				{
					if(scw_backtrack(pwdict,inbuf,inlen,flemma,newfpos,slemma,newspos) == 0)// select slemma
					{
						if(ftlemma != LEMMA_NULL)
						{
							flemma = fplm->m_prelm_pos;
							if(flemma != LEMMA_NULL)
							{
								fplm = pwdict->m_lemmalist + flemma;
								fpos = spos + fplm->m_length - splm->m_length;
								slemma=stlemma;
								state = 3;
								break;
							}
							else
							{
								if(append_lemma(pwdict, pir,splm) < 0)
								{
									WARNING_LOG("error: append_lemma() failed");
									return -1;
								}
								pos = spos;
								state = 1;
							}
						}

						break;
					}
					else // select flemma
					{
						if(ftlemma != LEMMA_NULL)        
						{
							slemma = ftlemma;
							splm = pwdict->m_lemmalist + slemma;
							flemma = splm->m_prelm_pos;
							if(flemma != LEMMA_NULL)  
							{
								fplm = pwdict->m_lemmalist + flemma;
								spos = fpos;
								fpos = spos + fplm->m_length - splm->m_length;
								state = 3;
								break;
							}
							else
							{
								if(append_lemma(pwdict,pir,fplm) < 0)
								{
									WARNING_LOG("error: append_lemma() failed");
									return -1;
								}
								pos = fpos;
								state = 1;
							}
						}
					}
				}
				// 中文前缀消歧策略：只比较最长和次长前缀
				else
				{
					if(scw_backtrack(pwdict,inbuf,inlen,flemma,newfpos,slemma,newspos) == 0)// select slemma
					{
						if(append_lemma(pwdict, pir,splm) < 0)
						{
							WARNING_LOG("error: append_lemma() failed");
							return -1;
						}
						if(IS_MANWORD(splm->m_property))
						{
							//fprintf(stderr,"this is a manword\n");
							/*if(pir->m_mandictword_cnt + 1 > pir->m_mandictword_cnt)
							{
								WARNING_LOG("error: mandictword add more");
								return -1;
							}
							pir->m_mandictword_prop[pir->m_mandictword_cnt++]=splm->m_length<<24|pos;*/
							if(append_manword(pwdict, splm, pir, pos) < 0)
							{
								WARNING_LOG("error:mandict add");
								return -1;
							}
						}

						pos = spos;
						state = 1;

						if(slemma != LEMMA_NULL)
						{
							splm = pwdict->m_lemmalist + slemma;
							flemma = splm->m_prelm_pos;
							pos = spos;

							if(flemma != LEMMA_NULL)
							{
								fplm = pwdict->m_lemmalist + flemma;
								fpos = spos + fplm->m_length;
								spos = newspos;
								state = 3;
								break;
							}
							else
							{
								spos = newspos;
								state = 2;
								break;
							}
						}

						break;
					}
					else // select flemma
					{
						if(append_lemma(pwdict, pir,fplm) < 0)
						{
							WARNING_LOG("error: append_lemma() failed");
							return -1;
						}
						if(IS_MANWORD(fplm->m_property))
						{
						//	fprintf(stderr,"this is a manword\n");
							/*if(pir->m_mandictword_cnt + 1 > pir->m_mandictword_cnt)
							{
								WARNING_LOG("error: mandictword add more");
								return -1;
							}
							pir->m_mandictword_prop[pir->m_mandictword_cnt++]=fplm->m_length<<24|pos;*/
							if(append_manword(pwdict, fplm, pir, pos) < 0)
							{
								WARNING_LOG("error:manword add");
								return -1;
							}
						}

						pos = fpos;
						state = 1;

						if(flemma != LEMMA_NULL)        
						{
							slemma = flemma;
							splm = pwdict->m_lemmalist + slemma;
							flemma = splm->m_prelm_pos;
							if(flemma != LEMMA_NULL)  
							{
								fplm = pwdict->m_lemmalist + flemma;
								fpos = fpos + fplm->m_length;
								spos = newfpos;
								state = 3;
								break;
							}
							else
							{
								spos = newfpos;
								state = 2;
								break;
							}
						}
					}
				}
				break;
			default:
				break;
		}
	}

	// 动态数量词合并开关
	if( g_scw_tn )
	{  
		if(tn_recg(pwdict, pir)<0)
			return -1;
	}
	if(pir->needremove)
		remove_tag( pwdict,  pir);

	return 1;
}

//  scw_seg_pajm
/*========================================================================================
 * function : segment the input pajm buffer
 * argu     : pwdict, the worddict,
 *          : pir, the internal segment structure
 *          : inbuf, the input buffer
 * return   : 1 if success, <0 if failed.
 ========================================================================================*/
int scw_seg_pajm(scw_worddict_t * pwdict,scw_inner_t * pir,
		const char* inbuf,const int inlen)
{
	int pos = 0;
	int is_end = false;
	/*
	 * 1 : not meet the first lemma
	 * 2 : after meet first lemma, can not meet the second
	 * 3 : after two lemma, not meet the next
	 */
	int state = 1;

	u_int cur_lmpos = LEMMA_NULL;
	scw_lemma_t * fplm = NULL,*splm = NULL;
	u_int flemma = COMMON_NULL;
	int fpos = 0;
	int newfpos = 0;
	u_int slemma = COMMON_NULL;
	int spos = 0;
	int newspos = 0;

	while(!is_end)
	{  
		switch(state)
		{
			case 1:
				if(pos >= inlen)
				{
					is_end = true;
					break;
				}

				spos = pos;
				cur_lmpos = scw_get_fmm_word(pwdict,inbuf,inlen,spos);
				if(cur_lmpos != LEMMA_NULL)
				{
					slemma = cur_lmpos;
					splm = pwdict->m_lemmalist + cur_lmpos;
					if((flemma = splm->m_prelm_pos) == LEMMA_NULL)
					{
						state = 2;
					}
					else
					{
						fplm = pwdict->m_lemmalist + flemma;
						fpos = pos + fplm->m_length;
						state = 3;
					}
					break;
				}
				else//find a chr not in entrance
				{
					WARNING_LOG("error: find a chr not in entrance");
				}

				break;
			case 2:  
				if(append_lemma(pwdict, pir,splm) < 0)
				{
					WARNING_LOG("error: append_lemma");
					return -1;
				}
				pos = spos;
				state = 1;
				break;
			case 3:
				// max match    
				newfpos = fpos;
				newspos = spos;

				// select slemma
				if(scw_backtrack(pwdict,inbuf,inlen,flemma,newfpos,slemma,newspos) == 0)
				{
					if(append_lemma(pwdict, pir,splm) < 0)
					{
						WARNING_LOG("error: append_lemma");
						return -1;
					}

					pos = spos;
					state = 1;

					if(slemma != LEMMA_NULL)
					{
						splm = pwdict->m_lemmalist + slemma;
						flemma = splm->m_prelm_pos;
						pos = spos;

						if(flemma != LEMMA_NULL)
						{
							fplm = pwdict->m_lemmalist + flemma;
							fpos = spos + fplm->m_length;
							spos = newspos;
							state = 3;
							break;
						}
						else
						{
							spos = newspos;
							state = 2;
							break;
						}
					}

					break;
				}
				else // select flemma
				{
					if(append_lemma(pwdict, pir,fplm) < 0)
					{
						WARNING_LOG("error: append_lemma");
						return -1;
					}

					pos = fpos;
					state = 1;

					if(flemma != LEMMA_NULL)        
					{
						slemma = flemma;
						splm = pwdict->m_lemmalist + slemma;
						flemma = splm->m_prelm_pos;
						if(flemma != LEMMA_NULL)  
						{
							fplm = pwdict->m_lemmalist + flemma;
							fpos = fpos + fplm->m_length;
							spos = newfpos;
							state = 3;
							break;
						}
						else
						{
							spos = newfpos;
							state = 2;
							break;
						}
					}
				}
				break;
			default:
				break;
		}
	}

	return 1;
}

//  create_dynword
/*========================================================================================
 * function : create a dynamic basic word 
 * argu     : pout, the out struct
 *          : str, the word string
 * return   : the postion of the lemma in pout->m_dynlm if success, <0 if failed.
 * notice  : phrase created by this function do not have subphrase.
 ========================================================================================*/
int create_dynword(scw_inner_t* pir, char* str)
{  
	int len= strlen(str);

	if(pir->m_dynb_curpos+len+1 >= pir->m_dynb_size)
	{
		WARNING_LOG("error: m_dynb_curpos out of range");
		return -1;
	}

	if(pir->m_dynlm_cnt >= pir->m_dynlm_size)
	{
		WARNING_LOG("error: m_dynlm_cnt out of range");
		return -1;
	}

	scw_lemma_t* plm = &pir->m_dynlm[pir->m_dynlm_cnt];
	pir->m_dynlm_cnt++;

	plm->m_word_bpos = pir->m_dynb_curpos;
	memcpy(pir->m_dynbuf+pir->m_dynb_curpos, str, len+1);
	pir->m_dynb_curpos += len+1;

	plm->m_type     = LEMMA_TYPE_DBASIC;
	INIT_PROPERTY(plm->m_property);
	SET_TEMP(plm->m_property);
	plm->m_length  = len;
	plm->m_weight    = 1;
	plm->m_prelm_pos  = LEMMA_NULL;
	plm->m_utilinfo_pos  = COMMON_NULL;

	return pir->m_dynlm_cnt-1;
}


// get_lmoff_step
/*========================================================================================
 * function : calculate how many offset does a certain lemma cross, one step is one basic
 *       : word, so speak in another way, this function returns how many basic word 
 *       : does a lemma contain.
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 *          : plm, the lemma to be appended
 * return   : >=1, lemma_step, <0 failed.
 ========================================================================================*/
int get_lmoff_step(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm)
{
	int step = 0;

	if(plm==NULL){
		return 0;
	}

	int type=plm->m_type;
	switch(type)
	{
		case LEMMA_TYPE_SBASIC:
		case LEMMA_TYPE_DBASIC:
			step=1;
			break;
		case LEMMA_TYPE_SPHRASE:
			step=pwdict->m_phinfo[plm->m_phinfo_bpos];
			break;
		case LEMMA_TYPE_DPHRASE:
			step=pir->m_dynphinfo[plm->m_phinfo_bpos];
			break;
		default:
			return -1;
	}

	return step;

}


// <token-parse>
/*_get_token_str [called-by] get_lmstr().
 * get the string of token recursively.  
 * David Dai  2007.8.23
 */
int _get_token_str( scw_worddict_t* pwdict, 
		scw_inner_t* pir, 
		scw_lemma_t* plm, 
		char* buffer, u_int size, int& start )
{
	//assert(buffer && (size > 0) && (start >= 0));

	if(plm == NULL){
		return -1;
	}

	int type = plm->m_type;
	int cnt = 0, cnt2 =0;
	int i = 0;
	scw_lemma_t* plm2 = NULL;
	scw_lemma_t* plm3 = NULL;

	switch(type)
	{
		case LEMMA_TYPE_SBASIC:  
			memcpy(buffer+start, pwdict->m_wordbuf+plm->m_word_bpos, plm->m_length);
			start += plm->m_length;
			break;

		case LEMMA_TYPE_DBASIC:  
			memcpy(buffer+start, pir->m_dynbuf+plm->m_word_bpos, plm->m_length);
			start += plm->m_length;
			break;

			// 如果是短语，则递归获取
		case LEMMA_TYPE_SPHRASE:  
			{
				cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
				for(i=0; i<cnt; i++)
				{
					int pos = pwdict->m_phinfo[plm->m_phinfo_bpos+i+1];
					plm2= &pwdict->m_lemmalist[pos];

					_get_token_str(pwdict,pir,plm2,buffer,size,start);          
				}
			}
			break;

		case LEMMA_TYPE_DPHRASE:
			{
				cnt = pir->m_dynphinfo[plm->m_phinfo_bpos];
				for(i=0; i<cnt; i++)
				{
					plm2= (scw_lemma_t*)pir->m_dynphinfo[plm->m_phinfo_bpos+i+1];

					if(plm2->m_type == LEMMA_TYPE_SBASIC)
						memcpy(buffer+start, pwdict->m_wordbuf+plm2->m_word_bpos, plm2->m_length);
					else
						memcpy(buffer+start, pir->m_dynbuf+plm2->m_word_bpos, plm2->m_length);

					start += plm2->m_length;
				}
			}
			break;
		case LEMMA_TYPE_MAN:
			cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
			for(i = 0; i < cnt; i++)
			{
				plm2= &(pwdict->m_lemmalist[pwdict->m_phinfo[plm->m_phinfo_bpos + i + 1]]);
				switch(plm2->m_type)
				{
					case LEMMA_TYPE_SBASIC:
						memcpy(buffer+start, pwdict->m_wordbuf+plm2->m_word_bpos, plm2->m_length+1);
						start += plm2->m_length;
						break;
					case LEMMA_TYPE_DBASIC:
						memcpy(buffer+start, pwdict->m_dynbuf+plm2->m_word_bpos, plm2->m_length+1);
						start += plm2->m_length;
						break;
					case LEMMA_TYPE_SPHRASE:
						cnt2 = pwdict->m_phinfo[plm2->m_phinfo_bpos];
						for(int j=0;j<cnt2;j++)
						{
							plm3= &(pwdict->m_lemmalist[pwdict->m_phinfo[plm2->m_phinfo_bpos+j+1]]);
							if(plm3->m_type == LEMMA_TYPE_SBASIC)
							{
								memcpy(buffer+start, pwdict->m_wordbuf+plm3->m_word_bpos, 
										plm3->m_length+1);
								start += plm3->m_length;
							}
							else
							{
								return -1;
							}
						}
						break;
					case LEMMA_TYPE_DPHRASE:
						cnt2 = pwdict->m_dynphinfo[plm2->m_phinfo_bpos];
						for(int j=0;j<cnt2;j++)
						{
							plm3 = &(pwdict->m_lemmalist[pwdict->m_dynphinfo[plm2->m_phinfo_bpos + j +1]]);
							if(plm3->m_type == LEMMA_TYPE_DBASIC)
							{
								memcpy(buffer+start, pwdict->m_dynbuf+plm3->m_word_bpos,
										plm3->m_length+1);
								start += plm3->m_length;
							}
							else if(plm3->m_type == LEMMA_TYPE_SBASIC)
							{
								memcpy(buffer+start, pwdict->m_wordbuf+plm3->m_word_bpos,
										plm3->m_length+1);
								start += plm3->m_length;
							}
							else
							{
								return -1;
							}
						}
						break;
					default:
						return -1;
				}
			}
			break;
		default:
			return -1;
	}

	return start;
}
// </token-parse>

// get_lmstr
/*========================================================================================
 * function : get the a string of a lemma
 * argu     : pout, the output struct
 *          : plm, the lemma
 *          : dest, buffer the hold the lemma string
 *          : destlen, sizeof dest, it must be larger than 256;
 ========================================================================================*/
int get_lmstr(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, char* dest, u_int destlen)
{  
	int cnt=0,cnt2=0;
	int i =0;
	int pos=0;
	int type=0;
	int ret=0;
	int dpos=0;
	scw_lemma_t* plm2=NULL;  
	scw_lemma_t* plm3=NULL;  

	if( destlen <= 0 )
	{
		WARNING_LOG("error: destlen illegal");
		return -1;
	}
	dest[0]=0;

	if(plm==NULL){
		return -1;
	}

	type = plm->m_type;
	switch(type)
	{
		case LEMMA_TYPE_SBASIC:
			ret = plm->m_length;
			memcpy(dest, pwdict->m_wordbuf+plm->m_word_bpos, plm->m_length+1);
			dpos += plm->m_length;
			break;

		case LEMMA_TYPE_DBASIC:
			ret = plm->m_length;
			memcpy(dest, pir->m_dynbuf+plm->m_word_bpos, plm->m_length+1);
			dpos += plm->m_length;
			break;

		case LEMMA_TYPE_SPHRASE:
			cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
			for(i=0;i<cnt;i++)
			{
				pos = pwdict->m_phinfo[plm->m_phinfo_bpos+i+1];
				plm2= &pwdict->m_lemmalist[pos];

				//if(dpos+plm2->m_length>=destlen)
				//  return -1;

				//! 如果这里还是短语的话下面得代码就不对了，已经修改此bug！
				/* 
				   <bug>
				   memcpy(dest+dpos, pwdict->m_wordbuf+plm2->m_word_bpos, plm2->m_length);
				   dpos+=plm2->m_length;
				   </bug> 
				   */

				// 这里需要递归获取短语的组成，防止出现超出既定层级的短语嵌套情况。
				_get_token_str(pwdict,pir,plm2,dest,destlen,dpos);
			}
			dest[dpos]=0;
			ret = dpos;
			break;

		case LEMMA_TYPE_DPHRASE:
			cnt = pir->m_dynphinfo[plm->m_phinfo_bpos];
			for(i=0;i<cnt;i++)
			{
				plm2= (scw_lemma_t*)pir->m_dynphinfo[plm->m_phinfo_bpos+i+1];

				//if(dpos+plm2->m_length>=destlen)
				//  return -1;

				if(plm2->m_type==LEMMA_TYPE_SBASIC)
				{
					memcpy(dest+dpos, pwdict->m_wordbuf+plm2->m_word_bpos, plm2->m_length);
					dpos+=plm2->m_length;
				}
				else if( plm2->m_type == LEMMA_TYPE_SPHRASE)
				{
					_get_token_str(pwdict,pir,plm2,dest,destlen,dpos);
				}
				else if( plm2->m_type == LEMMA_TYPE_DBASIC)
				{
					memcpy(dest+dpos, pir->m_dynbuf+plm2->m_word_bpos, plm2->m_length);
					dpos+=plm2->m_length;
				}
				else
				{
					_get_token_str(pwdict,pir,plm2,dest,destlen,dpos);
				}
			}
			dest[dpos]=0;
			ret=dpos;
			break;
		case LEMMA_TYPE_MAN:
			cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
			for(i = 0; i < cnt; i++)
			{
				plm2= &(pwdict->m_lemmalist[pwdict->m_phinfo[plm->m_phinfo_bpos + i + 1]]);
				switch(plm2->m_type)
				{
					case LEMMA_TYPE_SBASIC:
						memcpy(dest+dpos, pwdict->m_wordbuf+plm2->m_word_bpos, plm2->m_length+1);
						dpos += plm2->m_length;
						break;
					case LEMMA_TYPE_DBASIC:
						memcpy(dest+dpos, pwdict->m_dynbuf+plm2->m_word_bpos, plm2->m_length+1);
						dpos += plm2->m_length;
						break;
					case LEMMA_TYPE_SPHRASE:
						cnt2 = pwdict->m_phinfo[plm2->m_phinfo_bpos];
						for(int j=0;j<cnt2;j++)
						{
							plm3= &(pwdict->m_lemmalist[pwdict->m_phinfo[plm2->m_phinfo_bpos+j+1]]);
							if(plm3->m_type == LEMMA_TYPE_SBASIC)
							{
								memcpy(dest+dpos, pwdict->m_wordbuf+plm3->m_word_bpos, 
										plm3->m_length+1);
								dpos += plm3->m_length;
							}
							else
							{
								return -1;
							}
						}
						break;
					case LEMMA_TYPE_DPHRASE:
						cnt2 = pwdict->m_dynphinfo[plm2->m_phinfo_bpos];
						for(int j=0;j<cnt2;j++)
						{
							plm3 = &(pwdict->m_lemmalist[pwdict->m_dynphinfo[plm2->m_phinfo_bpos + j +1]]);
							if(plm3->m_type == LEMMA_TYPE_DBASIC)
							{
								memcpy(dest+dpos, pwdict->m_dynbuf+plm3->m_word_bpos,
										plm3->m_length+1);
								dpos += plm3->m_length;
							}
							else if(plm3->m_type == LEMMA_TYPE_SBASIC)
							{
								memcpy(dest+dpos, pwdict->m_wordbuf+plm3->m_word_bpos,
										plm3->m_length+1);
								dpos += plm3->m_length;
							}
							else
							{
								return -1;
							}
						}
						break;
					default:
						return -1;
				}
			}
			dest[dpos]=0;
			ret=dpos;
			break;
		default:
			WARNING_LOG("error: do not define this type in scwdef.h");
			return -1;
	}
	return dpos;
}



//  humanname_recg
/*========================================================================================
 * function : humanname recgnition process on the internal result.
 * argu     : pdict, the dictionary
 *       : pout, the output struct
 ========================================================================================*/
int judge_jpname(scw_worddict_t * pwdict,scw_inner_t* pir,u_int bpos,int lmcnt, bool human_allow_blank)
{
	float namepower = 0.0;
	float condition = 0.0;
	int len = 0;
	int pos = 0;
	int pos1 = 0;
	char namestr[256]={};
	char namestr1[256]={};
	scw_lemma_t * tmplm = NULL;
	scw_lemma_t * tmplm2 = NULL;
	int lmpos;
	int i = lmcnt-1;
	char buffer[256]={};

#ifdef JPHUMAN_ACROSS_BLANK_STRICT
	int loc_count = 0;
	int noun_count = 0;
	int suf_count = 0;
	float cond_incre = 0.0;
#endif

	int * ppb = NULL;
	scw_lemma_t * plm = NULL;
	int have_space = 0;
	int space_cnt = 0;
	bool have_jm = false;

	if(lmcnt <= 1 || lmcnt > 10){
		return 0;
	}

	if(lmcnt == 2 
			&& (IS_PJM(pir->m_ppseg[bpos]->m_property) 
				|| IS_PJM(pir->m_ppseg[bpos+1]->m_property)))
	{
		return 0;
	}
	while(i >= 0)
	{
		tmplm = pir->m_ppseg[bpos+i-1];
		if(tmplm !=NULL ){ //MYMY
			break;
		}
		i--;
	}

	if(i >= 0 && !IS_PJM(tmplm->m_property) 
			&& IS_PJM(pir->m_ppseg[bpos+lmcnt-1]->m_property))
	{
		return 0;
	}

	for(i = 0;i < lmcnt;i++)
	{
		plm = pir->m_ppseg[i+bpos];
		if(plm == NULL){
			continue;
		}
		if(IS_SPACE(plm->m_property)){
			have_space = 1;
			space_cnt++;
			len = get_lmstr(pwdict,pir,plm,namestr+pos, sizeof(namestr)-pos);
			if(len < 0)
			{
				return 0;
			}
			pos += len;
			continue;
		}
		if(IS_PAJMSTR(plm->m_property))
		{
			have_jm = true;
		}
		len = get_lmstr(pwdict,pir,plm,namestr1+pos1, sizeof(namestr1)-pos1);
		if(len < 0)
		{
			return 0;
		}
		pos1 += len;
		len = get_lmstr(pwdict,pir,plm,namestr+pos, sizeof(namestr)-pos);
		if(len < 0)
		{
			return 0;
		}
		pos += len;

		if((ppb = scw_get_nameprob(pwdict,plm,DATA_JNAME)) == NULL){
			return 0;
		}  

		if(i == 0){
			namepower += ppb[NAME_BGN];
		}
		else if(i == lmcnt-1){
			namepower += ppb[NAME_END]; 
		}
		else{
			namepower += ppb[NAME_MID];
		}

		condition += ppb[NAME_OTH];

#ifdef JPHUMAN_ACROSS_BLANK_STRICT
		if(IS_LOC(plm->m_property) || IS_ORGNAME(plm->m_property))
		{
			cond_incre += (float)3.8*plm->m_length/2;
			loc_count++;
		}
		if(IS_NOUN(plm->m_property))
		{
			cond_incre += (float)2.25*plm->m_length/2;
			noun_count++;
		}
		if(IS_SURNAME(plm->m_property))
		{
			suf_count++;
		}
#endif
	}

#ifdef JPHUMAN_ACROSS_BLANK_STRICT
	if(have_space)
	{
		condition += cond_incre;
	}
	else
	{
		//condition += (float)cond_incre*3.0/5.0;
	}

	if(have_space && lmcnt > (2*space_cnt+1))
	{
		return 0;
	}

	if(have_space && lmcnt==3 && IS_ONEWORD(pir->m_ppseg[bpos]->m_property) &&
			IS_ONEWORD(pir->m_ppseg[bpos+2]->m_property))
	{
		return 0;
	}
	if(have_space && suf_count > 1)
	{
		return 0;
	}
#endif

	if(!have_space || human_allow_blank)
	{
		condition = condition * 1.2;
	}
	if(have_space && (u_int)(lmpos = scw_seek_lemma(pwdict,namestr1,pos1)) != LEMMA_NULL)
	{
		if((tmplm2 = &(pwdict->m_lemmalist[lmpos])) != NULL)
		{
			if(IS_HUMAN_NAME(tmplm2->m_property))
			{
				if(human_allow_blank)
				{
					return 2;
				}
				else
				{
					for(i = 0;i < lmcnt;i++)
					{
						plm = pir->m_ppseg[i+bpos];
						if(plm == NULL){
							continue;
						}
						len = get_lmstr(pwdict,pir,plm,namestr,sizeof(namestr));
						write_prop_to_str(plm->m_property, buffer, pwdict->m_wdtype);
						if(IS_PHRASE(plm->m_property))
						{
							if((lmpos = create_dphrase(pwdict, pir, i+bpos, 1)) < 0)
							{
								WARNING_LOG("error: create_dphrase() failed");
								return -1;
							}
						}
						else
						{
							if((lmpos = create_dynword(pir, namestr)) < 0)
							{
								WARNING_LOG("error: create_dynword() failed");
								return -1;  
							}
						}
						SET_JPNAME_BLANK(pir->m_dynlm[lmpos].m_property);
						set_prop_by_str(buffer, pir->m_dynlm[lmpos].m_property, pwdict->m_wdtype);
						pir->m_ppseg[i+bpos] = &(pir->m_dynlm[lmpos]);
					}
					return 0;
				}
			}
		}
	}

	if(namepower < condition){
		return 0;
	}

	if(have_space && !human_allow_blank)
	{
		for(i = 0;i < lmcnt;i++)
		{
			plm = pir->m_ppseg[i+bpos];
			if(plm == NULL){
				continue;
			}
			len = get_lmstr(pwdict,pir,plm,namestr,sizeof(namestr));
			write_prop_to_str(plm->m_property, buffer, pwdict->m_wdtype);
			if(IS_PHRASE(plm->m_property))
			{
				if((lmpos = create_dphrase(pwdict, pir, i+bpos, 1)) < 0)
				{
					WARNING_LOG("error: create_dphrase() failed");
					return -1;
				}
			}
			else
			{
				if((lmpos = create_dynword(pir, namestr)) < 0)
				{
					WARNING_LOG("error: create_dynword() failed");
					return -1;
				}
			}
			SET_JPNAME_BLANK(pir->m_dynlm[lmpos].m_property);
			set_prop_by_str(buffer, pir->m_dynlm[lmpos].m_property, pwdict->m_wdtype);
			pir->m_ppseg[i+bpos] = &(pir->m_dynlm[lmpos]);
		}
		return 0;
	}

	if(have_space)
	{
		return 2;
	}
	return 1;
}

/*!
 * 
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
int jphumanname_recg(scw_worddict_t * pwdict,scw_inner_t* pir, bool human_allow_blank)
{
	scw_lemma_t * plm = NULL;
	u_int i = 0;
	int j = 0;
	int k = 0;
	int m = 0;
	int step = 0;
	int pos = 0;
	int spacecnt = 0;
	int ret = 0;

	while(i<pir->m_ppseg_cnt)
	{
		plm = pir->m_ppseg[i];
		if(plm == NULL)
		{
			i++;
			continue;
		}

		step = 1;
		j = 1;
		k = 1;
		spacecnt = 0 ;

		if(IS_SURNAME(plm->m_property))
		{
			while(i+j<pir->m_ppseg_cnt)
			{
				plm = pir->m_ppseg[i+j];
				if(plm==NULL)
				{
					j++;
					continue;
				}

				if(IS_SPACE(plm->m_property))
				{
					j++;
					spacecnt++;
					continue;
				}

				if(spacecnt > 2)
				{
					break;
				}

				if(IS_NAMEMID(plm->m_property))
				{
					j++;
					k++;
					if(IS_NAMEEND(plm->m_property) || ( k == 2 && IS_GIVENNAME(plm->m_property) && !IS_ONEWORD(plm->m_property)))
					{
						step = j;
					}
				}
				else if(IS_NAMEEND(plm->m_property) || ( k == 1 && IS_GIVENNAME(plm->m_property) && !IS_ONEWORD(plm->m_property)))
				{
					j++;
					k++;
					step = j;
					break;
				}
				else{
					break;
				}
			}

			if(step > 1)
			{
				if(spacecnt > 0)
				{
					//带空格的人名识别，姓前名后不应该有非空格及其他分隔符及非人名强后缀
					if(i > 0)
					{
						m = i-1;
						while(m >= 0)
						{
							if(pir->m_ppseg[m] != NULL)
							{
								break;
							}
							m--;
						}

						if(m>=0 && (!IS_SPACE(pir->m_ppseg[m]->m_property)) &&
								(!IS_MARK(pir->m_ppseg[m]->m_property)))
						{
							step = 1;
							i += step;
							continue;
						}
					}

					if((i+step < pir->m_ppseg_cnt) && 
							(!IS_SPACE(pir->m_ppseg[i+step]->m_property)) &&
							(!IS_MARK(pir->m_ppseg[i+step]->m_property)) && 
							(!IS_NAME_SUFFIX(pir->m_ppseg[i+step]->m_property)) )
						//加入人名强后缀（如sama、san）特征
					{
						step = 1;
						i += step;
						continue;
					}
				}
				if((ret=judge_jpname(pwdict,pir,i,step, human_allow_blank))>0)
				{
					pos = poly_lemmas(pwdict, pir, i, step, 1);

					if(pos==-1)
					{
						WARNING_LOG("error: poly_lemmas() failed");
						return -1;
					}
					if(pos!= -2)
					{
						SET_HUMAN_NAME(pir->m_dynlm[pos].m_property);
					}
					if(ret==2)
						SET_BIG_PHRASE(pir->m_dynlm[pos].m_property);
					i += step;
					continue;
				}
			}
			step = 1;
		}

		i+=step;
	}

	return 0;
}

/*
   int humanname_recg(scw_worddict_t* pwdict, scw_inner_t* pir)
   {  
   int step=0;
   int pos;
   u_int i;
   scw_lemma_t* plm;

   for( i=0; i<pir->m_ppseg_cnt; )
   {
   plm = pir->m_ppseg[i];
   if(plm == NULL)
   {
   i++;
   continue;
   }
   step = 1;

// if it's a two word phrase
if( IS_2WHPREFIX(plm->m_property) ) 
{
if( canbe_long_name(pwdict, pir, i) )
step = 2;
else
step = 1;
}
// 当前词为姓，获取姓氏左边界
else if( is_real_surname(pwdict, pir, i) )
{
if(is_3word_cname(pwdict, pir, i) == 1) // 三个连续单字的情况
step = 3;
else if(is_2word_cname(pwdict, pir, i) == 1) // 两个连续切分片段
step = 2;
else
step = 1;
}
else
{
step = 1;
}

if(step > 1)
{
pos = poly_lemmas(pwdict, pir, i, step);

if(pos==-1)
{
WARNING_LOG("error: poly_lemmas() failed");
return -1;
}
if(pos!= -2)
{
SET_HUMAN_NAME(pir->m_dynlm[pos].m_property);
SET_CH_NAME(pir->m_dynlm[pos].m_property);
}
}

i += step;
}

return 0;
}
*/

// 
/*========================================================================================
 * function : judge if the lmlist is a foreign name.
 * argu     : pdict, the dictionary
 *                      : pir, bpos lmcnt
 *                      
 *========================================================================================*/
/*
   int judge_fname(scw_worddict_t * pwdict,scw_inner_t* pir,u_int bpos,int lmcnt)
   {
   int namepower = 0;
   int condition = 0;
   int condition1 = 0,condition2 = 0;
   int * ppb = NULL;
   scw_lemma_t * plm = NULL,* pblm = NULL;
   char namestr[MAX_WORD_LEN];
   int i = 0,len = 0;
   int pos = 0;
   int lmpos = 0;
   Sdict_snode   snode;
   Sdict_search * rdict = pwdict->pdict_rsn_rule;

   int state = 0;
   int ret = 0;
   int backlmcnt = 0;
   int backlmpos = 0;
   int meet_end = 0;
   int meet_bgn = 0;
   int c = 0;
   int unvalid = 1;

   if(lmcnt <= 1)
   {
   return 0;
   }

//@@add test
int m_nonull = 0;
for(int m = 0; m < lmcnt; m ++)
{
if(pir->m_ppseg[m+bpos] == NULL)
{
continue;
}
m_nonull ++;
}
//@@end test
if(m_nonull == 2
&& IS_ONEWORD(pir->m_ppseg[bpos]->m_property)
)
{

for(int m = 1; m < lmcnt; m ++)
{
if(pir->m_ppseg[m+bpos] == NULL)
continue;
if(IS_ONEWORD(pir->m_ppseg[bpos+m]->m_property)
&& !IS_2FNAMEEND(pir->m_ppseg[bpos+m]->m_property))
{
return 0;
}
}
}

pblm = pir->m_ppseg[bpos];
len += pblm->m_length;

for(i = 1;i < lmcnt;i++)
{
plm = pir->m_ppseg[i+bpos];
if(plm == NULL)
{
continue;
}

len += plm->m_length;

if(plm != pblm)
{
	unvalid = 0;
}
}


if(unvalid || len > MAX_FNAME_LEN || len >= (int)MAX_LEMMA_LENGTH)
{
	return 0;
}

len = 0;

for(i = 0;i < lmcnt;i++)
{
	plm = pir->m_ppseg[i+bpos];
	if(plm == NULL)
	{
		continue;
	}

	len = get_lmstr(pwdict,pir,plm,namestr+pos, sizeof(namestr)-pos);
	pos += len;

	if((ppb = scw_get_nameprob(pwdict,plm,DATA_FNAME)) == NULL)
	{
		return 0;
	}

	if(i == 0)
	{
		namepower += ppb[NAME_BGN];
	}
	else if(i == lmcnt-1)
	{
		namepower += ppb[NAME_END];
	}
	else
	{
		namepower += ppb[NAME_MID];
		if(IS_FNAMEEND(plm->m_property))
		{
			if(c == 0)
			{
				backlmcnt = i+1;
				backlmpos = pos;
			}
			meet_end = 1;
			c++;
		}
		if(IS_FNAMEBGN(plm->m_property))
		{
			meet_bgn = 1;
		}
	}

	condition += ppb[NAME_OTH];  
}

creat_sign_fs64(namestr,pos,&(snode.sign1),&(snode.sign2));  
if(ds_op1(rdict, &snode, SEEK) == 1)
	return 0;

if(namepower < condition)
{
	return 0;
}

//  state: 0,find bgn ,not find end;
//  state: 1,find end, not find bgn;

condition1 = 0;
if(meet_end)
{  
	for(i = 0;i<lmcnt;i++)
	{   
		plm = pir->m_ppseg[i+bpos];
		if(plm == NULL)
		{
			continue;
		}

		if((ppb = scw_get_nameprob(pwdict,plm,DATA_FNAME)) == NULL)
		{
			return 0;
		}

		if(i == 0)
		{
			condition1 += ppb[NAME_BGN];
			continue;
		}

		if(state == 0)
		{
			if(IS_FNAMEEND(plm->m_property))
			{
				state = 1;
				condition1 += ppb[NAME_END];
				continue;
			}
			else
			{
				condition1 += ppb[NAME_MID];
			}
		}
		else
		{  
			if(IS_FNAMEBGN(plm->m_property) && i<lmcnt-1)
			{
				state = 0;
				condition1 += ppb[NAME_BGN];
				continue;
			}
			else
			{
				condition1 += ppb[NAME_OTH];  
			}
		}
	}
}
else
{
	condition1 = namepower;
}

condition2 = 0;
state = 1;
if(meet_bgn)
{  
	for(i = lmcnt-1;i>=0;i--)
	{
		plm = pir->m_ppseg[i+bpos];
		if(plm == NULL)
		{
			continue;
		}

		if((ppb = scw_get_nameprob(pwdict,plm,DATA_FNAME)) == NULL)
		{
			return 0;
		}

		if(i == lmcnt-1)
		{
			condition2 += ppb[NAME_END];
			continue;
		}

		if(state == 1)
		{
			if(IS_FNAMEBGN(plm->m_property))
			{
				state = 0;
				condition2 += ppb[NAME_BGN];
			}
			else
			{
				condition2 += ppb[NAME_MID];
			}
		}
		else
		{
			if(IS_FNAMEEND(plm->m_property) && i > 0)
			{
				state = 1;
				condition2 += ppb[NAME_END];
			}
			else
			{
				condition2 += ppb[NAME_OTH];
			}
		}
	}
}
else{
	condition2 = namepower;
}

if(namepower < condition1 || namepower < condition2)
{
	if(condition1 > condition2)
	{
		ret = backlmcnt;
		namestr[backlmpos] = 0;
		creat_sign_fs64(namestr,backlmpos,&(snode.sign1),&(snode.sign2));
		if(ds_op1(rdict, &snode, SEEK) == 1)
			return 0; 
	}
	else
	{
		ret = 1;
	}
}
else
{
	ret = lmcnt;
}

if(ret > 1)
{
	if((lmpos = create_dynword(pir,namestr)) < 0)
	{
		WARNING_LOG("error: create_dynword() failed");
		return -1;
	}
	pir->m_ppseg[bpos]=&pir->m_dynlm[lmpos];
	// SET_HUMAN_NAME(pir->m_ppseg[bpos]->m_property);
	SET_FMNAME(pir->m_ppseg[bpos]->m_property);
	memset(pir->m_ppseg+bpos+1,0,sizeof(scw_lemma_t *)*(ret-1));
}

return ret;
}
*/

/*
   int fhumanname_recg(scw_worddict_t* pwdict, scw_inner_t* pir)
   {
//  char* where="fhumanname_recg";
int step = 0;
u_int i = 0,j = 0;
scw_lemma_t* plm = NULL;
int ret = 0;

for(i=0;i< pir->m_ppseg_cnt; )
{
plm = pir->m_ppseg[i];
if(plm == NULL)
{
i++;                        
continue;                
}

step = 1;
j = 1;

if(IS_FNAMEBGN(plm->m_property))
{
while(i+j < pir->m_ppseg_cnt)
{
plm = pir->m_ppseg[i+j];
if(plm == NULL)
{
j++;
continue;
}

if(IS_FNAMEMID(plm->m_property))
{
j++;
if(IS_FNAMEEND(plm->m_property))
{
step = j;
}
}
else if(IS_FNAMEEND(plm->m_property))
{
j++;
step = j;
break;
}
else
{
break;
}
}

if(step > 1)
{
if((ret = judge_fname(pwdict,pir,i,step)) > 0)
{
i += ret;
continue;
}
}

step = 1;
}

i += step;
}
return 0;
}
*/

// canbe_long_name
/*========================================================================================
 * function : judge is a two word phrase could form a humanname with the next single word lemma;
 * argu     : pdict, the dictionary
 *           : pir, the internal segment struct
 *           : i, the postion in pir->m_ppseg
 ========================================================================================*/
/*
   int canbe_long_name(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i)
   {  
   int pos1, pos2;
   int lmpos=0;
   scw_lemma_t *plm=NULL, *pllm=NULL, *plm1=NULL, *plm2=NULL;
   char buff[4*MAX_WORD_LEN];
   int len=0,len1=0,len2=0,len3=0,llen=0;
   u_int value=0;
   char tmpstr[3];
   Sdict_snode   snode;
   Sdict_search* ldict  = pwdict->pdict_lsn_rule;
   Sdict_search* rdict = pwdict->pdict_rsn_rule;

   plm= pir->m_ppseg[i];

   if(i==pir->m_ppseg_cnt-1)
   return false;

   scw_lemma_t* pnlm= pir->m_ppseg[i+1];
   if(!pnlm || !IS_1W_NAMEEND(pnlm->m_property))
   return 0;

   if( i > 0 )
   {
   pllm = pir->m_ppseg[i-1];
   if(pllm)
   llen = pllm->m_length;
   }

// split the two word into two lemma
if(IS_PHRASE(plm->m_property))// is a phrase
{
pos1 = pwdict->m_phinfo[plm->m_phinfo_bpos+1];
plm1 = &pwdict->m_lemmalist[pos1];
pos2 = pwdict->m_phinfo[plm->m_phinfo_bpos+2];
plm2 = &pwdict->m_lemmalist[pos2];

// judge if the first one is a real surname
if(!IS_SURNAME(plm1->m_property)|| !IS_NAMEMID(plm2->m_property))
return 0;

len1=plm1->m_length;
len2=plm2->m_length;

len = get_lmstr(pwdict, pir, plm, buff+llen, sizeof(buff)-llen);
}
else//is basic word
{  
len=get_lmstr(pwdict, pir, plm, buff+llen, sizeof(buff)-llen);

value = scw_get_word(buff+llen, (int&)len1,len, "");
plm1 = get_oneword_lemma(pwdict,value);
value = scw_get_word(buff+llen+len1, (int&)len2,len, "");
plm2 = get_oneword_lemma(pwdict,value);

// judge if the first one is a real surneme
if(!IS_SURNAME(plm1->m_property)|| !IS_NAMEMID(plm2->m_property))
return 0;
}

len3 = get_lmstr(pwdict, pir, pnlm, buff+llen+len1+len2, sizeof(buff)-llen-len1-len2);
buff[llen+len1+len2+len3]=0;

creat_sign_fs64(buff+llen,len+len3,&(snode.sign1),&(snode.sign2));
if(ds_op1(rdict, &snode, SEEK) == 1)
return 0; // satisfy a right rule

if(pllm)
{
memcpy(tmpstr,buff+llen,2);
llen=get_lmstr(pwdict,pir,pllm, buff,llen);
memcpy(buff+llen,tmpstr,2);
creat_sign_fs64(buff,llen+len1,&(snode.sign1),&(snode.sign2));
if(ds_op1(ldict, &snode, SEEK) == 1)
	return 0;// satisfy a left rule
	}

// 3字人名的名作为一个单独的动态词存储
if(judge_3cname(pwdict,plm1,plm2,pnlm,0x0))//set flag 0x0 
{
	if((lmpos=create_dynword(pir,buff+llen+len1)) < 0)
	{
		WARNING_LOG("error: create_dynword() failed");
		return -1;
	}
	pir->m_ppseg[i]=plm1;
	pir->m_ppseg[i+1]=&pir->m_dynlm[lmpos];
	return 1;
}

return 0;
}
*/

//  is_3word_cname
/*========================================================================================
 * function : judge if the successive 3 lemma can form a Chinese human name 
 * argu     : pdict, the dictionary
 *           : pir, the output struct
 *           : pos, the postion in pir->m_ppseg
 ========================================================================================*/
/*
   int is_3word_cname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos)
   {
   int len = 0;
   int lmpos = 0;
   char gname[2*MAX_WORD_LEN];

   if(pos+2 >= pir->m_ppseg_cnt)
   return 0;

   if(pir->m_ppseg[pos]==NULL
   || pir->m_ppseg[pos+1]==NULL
   || pir->m_ppseg[pos+2]==NULL)
   return 0;

   if(!IS_NAMEMID(pir->m_ppseg[pos+1]->m_property) 
   || !IS_1W_NAMEEND(pir->m_ppseg[pos+2]->m_property))
   return 0;

   if(judge_3cname(pwdict,pir->m_ppseg[pos],pir->m_ppseg[pos+1],pir->m_ppseg[pos+2]))
   {
   len = get_lmstr(pwdict, pir, pir->m_ppseg[pos+1], gname, sizeof(gname));
   len = get_lmstr(pwdict, pir, pir->m_ppseg[pos+2], &gname[len], sizeof(gname)-len);

   if( (lmpos=create_dynword(pir,gname)) < 0 )
   {
   WARNING_LOG("error: create_dynword() failed");
   return -1;
   }
   pir->m_ppseg[pos+1]=&pir->m_dynlm[lmpos];
   pir->m_ppseg[pos+2]=NULL;
   return 1;
   }

   return 0;
   }
   */

// judge_3cname
/*========================================================================================
 * function : judge if the successive 3 lemma can form a Chinese human name 
 * argu     : pdict, the dictionary
 *          : plm1,plm2,plm3, the lemmas
 *           : flag
 * return : 1 yes,0 no
 ========================================================================================*/
/*
   int judge_3cname(scw_worddict_t* pwdict,scw_lemma_t* plm1,scw_lemma_t* plm2,scw_lemma_t* plm3,int flag)
   {
   int *ppb1=NULL,*ppb2=NULL,*ppb3 = NULL;
   int name_power = 0,condition = 0;
   int tmp = 0;

   ppb1=scw_get_nameprob(pwdict,plm1,DATA_CNAME);
   ppb2=scw_get_nameprob(pwdict,plm2,DATA_CNAME);
   ppb3=scw_get_nameprob(pwdict,plm3,DATA_CNAME);

   if(!ppb1 || !ppb2 || !ppb3)
   {
   return 0;
   }  

   name_power=ppb1[NAME_BGN]+ppb2[NAME_MID]+ppb3[NAME_END];

   tmp=ppb3[NAME_OTH];
   if(tmp < ppb3[NAME_BGN])
   tmp=ppb3[NAME_BGN];

   condition=ppb1[NAME_OTH]+ppb2[NAME_OTH]+tmp;
   if(name_power < condition)
   return 0;
   condition=ppb1[NAME_OTH]+ppb2[NAME_BGN]+ppb3[NAME_END];
   if(name_power < condition)
   return 0;

   if( (flag & 0x1) && IS_NAME_STICK(plm3->m_property)) // 如果最后一个字是基本只能单独使用的字的话
   return 1;

   condition=ppb1[NAME_BGN]+ppb2[NAME_END]+tmp;
   if(name_power < condition)
   return 0;

   return 1;
   }
   */

// is_2word_cname
/*========================================================================================
 * function : judge if the successive 2 lemma can form a Chinese human name 
 * argu     : pdict, the dictionary
 *           : pir, the output struct
 *           : pos, the postion in pir->m_ppseg
 ========================================================================================*/
/*
   int is_2word_cname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos)
   {
   int name_power=0, condition=0;
   int tmp=0, len=0;
   int lmpos = 0;
   scw_lemma_t * plm1=NULL, *plm2=NULL;
   int* ppb1=NULL,*ppb2=NULL;
   char buff[2*MAX_WORD_LEN] = {0};

   if( pos+1 >= pir->m_ppseg_cnt )
   return 0;

   if( (plm1 = pir->m_ppseg[pos])==NULL ||
   (plm2 = pir->m_ppseg[pos+1])==NULL )
   return 0;  

// 姓 + 单字或者两字词的情况（只有带有-8属性的两字词语才检查！）
if( !IS_NAMEEND(plm2->m_property) || IS_HOUZHUI(plm2->m_property) )
return 0;

ppb1 = scw_get_nameprob(pwdict,plm1,DATA_CNAME);
ppb2 = scw_get_nameprob(pwdict,plm2,DATA_CNAME);

if( !ppb1 || !ppb2 )
{
return 0;
}

name_power = ppb1[NAME_BGN] + ppb2[NAME_END];

tmp = ppb2[NAME_OTH];
if(tmp < ppb2[NAME_BGN])
tmp = ppb2[NAME_BGN];

condition = ppb1[NAME_OTH] + tmp;

if(name_power < condition)
return 0;

if( IS_PHRASE(plm2->m_property) )
{
len = get_lmstr(pwdict,pir,plm2,buff,sizeof(buff));
if( (lmpos=create_dynword(pir,buff)) < 0 )
{
WARNING_LOG("error: create_dynword() failed");
return -1;
}
pir->m_ppseg[pos+1]=&pir->m_dynlm[lmpos];
}

return 1;
}
*/
// is_real_surname
/*========================================================================================
 * function : judge if a lemma is a real surname 
 * argu     : pdict, the dictionary
 *           : pir, the output struct
 *           : pos, the postion in pir->m_ppseg
 * return   : 1 is OK, or return 0;
 ========================================================================================*/
/*
   int is_real_surname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i)
   {  
   int len;
   scw_lemma_t *plm, *pllm, *prlm;
   char buff[512];
   Sdict_snode   snode;
   Sdict_search* ldict  = pwdict->pdict_lsn_rule;
   Sdict_search* rdict = pwdict->pdict_rsn_rule;

   plm = pir->m_ppseg[i];  
   if( !IS_SURNAME(plm->m_property) )
   return 0;

   if( i > 0 )
   {
   pllm = pir->m_ppseg[i-1];
   len = get_lmstr(pwdict, pir, pllm, buff, sizeof(buff));
   len  += get_lmstr(pwdict, pir, plm, &buff[len], sizeof(buff)-len);

   creat_sign_fs64(buff,len,&(snode.sign1),&(snode.sign2));
   if(ds_op1(ldict, &snode, SEEK) == 1)
   return 0; // satisfy a left rule
   }

   if( i < pir->m_ppseg_cnt-1 )
   {
   prlm = pir->m_ppseg[i+1];
   len = get_lmstr(pwdict, pir, plm, buff, sizeof(buff));
   len += get_lmstr(pwdict, pir, prlm, &buff[len], sizeof(buff)-len);
   creat_sign_fs64(buff,len,&(snode.sign1),&(snode.sign2));
   if(ds_op1(rdict, &snode, SEEK) == 1)
   return 0; // satisfy a right rule
   }

   return 1;
   }
   */

//  bookname_recg
/*========================================================================================
 * function : bookname recgnition process on the internal segged result. 
 * argu     : pdict, the dictionary
 *       : pir, the inner seg result struct
 ========================================================================================*/
int bookname_recg(scw_worddict_t* pwdict, scw_inner_t* pir)
{
	int bn_bpos= -1;
	int i=0;
	int pos=0;
	scw_lemma_t* plm=NULL;

	for(i=0; i< (int)pir->m_ppseg_cnt; i++)
	{
		plm = pir->m_ppseg[i];
		if(!plm){continue;}
		//if(IS_WORDNOTDYN(plm->m_property))
		/*if(pir->m_ppseg_flag[i] == 1 && bn_bpos >= 0)
		{ 
			break;
		}*/

		if(IS_BOOKNAME_START(plm->m_property)){
			bn_bpos = i;        
		}
		else if(IS_BOOKNAME_END(plm->m_property))
		{
			if(bn_bpos >= 0)
			{
				pos = poly_lemmas(pwdict, pir, bn_bpos, i-bn_bpos+1);
				if(pos==-1)
				{
					WARNING_LOG("error: poly_lemmas() failed");
					return -1;
				}
				if(pos!= -2){
					SET_BOOKNAME(pir->m_dynlm[pos].m_property);
				}
			}
			bn_bpos = -1;
		}  
	}

	return 0;
}



//  proc_ascii
/*========================================================================================
 * function : ascii recgnition process on a string buffer. 
 * argu     : pdict, the dictionary
 *       : pir, the inner seg result struct
 *       : src,slen,  the string buffer and its length
 *       : bpos, the begin position in src for ascii recgnition.
 *       : lpos, last static lemma pos
 *       : newpos, the the process succeed, record the postion after the ascii string
 * return  : 1, there is ascii string;  <0, error accur
 ========================================================================================*/
int append_asciistr(scw_worddict_t * pwdict,scw_inner_t * pir,char * word,int len,int chrcnt ,int flag = 1)
{
	int lmpos = 0;
	u_int lmpos1 = 0;
	scw_lemma_t * plm = NULL;
	int lmbpos = 0;
	int pos = 0;
	int lpos = 0;
	int lmcnt = 0;
	int ret = 0;

	if(chrcnt > MIN_SPECSTR_LEN)
	{
		lmbpos = pir->m_ppseg_cnt;
		lpos = pos;

		while((lmpos1 = scw_get_fmm_word(pwdict,word,len,pos)) != LEMMA_NULL)
		{
			plm = pwdict->m_lemmalist + lmpos1;

			if(IS_SPEC_ENSTR(plm->m_property))
			{
				if(append_lemma(pwdict, pir,plm) < 0)
				{
					WARNING_LOG("error: append_lemma() failed");
					return -1;
				}
				lpos = pos;
				lmcnt++;
			}
			else
			{
				break;
			}
		}

		if(lmcnt > 0)
		{
			if(lpos != len)
			{
				if(flag)
				{
					pir->m_ppseg_cnt -= lmcnt;
				}
			}
			else
			{
				if(lmcnt == 1)
				{
					return len;
				}
				else
				{
					if((ret = poly_lemmas(pwdict, pir,lmbpos,lmcnt)) < 0)
					{
						WARNING_LOG("error: poly_lemmas failed");
						return -1;
					}

					plm = &pir->m_dynlm[ret];
					SET_ASCIIWORD(plm->m_property);

					pir->m_ppseg_cnt -= lmcnt-1;

					return len;
				}  
			}  
		}

		if(flag == 0)
		{
			return lpos;
		}
	}
	else
	{
		lpos = len;
	}

	if(len==1 && (lmpos1 = scw_seek_lemma(pwdict, word, len)) != LEMMA_NULL)
	{
		if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
		{
			WARNING_LOG("error: append_lemma() failed");
			return -1;
		}
	}
	else
	{
		if((lmpos = create_dynword(pir, word)) < 0)
		{
			WARNING_LOG("error: create_dynword() failed");
			return -1;
		}
		if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
		{
			WARNING_LOG("error: append_lemma() failed");
			return -1;
		}
		plm = &pir->m_dynlm[lmpos];
		SET_ASCIIWORD(plm->m_property);
	}

	return lpos;
}

/*!
 * 
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
int proc_ascii(scw_worddict_t * pwdict, scw_inner_t * pir,
		const char * src, int slen, int bpos, int lpos,
		scw_lemma_t * pllm, int& newpos)
{
	char word[MAX_WORD_LEN]={};
	char tmpstr[MAX_WORD_LEN]={};//store the letter num mixed str

	int tmpbpos = -1,lmbpos = 0;//seg pos
	int len = 0,dgtbgn = -1,dgtlen = 0,ltrlen = 0,linklen = 0;//buf pos
	int tmpcnt = 0,dgtcnt = 0,dcnt = 0;
	int pos = bpos,dpos = 0;
	int vbpos = -1,vcnt = 0;//the version pos
	int lmpos = 0;
	u_int lmpos1 = 0;
	int off = 0,off2 = 0;

	int has_mid_mark = 0;//has mid mark in str
	int has_point_mark = 0;//has '.' in str
	int has_split_dgtstr = 0;// find xxxx num in front or  find num in back 
	int is_pure_digit = 1;
	int is_first_digit = 0;
	int is_overflow = 0;//if the string longer than MAX_WORD_LEN

	scw_lemma_t * plm = NULL;
	char tmpchr = 0;

	if(pos >= slen || !is_my_ascii((char *)(src + pos)))
		return 0;

	if(IS_SPEC_ENSTR(pllm->m_property))
	{
		len = pllm->m_length;
		if(pos + len >= slen || !is_my_ascii((char *)&src[pos+len]))
		{
			if(append_lemma(pwdict, pir,pllm) < 0)
			{
				WARNING_LOG("error: append_lemma() failed");
				return -1;
			}

			newpos = pos + pllm->m_length;
			return 1;
		}
	}
	else
	{
		while((off = is_split_headmark((char *)(src + pos))))// if the head is head link
		{
			word[0] = src[pos];
			word[1] = 0;
			if(off == 2)
			{
				word[1] = src[pos+1];
				word[2] = 0;
			}

			if(off==1 && (lmpos1 = scw_seek_lemma(pwdict, word, off)) != LEMMA_NULL)
			{
				if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
				{
					WARNING_LOG("error: append_lemma() failed");
					return -1;
				}
			}
			else
			{
				if((lmpos = create_dynword(pir, word)) < 0)
				{
					WARNING_LOG("error: create_dynword() failed");
					return -1;
				}
				if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
				{
					WARNING_LOG("error: append_lemma() failed");
					return -1;
				}
			}

			pos += off;
		}

		if(!is_my_ascii((char *)(src+pos)))
		{
			newpos = pos;
			return 1;
		}
	}

	if(is_my_num((char *)(src+pos)))
	{
		is_first_digit = 1;
	}

	lmbpos = pir->m_ppseg_cnt;
	tmpstr[0] = 0;
	word[0] = 0;

	//while( (off = is_my_ascii((char*)(src+pos))) ) // 1.2.5之前版本这里没有判断pos是否超越用户限制，修正如下:
	while( (off = is_my_ascii((char*)(src+pos))) && (pos < slen) ) 
	{
		if(dpos >= MAX_WORD_LEN-2)
		{
			word[dpos]=0;

			if((lmpos = create_dynword(pir, word)) < 0)
			{
				WARNING_LOG("error: create_dynword() failed");
				return -1;
			}

			if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
			{
				WARNING_LOG("error: append_lemma() failed ");
				return -1;
			}

			plm = &pir->m_dynlm[lmpos];
			SET_ASCIIWORD(plm->m_property);

			word[0] = 0;
			tmpstr[0] = 0;
			dpos = 0;
			dcnt = 0;
			dgtbgn = -1;
			dgtlen = 0;
			dgtcnt = 0;
			ltrlen = 0;
			tmpbpos = -1;
			tmpcnt = 0;
			vbpos = -1;
			vcnt = 0;
			has_mid_mark = 0;
			has_point_mark = 0;
			is_first_digit = 0;
			has_split_dgtstr = 0;
			is_overflow = 1;

			continue;
		}

		if(is_my_num((char *)(src+pos)))
		{
			if(dgtlen == 0)
				dgtbgn = dpos;
			dgtlen += off;
			dgtcnt++;
		}
		else if(is_mid_mark((char *)(src+pos)))
		{
			while(pos + linklen < slen && (off2 = is_split_tailmark((char *)&src[pos + linklen])))
			{
				linklen += off2;
			}

			if(linklen > 0 && (pos + linklen >= slen || !is_my_ascii((char *)&src[pos + linklen])))
			{
				pos += linklen;
				is_pure_digit = 0;

				break;
			}

			has_mid_mark = 1;

			if(is_radix_point((char *)(src+pos)) && pos+off < slen && is_my_num((char *)(src+pos+off)))
			{
				ltrlen = dpos-dgtlen;//the pure letter length 

				if(ltrlen > 0 && dgtlen>0)//store the mixed str
				{
					memcpy(tmpstr,word,dpos);
					tmpstr[dpos] = 0;
					tmpcnt = dcnt;          
					tmpbpos = pir->m_ppseg_cnt;
				}

				if(ltrlen > 0)
				{
					tmpchr = word[ltrlen];
					word[ltrlen] = 0;

					if(append_asciistr(pwdict,pir,word,ltrlen,dcnt-dgtcnt)<0)
					{
						WARNING_LOG("error: append_asciistr() failed");
						return -1;
					}

					word[ltrlen] = tmpchr;
				}

				if(dgtlen > 0)
				{
					word[dpos] = 0;
					if(dpos-dgtbgn==1 &&(lmpos1 = scw_seek_lemma(pwdict, word+dgtbgn, dpos-dgtbgn)) != LEMMA_NULL)
					{
						if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
						{
							WARNING_LOG("error: append_lemma() failed");
							return -1;
						}
					}
					else
					{
						if((lmpos = create_dynword(pir, word+dgtbgn)) < 0)
						{
							WARNING_LOG("error: create_dynword() failed");
							return -1;
						}
						if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
						{
							WARNING_LOG("error: append_lemma() failed");
							return -1;
						}
						plm = &pir->m_dynlm[lmpos];
						SET_ASCIIWORD(plm->m_property);
						SET_ASCIINUM(plm->m_property);
					}

					if(vbpos < 0)
					{
						vbpos = pir->m_ppseg_cnt-1;
						vcnt = 1;
					}
					else
					{
						vcnt++;
					}
				}

				has_point_mark=1;      
			}
			else
			{  
				if(dpos > 0)
				{
					word[dpos] = 0;
					if(append_asciistr(pwdict,pir,word,dpos,dcnt)<0)
					{
						WARNING_LOG("error: append_asciistr() failed");
						return -1;
					}
				}

				vbpos = -1;
				vcnt = 0;
			}

			if(linklen == 0){
				linklen = off;
			}

			int i = 0;
			while(i < linklen)
			{
				if(src[pos] >= 0){
					word[0] = src[pos++];
					word[1] = 0;
					i++;
				}
				else{
					word[0] = src[pos++];
					word[1] = src[pos++];
					word[2] = 0;
					i += 2;
				}

				if(strlen(word)==1 && (lmpos1 = scw_seek_lemma(pwdict, word, strlen(word))) != LEMMA_NULL)
				{
					if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
					{
						WARNING_LOG("error: append_lemma() failed");
						return -1;
					}
				}
				else
				{
					if((lmpos = create_dynword(pir, word)) < 0)
					{
						WARNING_LOG("error: create_dynword() failed");
						return -1;
					}
					if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
					{
						WARNING_LOG("error: append_lemma() failed");
						return -1;
					}
					plm = &pir->m_dynlm[lmpos];
					SET_ASCIIWORD(plm->m_property);
				}
			}

			if(vbpos >= 0)
			{
				vcnt++;  
			}

			linklen = 0;
			ltrlen = 0;
			is_pure_digit = 0;
			dpos = 0;
			dcnt  = 0;
			dgtbgn = -1;
			dgtlen = 0;
			dgtcnt = 0;

			continue;
		}
		else{
			if(is_first_digit && !has_point_mark && dgtcnt > 2)
			{
				word[dgtlen]=0;
				//find a num xxxx in front

				if(dgtlen==1 && (lmpos1 = scw_seek_lemma(pwdict, word, dgtlen)) != LEMMA_NULL)
				{
					if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
					{
						WARNING_LOG("error: append_lemma() failed");
						return -1;
					}
				}
				else
				{
					if((lmpos = create_dynword(pir, word)) < 0)
					{
						WARNING_LOG("error: create_dynword() failed");
						return -1;
					}
					if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
					{
						WARNING_LOG("error: append_lemma() failed");
						return -1;
					}
					plm = &pir->m_dynlm[lmpos];
					SET_ASCIIWORD(plm->m_property);
					SET_ASCIINUM(plm->m_property);
				}

				dpos = 0;
				dcnt = 0;
				has_split_dgtstr = 1;
			}
			else if(tmpstr[0] != 0 && tmpbpos >= 0)
			{    
				if(append_asciistr(pwdict,pir,tmpstr,strlen(tmpstr),tmpcnt) < 0)
				{
					WARNING_LOG("error: append_asciistr() failed");
					return -1;
				}

				plm = pir->m_ppseg[pir->m_ppseg_cnt-1];
				pir->m_ppseg_cnt--;

				pir->m_ppseg[tmpbpos] = plm;
				//assert(tmpbpos+1 <(int) pir->m_ppseg_cnt);
				if(tmpbpos+1 >=(int) pir->m_ppseg_cnt)
				{
					WARNING_LOG("error: tmpbpos out of range");
					return -1;
				}
				pir->m_ppseg[tmpbpos+1] = NULL;

				tmpbpos = -1;
				tmpstr[0] = 0;
				tmpcnt = 0;
			}

			has_point_mark = 0;
			is_first_digit = 0;
			dgtbgn = -1;
			dgtlen = 0;
			dgtcnt = 0;
			vbpos = -1;
			vcnt = 0;
			is_pure_digit = 0;
		}

		word[dpos++] = src[pos++];
		if(off == 2)
		{
			word[dpos++] = src[pos++];
		}
		dcnt++;
	}

	if(dpos>0)
	{
		word[dpos] = 0;

		if(dgtlen > 0) 
		{
			ltrlen = dpos-dgtlen;

			if(dcnt-dgtcnt > MIN_LTR_LEN)
			{
				tmpbpos = pir->m_ppseg_cnt;
				len = append_asciistr(pwdict,pir,word,dpos,dcnt,0);

				if(len != dpos)
				{
					if(len != ltrlen)
					{
						pir->m_ppseg_cnt = tmpbpos ;

						tmpchr = word[ltrlen];
						word[ltrlen] = 0;
						if(ltrlen==1 && (lmpos1 = scw_seek_lemma(pwdict, word, ltrlen)) != LEMMA_NULL)
						{
							if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
							{
								WARNING_LOG("error: append_lemma() failed");
								return -1;
							}
						}
						else
						{
							if((lmpos = create_dynword(pir, word)) < 0)
							{
								WARNING_LOG("error: create_dynword() failed");
								return -1;
							}
							if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
							{
								WARNING_LOG("error: append_lemma() failed");
								return -1;
							}
							plm = &pir->m_dynlm[lmpos];
							SET_ASCIIWORD(plm->m_property);
						}

						word[ltrlen] = tmpchr;
						ltrlen = 0;
					}

					if(dpos-dgtbgn==1 && (lmpos1 = scw_seek_lemma(pwdict, word+dgtbgn, dpos-dgtbgn)) != LEMMA_NULL)
					{
						if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
						{
							WARNING_LOG("error: append_lemma() failed");
							return -1;
						}
					}
					else
					{
						if((lmpos = create_dynword(pir, word+dgtbgn)) < 0)
						{
							WARNING_LOG("error: create_dynword_() failed");
							return -1;
						}
						if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
						{
							WARNING_LOG("error: append_lemma() failed");
							return -1;
						}
						plm = &pir->m_dynlm[lmpos];
						SET_ASCIIWORD(plm->m_property);
						SET_ASCIINUM(plm->m_property);
					}

					has_split_dgtstr = 1;
				}
			}
			else{
				if(dpos+ltrlen-dgtbgn==1 && (lmpos1 = scw_seek_lemma(pwdict, word+dgtbgn-ltrlen, dpos+ltrlen-dgtbgn)) != LEMMA_NULL)
				{
					if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
					{
						WARNING_LOG("error: append_lemma() failed");
						return -1;
					}
				}
				else
				{
					if((lmpos = create_dynword(pir, word+dgtbgn-ltrlen)) < 0)
					{
						WARNING_LOG("error: create_dynword() failed");
						return -1;
					}
					if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
					{
						WARNING_LOG("error: append_lemma() failed");
						return -1;
					}
					plm = &pir->m_dynlm[lmpos];
					SET_ASCIIWORD(plm->m_property);
					if(ltrlen == 0)
					{
						SET_ASCIINUM(plm->m_property);
					}
				}

				if(vbpos >= 0 && ltrlen == 0)
				{
					vcnt++;
				}
			}
		}
		else
		{
			if(append_asciistr(pwdict,pir,word,dpos,dcnt) < 0)
			{
				WARNING_LOG("error: append_asciistr() failed");
				return -1;
			}
		}

		dpos = 0;
	}

	// judge if the dynamic lemma is longer than the static one
	if(pos <= lpos)
	{
		pir->m_ppseg_cnt = lmbpos;
		return 0;
	}

	if(is_pure_digit)
	{
		plm = pir->m_ppseg[pir->m_ppseg_cnt-1];
		if(IS_TEMP(plm->m_property))
		{
			SET_ASCIINUM(plm->m_property);
			SET_ASCIIWORD(plm->m_property);
		}
	}
	else
	{
		if(vbpos - lmbpos > 0 && vcnt > 1 && has_point_mark)
		{
			//find a version x.x....x.x in back

			bool bflag = false;
			int off1 = 0;
			int v_num = 0;
			int j = 0;
			int vboff = 0;

			vboff = is_my_ascii((char*)(src+vbpos));
			while(v_num < vcnt)
			{
				if(is_radix_point((char *)(src+vboff+j)))
				{
					v_num += 2;
					if(bflag && v_num-1<vcnt)
					{
						if((lmpos = poly_lemmas(pwdict, pir, vbpos, v_num-1)) == -1)
						{
							WARNING_LOG("error: poly_lemmas() failed");
							return -1;
						}

						if(lmpos >= 0){
							plm = &pir->m_dynlm[lmpos];
							SET_ASCIIWORD(plm->m_property);
						}
					}
					bflag = true;
				}
				else if(!is_my_ascii((char*)(src+vboff+j)))
				{
					break;
				}
				off1 = is_my_ascii((char*)(src+vboff+j));
				j += off1;
			}

			if((lmpos = poly_lemmas(pwdict, pir, vbpos,vcnt)) == -1)
			{
				WARNING_LOG("error: poly_lemmas() failed");
				return -1;
			}

			if(lmpos >= 0){
				plm = &pir->m_dynlm[lmpos];         
				SET_ASCIIWORD(plm->m_property);
			}
		}  

		if(pir->m_ppseg_cnt - lmbpos > 1 && (has_mid_mark || has_split_dgtstr))
		{  
			if((lmpos = poly_lemmas(pwdict, pir, lmbpos, pir->m_ppseg_cnt-lmbpos)) == -1)
			{
				WARNING_LOG("error: poly_lemmas() failed");
				return -1;
			}

			if(lmpos >= 0)
			{
				plm = &pir->m_dynlm[lmpos];
				SET_ASCIIWORD(plm->m_property);

				if(append_asciistr_subphrase(pwdict,pir, plm) < 0)
				{
					WARNING_LOG("error: append_asciistr_subphrase() failed");
					return -1;
				}

			}
		}
	}

	if(linklen > 0)
	{
		dpos = pos-linklen;
		int i = 0;
		while(i < linklen)
		{
			if(src[dpos+i] >= 0)
			{
				word[0] = src[dpos+i];
				word[1] = 0;
				i++;
			}
			else
			{
				word[0] = src[dpos+i];
				word[1] = src[dpos+i+1];
				word[2] = 0;
				i+=2;
			}      

			if(strlen(word)==1 && (lmpos1 = scw_seek_lemma(pwdict, word, strlen(word))) != LEMMA_NULL)
			{
				if(append_lemma(pwdict, pir, lmpos1, SCW_STATIC)<0)
				{
					WARNING_LOG("error: append_lemma() failed");
					return -1;
				}
			}
			else
			{
				if((lmpos = create_dynword(pir, word)) < 0)
				{
					WARNING_LOG("error: create_dynword() failed");
					return -1;
				}
				if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
				{
					WARNING_LOG("error: append_lemma() failed");
					return -1;
				}
			}
		}  
	}

	newpos = pos;
	return 1;
}

//  append_asciistr_subphrase
/*========================================================================================
 * function : 增加ascii码的subphrase粒度
 *       : result.
 * argu     : pdict, the dictionary
 *       : pir, the inner seg result struct
 *       : plm, ascii word
 * return  : 0, successful
 *       : <0, failed.
 ========================================================================================*/
int append_asciistr_subphrase(scw_worddict_t * pwdict,scw_inner_t* pir, scw_lemma_t * plm)
{
	if(pwdict == NULL || pir == NULL || plm == NULL)
	{
		WARNING_LOG("invalid parameters, plm or pir or pwdict is null!");
		return -1;
	}

	int ret = 0;
	u_int i = 0, j = 0, type = 0, cnt = 0, mark_cnt = 0;;
	char word[MAX_WORD_LEN] = "\0";
	int len = 0;
	int bpos = plm->m_phinfo_bpos;
	int lmpos = 0;
	scw_lemma_t * plm3=NULL, *plm2=NULL;
	int begin = 0;		

	type = plm->m_type;
	if(type != LEMMA_TYPE_DPHRASE)
		return 0;
	cnt = pir->m_dynphinfo[bpos];

	u_int *dot_mark = NULL;
	u_int *termcnt_between_mark = NULL;
	u_int *mark = NULL;
	//	uintptr_t *phinfo = NULL;
	//	int phinfopos = 0;

	mark = (u_int *) malloc (sizeof(u_int) * 3 * (cnt+1));
	if(mark == NULL)
	{
		WARNING_LOG("error: malloc for mark failed");
		ret = -1;
		goto exit;
	}	
	memset(mark, 0, 3*(cnt+1)*sizeof(u_int));

	dot_mark = &(mark[cnt+1]);
	termcnt_between_mark = &(mark[(cnt+1) * 2]);	

	/*	phinfo = (uintptr_t *) malloc (sizeof(uintptr_t) * (pir->m_dynph_size - pir->m_dynph_curpos));
		if(phinfo == NULL)
		{
		WARNING_LOG("error:malloc for phinof failed");
		ret = -1;
		goto exit;
		}
		*/
	
	pir->m_dynph_curpos_tmp = 0;
	pir->m_dynphinfo_tmp[0] = 0;
	pir->m_dynph_curpos_tmp ++;

	termcnt_between_mark[0] = 0;
	//1、找到所有的mark和点,存储其相应的basic位置
	for(i = 0; i < cnt; i ++)
	{
		plm2= (scw_lemma_t*)pir->m_dynphinfo[bpos+1+i];
		if(!plm2)
		{
			WARNING_LOG("error type of phrase's basic term!");
			ret = -1;
			goto exit;
		}
		len = get_lmstr(pwdict, pir, plm2, word, sizeof(word));
		if(len < 0)
		{
			ret = -1;
			goto exit;
		}

		//find a mark
		if((len == 1 || len == 2) && is_mid_mark(word))
		{
			mark[mark_cnt++] = i;
			if(is_radix_point(word))
			{
				dot_mark[i] = 1;
			}
			continue;
		}

		//存储每两个mark之间的basic数目
		termcnt_between_mark[mark_cnt] ++; 
	}

	if(mark_cnt == 0)
	{
		ret = 0;
		goto exit;
	}
	//如果cnt-1的basic非mark，则将cnt存储为结尾的mark
	if(mark[mark_cnt - 1] != cnt - 1)
	{
		mark[mark_cnt ++] = cnt;
	}

	//生成从开始到第一个非点的连接符之间的subphrase
	//如果第一个basic为连接符后面会有相应的操作
	if(mark[0] != 0 && (dot_mark[0] == 0) 
			&& (mark_cnt > 2) && dot_mark[mark[0]] == 1 && dot_mark[mark[1]] == 1)
	{
		for(j = 2; j < mark_cnt; j ++)
		{
			if((dot_mark[mark[j]] == 0) && (mark[j] < cnt))
			{
				lmpos = create_ascii_dphrase( pwdict, pir, bpos, 0 ,mark[j]);
				if(lmpos < 0)
				{
					WARNING_LOG("error: create_ascii_dphrase() failed");
					ret = -1;
					goto exit;
				}

				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = 0;
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++]=(uintptr_t)&pir->m_dynlm[lmpos];
				pir->m_dynphinfo_tmp[0] ++;					
				break;
			}
		}
	}

	for(i = 0; i < mark_cnt; i ++)
	{
		if(i == 0)
		{
			if(mark[0] != 0)
			{
				begin = 0;
			}
			else 
			{
				begin = 1;
			}
		}
		else
		{
			begin = mark[i -1] + 1;
		}

		//mark[i-1],mark[i], mark[i+1]之间的term组成一个subphrase
		//要求mark[i]前后的bigram必须有非连接符以外的字符串
		if(i + 1 < mark_cnt && (mark[i+1] - begin < cnt) 
				&& (mark[i] > 0) &&(mark[i+1] - mark[i] > 1) && (mark[i] - begin >= 1))
		{	
			lmpos = create_ascii_dphrase(pwdict, pir, bpos, begin, mark[i+1] - begin);
			if(lmpos < 0)
			{
				WARNING_LOG("error: create_ascii_dphrase() failed");
				ret = -1;
				goto exit;
			}
			pir->m_dynphinfo_tmp[0] ++;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = begin;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] =(uintptr_t)&pir->m_dynlm[lmpos];
		}

		if(termcnt_between_mark[i] > 1 && (mark[i] - begin < cnt))
		{
			//将mark[i-1] 与mark[i]之间的term组成一个subphrse；			
			lmpos=create_ascii_dphrase(pwdict, pir, bpos, begin, mark[i] - begin);
			if(lmpos < 0)
			{
				WARNING_LOG("error: create_ascii_dphrase() failed");
				ret = -1;
				goto exit;
			}
			pir->m_dynphinfo_tmp[0] ++;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = begin;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] =(uintptr_t)&pir->m_dynlm[lmpos];
		}

		//连接符及其左右的数字，组成subphrase
		if((i + 1 < mark_cnt) && (mark[i] > 0 )
				&& ((mark[i+1] - mark[i] > 2) || (mark[i] - begin >= 2)))
		{
			plm2 = (scw_lemma_t *)pir->m_dynphinfo[bpos + mark[i]];
			plm3 = (scw_lemma_t *)pir->m_dynphinfo[bpos + mark[i] + 2];
			if(IS_ASCIINUM(plm2->m_property)&& IS_ASCIINUM(plm3->m_property))
			{
				lmpos = create_ascii_dphrase(pwdict, pir, bpos, mark[i] - 1, 3);
				if(lmpos < 0)
				{
					WARNING_LOG("error: create_ascii_dphrase() failed");
					ret = -1;
					goto exit;
				}
				pir->m_dynphinfo_tmp[0] ++;
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = mark[i] - 1;
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] =(uintptr_t)&pir->m_dynlm[lmpos];
			}
		}

		if(dot_mark[mark[i]] == 0)
		{
			//从这个开始，向后的非点的连接符之间的组成一个subphrase
			if(((i+1 < mark_cnt) && dot_mark[mark[i+1]] == 0) 
					|| ((i+2 < mark_cnt) && dot_mark[mark[i+2]] == 0))
				continue;
			for(j = i+3; j < mark_cnt; j ++)
			{
				if(dot_mark[mark[j]] == 0)
				{
					lmpos = create_ascii_dphrase( pwdict, pir, bpos, mark[i] + 1,mark[j] - mark[i] - 1);
					if(lmpos < 0)
					{
						WARNING_LOG("error: create_ascii_dphrase() failed");
						ret = -1;
						goto exit;
					}
					pir->m_dynphinfo_tmp[0] ++;
					pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = mark[i] + 1;
					pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++]=(uintptr_t)&pir->m_dynlm[lmpos];
					break;
				}
			}
		}
	}

	if(pir->m_dynphinfo_tmp[0] > 0)
	{
		int subphpos = plm->m_subphinfo_bpos;
		plm->m_subphinfo_bpos = pir->m_dynph_curpos;

		if(subphpos >= 0)
		{		
			// 需要按照offset的顺序输出，而且不能输出重复的。
			pir->m_dynphinfo[plm->m_subphinfo_bpos] = 0;
			pir->m_dynph_curpos ++ ;
			int tmppos = 1 + 2 * (pir->m_dynphinfo[subphpos]);//当前subphrase所在的位置
			i = 0;
			j = 0;

			if(pir->m_dynph_curpos+ pir->m_dynph_curpos_tmp + tmppos >= pir->m_dynph_size)
			{
				WARNING_LOG("error: m_dynph_curpos out of range");
				ret = -1;
				goto exit;
			}
			while((i < pir->m_dynphinfo[subphpos]) && (j < pir->m_dynphinfo_tmp[0]))
			{
				//先比较offset，然后比较子basic的term是否一样
				if(pir->m_dynphinfo[subphpos + 2*i + 1] > pir->m_dynphinfo_tmp[2*j + 1])
				{
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +1];
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +2];
					j ++ ;
					pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

				}
				else if(pir->m_dynphinfo[subphpos + 2*i + 1] == pir->m_dynphinfo_tmp[2*j + 1])
				{
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[subphpos+ 2*i + 2];
					plm3 = (scw_lemma_t*)pir->m_dynphinfo_tmp[2*j + 2];

					u_int bwcnt = 0;
					switch(plm2->m_type)
					{
						case LEMMA_TYPE_SPHRASE:
							bwcnt = pwdict->m_phinfo[plm2->m_phinfo_bpos];
							break;
						case LEMMA_TYPE_DPHRASE:
							bwcnt = pir->m_dynphinfo[plm2->m_phinfo_bpos];
							break;
						default:
							break;
					}

					//子basic数目一样，说明是一样的。
					if(bwcnt ==  pir->m_dynphinfo[plm3->m_phinfo_bpos])
					{
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 1];
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 2];
						pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

						i ++ ;
						j ++;
					}
					else if(bwcnt > pir->m_dynphinfo[plm3->m_phinfo_bpos])
					{
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 1];
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 2];
						pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

						i ++ ;
					}
					else
					{
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +1];
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +2];
						j ++ ;
						pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;
					}

				}
				else
				{
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 1];
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 2];
					pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

					i ++ ;
				}
			}
			if(i < pir->m_dynphinfo[subphpos])
			{
				memcpy(pir->m_dynphinfo+pir->m_dynph_curpos,
						pir->m_dynphinfo+subphpos+i*2+1, sizeof(scw_lemma_t*)* (pir->m_dynphinfo[subphpos] - i) * 2 );
				pir->m_dynphinfo[plm->m_subphinfo_bpos] += pir->m_dynphinfo[subphpos] - i;
				pir->m_dynph_curpos += (pir->m_dynphinfo[subphpos] - i) * 2;
			}
			if(j < pir->m_dynphinfo_tmp[0])
			{
				memcpy(pir->m_dynphinfo+pir->m_dynph_curpos, pir->m_dynphinfo_tmp+1+j*2, sizeof(scw_lemma_t*)* (pir->m_dynphinfo_tmp[0] - j) * 2);
				pir->m_dynphinfo[plm->m_subphinfo_bpos] += pir->m_dynphinfo_tmp[0] - j;
				pir->m_dynph_curpos += (pir->m_dynphinfo_tmp[0] - j) * 2;
			}
		}
		else
		{	
			if(pir->m_dynph_curpos+ pir->m_dynph_curpos_tmp  >= pir->m_dynph_size)
			{
				WARNING_LOG("error: m_dynph_curpos out of range");
				ret = -1;
				goto exit;
			}
			memcpy(pir->m_dynphinfo+pir->m_dynph_curpos, pir->m_dynphinfo_tmp, sizeof(scw_lemma_t*)* pir->m_dynph_curpos_tmp);
			pir->m_dynph_curpos += pir->m_dynph_curpos_tmp ;
		}
	}

exit:
	if(mark)
	{
		free(mark);
		mark = NULL;
		dot_mark = NULL;
		termcnt_between_mark = NULL;
	}
	/*
	   if(phinfo)
	   {
	   free(phinfo);
	   phinfo = NULL;
	   }
	   */	
	return ret;
}

int append_asciistr_subphrase_crf(scw_worddict_t * pwdict,scw_inner_t* pir, scw_lemma_t * plm)
{
	if(pwdict == NULL || pir == NULL || plm == NULL)
	{
		WARNING_LOG("invalid parameters, plm or pir or pwdict is null!");
		return -1;
	}

	int ret = 0;
	u_int i = 0, j = 0, type = 0, cnt = 0, mark_cnt = 0;;
	char word[MAX_WORD_LEN] = "\0";
	int len = 0;
	int bpos = plm->m_phinfo_bpos;
	int lmpos = 0;
	scw_lemma_t * plm3=NULL, *plm2=NULL;
	int begin = 0;		

	type = plm->m_type;
	if(type != LEMMA_TYPE_DPHRASE)
		return 0;
	cnt = pir->m_dynphinfo[bpos];

	u_int *dot_mark = NULL;
	u_int *termcnt_between_mark = NULL;
	u_int *mark = NULL;
	int vbpos = -1;
	u_int vb_cnt = 0;
	u_int vb_begin = 0;
	u_int vcnum = 0;
	int flag = 0;
	//	uintptr_t *phinfo = NULL;
	//	int phinfopos = 0;

	mark = (u_int *) malloc (sizeof(u_int) * 3 * (cnt+1));
	if(mark == NULL)
	{
		WARNING_LOG("error: malloc for mark failed");
		ret = -1;
		goto exit;
	}	
	memset(mark, 0, 3*(cnt+1)*sizeof(u_int));

	dot_mark = &(mark[cnt+1]);
	termcnt_between_mark = &(mark[(cnt+1) * 2]);	

	pir->m_dynph_curpos_tmp = 0;
	pir->m_dynphinfo_tmp[0] = 0;
	pir->m_dynph_curpos_tmp ++;

	termcnt_between_mark[0] = 0;
	//1、找到所有的mark和点,存储其相应的basic位置
	for(i = 0; i < cnt; i ++)
	{
		plm2= (scw_lemma_t*)pir->m_dynphinfo[bpos+1+i];
		if(!plm2)
		{
			WARNING_LOG("error type of phrase's basic term!");
			ret = -1;
			goto exit;
		}
		len = get_lmstr(pwdict, pir, plm2, word, sizeof(word));
		if(len < 0)
		{
			ret = -1;
			goto exit;
		}

		//find a mark
		if((len == 1 || len == 2) && is_mid_mark(word))
		{
			mark[mark_cnt++] = i;
			if(is_radix_point(word))
			{
				dot_mark[i] = 1;
			}
			continue;
		}

		//存储每两个mark之间的basic数目
		termcnt_between_mark[mark_cnt] ++; 
	}

	if(mark_cnt == 0)
	{
		ret = 0;
		goto exit;
	}
	//如果cnt-1的basic非mark，则将cnt存储为结尾的mark
	if(mark[mark_cnt - 1] != cnt - 1)
	{
		mark[mark_cnt ++] = cnt;
	}

	//生成从开始到第一个非点的连接符之间的subphrase
	//如果第一个basic为连接符后面会有相应的操作
	if(mark[0] != 0 && (dot_mark[0] == 0) 
			&& (mark_cnt > 2) && dot_mark[mark[0]] == 1 && dot_mark[mark[1]] == 1)
	{
		for(j = 2; j < mark_cnt; j ++)
		{
			if((dot_mark[mark[j]] == 0) && (mark[j] < cnt))
			{
				lmpos = create_ascii_dphrase( pwdict, pir, bpos, 0 ,mark[j]);
				if(lmpos < 0)
				{
					WARNING_LOG("error: create_ascii_dphrase() failed");
					ret = -1;
					goto exit;
				}

				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = 0;
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++]=(uintptr_t)&pir->m_dynlm[lmpos];
				pir->m_dynphinfo_tmp[0] ++;					
				break;
			}
		}
	}

	for(i = 0; i < mark_cnt; i ++)
	{
		if(i == 0)
		{
			if(mark[0] != 0)
			{
				begin = 0;
			}
			else 
			{
				begin = 1;
			}
		}
		else
		{
			begin = mark[i -1] + 1;
		}

		//mark[i-1],mark[i], mark[i+1]之间的term组成一个subphrase
		//要求mark[i]前后的bigram必须有非连接符以外的字符串
		if(i + 1 < mark_cnt && (mark[i+1] - begin < cnt) 
				&& (mark[i] > 0) &&(mark[i+1] - mark[i] > 1) && (mark[i] - begin >= 1))
		{	
			lmpos = create_ascii_dphrase(pwdict, pir, bpos, begin, mark[i+1] - begin);
			if(lmpos < 0)
			{
				WARNING_LOG("error: create_ascii_dphrase() failed");
				ret = -1;
				goto exit;
			}
			pir->m_dynphinfo_tmp[0] ++;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = begin;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] =(uintptr_t)&pir->m_dynlm[lmpos];
		}

		if(termcnt_between_mark[i] > 1 && (mark[i] - begin < cnt))
		{
			//将mark[i-1] 与mark[i]之间的term组成一个subphrse；			
			lmpos=create_ascii_dphrase(pwdict, pir, bpos, begin, mark[i] - begin);
			if(lmpos < 0)
			{
				WARNING_LOG("error: create_ascii_dphrase() failed");
				ret = -1;
				goto exit;
			}
			pir->m_dynphinfo_tmp[0] ++;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = begin;
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] =(uintptr_t)&pir->m_dynlm[lmpos];
		}

		//连接符及其左右的数字，组成subphrase
		if((i + 1 < mark_cnt) && (mark[i] > 0 )
				&& ((mark[i+1] - mark[i] > 2) || (mark[i] - begin >= 2)))
		{
			plm2 = (scw_lemma_t *)pir->m_dynphinfo[bpos + mark[i]];
			plm3 = (scw_lemma_t *)pir->m_dynphinfo[bpos + mark[i] + 2];
			if(IS_ASCIINUM(plm2->m_property)&& IS_ASCIINUM(plm3->m_property))
			{
				lmpos = create_ascii_dphrase(pwdict, pir, bpos, mark[i] - 1, 3);
				if(lmpos < 0)
				{
					WARNING_LOG("error: create_ascii_dphrase() failed");
					ret = -1;
					goto exit;
				}
				pir->m_dynphinfo_tmp[0] ++;
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = mark[i] - 1;
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] =(uintptr_t)&pir->m_dynlm[lmpos];
			}
		}

		if(dot_mark[mark[i]] == 0 && mark[i] != cnt)
		{
			vbpos = -1;
			vb_cnt = 0;
			//从这个开始，向后的非点的连接符之间的组成一个subphrase
			if(((i+1 < mark_cnt) && dot_mark[mark[i+1]] == 0) 
					|| ((i+2 < mark_cnt) && dot_mark[mark[i+2]] == 0))
				continue;
			for(j = i+3; j < mark_cnt; j ++)
			{
				if(dot_mark[mark[j]] == 0)
				{
					lmpos = create_ascii_dphrase( pwdict, pir, bpos, mark[i] + 1,mark[j] - mark[i] - 1);
					if(lmpos < 0)
					{
						WARNING_LOG("error: create_ascii_dphrase() failed");
						ret = -1;
						goto exit;
					}
					pir->m_dynphinfo_tmp[0] ++;
					pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = mark[i] + 1;
					pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++]=(uintptr_t)&pir->m_dynlm[lmpos];
					break;
				}
			}
		}
		else if((i + 1 < mark_cnt) && (mark[i+1] - mark[i] == 2) 
				&& (mark[i] - begin == 2 || mark[i]-begin == 1))
		{
			//fprintf(stderr,"this maybe a version %d\n",i);
			plm2 = (scw_lemma_t *)pir->m_dynphinfo[bpos + mark[i]];
			plm3 = (scw_lemma_t *)pir->m_dynphinfo[bpos + mark[i] + 2];
			if(IS_ASCIINUM(plm2->m_property)&& IS_ASCIINUM(plm3->m_property))
			{
				if(vbpos == -1)
				{
					vbpos = mark[i]-1;
					vb_cnt  = 1;
				}
				else
				{
					vb_cnt ++;
				}
			}
			else
			{
				vbpos = -1;
				vb_cnt = 0;
			}
		}
		else if(i+1 == mark_cnt)
		{
			continue;
		}
		else
		{
			vbpos = -1;
			vb_cnt = 0;
		}
	}
	if(vbpos > 0 && vb_cnt >= 2)
	{
		if(vbpos == 0)
		{
			vb_cnt -= 1;
		}
		j = vb_cnt;
		while(j >= 2)
		{
			vcnum = j*2+1;
			for(vb_begin=pir->m_dynph_curpos_tmp;vb_begin>1;vb_begin-=2)
			{
				if(pir->m_dynphinfo_tmp[vb_begin-2]==(u_int)vbpos)
				{
					plm2 = (scw_lemma_t*)pir->m_dynphinfo_tmp[vb_begin-1];
					if(vcnum==pir->m_dynphinfo[plm2->m_phinfo_bpos])
					{
						flag = 1;
						break;
					}
				}
			}
			if(flag == 1)
			{
				j--;
				flag = 0;
				continue;
			}
			lmpos = create_ascii_dphrase( pwdict, pir, bpos, vbpos,j*2+1);
			if(lmpos < 0)
			{
				WARNING_LOG("error: create_ascii_dphrase() failed");
				ret = -1;
				goto exit;
			}
			pir->m_dynphinfo_tmp[0] ++;
            		for(vb_begin=pir->m_dynph_curpos_tmp; vb_begin>1; vb_begin-=2)
            		{
                		if(pir->m_dynphinfo_tmp[vb_begin-2]==(u_int)vbpos)
                		{
                    			plm2 = (scw_lemma_t*)pir->m_dynphinfo_tmp[vb_begin-1];
                    			if(vcnum<=pir->m_dynphinfo[plm2->m_phinfo_bpos])
                    			{
                        			pir->m_dynphinfo_tmp[vb_begin] = vbpos;
                        			pir->m_dynphinfo_tmp[vb_begin+1] = (uintptr_t)&pir->m_dynlm[lmpos];
                        			break;
                    			}
                    			else
                    			{
                        			pir->m_dynphinfo_tmp[vb_begin] = pir->m_dynphinfo_tmp[vb_begin-2];
                        			pir->m_dynphinfo_tmp[vb_begin+1]=pir->m_dynphinfo_tmp[vb_begin-1]; 
                    			}
                		}
                		else if(pir->m_dynphinfo_tmp[vb_begin-2] < (u_int)vbpos)
                		{
                    			pir->m_dynphinfo_tmp[vb_begin] = vbpos;
                    			pir->m_dynphinfo_tmp[vb_begin+1] = (uintptr_t)&pir->m_dynlm[lmpos];
                    			break;
                		}
                		else
                		{
                    			pir->m_dynphinfo_tmp[vb_begin] = pir->m_dynphinfo_tmp[vb_begin-2];
                    			pir->m_dynphinfo_tmp[vb_begin+1]=pir->m_dynphinfo_tmp[vb_begin-1]; 
                		}
            		}
			if(vb_begin == 1)
			{	
                    		pir->m_dynphinfo_tmp[vb_begin] = vbpos;
                    		pir->m_dynphinfo_tmp[vb_begin+1] = (uintptr_t)&pir->m_dynlm[lmpos];
			}
			pir->m_dynph_curpos_tmp += 2;
			j--;
			//vcnum+=2;
		}
	}

	if(pir->m_dynphinfo_tmp[0] > 0)
	{
		int subphpos = plm->m_subphinfo_bpos;
		plm->m_subphinfo_bpos = pir->m_dynph_curpos;

		if(subphpos >= 0)
		{		
			// 需要按照offset的顺序输出，而且不能输出重复的。
			pir->m_dynphinfo[plm->m_subphinfo_bpos] = 0;
			pir->m_dynph_curpos ++ ;
			int tmppos = 1 + 2 * (pir->m_dynphinfo[subphpos]);//当前subphrase所在的位置
			i = 0;
			j = 0;

			if(pir->m_dynph_curpos+ pir->m_dynph_curpos_tmp + tmppos >= pir->m_dynph_size)
			{
				WARNING_LOG("error: m_dynph_curpos out of range");
				ret = -1;
				goto exit;
			}
			while((i < pir->m_dynphinfo[subphpos]) && (j < pir->m_dynphinfo_tmp[0]))
			{
				//先比较offset，然后比较子basic的term是否一样
				if(pir->m_dynphinfo[subphpos + 2*i + 1] > pir->m_dynphinfo_tmp[2*j + 1])
				{
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +1];
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +2];
					j ++ ;
					pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

				}
				else if(pir->m_dynphinfo[subphpos + 2*i + 1] == pir->m_dynphinfo_tmp[2*j + 1])
				{
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[subphpos+ 2*i + 2];
					plm3 = (scw_lemma_t*)pir->m_dynphinfo_tmp[2*j + 2];

					u_int bwcnt = 0;
					switch(plm2->m_type)
					{
						case LEMMA_TYPE_SPHRASE:
							bwcnt = pwdict->m_phinfo[plm2->m_phinfo_bpos];
							break;
						case LEMMA_TYPE_DPHRASE:
							bwcnt = pir->m_dynphinfo[plm2->m_phinfo_bpos];
							break;
						default:
							break;
					}

					//子basic数目一样，说明是一样的。
					if(bwcnt ==  pir->m_dynphinfo[plm3->m_phinfo_bpos])
					{
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 1];
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 2];
						pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

						i ++ ;
						j ++;
					}
					else if(bwcnt > pir->m_dynphinfo[plm3->m_phinfo_bpos])
					{
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 1];
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 2];
						pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

						i ++ ;
					}
					else
					{
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +1];
						pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo_tmp[2*j +2];
						j ++ ;
						pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;
					}

				}
				else
				{
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 1];
					pir->m_dynphinfo[pir->m_dynph_curpos ++] = pir->m_dynphinfo[subphpos + 2*i + 2];
					pir->m_dynphinfo[plm->m_subphinfo_bpos] ++;

					i ++ ;
				}
			}
			if(i < pir->m_dynphinfo[subphpos])
			{
				memcpy(pir->m_dynphinfo+pir->m_dynph_curpos,
						pir->m_dynphinfo+subphpos+i*2+1, sizeof(scw_lemma_t*)* (pir->m_dynphinfo[subphpos] - i) * 2 );
				pir->m_dynphinfo[plm->m_subphinfo_bpos] += pir->m_dynphinfo[subphpos] - i;
				pir->m_dynph_curpos += (pir->m_dynphinfo[subphpos] - i) * 2;
			}
			if(j < pir->m_dynphinfo_tmp[0])
			{
				memcpy(pir->m_dynphinfo+pir->m_dynph_curpos, pir->m_dynphinfo_tmp+1+j*2, sizeof(scw_lemma_t*)* (pir->m_dynphinfo_tmp[0] - j) * 2);
				pir->m_dynphinfo[plm->m_subphinfo_bpos] += pir->m_dynphinfo_tmp[0] - j;
				pir->m_dynph_curpos += (pir->m_dynphinfo_tmp[0] - j) * 2;
			}
		}
		else
		{	
			if(pir->m_dynph_curpos+ pir->m_dynph_curpos_tmp  >= pir->m_dynph_size)
			{
				WARNING_LOG("error: m_dynph_curpos out of range");
				ret = -1;
				goto exit;
			}
			memcpy(pir->m_dynphinfo+pir->m_dynph_curpos, pir->m_dynphinfo_tmp, sizeof(scw_lemma_t*)* pir->m_dynph_curpos_tmp);
			pir->m_dynph_curpos += pir->m_dynph_curpos_tmp ;
		}
	}

exit:
	if(mark)
	{
		free(mark);
		mark = NULL;
		dot_mark = NULL;
		termcnt_between_mark = NULL;
	}
	/*
	   if(phinfo)
	   {
	   free(phinfo);
	   phinfo = NULL;
	   }
	   */	
	return ret;
}

//  create_ascii_dphrase
/*========================================================================================
 * function : create a dynamic ascii phrase by a number of successive lemma in interal segged
 *       : result.
 * argu     : pdict, the dictionary
 *       : pir, the inner seg result struct
 *       : phrase_bpos, the begin position in phrase plm.
 *       : bpos, the begin position of basic word
 *       : count, how many successive lemmas does the dynamic phrase contain.
 * return  : >=0, successful,the positon of the dyanmaic lemma in pir->m_dynlm
 *       : <0, failed.
 ========================================================================================*/
int create_ascii_dphrase(scw_worddict_t* pwdict, scw_inner_t* pir, int phrase_bpos, int bpos, int count)
{
	if(pwdict == NULL || pir == NULL)
	{
		WARNING_LOG("error in create_ascii_dphrase, pwdict or pir is null!");
		return -1;
	}
	if((u_int)(bpos + count) > pir->m_dynphinfo[phrase_bpos])
	{
		WARNING_LOG("basic term out of range!");
		return -1;
	}

	int phlen=0;
	scw_lemma_t *plm=NULL, *plm2=NULL;
	char word[MAX_WORD_LEN] = "\0";

	if(pir->m_dynlm_cnt>=pir->m_dynlm_size)
	{
		WARNING_LOG("error: m_dynlm_cnt out of range");
		return -1;
	}
	if(pir->m_dynph_curpos+MAX_EXTEND_PHINFO_SIZE>=pir->m_dynph_size)
	{
		WARNING_LOG("error: m_dynph_curpos out of range");
		return -1;
	}

	plm= &pir->m_dynlm[pir->m_dynlm_cnt];
	plm->m_type    = LEMMA_TYPE_DPHRASE;
	INIT_PROPERTY(plm->m_property);
	SET_PHRASE(plm->m_property);
	SET_TEMP(plm->m_property);
	plm->m_weight    = 1;
	plm->m_subphinfo_bpos  = COMMON_NULL;
	plm->m_phinfo_bpos  = pir->m_dynph_curpos;
	plm->m_prelm_pos  = LEMMA_NULL;
	plm->m_utilinfo_pos  = COMMON_NULL;
	pir->m_dynph_curpos++;

	for(int i=0; i<count; i++)
	{
		plm2= (scw_lemma_t*)pir->m_dynphinfo[phrase_bpos+bpos+i+1];
		if(!plm2){
			continue;
		}
		switch(plm2->m_type)
		{
			case LEMMA_TYPE_SBASIC:
			case LEMMA_TYPE_DBASIC:
				pir->m_dynphinfo[pir->m_dynph_curpos++] = (uintptr_t)plm2;
				break;
			default:
				WARNING_LOG("error: type undefined");
				return -1;
		}	
		phlen += get_lmstr(pwdict, pir, plm2, word, sizeof(word));
	}

	if((u_int)phlen>=MAX_LEMMA_LENGTH)
		return -2;
	plm->m_length = phlen;

	int bwnum = pir->m_dynph_curpos - plm->m_phinfo_bpos-1;
	//assert(bwnum>1);
	if(bwnum <= 1)
	{
		WARNING_LOG("error: bwnum is invalid");
		return -1;
	}
	pir->m_dynphinfo[plm->m_phinfo_bpos]=bwnum;

	pir->m_dynlm_cnt++;

	return pir->m_dynlm_cnt-1;
}

/*!
 * 
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
int proc_pajm(scw_worddict_t * pwdict,scw_inner_t * pir, 
		const char * buf,int slen, int bpos, int lpos, int& newpos)
{  
	char word[MAX_LEMMA_LENGTH]={};
	int pos = bpos;
	int lmpos = 0;
	scw_lemma_t* plm = NULL;
	int pjmcnt = 0;
	int lmcnt = 0;
	int len;
	int ret;
	int lmbpos = pir->m_ppseg_cnt;

	while(pos+1 < slen
			&& is_gb_pajm((char *)(buf+pos)))
	{
		pos += 2;
		pjmcnt++;
		if(pjmcnt > 12){
			return 0;
		}
		if((pos-bpos)>=(int)MAX_LEMMA_LENGTH-4)
			break;
	}

	if(pos <= lpos)
		return 0;

	if(pjmcnt < 5)
	{
		memcpy(word, buf+bpos, pos-bpos);
		word[pos-bpos] = '\0';
		if((lmpos = create_dynword(pir,word))<0)
		{
			WARNING_LOG("error: create_dynword() failed");
			return -1;
		}

		if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
		{
			WARNING_LOG("error: append_lemma() failed");
			return -1;
		}

		plm = pir->m_ppseg[pir->m_ppseg_cnt-1];
		SET_PAJMSTR(plm->m_property);

		newpos = pos;
		return 1;
	}

	memcpy(word, buf+bpos, pos-bpos);
	word[pos-bpos] = '\0';
	len = pos-bpos;

	if(scw_seg_pajm(pwdict, pir, word, len) == -1)
	{
		pir->m_ppseg_cnt = lmbpos;
	}
	else
	{
		lmcnt = pir->m_ppseg_cnt-lmbpos;
		if(lmcnt>0)
		{
			if(lmcnt == 1)
			{
				newpos = pos;
				return 1;
			}
			else
			{
				if((ret = poly_lemmas(pwdict, pir,lmbpos,lmcnt)) < 0)
				{
					WARNING_LOG("error: poly_lemmas() failed");
					return -1;
				}

				plm = &pir->m_dynlm[ret];
				SET_PAJMSTR(plm->m_property);

				pir->m_ppseg_cnt -= lmcnt-1;
				newpos = pos;

				return 1;
			}
		}
	}

	if((lmpos = create_dynword(pir,word))<0)
	{
		WARNING_LOG("error: create_dynword() failed");
		return -1;
	}

	if(append_lemma(pwdict, pir, lmpos, SCW_DYNAMIC)<0)
	{
		WARNING_LOG("error: append_lemma() failed");
		return -1;
	}

	plm = pir->m_ppseg[pir->m_ppseg_cnt-1];
	SET_PAJMSTR(plm->m_property);

	newpos = pos;
	return 1;
}

//  proc_cnum
/*========================================================================================
 * function : Chinese number/time recgnition process on a string buffer. 
 * argu     : pdict, the dictionary
 *       : pir, the inner seg result struct
 *       : inbuf,inlen,  the string buffer and its length
 *       : pos, the begin position in src for ascii recgnition.
 *       : llmpos, the last lemma pos. newpos should be bigger than llmpos if the 
 *       :       function returned successfully.
 *       : newpos, the the process succeed, record the postion after the number/time
 *       : string
 * return  : 1, there is number/time string; -1, error
 ========================================================================================*/
int proc_cnum(scw_worddict_t * pwdict,scw_inner_t * pir, 
		const char* inbuf, int inlen, u_int pos, int lpos, int &newpos)
{
	char word[MAX_LEMMA_LENGTH]={};
	int oldpos=0, tmppos=0;
	scw_lemma_t *plm=NULL;
	int num_bpos=0;
	int wordval=0;

	num_bpos=oldpos= pos;
	wordval = scw_get_word(inbuf, (int&)pos, inlen, "");
	plm = get_oneword_lemma(pwdict, wordval);
	if(!plm || !IS_CNUMBER(plm->m_property))
		return 0;

	while(plm && IS_CNUMBER(plm->m_property))
	{
		oldpos=pos;
		wordval = scw_get_word(inbuf, (int&)pos, inlen, "");
		plm= get_oneword_lemma(pwdict, wordval);

		if((u_int)(oldpos-num_bpos)>=MAX_LEMMA_LENGTH-4)
			break;
	}

	// judege whether the dynamic lemma is longer than than origial 
	// static one
	if(oldpos<=lpos)
		return 0;

	int len = oldpos - num_bpos;
	memcpy(word, &inbuf[num_bpos], len);
	word[len]=0;

	if((tmppos = create_dynword(pir, word))<0)
	{
		WARNING_LOG("error: create_dynword() failed");
		return -1;
	}

	if(append_lemma(pwdict, pir, tmppos, SCW_DYNAMIC)<0)
	{
		WARNING_LOG("error: append_lemma() failed");
		return -1;
	}

	plm= pir->m_ppseg[pir->m_ppseg_cnt-1];
	SET_CNUMBER(plm->m_property);

	newpos=oldpos;

	return 1;
}



//  create_dphrase
/*========================================================================================
 * function : create a dynamic phrase by a number of successive lemma in interal segged
 *       : result.
 * argu     : pdict, the dictionary
 *       : pir, the inner seg result struct
 *       : bpos, the begin position in pir->m_ppseg.
 *       : count, how many successive lemmas does the dynamic phrase contain.
 * return  : >=0, successful,the positon of the dyanmaic lemma in pir->m_dynlm
 *       : <0, failed.
 ========================================================================================*/
int create_dphrase(scw_worddict_t* pwdict, scw_inner_t* pir, int bpos, int count)
{
	int phlen=0, cnt=0, lmpos=0;
	scw_lemma_t *plm=NULL, *plm2=NULL, *plm3=NULL;

	if(pir->m_dynlm_cnt>=pir->m_dynlm_size)
	{
		WARNING_LOG("error: m_dynlm_cnt out of range");
		return -1;
	}
	if(pir->m_dynph_curpos+MAX_EXTEND_PHINFO_SIZE>=pir->m_dynph_size)
	{
		WARNING_LOG("error: m_dynph_curpos out of range");
		return -1;
	}

	plm= &pir->m_dynlm[pir->m_dynlm_cnt];
	plm->m_type    = LEMMA_TYPE_DPHRASE;
	INIT_PROPERTY(plm->m_property);
	SET_PHRASE(plm->m_property);
	SET_TEMP(plm->m_property);
	plm->m_weight    = 1;
	for(int i=bpos;i<bpos+count; i++)
	{
		if(pir->m_ppseg[i] == NULL)
		{
			continue;
		}
		phlen+= pir->m_ppseg[i]->m_length;
	}
	plm->m_length    = phlen;
	plm->m_subphinfo_bpos  = COMMON_NULL;
	plm->m_phinfo_bpos  = pir->m_dynph_curpos;
	plm->m_prelm_pos  = LEMMA_NULL;
	plm->m_utilinfo_pos  = COMMON_NULL;
	pir->m_dynph_curpos++;

	for(int i=bpos; i<bpos+count; i++)
	{
		plm2= pir->m_ppseg[i];
		if(plm2 == NULL)
		{
			continue;
		}
		switch(plm2->m_type)
		{
			case LEMMA_TYPE_SBASIC:
			case LEMMA_TYPE_DBASIC:
				pir->m_dynphinfo[pir->m_dynph_curpos++] = (uintptr_t)plm2;
				break;

			case LEMMA_TYPE_SPHRASE:
				cnt = pwdict->m_phinfo[plm2->m_phinfo_bpos];
				for(int j=0;j<cnt;j++){
					lmpos=pwdict->m_phinfo[plm2->m_phinfo_bpos+j+1];
					plm3= &pwdict->m_lemmalist[lmpos];
					pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm3;
				}
				break;
			case LEMMA_TYPE_DPHRASE:
				cnt = pir->m_dynphinfo[plm2->m_phinfo_bpos];
				memcpy(pir->m_dynphinfo+pir->m_dynph_curpos, 
						pir->m_dynphinfo+plm2->m_phinfo_bpos+1,sizeof(scw_lemma_t*)*cnt);
				pir->m_dynph_curpos+= cnt;
				break;
			default:
				WARNING_LOG("error: type undefined");
				return -1;
		}
	}

	int bwnum = pir->m_dynph_curpos - plm->m_phinfo_bpos-1;
	//assert(bwnum>1);
	if(bwnum <= 1)
	{
		WARNING_LOG("error: bwnum is invalid");
		return -1;
	}
	pir->m_dynphinfo[plm->m_phinfo_bpos]=bwnum;

	pir->m_dynlm_cnt++;

	return pir->m_dynlm_cnt-1;
}


//  get_oneword_lemma
/*========================================================================================
 * function : the the lemma of a certain single word. 
 *       : 
 * argu    : pwdict, the dictionary
 *       : wordvalue, the value of the single word.
 * return  : NULL, the single word does not have a lemma
 *       : other, the lemma of the single word.
 ========================================================================================*/
scw_lemma_t* get_oneword_lemma(scw_worddict_t* pwdict, int wordvalue)
{
	u_int de=0;
	u_int lmpos=0;
	de = scw_seek_entry(pwdict, DENTRY_FIRST, wordvalue);
	if(de==DENTRY_NULL){
		return NULL;
	}

	lmpos=pwdict->m_dictentry[de].m_lemma_pos;
	if(lmpos==COMMON_NULL){
		return NULL;
	}
	else{
		return &pwdict->m_lemmalist[lmpos];
	}
}



//  poly_lemmas
/*========================================================================================
 * function : poly the successive n lemmas into a single dynamic phrase 
 * 
 * argu  : pwdict, the dictionary
 *       : pir, the inner seg result struct
 *       : bpos, the begining position of the successive lemma in pir->m_ppseg.
 *       : count, the number of the successive lemmas.
 * return  : >=0, the postion of the dynamic lemma in pir->m_dynlm
 *       : <0, failed.
 ========================================================================================*/
int poly_lemmas(scw_worddict_t* pwdict, scw_inner_t* pir, int bpos, int count,int bname)
{
	int phlen=0, i=0,j=0, type=0, pos=0;
	int bwcnt=0,spcnt=0, cnt=0;
	int newoff=0,offset=0;
	scw_lemma_t* plm=NULL, *plm2=NULL, *pcurlm=NULL;

	//assert(count>1);
	if(count <= 1)
	{
		WARNING_LOG("error: count is invalid");
		return -1;
	}
	if(pir->m_dynlm_cnt>=pir->m_dynlm_size)
	{
		WARNING_LOG("error: m_dynlm_cnt out of range");
		return -1;
	}
	if(pir->m_dynph_curpos+MAX_EXTEND_PHINFO_SIZE>=pir->m_dynph_size)
	{
		WARNING_LOG("error: m_dynph_curpos out of range");
		return -1;
	}

	// 长度判断，上移到这里
	for(i=0; i<count; i++)
	{
		if(pir->m_ppseg[bpos+i]!=NULL)
			phlen += pir->m_ppseg[bpos+i]->m_length;
	}
	if((u_int)phlen>=MAX_LEMMA_LENGTH)
		return -2;
	//上移done

	// add the phrase itself
	pcurlm=&(pir->m_dynlm[pir->m_dynlm_cnt]);
	pcurlm->m_type    = LEMMA_TYPE_DPHRASE;
	pcurlm->m_weight  = 1;
	pcurlm->m_subphinfo_bpos = COMMON_NULL;
	pcurlm->m_phinfo_bpos   = COMMON_NULL;
	INIT_PROPERTY(pcurlm->m_property);
	SET_PHRASE(pcurlm->m_property);
	SET_TEMP(pcurlm->m_property);

	pcurlm->m_length = phlen;

	// build basic word info
	pcurlm->m_phinfo_bpos  =   pir->m_dynph_curpos;
	pir->m_dynph_curpos++;
	for( i=bpos; i<bpos+count; i++ )
	{
		plm= pir->m_ppseg[i];
		if(plm==NULL)
			continue;

		type = plm->m_type;
		switch(type)
		{
			case LEMMA_TYPE_SBASIC:
			case LEMMA_TYPE_DBASIC:
				pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm;
				bwcnt++;
				break;

			case LEMMA_TYPE_SPHRASE:
				cnt= pwdict->m_phinfo[plm->m_phinfo_bpos];
				for(j=0;j<cnt;j++)
				{
					int lmpos = pwdict->m_phinfo[plm->m_phinfo_bpos+j+1];
					plm2= &pwdict->m_lemmalist[lmpos];
					pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm2;
				}
				bwcnt+= cnt;
				break;
			case LEMMA_TYPE_DPHRASE:
				cnt= pir->m_dynphinfo[plm->m_phinfo_bpos];
				memcpy(pir->m_dynphinfo+pir->m_dynph_curpos, 
						pir->m_dynphinfo+plm->m_phinfo_bpos+1, cnt*sizeof(scw_lemma_t*));
				pir->m_dynph_curpos+= cnt;
				bwcnt+= cnt;
				break;
			default:
				return -1;
		}
	}
	pir->m_dynphinfo[pcurlm->m_phinfo_bpos] = bwcnt;

	// build subphrase info
	pcurlm->m_subphinfo_bpos = pir->m_dynph_curpos;
	pir->m_dynph_curpos++;
	offset=0;
	for( i=bpos; i<bpos+count; i++ )
	{
		plm= pir->m_ppseg[i];
		if(plm==NULL)
			continue;

		type = plm->m_type;
		switch(type)
		{
			case LEMMA_TYPE_SBASIC:
			case LEMMA_TYPE_DBASIC:
				break;
			case LEMMA_TYPE_SPHRASE:
				pir->m_dynphinfo[pir->m_dynph_curpos++]=offset;
				pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm;
				spcnt++;
				if(plm->m_subphinfo_bpos!=COMMON_NULL)
				{
					cnt= pwdict->m_phinfo[plm->m_subphinfo_bpos];
					for(j=0;j<cnt;j++)
					{
						newoff=offset+pwdict->m_phinfo[plm->m_subphinfo_bpos+2*j+1];
						pos = pwdict->m_phinfo[plm->m_subphinfo_bpos+2*j+2];
						plm2= &pwdict->m_lemmalist[pos];
						pir->m_dynphinfo[pir->m_dynph_curpos++]=newoff;
						pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm2;
						spcnt++;
					}
				}
				break;
			case LEMMA_TYPE_DPHRASE:
				pir->m_dynphinfo[pir->m_dynph_curpos++]=offset;
				pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm;
				spcnt++;

				if(plm->m_subphinfo_bpos!=COMMON_NULL)
				{
					cnt= pir->m_dynphinfo[plm->m_subphinfo_bpos];
					for(j=0;j<cnt;j++)
					{
						newoff=offset+pir->m_dynphinfo[plm->m_subphinfo_bpos+2*j+1];
						plm2=(scw_lemma_t*)pir->m_dynphinfo[plm->m_subphinfo_bpos+2*j+2];
						pir->m_dynphinfo[pir->m_dynph_curpos++]=newoff;
						pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)plm2;
						spcnt++;
					}
				}
				break;
			default:
				return -1;
		}
		offset += get_lmoff_step(pwdict, pir, plm);
	}
	if(spcnt>0)
		pir->m_dynphinfo[pcurlm->m_subphinfo_bpos]= spcnt;
	else
	{
		pcurlm->m_subphinfo_bpos=COMMON_NULL;
		//  pir->m_dynph_curpos--;
	}

	pir->m_ppseg[bpos] = &pir->m_dynlm[pir->m_dynlm_cnt];
	for(i=bpos+1; i< bpos+count; i++)
		pir->m_ppseg[i] = NULL;

	pir->m_dynlm_cnt++;
	return pir->m_dynlm_cnt-1;
}

/*
 *	function	: phrase across mark(currently jap dadian limited) binding
 *	return		: -1, error accur; else succeed.
 */
int big_phrase_recg(scw_worddict_t* pwdict, scw_inner_t* pir)
{
	scw_lemma_t* plm=NULL;
	int lpos=-1;
	int rpos=-1;
	char word[MAX_WORD_LEN+1]={};
	int len = 0;
	int pajm_cnt = 0;

	for(int i=1; i<(int)pir->m_ppseg_cnt-1;i++)
	{
		plm = NULL;
		lpos = -1;
		rpos = -1;
		pajm_cnt = 0;

		plm = pir->m_ppseg[i];
		if(plm==NULL)
		{
			continue;
		}
		len = get_lmstr(pwdict, pir, plm, word, sizeof(word));

		if(len==4 && is_fourbit_point(word))
		{
			int j = i-1;
			while(j>=0)
			{
				plm = pir->m_ppseg[j];
				if(plm==NULL)
				{
					j--;
					continue;
				}
				len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
				if(IS_PAJMSTR(plm->m_property))
				{
					lpos = j;
					pajm_cnt += len;
					j--;
				}
				else if(len==4 && is_fourbit_point(word))
				{
					j--;
				}
				else
				{
					break;
				}
			}
			if(lpos < 0)
			{
				continue;
			}

			j = i+1;
			while(j<(int)pir->m_ppseg_cnt)
			{
				plm = pir->m_ppseg[j];
				if(plm==NULL)
				{
					j++;
					continue;
				}
				len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
				if(IS_PAJMSTR(plm->m_property))
				{
					rpos = j;
					pajm_cnt += len;
					j++;
				}
				else if(len==4 && is_fourbit_point(word))
				{
					j++;
				}
				else
				{
					break;
				}
			}
			if(rpos < 0)
			{
				continue;
			}

			if(pajm_cnt > 25)
			{
				continue;
			}
			int poly_ret = 0;
			poly_ret = poly_lemmas(pwdict, pir, lpos, rpos-lpos+1);
			if(poly_ret == -1){
				WARNING_LOG("error: poly_lemmas() failed");
				return -1;
			}

			//for A·······························B的超过最大词条合并长度情况
			if(poly_ret == -2)
			{
				return 1;
			}
			//超长处理 done

			plm = pir->m_ppseg[lpos];
			SET_BIG_PHRASE(plm->m_property);
			SET_PAJMSTR(plm->m_property);
		}
		// i++;
	}

	return 1;
}

/*
 *  function  : number with time/quantity binding
 *  return    : -1, error accur; else succeed.
 */
int tn_recg(scw_worddict_t* pwdict, scw_inner_t* pir)
{
	scw_lemma_t* pclm=NULL, *pnlm=NULL, *pllm=NULL;
	int lpos=0, npos=0; 

	for(int i=0; i<(int)pir->m_ppseg_cnt-1;i++)
	{
		pnlm=pllm=NULL;
		lpos=npos=0;
		pclm = pir->m_ppseg[i];

		// skip the none number lemma
		if(!pclm){
			continue;
		}
		if(!(IS_CNUMBER(pclm->m_property)||IS_ASCIINUM(pclm->m_property))){
			continue;
		}

		// find next lemma  
		npos = i+1;
		pnlm = pir->m_ppseg[npos];
		while(npos<(int)pir->m_ppseg_cnt && pnlm==NULL)
		{
			npos++;
			pnlm = pir->m_ppseg[npos];
		}
		// if the current lemma can not be combined with surroundings
		if(!(pnlm && (IS_TIME(pnlm->m_property)||IS_QUANTITY(pnlm->m_property))))
			continue;

		// find the previous lemma
		if(i>0)
		{
			lpos = i-1;
			pllm = pir->m_ppseg[lpos];
			while(lpos>0 && pllm==NULL)
			{
				lpos --;
				pllm = pir->m_ppseg[lpos];
			}
		}

		if(pllm && IS_DI(pllm->m_property))
		{
			int pos1=create_dphrase(pwdict, pir, lpos, i-lpos+1);
			int pos2=create_dphrase(pwdict, pir, i, npos-i+1);
			if(pos1<0 ||pos2<0)
			{
				WARNING_LOG("error: create_dphrase() failed");
				return -1;
			}

			if(poly_lemmas(pwdict, pir, lpos, npos-lpos+1)==-1)
			{
				WARNING_LOG("error: poly_lemmas() failed");
				return -1;
			}

			scw_lemma_t* plm=pir->m_ppseg[lpos];
			plm->m_subphinfo_bpos= pir->m_dynph_curpos;
			pir->m_dynphinfo[pir->m_dynph_curpos++]=2;
			pir->m_dynphinfo[pir->m_dynph_curpos++]=0;
			pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)&pir->m_dynlm[pos1];
			pir->m_dynphinfo[pir->m_dynph_curpos++]=1;
			pir->m_dynphinfo[pir->m_dynph_curpos++]=(uintptr_t)&pir->m_dynlm[pos2];

		}
		else
		{
			if(poly_lemmas(pwdict, pir, i, npos-i+1)== -1)
			{
				WARNING_LOG("error: poly_lemmas() failed");
				return -1;
			}
			pclm = pir->m_ppseg[i];
			if(IS_TIME(pnlm->m_property))
				SET_TIME(pclm->m_property);
		}

		i++;
	}
	return 1;
}

//
/*
   int * scw_get_nameprob(scw_worddict_t* pwdict,scw_lemma_t * plm,int type)
   {
   u_int utilpos = 0;
   u_int probpos = 0;

   if(plm == NULL || (utilpos = plm->m_utilinfo_pos) == COMMON_NULL)
   {
   return NULL;
   }

   switch(type)
   {
   case DATA_CNAME:
   probpos = pwdict->m_utilinfo[utilpos].m_cnameprob_pos;
   break;
   case DATA_JNAME:
   probpos = pwdict->m_utilinfo[utilpos].m_jnameprob_pos;
   break;
   case DATA_FNAME:
   probpos = pwdict->m_utilinfo[utilpos].m_fnameprob_pos;
   break;
   default:
   return NULL;
   }

   if(probpos == COMMON_NULL)
   {
   return NULL;
   }

   return pwdict->m_probbuf + probpos;
   }
   */

