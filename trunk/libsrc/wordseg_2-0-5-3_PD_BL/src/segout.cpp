/***************************************************************************
 * Filename: scwout.h -> scwdef.cpp
 * Descript: This is a defintion file for those functions used to output.
 * Author  : Paul Yang, zhenhaoji@sohu.com
 * Time    : 2004-12-31
 **************************************************************************/

#include <ul_log.h>
#include "property.h"
#include "scwdef.h"
#include "international.h"
extern int get_lmoff_step(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm);
extern int get_lmstr(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, char* dest, u_int destlen);
extern void reset_inner(scw_inner_t* pir);
extern u_int scw_seek_lemma(scw_worddict_t * pwdict,const char * term,int len);
extern int scw_insert_dynword(Sdict_build * pdynwdict,const char * term ,int len,u_int lmpos);
extern void write_prop_to_str(scw_property_t& property, char* buffer,int wdtype);

static inline int append_item_buf(scw_item_t* pitem, char* buff, int len, int offset, int isbasic);
// functions for scw_inner_t structure


static inline void reset_out(scw_out_t * pout, bool b_reset_pir=true);
static inline int write_buf(scw_worddict_t* pwdict, scw_out_t* pout);
static inline int append_buf_bword(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset);
static inline int append_buf_wpcomp(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset);
static inline int append_buf_subph(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset);
static inline int append_buf_humanname(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset);
static inline int append_buf_bookname(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset);
static inline int append_out_buf(scw_out_t* pout, char* buff, int len, int offset, int type);

// scw_create_out
/*========================================================================================
 * function : create  the output struct
 * argu     : tsize, total word count
 * return   : the pointer if success, NULL if failed
 ========================================================================================*/
scw_out_t * scw_create_out(u_int maxterm, int flag)
{
	const char* where="scw_create_out";
	scw_out_t * pout = NULL; 

	if((pout=(scw_out_t*)calloc(1,sizeof(scw_out_t)))==NULL)
		goto failed;

	if(flag & SCW_OUT_BASIC)
	{
		pout->wordsepbuf  = (char*)calloc(maxterm*10,sizeof(char));
		pout->wsbtermoffsets  = (u_int*)calloc(maxterm, sizeof(u_int));
		pout->wsbtermpos  = (u_int*)calloc(maxterm,sizeof(u_int));
		if(flag & SCW_OUT_PROP)
		{
			pout->wsbtermprop = (scw_property_t*)calloc(maxterm,sizeof(scw_property_t));
			if(!pout->wsbtermprop) goto failed;
		}
		if(!(pout->wordsepbuf && pout->wsbtermoffsets && pout->wsbtermpos))
			goto failed;
		pout->wsbsize = maxterm*10;
	}

	if(flag & SCW_OUT_WPCOMP)
	{
		pout->wpcompbuf  = (char*)calloc(maxterm*10,sizeof(char));
		pout->wpbtermoffsets  = (u_int*)calloc(maxterm, sizeof(u_int));
		pout->wpbtermpos  = (u_int*)calloc(maxterm,sizeof(u_int));
		if(flag & SCW_OUT_PROP)
		{
			pout->wpbtermprop = (scw_property_t*)calloc(maxterm,sizeof(scw_property_t));
			if(!pout->wpbtermprop) goto failed;
		}
		if(!(pout->wpcompbuf && pout->wpbtermoffsets && pout->wpbtermpos))
			goto failed;
		pout->wpbsize = maxterm*10;
	}

	if(flag & SCW_OUT_SUBPH)
	{
		pout->subphrbuf  = (char*)calloc(maxterm*10,sizeof(char));
		pout->spbtermoffsets  = (u_int*)calloc(maxterm, sizeof(u_int));
		pout->spbtermpos  = (u_int*)calloc(maxterm,sizeof(u_int));
		if(flag & SCW_OUT_PROP)
		{
			pout->spbtermprop = (scw_property_t*)calloc(maxterm,sizeof(scw_property_t));
			if(!pout->spbtermprop) goto failed;
		}
		if(!(pout->subphrbuf && pout->spbtermoffsets && pout->spbtermpos))
			goto failed;
		pout->spbsize = maxterm*10;
	}

	if(flag & SCW_OUT_HUMANNAME)
	{
		pout->namebuf    = (char*)calloc(maxterm*10,sizeof(char));
		pout->namebtermoffsets    = (u_int*)calloc(maxterm,sizeof(u_int));
		pout->namebtermpos  = (u_int*)calloc(maxterm,sizeof(u_int));
		if(flag & SCW_OUT_PROP)
		{
			pout->namebtermprop = (scw_property_t*)calloc(maxterm,sizeof(scw_property_t));
			if(!pout->namebtermprop) goto failed;
		}
		if(!(pout->namebuf && pout->namebtermoffsets&&pout->namebtermpos))
			goto failed;
		pout->namebsize = maxterm*10;
	}

	if(flag & SCW_OUT_BOOKNAME)
	{
		pout->booknamebuf  = (char*)calloc(maxterm*10, sizeof(char));
		pout->bnbtermoffsets  = (u_int*)calloc(maxterm, sizeof(u_int));
		pout->bnbtermpos  = (u_int*)calloc(maxterm, sizeof(u_int));

		if(flag & SCW_OUT_PROP)
		{
			pout->bnbtermprop = (scw_property_t*)calloc(maxterm,sizeof(scw_property_t));
			if(!pout->bnbtermprop) goto failed;
		}
		if(!(pout->booknamebuf&&pout->bnbtermoffsets&&pout->bnbtermpos))
			goto failed;
		pout->bnbsize = maxterm*10;
	}

	pout->mergebuf  = (char*)calloc(maxterm*10, sizeof(char));
	pout->mbtermoffsets  = (u_int*)calloc(maxterm, sizeof(u_int));
	pout->mbtermpos  = (u_int*)calloc(maxterm,sizeof(u_int));
	if(!(pout->mergebuf && pout->mbtermoffsets && pout->mbtermpos))
		goto failed;
	pout->mbsize    = maxterm*10;

	if((pout->m_pir= scw_create_inner(maxterm, flag))==NULL)
		goto failed;

	return pout;

failed:

	if(pout)
	{
		free(pout->wordsepbuf);
		free(pout->wsbtermoffsets);
		free(pout->wsbtermpos);
		free(pout->wsbtermprop);

		free(pout->wpcompbuf);
		free(pout->wpbtermoffsets);
		free(pout->wpbtermpos);
		free(pout->wpbtermprop);

		free(pout->subphrbuf);
		free(pout->spbtermoffsets);
		free(pout->spbtermpos);
		free(pout->spbtermprop);

		free(pout->namebuf);
		free(pout->namebtermoffsets);
		free(pout->namebtermpos);
		free(pout->namebtermprop);

		free(pout->booknamebuf);
		free(pout->bnbtermoffsets);
		free(pout->bnbtermpos);
		free(pout->bnbtermprop);

		free(pout->mergebuf);
		free(pout->mbtermoffsets);
		free(pout->mbtermpos);
	}

	ul_writelog(UL_LOG_FATAL, "error in %s calloc error\n", where);

	return NULL;
}


// reset_out
/*========================================================================================
 * function : reset the out struct
 * argu     : pout, the out structure
 ========================================================================================*/
void reset_out(scw_out_t * pout, bool b_reset_pir)
{
	char *where="reset_out";
	//assert(pout);
	if(!pout)
	{
		ul_writelog(UL_LOG_FATAL, "error in %s pout NULL\n", where);
		return ;
	}

	int flag= pout->m_pir->m_flag;

	if(b_reset_pir)
		reset_inner(pout->m_pir);

	if(flag & SCW_OUT_BASIC){
		pout->wordsepbuf[0]  = '\t';
		pout->wordsepbuf[1]  = 0;
		pout->wsb_curpos    = 1;
		pout->wsbtermcount    = 0;
		pout->wordtotallen  = 0;
	}

	if(flag & SCW_OUT_WPCOMP){
		pout->wpcompbuf[0]  = '\t';
		pout->wpcompbuf[1]  = 0;
		pout->wpb_curpos    = 1;
		pout->wpbtermcount    = 0;
	}

	if(flag & SCW_OUT_SUBPH){
		pout->subphrbuf[0]  = '\t';
		pout->subphrbuf[1]  = 0;
		pout->spb_curpos    = 1;
		pout->spbtermcount    = 0;
	}

	if(flag & SCW_OUT_HUMANNAME){
		pout->namebuf[0]    = '\t';
		pout->namebuf[1]    = 0;
		pout->nameb_curpos    = 1;
		pout->namebtermcount  = 0;
	}

	if(flag & SCW_OUT_BOOKNAME){
		pout->booknamebuf[0]  = '\t';
		pout->booknamebuf[1]  = 0;
		pout->bnb_curpos    = 1;
		pout->bnbtermcount    = 0;
	}

	pout->mergebuf[0]  = '\t';
	pout->mergebuf[1]  = 0;
	pout->mb_curpos    = 1;
	pout->mbtermcount  = 0;
	pout->phrase_merged = 0;

	return ;
}


// scw_destroy_out
/*========================================================================================
 * function : destroy the out struct
 * argu     : pout, the pointer to the out structure
 * return   : void
 ========================================================================================*/
void scw_destroy_out(scw_out_t* pout)
{
	char *where="scw_destroy_out";
	//assert(pout);
	if(!pout)
	{
		ul_writelog(UL_LOG_FATAL, "error in %s pout NULL\n", where);
		return ;
	}

	if(pout != NULL)
	{
		free(pout->wordsepbuf);
		free(pout->wsbtermoffsets);
		free(pout->wsbtermpos);
		free(pout->wsbtermprop);

		free(pout->wpcompbuf);
		free(pout->wpbtermoffsets);
		free(pout->wpbtermpos);
		free(pout->wpbtermprop);

		free(pout->subphrbuf);
		free(pout->spbtermoffsets);
		free(pout->spbtermpos);
		free(pout->spbtermprop);

		free(pout->namebuf);
		free(pout->namebtermoffsets);
		free(pout->namebtermpos);
		free(pout->namebtermprop);

		free(pout->booknamebuf);
		free(pout->bnbtermoffsets);
		free(pout->bnbtermpos);
		free(pout->bnbtermprop);

		free(pout->mergebuf);
		free(pout->mbtermoffsets);
		free(pout->mbtermpos);

		scw_destroy_inner(pout->m_pir);

		free(pout);
		pout=NULL;
	}
	return;
}


//  scw_segment_words
/*========================================================================================
 * function : segment the input buffer
 * argu     : pwdict, the worddict,
 *          : pout, the out struct
 *          : inbuf, the input buffer
 * return   : 1 if success, <0 if failed.
 ========================================================================================*/
int scw_segment_words(scw_worddict_t * pwdict,scw_out_t * pout,
		const char* inbuf,const int inlen, bool human_allow_blank)
{
	const char* where="scw_segment_words";

	reset_out(pout);

	if(scw_seg(pwdict, pout->m_pir, inbuf, inlen, human_allow_blank) < 0)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s scw_seg error\n", where);
		return  -1;
	}

	if(write_buf(pwdict, pout)<0)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s write_buf error\n", where);
		return -1;
	}

	return 1;
}

//    get_lgt_scw_segment_words
/*========================================================================================
 * function : get logistic result of scw_segment_words  **Jap wordseg only**
 * argu     : pwdict, the worddict,
 *          : pout, the out struct
 * return   : 1 if success, <0 if failed.
 * ==================================================================================*/
int get_lgt_scw_segment_words(scw_worddict_t * pwdict, scw_out_t * pout)
{
	const char* where="get_lgt_scw_segment_words";
	int ret = 0;

	ret = get_lgt_scw_seg(pwdict, pout->m_pir);
	if(ret == -1)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return  -1;
	}
	else if(ret == 0)
	{
		return 0;
	}
	else
	{
		reset_out(pout, false);
		if(write_buf(pwdict, pout)<0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		return 1;
	}
}

//    get_logic_word
/*========================================================================================
 * function : get logistic result of word   **Jap wordseg only**
 * argu     : word, the input string
 * return   : 0 if success, <0 if failed
 * ==================================================================================*/
int get_logic_word(char * str)
{
	int len = 0;
	int i = 0;
	int del_num = 0;
	u_char *tmpbuf = NULL;

	if(str == NULL)
	{
		return -1;
	}

	tmpbuf = (u_char *)str;
	len = strlen(str);
	i = 0;

	while(i < len)
	{
		if(tmpbuf[i] < 0x80)
		{
			if(str[i] == ' ')
			{
				del_num++;
			}
			else if(del_num > 0)
			{
				str[i-del_num] = str[i];
			}
			i++;
		}
		else if((i+1<len) &&(tmpbuf[i] >= 0x81 && tmpbuf[i] <= 0xfe)
				&& (tmpbuf[i+1] >= 0x40 && tmpbuf[i+1] <= 0xfe))
		{
			if(del_num > 0)
			{
				str[i-del_num] = str[i];
				str[i-del_num+1] = str[i+1];
			}
			i += 2;
		}
		else if((i+3<len) &&(tmpbuf[i] >= 0x81 && tmpbuf[i] <= 0xfe)
				&& (tmpbuf[i+1] >= 0x30 && tmpbuf[i+1] <= 0x39)
				&& (tmpbuf[i+2] >= 0x81 && tmpbuf[i+2] <= 0xfe)
				&& (tmpbuf[i+3] >= 0x30 && tmpbuf[i+3] <= 0x39))
		{
			if(tmpbuf[i]==0x81 && tmpbuf[i+1]==0x39 && tmpbuf[i+2]==0xA7 && tmpbuf[i+3]==0x39)
			{
				del_num += 4;
			}
			else if(del_num > 0)
			{
				str[i-del_num] = str[i];
				str[i-del_num+1] = str[i+1];
				str[i-del_num+2] = str[i+2];
				str[i-del_num+3] = str[i+3];
			}
			i += 4;
		}
		else
		{
			if(del_num > 0)
			{
				str[i-del_num] = str[i];
				str[i-del_num+1] = str[i+1];
			}
			i+=2;
		}
	}
	str[i-del_num] = '\0';

	return 0;
}

//      scw_get_blank_deleted
/*========================================================================================
 * function : indicate blank in japan human name **Jap wordseg only**
 * argu     : pwdict, the worddict
 *          : pout, the out struct
 *          : inbuf, the input buffer
 *          : inlen, the len of input buffer
 *          : blank_deleted, the blank saved array
 *          : blank_size, the size of blank_deleted
 * return   : >=0 then is the number of blank deleted, -1 if failed (==0 if in chinese segmentation)
 ========================================================================================*/
int scw_get_blank_deleted(scw_worddict_t * pwdict, scw_out_t * pout, const char * inbuf, const int inlen, u_int * blank_deleted, int blank_size)
{
	const char* where="scw_get_blank_deleted";
	u_int i=0, j=0;
	u_int toff;
	u_int tlen;
	int blank_num = 0;

	if(pwdict->m_wdtype == SCW_WD_CH)
	{
		return 0;
	}

	if(blank_deleted == NULL)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	while(i<pout->wsbtermcount && (int)j<inlen)
	{
		toff = GET_TERM_POS(pout->wsbtermpos[i]);
		tlen = GET_TERM_LEN(pout->wsbtermpos[i]);

		if(strncmp(inbuf+j, pout->wordsepbuf+toff, tlen) == 0)
		{
			i++;
			j += tlen;
			continue;
		}
		else if(inbuf[j] == ' ')
		{
			while(inbuf[j] == ' ')
			{
				if(blank_num > blank_size-1)
				{
					ul_writelog(UL_LOG_WARNING, "buffer [%s] blank_size too little [%d]\n", inbuf, blank_size);
					ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
					return blank_num;
				}
				blank_deleted[blank_num] = j;
				blank_num++;
				j++;
			}
			continue;
		}
		else
		{
			ul_writelog(UL_LOG_WARNING, "wordsegbuf differs with original buff [%s] i:%d j:%d\n", inbuf, i, j);
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
	}

	return blank_num;
}
//      }}}

//  write_buf
/*========================================================================================
 * function : write the word segment result to result buffers
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 * return   : 0,successful, <0 failed.
 ========================================================================================*/
int write_buf(scw_worddict_t* pwdict, scw_out_t* pout)
{
	const char *where="write_buf";
	scw_lemma_t* plm;
	u_int offset = 0;
	scw_inner_t* pir= pout->m_pir;


	u_int flag = pir->m_flag;

	for(u_int i=0; i<pir->m_ppseg_cnt; i++){
		plm = pir->m_ppseg[i];
		if(!plm)
			continue;

		if(flag & SCW_OUT_BASIC){
			if(append_buf_bword(pwdict, pout, plm, offset)<0){
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
		}
		if(flag & SCW_OUT_WPCOMP){
			if(append_buf_wpcomp(pwdict, pout, plm, offset)<0){
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
		}
		if(flag & SCW_OUT_SUBPH){
			if(append_buf_subph(pwdict, pout, plm,offset)<0){
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
		}
		if(flag & SCW_OUT_HUMANNAME){
			if(append_buf_humanname(pwdict, pout, plm, offset)<0){
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
		}
		if(flag & SCW_OUT_BOOKNAME){
			if(append_buf_bookname(pwdict, pout, plm, offset)<0){
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
		}

		offset += get_lmoff_step(pwdict,pir, plm);
	}

	pout->pnewword = pir->pnewword;

	return 0;
}



//  append_buf_bword
/*========================================================================================
 * function : append the basic words of a lemma to the basic word result buffer
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 *          : plm, the lemma to be appended
 *          : offset, the offset of the lemma
 * return   : 0,successful, <0 failed.
 ========================================================================================*/
int append_buf_bword(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset)
{
	const char* where="append_buf_bword";
	char word[MAX_WORD_LEN];
	int type=0, len=0,flag = 0;
	int cnt, i, pos;
	scw_lemma_t* plm2;
	scw_inner_t* pir;
	u_int bpos = 0;

	pir = pout->m_pir;

	if(plm==NULL)
		return 0;

	type= plm->m_type;
	flag = pout->m_pir->m_flag;
	switch(type){
		case LEMMA_TYPE_SBASIC:
		case LEMMA_TYPE_DBASIC:
			len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
			if(len < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
				return -1;
			}
			if(append_out_buf(pout, word, len, offset, SCW_OUT_BASIC)<0){
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			if(flag & SCW_OUT_PROP){
				memcpy(&pout->wsbtermprop[pout->wsbtermcount-1] ,&plm->m_property,sizeof(scw_property_t));
			}
			break;
		case LEMMA_TYPE_SPHRASE:
			bpos = plm->m_phinfo_bpos;
			cnt = pwdict->m_phinfo[bpos];
			for(i=0; i<cnt; i++){
				pos = pwdict->m_phinfo[bpos+i+1];
				plm2= &pwdict->m_lemmalist[pos];
				len = get_lmstr(pwdict, pir, plm2, word, sizeof(word));
				if(len < 0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
					return -1;
				}
				if(append_out_buf(pout, word,len, offset+i, SCW_OUT_BASIC)<0){
					ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
					return -1;
				}
				if(flag & SCW_OUT_PROP){
					memcpy(&pout->wsbtermprop[pout->wsbtermcount-1], &plm2->m_property,sizeof(scw_property_t));
				}
			}
			break;
		case LEMMA_TYPE_DPHRASE:
			bpos = plm->m_phinfo_bpos;
			cnt = pir->m_dynphinfo[bpos];
			for(i=0; i<cnt; i++){
				plm2= (scw_lemma_t*)pir->m_dynphinfo[bpos+i+1];
				len = get_lmstr(pwdict, pir, plm2, word, sizeof(word));
				if(len < 0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
					return -1;
				}
				if(append_out_buf(pout, word,len, offset+i, SCW_OUT_BASIC)<0){
					ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
					return -1;
				}
				if(flag & SCW_OUT_PROP){
					memcpy(&pout->wsbtermprop[pout->wsbtermcount-1], &plm2->m_property,sizeof(scw_property_t));
				}
				offset+= get_lmoff_step(pwdict, pir, plm2);
			}
			break;

		default:
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
	}

	return 0;

}


//  append_buf_bword_item
/*========================================================================================
 * function : append the basic words of a lemma to the basic word result buffer
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 *          : plm, the lemma to be appended
 *          : offset, the offset of the lemma
 * return   : 0,successful, <0 failed.
 ========================================================================================*/
int append_buf_bword_item(scw_item_t* pitem, scw_worddict_t* pwdict, 
		scw_inner_t* pir, scw_lemma_t* plm, u_int offset)
{
	const char* where="append_buf_bword_item";
	char word[MAX_WORD_LEN];
	int type, len=0,flag = 0;
	int cnt, i, pos;
	scw_lemma_t* plm2;
	u_int bpos = 0;

	if(plm==NULL || pitem->m_tmcnt+1 > pir->m_maxterm)
		return 0;

	type = plm->m_type;
	flag = pir->m_flag;

	switch(type)
	{
		case LEMMA_TYPE_SBASIC:
		case LEMMA_TYPE_DBASIC:
			len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
			if(len < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
				return -1;
			}
			if(append_item_buf(pitem, word,len, offset, 1) < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			if(flag & SCW_OUT_PROP)
			{
				memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1], &plm->m_property,sizeof(scw_property_t));
			}
			break;

		case LEMMA_TYPE_SPHRASE:
			bpos = plm->m_phinfo_bpos;
			cnt = pwdict->m_phinfo[bpos];
			for(i=0; i<cnt; i++)
			{
				pos = pwdict->m_phinfo[bpos+i+1];
				plm2= &pwdict->m_lemmalist[pos];
				len = get_lmstr(pwdict, pir, plm2, word, sizeof(word));
				if(len < 0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
					return -1;
				}
				if(append_item_buf(pitem, word,len,offset+i, 1)<0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
					return -1;
				}
				if(flag & SCW_OUT_PROP)
				{
					memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1] ,&plm2->m_property,sizeof(scw_property_t));
				}
			}
			break;

		case LEMMA_TYPE_DPHRASE:
			bpos = plm->m_phinfo_bpos;
			cnt = pir->m_dynphinfo[bpos];
			for(i=0; i<cnt; i++)
			{
				plm2= (scw_lemma_t*)pir->m_dynphinfo[bpos+i+1];
				len = get_lmstr(pwdict, pir, plm2, word, sizeof(word));
				if(len < 0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
					return -1;
				}
				if(append_item_buf(pitem, word,len,offset+i, 1)<0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
					return -1;
				}

				if(flag & SCW_OUT_PROP)
				{
					memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1], &plm2->m_property,sizeof(scw_property_t));
				}
				offset+= get_lmoff_step(pwdict, pir, plm2);
			}
			break;

		default:
			ul_writelog(UL_LOG_WARNING, "error in %s type undefined\n", where);
			return -1;
	}

	return 0;

}



//  append_buf_wpcomp
/*========================================================================================
 * function : append the phrase string of a lemma to the wpcomp buffer
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 *          : plm, the lemma to be appended
 *          : offset, the offset of the lemma
 * return   : 0,successful, <0 failed.
 ========================================================================================*/
int append_buf_wpcomp(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset)
{
	const char* where="append_buf_wpcomp";
	char lmstr[MAX_WORD_LEN];
	int len=0;
	int flag = pout->m_pir->m_flag;

	if(plm==NULL)
		return 0;

	len = get_lmstr(pwdict, pout->m_pir, plm, lmstr, sizeof(lmstr));
	if(len < 0)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
		return -1;
	}
	if(append_out_buf(pout, lmstr,len, offset, SCW_OUT_WPCOMP)<0)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	if(flag & SCW_OUT_PROP)
	{
		memcpy(&pout->wpbtermprop[pout->wpbtermcount-1], &plm->m_property,sizeof(scw_property_t));
	}

	return 0;
}


int append_buf_wpcomp_item(scw_item_t* pitem, scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, int offset)
{
	const char* where="append_buf_wpcomp_item";
	char lmstr[MAX_WORD_LEN];
	int len = 0;
	int flag = pir->m_flag;

	if(plm==NULL || pitem->m_tmcnt+1 > pir->m_maxterm)
		return 0;

	len = get_lmstr(pwdict, pir, plm, lmstr, sizeof(lmstr));
	if(len < 0)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
		return -1;
	}
	if(append_item_buf(pitem, lmstr,len, offset, 0)<0)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	if(flag & SCW_OUT_PROP)
	{  
		memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1], &plm->m_property,sizeof(scw_property_t));
	}
	return 0;
}

//  append_buf_subph
/*========================================================================================
 * function : if it's a phrase lemma, append the phrase together with its subphrase to 
 *       : the subphrase result buffer.
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 *          : plm, the lemma to be appended
 *          : offset, the offset of the lemma
 * return   : 0,successful, <0 failed.
 ========================================================================================*/
int append_buf_subph(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset)
{
	const char* where="append_buf_subph";
	char lmstr[MAX_WORD_LEN];
	int cnt, i, newoff, pos, len=0;
	scw_lemma_t* plm2;
	scw_inner_t* pir;
	u_int bpos = 0;
	int flag = 0;

	pir = pout->m_pir;
	flag = pir->m_flag;

	if(plm==NULL)
		return 0;

	if(IS_HUMAN_NAME(plm->m_property) && IS_PHRASE(plm->m_property))
	{
		len = get_lmstr(pwdict, pir, plm, lmstr, sizeof(lmstr));
		if(len < 0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
			return -1;
		}
		if(append_out_buf(pout, lmstr,len, offset, SCW_OUT_SUBPH)<0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		if(flag & SCW_OUT_PROP)
		{
			memcpy(&pout->spbtermprop[pout->spbtermcount-1],&plm->m_property,sizeof(scw_property_t));
		}
		return 0;
	}

	int type = plm->m_type;
	switch(type)
	{
		case LEMMA_TYPE_SBASIC:
		case LEMMA_TYPE_DBASIC:
			return 0;
		case LEMMA_TYPE_SPHRASE:
			len = get_lmstr(pwdict, pir, plm, lmstr, sizeof(lmstr));
			if(len < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
				return -1;
			}
			if(append_out_buf(pout, lmstr, len, offset, SCW_OUT_SUBPH)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			if(flag & SCW_OUT_PROP)
			{
				memcpy(&pout->spbtermprop[pout->spbtermcount-1],&plm->m_property,sizeof(scw_property_t));
			}
			if((bpos = plm->m_subphinfo_bpos)!= COMMON_NULL)
			{
				cnt = pwdict->m_phinfo[bpos];
				int *mark = NULL;
				int mark_cnt = 0;
				int j=0;
				mark = (int*)malloc(sizeof(int)*3*(cnt+1));
				if(mark == NULL)
				{
					ul_writelog(UL_LOG_WARNING, "error:malloc memory for mark failed in %s\n", where);
					return -1;
				}
				for(i=0;i<cnt;i++)
				{
					newoff = pwdict->m_phinfo[bpos+i*2+1];
					pos = pwdict->m_phinfo[bpos+i*2+2];
					plm2 = &pwdict->m_lemmalist[pos];
					len = get_lmstr(pwdict, pir, plm2, lmstr,sizeof(lmstr));
					if(len < 0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
						return -1;
					}
					if(mark_cnt == 0)
					{
						mark[0] = newoff;
						mark[1] = len;
						mark[2] = pos;
						mark_cnt += 3;
						continue;
					}
					for(j=mark_cnt; j>0; j-=3)
					{
						if(mark[j-3] == newoff)
						{
							if(len <= mark[j-2])
							{
								mark[j] = newoff;
								mark[j+1] = len;
								mark[j+2] = pos;
								break;
							}
							else
							{
								mark[j] = mark[j-3];
								mark[j+1] = mark[j-2];
								mark[j+2] = mark[j-1];
							}
						}
						else if(mark[j-3] < newoff)
						{
							mark[j] = newoff;
							mark[j+1] = len;
							mark[j+2] = pos;
							break;
						}
						else
						{
							mark[j] = mark[j-3];
							mark[j+1] = mark[j-2];
							mark[j+2] = mark[j-1];
						}
					}
					if(j == 0)
					{
						mark[j] = newoff;
						mark[j+1] = len;
						mark[j+2] = pos;
					}
					mark_cnt += 3;
				}
				for(i=0;i<mark_cnt;i+=3){
					//newoff = offset+pwdict->m_phinfo[bpos+i*2+1];
					//pos = pwdict->m_phinfo[bpos+i*2+2];
					newoff = offset + mark[i];
					pos = mark[i+2];
					plm2 = &pwdict->m_lemmalist[pos];
					len = get_lmstr(pwdict, pir, plm2, lmstr,sizeof(lmstr));
					if(len < 0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
						return -1;
					}
					if(append_out_buf(pout, lmstr,len, newoff, SCW_OUT_SUBPH)<0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
						return -1;
					}
					if(flag & SCW_OUT_PROP)
					{
						memcpy(&pout->spbtermprop[pout->spbtermcount-1], &plm2->m_property,sizeof(scw_property_t));
					}
				}
				if(mark)
				{
					free(mark);
					mark = NULL;
				}
			}
			break;

		case LEMMA_TYPE_DPHRASE:
			len = get_lmstr(pwdict, pir, plm, lmstr, sizeof(lmstr));
			if(len < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
				return -1;
			}
			if(append_out_buf(pout, lmstr,len, offset, SCW_OUT_SUBPH)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			if(flag & SCW_OUT_PROP)
			{
				memcpy(&pout->spbtermprop[pout->spbtermcount-1], &plm->m_property,sizeof(scw_property_t));
			}
			if((bpos = plm->m_subphinfo_bpos)!=COMMON_NULL)
			{
				cnt = pir->m_dynphinfo[bpos];
				int *mark = NULL;
				int mark_cnt = 0;
				int j=0;
				mark = (int*)malloc(sizeof(int)*3*(cnt+1));
				if(mark == NULL)
				{
					ul_writelog(UL_LOG_WARNING, "error:malloc memory for mark failed in %s\n", where);
					return -1;
				}
				for(i=0;i<cnt;i++)
				{
					newoff = pir->m_dynphinfo[bpos+i*2+1];
					pos = bpos+i*2+2;
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[bpos+i*2+2];
					len = get_lmstr(pwdict, pir, plm2, lmstr,sizeof(lmstr));
					if(len < 0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
						return -1;
					}
					if(mark_cnt == 0)
					{
						mark[0] = newoff;
						mark[1] = len;
						mark[2] = pos;
						mark_cnt += 3;
						continue;
					}
					for(j=mark_cnt; j>0; j-=3)
					{
						if(mark[j-3] == newoff)
						{
							if(len <= mark[j-2])
							{
								mark[j] = newoff;
								mark[j+1] = len;
								mark[j+2] = pos;
								break;
							}
							else
							{
								mark[j] = mark[j-3];
								mark[j+1] = mark[j-2];
								mark[j+2] = mark[j-1];
							}
						}
						else if(mark[j-3] < newoff)
						{
							mark[j] = newoff;
							mark[j+1] = len;
							mark[j+2] = pos;
							break;
						}
						else
						{
							mark[j] = mark[j-3];
							mark[j+1] = mark[j-2];
							mark[j+2] = mark[j-1];
						}
					}
					if(j == 0)
					{
						mark[j] = newoff;
						mark[j+1] = len;
						mark[j+2] = pos;
					}
					mark_cnt += 3;
				}
				for(i=0;i<mark_cnt;i+=3){
					//newoff = offset+pwdict->m_phinfo[bpos+i*2+1];
					//pos = pwdict->m_phinfo[bpos+i*2+2];
					newoff = offset + mark[i];
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[mark[i+2]];
					len = get_lmstr(pwdict, pir, plm2, lmstr,sizeof(lmstr));
					if(len < 0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
						return -1;
					}
					if(append_out_buf(pout, lmstr,len, newoff, SCW_OUT_SUBPH)<0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
						return -1;
					}
					if(flag & SCW_OUT_PROP)
					{
						memcpy(&pout->spbtermprop[pout->spbtermcount-1], &plm2->m_property,sizeof(scw_property_t));
					}
				}
				if(mark)
				{
					free(mark);
					mark = NULL;
				}
			}
			break;
		default:
			ul_writelog(UL_LOG_WARNING, "error in %s type undefined\n", where);
			return -1;
	}

	return 0;
}


int append_buf_subph_item(scw_item_t* pitem, scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, int offset)
{
	const char* where="append_buf_subph_item";
	char lmstr[MAX_WORD_LEN];
	int cnt, i, newoff, pos, len=0;
	scw_lemma_t* plm2;
	u_int bpos = 0;
	int flag = 0;

	if(plm==NULL || pitem->m_tmcnt+1 > pir->m_maxterm)
		return 0;
	flag = pir->m_flag;

	if(IS_HUMAN_NAME(plm->m_property) && IS_PHRASE(plm->m_property))
	{
		len = get_lmstr(pwdict, pir, plm, lmstr, sizeof(lmstr));
		if(len < 0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
			return -1;
		}
		if(append_item_buf(pitem, lmstr,len, offset, 0)<0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		if(flag & SCW_OUT_PROP)
		{
			memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1] , &plm->m_property,sizeof(scw_property_t));
		}
		return 0;
	}

	int type = plm->m_type;
	switch(type)
	{
		case LEMMA_TYPE_SBASIC:
		case LEMMA_TYPE_DBASIC:
			return 0;
		case LEMMA_TYPE_SPHRASE:
			len = get_lmstr(pwdict, pir, plm, lmstr, sizeof(lmstr));
			if(len < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
				return -1;
			}
			if(append_item_buf(pitem, lmstr, len, offset, 0)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			if(flag & SCW_OUT_PROP)
			{
				memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1] , &plm->m_property,sizeof(scw_property_t));
			}
			if((bpos = plm->m_subphinfo_bpos)!= COMMON_NULL)
			{
				cnt = pwdict->m_phinfo[bpos];
				for(i=0;i<cnt;i++)
				{
					newoff = offset+pwdict->m_phinfo[bpos+i*2+1];
					pos = pwdict->m_phinfo[bpos+i*2+2];
					plm2 = &pwdict->m_lemmalist[pos];
					len = get_lmstr(pwdict, pir, plm2, lmstr,sizeof(lmstr));
					if(len < 0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
						return -1;
					}
					if(append_item_buf(pitem, lmstr,len, newoff, 0)<0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
						return -1;
					}
					if(flag & SCW_OUT_PROP)
					{
						memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1] , &plm2->m_property,sizeof(scw_property_t));
					}  
				}
			}
			break;

		case LEMMA_TYPE_DPHRASE:
			len = get_lmstr(pwdict, pir, plm, lmstr, sizeof(lmstr));
			if(len < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
				return -1;
			}
			if(append_item_buf(pitem, lmstr,len, offset, 0)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			if(flag & SCW_OUT_PROP)
			{
				memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1],&plm->m_property,sizeof(scw_property_t));
			}
			if((bpos = plm->m_subphinfo_bpos)!=COMMON_NULL)
			{
				cnt = pir->m_dynphinfo[bpos];
				for(i=0; i<cnt; i++)
				{
					newoff = offset+ pir->m_dynphinfo[bpos+i*2+1];
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[bpos+i*2+2];
					len = get_lmstr(pwdict, pir, plm2, lmstr,sizeof(lmstr));
					if(len < 0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
						return -1;
					}
					if(append_item_buf(pitem, lmstr,len, newoff, 0)<0)
					{
						ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
						return -1;
					}
					if(flag & SCW_OUT_PROP)
					{
						memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1], &plm2->m_property,sizeof(scw_property_t));
					}  
				}
			}
			break;
		default:
			ul_writelog(UL_LOG_WARNING, "error in %s type undefined\n", where);
			return -1;
	}

	return 0;
}

//  append_buf_humanname
/*========================================================================================
 * function : if the lemma contain human name, write humanname to the human name result 
 *       : buffer.
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 *          : plm, the lemma to be appended
 *          : offset, the offset of the lemma
 * return   : 0,successful, <0 failed.
 ========================================================================================*/
int append_buf_humanname(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset)
{
	const char* where="append_buf_humanname";
	char name[MAX_WORD_LEN];
	int cnt,i,newoff, pos, len=0;
	scw_lemma_t* plm2;
	scw_inner_t *pir = pout->m_pir;
	u_int bpos = 0;
	int flag = 0;

	if(plm==NULL)
		return 0;

	flag = pout->m_pir->m_flag;

	if(IS_HUMAN_NAME(plm->m_property))
	{
		len = get_lmstr(pwdict, pir, plm, name, sizeof(name));
		if(len < 0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
			return -1;
		}
		if(append_out_buf(pout, name, len,offset, SCW_OUT_HUMANNAME)<0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		if(flag & SCW_OUT_PROP)
		{
			memcpy(&pout->namebtermprop[pout->namebtermcount-1] , &plm->m_property,sizeof(scw_property_t));
		}
	}

	else if((bpos = plm->m_subphinfo_bpos)!=COMMON_NULL)
	{
		int type=plm->m_type;
		switch(type)
		{
			case LEMMA_TYPE_SPHRASE:
				cnt = pwdict->m_phinfo[bpos];
				for(i=0; i<cnt; i++)
				{
					newoff = offset+pwdict->m_phinfo[bpos+2*i+1];
					pos = pwdict->m_phinfo[bpos+2*i+2];
					plm2 = &pwdict->m_lemmalist[pos];
					if(IS_HUMAN_NAME(plm2->m_property))
					{
						len = get_lmstr(pwdict, pir, plm2, name, sizeof(name));
						if(len < 0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
							return -1;
						}
						if(append_out_buf(pout, name,len, newoff, SCW_OUT_HUMANNAME)<0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
							return -1;
						}

						if(flag & SCW_OUT_PROP)
						{
							memcpy(&pout->namebtermprop[pout->namebtermcount-1] , &plm2->m_property,sizeof(scw_property_t));
						}
					}
				}
				break;
			case LEMMA_TYPE_DPHRASE:
				cnt = pir->m_dynphinfo[bpos];
				for(i=0;i<cnt;i++){
					newoff = offset+pir->m_dynphinfo[bpos+2*i+1];
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[bpos+2*i+2];
					if(IS_HUMAN_NAME(plm2->m_property))
					{
						len = get_lmstr(pwdict, pir, plm2, name, sizeof(name));
						if(len < 0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
							return -1;
						}
						if(append_out_buf(pout, name,len, newoff, SCW_OUT_HUMANNAME)<0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
							return -1;
						}
						if(flag & SCW_OUT_PROP)
						{
							memcpy(&pout->namebtermprop[pout->namebtermcount-1], &plm2->m_property,sizeof(scw_property_t));
						}
					}
				}
				break;
			default:
				return 0;
		}
	}
	else
	{
		return 0;
	}

	return 0;
}

int append_buf_humanname_item(scw_item_t* pitem, scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, int offset)
{
	const char* where="append_buf_humanname_item";
	char name[MAX_WORD_LEN];
	int cnt,i,newoff, pos, len=0;
	scw_lemma_t* plm2;
	u_int bpos = 0;
	int flag =  0;

	if(plm==NULL || pitem->m_tmcnt+1 > pir->m_maxterm)
		return 0;

	flag = pir->m_flag;

	if(IS_HUMAN_NAME(plm->m_property))
	{
		len = get_lmstr(pwdict, pir, plm, name, sizeof(name));
		if(len < 0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
			return -1;
		}
		if(append_item_buf(pitem, name, len,offset, 0)<0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		if(flag & SCW_OUT_PROP)
		{
			memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1] ,&plm->m_property,sizeof(scw_property_t));
		}
	}
	else if((bpos = plm->m_subphinfo_bpos)!=COMMON_NULL)
	{
		int type=plm->m_type;
		switch(type)
		{
			case LEMMA_TYPE_SPHRASE:
				cnt = pwdict->m_phinfo[bpos];
				for(i=0; i<cnt; i++)
				{
					newoff = offset+pwdict->m_phinfo[bpos+2*i+1];
					pos = pwdict->m_phinfo[bpos+2*i+2];
					plm2 = &pwdict->m_lemmalist[pos];
					if(IS_HUMAN_NAME(plm2->m_property))
					{
						len = get_lmstr(pwdict, pir, plm2, name, sizeof(name));
						if(len < 0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
							return -1;
						}
						if(append_item_buf(pitem, name,len, newoff, 0)<0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
							return -1;
						}
						if(flag & SCW_OUT_PROP)
						{
							memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1], &plm2->m_property,sizeof(scw_property_t));
						}
					}
				}
				break;
			case LEMMA_TYPE_DPHRASE:
				cnt = pir->m_dynphinfo[bpos];
				for(i=0;i<cnt;i++){
					newoff = offset+pir->m_dynphinfo[bpos+2*i+1];
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[bpos+2*i+2];
					if(IS_HUMAN_NAME(plm2->m_property)){
						len = get_lmstr(pwdict, pir, plm2, name, sizeof(name));
						if(len < 0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
							return -1;
						}
						if(append_item_buf(pitem, name,len, newoff, 0)<0){
							ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
							return -1;
						}
						if(flag & SCW_OUT_PROP){
							memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1], &plm2->m_property,sizeof(scw_property_t));
						}
					}
				}
				break;
			default:
				return 0;
		}
	}
	else{
		return 0;
	}

	return 0;

}

//  append_buf_bookname
/*========================================================================================
 * function : if the lemma is a bookname, write bookname to the bookname result buffer.
 * argu     : pwdict, the worddict for segment
 *          : pout, the output struct
 *          : plm, the lemma to be appended
 *          : offset, the offset of the lemma
 * return   : 0,successful, <0 failed.
 ========================================================================================*/
int append_buf_bookname(scw_worddict_t* pwdict, scw_out_t* pout, scw_lemma_t* plm, u_int offset)
{
	const char* where="append_buf_bookname";
	char bookname[MAX_WORD_LEN];
	int len=0;
	int flag = 0;

	if(plm==NULL)
		return 0;

	flag = pout->m_pir->m_flag;

	if(IS_BOOKNAME(plm->m_property))
	{
		len = get_lmstr(pwdict,pout->m_pir, plm, bookname, sizeof(bookname));
		if(len < 0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
			return -1;
		}
		if(append_out_buf(pout, bookname,len, offset, SCW_OUT_BOOKNAME)<0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		if(flag & SCW_OUT_PROP)
		{
			memcpy(&pout->bnbtermprop[pout->bnbtermcount-1] , &plm->m_property,sizeof(scw_property_t));
		}
	}

	return 0;
}


int append_buf_bookname_item(scw_item_t* pitem, scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, int offset)
{
	const char* where="append_buf_bookname_item";
	char bookname[MAX_WORD_LEN];
	int len=0;
	int flag = 0;

	if(plm==NULL || pitem->m_tmcnt+1 > pir->m_maxterm)
		return 0;

	flag = pir->m_flag;

	if(IS_BOOKNAME(plm->m_property))
	{
		len = get_lmstr(pwdict,pir, plm, bookname, sizeof(bookname));
		if(len < 0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
			return -1;
		}
		if(append_item_buf(pitem, bookname,len, offset, 0)<0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		if(flag & SCW_OUT_PROP)
		{
			memcpy(&pitem->m_tmprop[pitem->m_tmcnt-1],&plm->m_property,sizeof(scw_property_t));
		}
	}

	return 0;

}

//  append_out_buf
/*========================================================================================
 * function : append a string to certain buffer of the output struct.
 * argu     : pout, the output struct
 *          : buff, the string to be appended
 *          : offset, the string's offset in the certain buffer
 *          : type: denotes which buffer the certain buffer is
 * return   : 0,successful; <0 failed.
 ========================================================================================*/
int append_out_buf(scw_out_t* pout, char* buff, int len, int offset, int type)
{
	const char* where="append_out_buf";

	if(type & SCW_OUT_BASIC)
	{    // 检查是否越界
		if(pout->wsb_curpos + len + 1 >= pout->wsbsize || pout->wsbtermcount+1 > pout->m_pir->m_maxterm)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s wsb_curpos out of range\n", where);
			return -1;
		}
		//assert(pout->wordsepbuf[pout->wsb_curpos]==0);
		if(pout->wordsepbuf[pout->wsb_curpos]!=0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s wsb_curpos of wordsepbuf error\n", where);
			return -1;    
		}
		pout->wsbtermpos[pout->wsbtermcount] = ((len) << 24 | pout->wsb_curpos & 0x00FFFFFF);
		strncpy(pout->wordsepbuf + pout->wsb_curpos, buff,len);
		pout->wsb_curpos += len;
		pout->wordsepbuf[pout->wsb_curpos++] = '\t';
		pout->wordsepbuf[pout->wsb_curpos] = 0;
		pout->wsbtermoffsets[pout->wsbtermcount] = pout->wordtotallen;
		pout->wordtotallen += len;
		pout->wsbtermcount++;

		return 1;
	}

	if(type & SCW_OUT_WPCOMP)
	{
		// 检查是否越界
		if(pout->wpb_curpos + len + 1 >= pout->wpbsize || pout->wpbtermcount+1 > pout->m_pir->m_maxterm)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s wpb_curpos out of range\n", where);
			return -1;
		}
		//assert(pout->wpcompbuf[pout->wpb_curpos]==0);
		if(pout->wpcompbuf[pout->wpb_curpos]!=0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s wpb_curpos of wpcompbuf error\n", where);
			return -1;
		}
		pout->wpbtermpos[pout->wpbtermcount] = ((len)<<24 | pout->wpb_curpos & 0x00FFFFFF);
		strcpy(pout->wpcompbuf+pout->wpb_curpos, buff);
		pout->wpb_curpos += len;
		pout->wpcompbuf[pout->wpb_curpos++] = '\t';
		pout->wpcompbuf[pout->wpb_curpos] = 0;
		pout->wpbtermoffsets[pout->wpbtermcount] = offset;
		pout->wpbtermcount++;

		return 1;
	}

	if(type & SCW_OUT_SUBPH)
	{
		// 检查是否越界
		if(pout->spb_curpos + len + 1 >= pout->spbsize || pout->spbtermcount+1 > pout->m_pir->m_maxterm)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s spb_curpos out of range\n", where);
			return -1;
		}
		//assert(pout->subphrbuf[pout->spb_curpos]==0);
		if(pout->subphrbuf[pout->spb_curpos]!=0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s spb_curpos of subphrbuf error\n", where);
			return -1;
		}

		pout->spbtermpos[pout->spbtermcount] = ((len)<<24 | pout->spb_curpos & 0x00FFFFFF);
		strcpy(pout->subphrbuf+pout->spb_curpos, buff);
		pout->spb_curpos += len;
		pout->subphrbuf[pout->spb_curpos++] = '\t';
		pout->subphrbuf[pout->spb_curpos] = 0;
		pout->spbtermoffsets[pout->spbtermcount] = offset;
		pout->spbtermcount++;

		return 1;
	}

	if(type & SCW_OUT_HUMANNAME)
	{
		// 检查是否越界
		if(pout->nameb_curpos + len + 1 >= pout->namebsize || pout->namebtermcount+1 > pout->m_pir->m_maxterm)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s nameb_curpos out of range\n", where);
			return -1;
		}
		//assert(pout->namebuf[pout->nameb_curpos]==0);
		if(pout->namebuf[pout->nameb_curpos]!=0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s nameb_curpos of namebuf error\n", where);
			return -1;
		}

		pout->namebtermpos[pout->namebtermcount] = ((len)<<24 | pout->nameb_curpos & 0x00FFFFFF);
		strcpy(pout->namebuf+pout->nameb_curpos, buff);
		pout->nameb_curpos += len;
		pout->namebuf[pout->nameb_curpos++] = '\t';
		pout->namebuf[pout->nameb_curpos] = 0;
		pout->namebtermoffsets[pout->namebtermcount] = offset;
		pout->namebtermcount++;

		return 1;
	}

	if(type & SCW_OUT_BOOKNAME)
	{
		// 检查是否越界
		if(pout->bnb_curpos + len + 1 >= pout->bnbsize || pout->bnbtermcount+1 > pout->m_pir->m_maxterm)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s bnb_curpos out of range\n", where);
			return -1;
		}
		//assert(pout->booknamebuf[pout->bnb_curpos]==0);
		if(pout->booknamebuf[pout->bnb_curpos]!=0)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s bnb_curpos of booknamebuf error\n", where);
			return -1;
		}
		pout->bnbtermpos[pout->bnbtermcount] = ((len)<<24 | pout->bnb_curpos & 0x00FFFFFF);
		strcpy(pout->booknamebuf+pout->bnb_curpos, buff);
		pout->bnb_curpos += len;
		pout->booknamebuf[pout->bnb_curpos++] = '\t';
		pout->booknamebuf[pout->bnb_curpos] = 0;
		pout->bnbtermoffsets[pout->bnbtermcount] = offset;
		pout->bnbtermcount++;

		return 1;
	}

	ul_writelog(UL_LOG_WARNING, "error in %s type undefined\n", where);
	return -1;
}


//   scw_get_item
int scw_get_item(scw_item_t* pitem, scw_out_t* pout, int level)
{
	const char* where = "scw_get_item";
	u_int tmcnt=0, len=0, bufsize=0, off_width = 0 ;
	u_int *tmpos=NULL, *tmoff=NULL;
	scw_property_t* tmprop = NULL;
	char* src=NULL;
	int flag = pout->m_pir->m_flag;

	if((flag & level) <= 1)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s level illegal\n", where);
		return -1;
	}

	if(level & SCW_OUT_BASIC)
	{
		src    = pout->wordsepbuf;
		len    = pout->wsb_curpos;
		bufsize  = pout->wsbsize;
		tmcnt  = pout->wsbtermcount;
		tmpos  = pout->wsbtermpos;
		tmoff  = pout->wsbtermoffsets;
		tmprop  = pout->wsbtermprop;
		off_width = 0;
	}
	else if(level & SCW_OUT_WPCOMP)
	{
		src    = pout->wpcompbuf;
		len    = pout->wpb_curpos;
		bufsize  = pout->wpbsize;
		tmcnt  = pout->wpbtermcount;
		tmpos  = pout->wpbtermpos;
		tmoff  = pout->wpbtermoffsets;
		tmprop  = pout->wpbtermprop;
		off_width = 0;
	}
	else if(level & SCW_OUT_SUBPH)
	{
		src    = pout->subphrbuf;
		len    = pout->spb_curpos;
		bufsize  = pout->spbsize;
		tmcnt  = pout->spbtermcount;
		tmpos  = pout->spbtermpos;
		tmoff  = pout->spbtermoffsets;
		tmprop  = pout->spbtermprop;
		off_width = 0;
	}
	else if(level & SCW_OUT_HUMANNAME)
	{
		src    = pout->namebuf;
		len    = pout->nameb_curpos;
		bufsize  = pout->namebsize;
		tmcnt  = pout->namebtermcount;
		tmpos  = pout->namebtermpos;
		tmoff  = pout->namebtermoffsets;
		tmprop  = pout->namebtermprop;
		off_width = 0;
	}
	else if(level & SCW_OUT_BOOKNAME)
	{
		src    = pout->booknamebuf;
		len    = pout->bnb_curpos;
		bufsize  = pout->bnbsize;
		tmcnt  = pout->bnbtermcount;
		tmpos  = pout->bnbtermpos;
		tmoff  = pout->bnbtermoffsets;
		tmprop  = pout->bnbtermprop;
		off_width = 0;
	}
	else if(level & SCW_OUT_NEWWORD)
	{
		scw_newword_t* pnewword = pout->pnewword;
		src    = pnewword->newwordbuf; 
		len    = pnewword->newwordb_curpos; 
		bufsize  = pnewword->newwordbsize;
		tmcnt  = pnewword->newwordbtermcount;
		tmpos  = pnewword->newwordbtermpos;
		tmoff  = pnewword->newwordbtermoffsets;
		tmprop  = pnewword->newwordbtermprop;
		off_width = 1;
	}
	else
	{    
		ul_writelog(UL_LOG_WARNING, "error in %s level illegal\n", where);
		return -1;
	}

	if(pitem->m_allocated == 0)
	{
		pitem->m_buff = src;
		pitem->m_size = 2*len+1;
		pitem->m_tmpos = tmpos;
		pitem->m_tmoff = tmoff;
		if((flag | level) & SCW_OUT_PROP == 0)
		{
			pitem->m_tmprop = NULL;
		}
		else
		{
			pitem->m_tmprop = tmprop;
		}
	}
	else
	{
		//assert(pitem->m_size>=bufsize);
		if(pitem->m_size < bufsize)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s size of pitem not enough\n", where);
			return -1;
		}
		memcpy(pitem->m_buff,src,len+1);
		memcpy(pitem->m_tmpos, tmpos, tmcnt*sizeof(int));
		memcpy(pitem->m_tmoff, tmoff, tmcnt*sizeof(int));

		if((flag & level) & SCW_OUT_PROP)
		{
			memcpy(pitem->m_tmprop, tmprop, tmcnt*sizeof(scw_property_t));
		}
	}

	pitem->m_curpos  = len;
	pitem->m_tmcnt  = tmcnt;
	pitem->m_off_width = off_width;

	return 1;  
}


// scw_get_result  
int scw_get_result(scw_item_t* pitem, scw_worddict_t* pwdict,scw_inner_t* pir, int level)
{
	const char* where = "scw_get_result";
	u_int i=0, offset=0;
	scw_lemma_t* plm=NULL;

	scw_reset_item(pitem);

	if((pir->m_flag & level) <= 1)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s level illegal\n", where);
		return -1;
	}

	if(level & SCW_OUT_BASIC)
	{
		for(i=0;i<pir->m_ppseg_cnt;i++)
		{
			plm = pir->m_ppseg[i];
			if(append_buf_bword_item(pitem, pwdict, pir, plm, offset)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			offset += get_lmoff_step(pwdict, pir, plm);  
		}
		return 1;
	}

	if(level & SCW_OUT_WPCOMP)
	{
		for(i=0;i<pir->m_ppseg_cnt;i++)
		{
			plm = pir->m_ppseg[i];
			if(append_buf_wpcomp_item(pitem, pwdict, pir, plm, offset)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			offset += get_lmoff_step(pwdict, pir, plm);
		}

		return 1;
	}

	if(level & SCW_OUT_SUBPH)
	{
		for(i=0;i<pir->m_ppseg_cnt;i++)
		{
			plm = pir->m_ppseg[i];
			if(append_buf_subph_item(pitem, pwdict, pir, plm, offset)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			offset += get_lmoff_step(pwdict, pir, plm);
		}
		return 1;
	}

	if(level & SCW_OUT_NEWWORD)
	{
		char word[256];
		u_int pos = 0, len  = 0;
		scw_newword_t* pnewword = pir->pnewword;
		for( i = 0; i<pnewword->newwordbtermcount; i++)
		{
			memset(word, 0 ,sizeof(word));
		//	printf("[%d] ", pnewword->newwordbtermoffsets[i]);
			pos = GET_TERM_POS(pnewword->newwordbtermpos[i]);
			len = GET_TERM_LEN(pnewword->newwordbtermpos[i]);
			strncpy(word, pnewword->newwordbuf + pos, len);
			word[len] = 0; 
			append_item_buf(pitem, word, len, pnewword->newwordbtermoffsets[i*2] , 0);
		}
		
		return 1;
	}


	if(level & SCW_OUT_HUMANNAME)
	{
		for(i=0;i<pir->m_ppseg_cnt;i++)
		{
			plm = pir->m_ppseg[i];
			if(append_buf_humanname_item(pitem, pwdict, pir, plm, offset)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			offset += get_lmoff_step(pwdict, pir, plm);
		}
		return 1;
	}

	if(level & SCW_OUT_BOOKNAME)
	{
		for(i=0;i<pir->m_ppseg_cnt;i++)
		{
			plm = pir->m_ppseg[i];
			if(append_buf_bookname_item(pitem, pwdict, pir, plm ,offset)<0)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
			offset += get_lmoff_step(pwdict, pir, plm);
		}
		return 1;
	}

	ul_writelog(UL_LOG_WARNING, "error in %s level illegal\n", where);
	return -1;  
}

//   scw_gi_tmstr
char* scw_gi_tmstr(scw_item_t* pitem, int i, char* term, int tlen)
{
	const char* where="scw_gi_tmstr";
	int pos=0, len=0;

	//assert(term && tlen>0);
	if(!(term && tlen>0))
	{
		ul_writelog(UL_LOG_WARNING, "error in %s term or tlen illegal\n", where);
		return NULL;
	}
	term[0]=0;

	if(i<0 || i>=(int)pitem->m_tmcnt)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s i out of range\n", where);
		return NULL;
	}

	pos = GET_TERM_POS(pitem->m_tmpos[i]);
	len = GET_TERM_LEN(pitem->m_tmpos[i]);
	if(pos>=(int)pitem->m_size || len>=tlen)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s, pitem error or buff of term not enough\n", where);
		return NULL;
	}
	//assert(pos<(int)pitem->m_size && len<tlen);

	memcpy(term, pitem->m_buff+pos, len);
	term[len]=0;

	return term;
}


//   scw_gi_tmlen
int scw_gi_tmlen(scw_item_t* pitem, int i)
{
	const char* where="scw_gi_tmlen";

	if(i<0 || i>=(int)pitem->m_tmcnt)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s i out of range\n", where);
		return -1;
	}

	return GET_TERM_LEN(pitem->m_tmpos[i]);
}


//  scw_gi_tmoff
int scw_gi_tmoff(scw_item_t* pitem, int i)
{
	const char* where="scw_gi_tmoff";

	if(i<0 || i>=(int)pitem->m_tmcnt)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s i out of range\n", where);
		return -1;
	}

	if(pitem->m_off_width > 0)
		return pitem->m_tmoff[i*2];

	return pitem->m_tmoff[i];
}


//  scw_gi_buf
char* scw_gi_buf(scw_item_t* pitem)
{
	return pitem->m_buff;
}

scw_property_t * scw_gi_tmprop(scw_item_t* pitem,int i)
{       
	const char* where="scw_gi_tmprop";
	if(i<0 || i>=(int)pitem->m_tmcnt || pitem->m_tmprop == NULL)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s i out of range or m_tmprop not malloc\n", where);
		return NULL;
	}       

	return (&pitem->m_tmprop[i]);
}       

void scw_reset_item(scw_item_t* pitem)
{

	char *where="scw_reset_item";  
	//assert(pitem && pitem->m_size>2);
	if(!pitem || pitem->m_size<=2)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s, pitem NULL or m_size not enough\n", where);
		return;
	}
	pitem->m_buff[0]   = '\t';
	pitem->m_buff[1]  = 0;

	pitem->m_curpos    = 1;
	pitem->m_tmcnt    = 0;
	return;
}

//   append_item_buf
/*=======================================================================================
 * function  : append a string to scw_item_t structure
 * argu    : 
 *
 * return  :
 *======================================================================================*/
int append_item_buf(scw_item_t* pitem, char* buff, int len, int offset, int isbasic)
{
	const char* where="append_item_buf";
	int orglen=0;

	if(pitem->m_curpos+len+1 >= pitem->m_size )
	{
		ul_writelog(UL_LOG_WARNING, "error in %s m_curpos out of range\n", where);
		return -1;
	}
	//assert(pitem->m_buff[pitem->m_curpos]==0);
	if(pitem->m_buff[pitem->m_curpos]!=0)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s m_curpos of m_buff error\n", where);
		return -1;
	}
	pitem->m_tmpos[pitem->m_tmcnt]=((len)<<24|pitem->m_curpos&0x00FFFFFF);
	memcpy(pitem->m_buff+pitem->m_curpos, buff, len);
	pitem->m_curpos += len;
	pitem->m_buff[pitem->m_curpos++]='\t';
	pitem->m_buff[pitem->m_curpos]=0;

	if(isbasic==true)
	{
		orglen= pitem->m_curpos-len-1-pitem->m_tmcnt;
		pitem->m_tmoff[pitem->m_tmcnt]= orglen;
	}
	else
	{
		pitem->m_tmoff[pitem->m_tmcnt] = offset;
	}
	pitem->m_tmcnt++;

	return 1;
}


//   scw_create_item
scw_item_t* scw_create_item(int maxterm)
{
	const char* where="scw_create_item";
	scw_item_t* pitem=NULL;

	if((pitem = (scw_item_t*)calloc(1, sizeof(scw_item_t)))==NULL)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s pitem calloc error\n", where);
		return NULL;
	}

	pitem->m_buff  = (char*) calloc(maxterm*10, sizeof(char));
	pitem->m_tmpos  = (u_int*) calloc(maxterm*3, sizeof(u_int));
	pitem->m_tmoff  = (u_int*) calloc(maxterm*3, sizeof(u_int));
	pitem->m_tmprop  = (scw_property_t*) calloc(maxterm*3, sizeof(scw_property_t));
	memset(pitem->m_tmprop,0,maxterm*3*sizeof(scw_property_t));

	if(!(pitem->m_buff && pitem->m_tmpos&& pitem->m_tmoff && pitem->m_tmprop))
	{
		ul_writelog(UL_LOG_WARNING, "error in %s calloc error\n", where);
		return NULL;
	}

	pitem->m_size  = maxterm*10;
	pitem->m_allocated = 1;
	return pitem;
}


int scw_output_dynwordinfo(scw_worddict_t* pwdict,scw_inner_t * pir,FILE * fp)
{
	if(fp == NULL)
		return 0;
	char lemma[MAX_WORD_LEN];
	char dynword[MAX_WORD_LEN];
	char property[MAX_WORD_LEN];
	char tmpbuf[MAX_WORD_LEN];
	char segbuf[2*MAX_WORD_LEN];
	char subbuf[2*MAX_WORD_LEN];
	scw_lemma_t * plm = NULL,* plm2 = NULL;
	int len = 0;
	u_int cnt = 0,offset = 0;  
	for(u_int i=0; i < pir->m_dynlm_cnt;i++)
	{
		plm = &(pir->m_dynlm[i]);
		if(plm == NULL)
			continue;
		len = get_lmstr(pwdict,pir,plm,dynword,sizeof(dynword));

		if(scw_seek_lemma(pwdict,dynword,plm->m_length) != LEMMA_NULL)
			continue;

		memset(property,0,sizeof(property));
		write_prop_to_str(plm->m_property,property,pwdict->m_wdtype);

		if(plm->m_type == LEMMA_TYPE_DBASIC)
		{
			fprintf(fp,"[%s] [0(%s)] [] %s 0\n",dynword,dynword,property);
		}
		else if(plm->m_type == LEMMA_TYPE_DPHRASE)
		{
			cnt = pir->m_dynphinfo[plm->m_phinfo_bpos];
			strcpy(segbuf,"[");
			for(u_int j=0; j<cnt; j++)
			{
				plm2= (scw_lemma_t*)pir->m_dynphinfo[plm->m_phinfo_bpos+j+1];
				len = get_lmstr(pwdict, pir, plm2,lemma,sizeof(lemma));
				sprintf(tmpbuf,"%d(%s)",j,lemma);
				strcat(segbuf,tmpbuf);
			}
			strcat(segbuf,"]");

			offset = 0;
			strcpy(subbuf,"[");
			if(plm->m_subphinfo_bpos!=COMMON_NULL)
			{  
				cnt = pir->m_dynphinfo[plm->m_subphinfo_bpos];
				for(u_int j=0; j<cnt; j++)
				{
					offset = pir->m_dynphinfo[plm->m_subphinfo_bpos+j*2+1];
					plm2 = (scw_lemma_t*)pir->m_dynphinfo[plm->m_subphinfo_bpos+j*2+2];
					len = get_lmstr(pwdict, pir, plm2,lemma,sizeof(lemma));
					sprintf(tmpbuf,"%d(%s)",offset,lemma);
					strcat(subbuf,tmpbuf);
				}
			}

			strcat(subbuf,"]");
			fprintf(fp,"[%s] %s %s %s 0\n",dynword,segbuf,subbuf,property);
		}
		continue;
	}

	return 0;
}

//  scw_destroy_item
void scw_destroy_item(scw_item_t* pitem)
{
	free(pitem->m_buff);
	free(pitem->m_tmpos);
	free(pitem->m_tmoff);
	free(pitem->m_tmprop);
	pitem->m_allocated=0;
	free(pitem);
}      

// get original form of Japenese verb and adj
int scw_get_goi(Sdict_search * dict, char * src, int slen, char * des, int dlen)
{
	char* where="scw_get_goi()";
	if( !dict )
	{
		ul_writelog(UL_LOG_WARNING, "error in %s: dict is NULL", where);
		return -1;
	}
	if( !src )
	{
		ul_writelog(UL_LOG_WARNING, "error in %s: input term is NULL", where);
		return -1;
	}
	if( slen<=0||dlen<=0||slen>MAX_WORD_LEN-1 )
	{
		ul_writelog(UL_LOG_WARNING, "error in %s: string length should be >0 and <%d", where, MAX_WORD_LEN);
		return -1;
	}

	char input[MAX_WORD_LEN];
	char output[MAX_WORD_LEN];
	strncpy(input,src,slen);
	input[slen]='\0';

	Sdict_snode node;
	creat_sign_f64(input, strlen(input), &node.sign1, &node.sign2);
	if(ds_op1(dict, &node, SEEK) == 0)
	{
		return 0;
	}
	int hinshi=node.code;

	int Katuyou_Kata = 0, Katuyou_Kei = 0;
	Katuyou_Kata = hinshi_info[hinshi][2];
	Katuyou_Kei = hinshi_info[hinshi][3];
	if (Katuyou_Kata != 0 && Katuyou_Kei != 0) {
		int start_at = strlen(input) - kihonglen[Katuyou_Kata][Katuyou_Kei-1];
		strncpy(output, input, strlen(input));
		strncpy(output+start_at, kihongkei[Katuyou_Kata], strlen(kihongkei[Katuyou_Kata]));
		output[start_at+strlen(kihongkei[Katuyou_Kata])] = 0;
	}

	if( (int)strlen(output)+1>dlen )
	{
		ul_writelog(UL_LOG_WARNING, "error in %s: result length:%d is too long for result buffer:%d", where, (int)strlen(output)+1, dlen);
		return -1;
	}
	strncpy(des,output,strlen(output));
	des[strlen(output)]='\0';

	return strlen(des);
}
