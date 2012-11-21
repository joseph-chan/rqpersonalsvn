#include <ul_log.h>
#include "chinese_gbk.h"
#include "international.h"

//extern int scw_seek_ambfrag(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos);
extern int get_lmstr(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, char* dest, u_int destlen);
//extern int is_fourbit_point(const char * word);
extern int get_lmoff_step(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm);
//extern int canbe_long_name(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
//extern int is_real_surname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
//extern int is_3word_cname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
//extern int is_2word_cname(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
extern int poly_lemmas(scw_worddict_t* pwdict, scw_inner_t* pir, int bpos, int count,int bname = 0);
//extern int * scw_get_nameprob(scw_worddict_t* pwdict,scw_lemma_t * plmi,int type);
extern int create_dynword(scw_inner_t* pinner, char* str);
//extern int judge_fname(scw_worddict_t * pwdict,scw_inner_t* pir,u_int bpos,int lmcnt);
extern u_int scw_get_word(const char* inbuf, int &pos, const int slen, const char* charset);
extern scw_lemma_t* get_oneword_lemma(scw_worddict_t* pwdict, int wordvalue);
//extern int judge_3cname(scw_worddict_t* pwdict,scw_lemma_t* plm1,scw_lemma_t* plm2,scw_lemma_t* plm3,int flag=0x1);
//extern int * scw_get_nameprob(scw_worddict_t* pwdict,scw_lemma_t * plmi,int type);
extern  char al_num_map[256];
//extern int scw_add_lmprob(scw_worddict_t * pwdict,scw_lemma_t * plm,int * prob,int pbsize,int type);
extern u_int scw_seek_lemma(scw_worddict_t * pwdict,const char* term,int len);
//extern int update_multichr_lmprob(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);
extern int get_static_lmstr(scw_worddict_t* pwdict,scw_lemma_t* plm, char* dest, u_int destlen);
extern int scw_resize_utilinfo(scw_worddict_t * pwdict,u_int newsize);
//extern void scw_init_utilinfo(scw_utilinfo_t * puti,int wdtype);
extern int scw_resize_probbuf(scw_worddict_t * pwdict,u_int newsize);

// <log>
 #define LOG_ERROR(message)  ul_writelog(UL_LOG_FATAL, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
 #define WARNING_LOG(message)  ul_writelog(UL_LOG_WARNING, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)


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
int scw_judge_backtrack_gbk_chinese(scw_worddict_t * pwdict,
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
	if(scw_seek_ambfrag(pwdict,lmlist[2],lmlist[3],stlmpos)){
		return 0;
	}
	if(scw_seek_ambfrag(pwdict,lmlist[0],lmlist[1],ftlmpos)){
		return 1;
	}
	// 如果最长前缀词为短语，则选择当前最长切分作为此次切分结果。
	// 这条规矩有点无聊，[经过调研，去掉此条策略效果明显变好！]
	/*if(IS_PHRASE(pwdict->m_lemmalist[flmpos].m_property))
	  {    
	  return 0;
	  }*/

	// 日文消岐策略
	if(pwdict->m_wdtype & SCW_WD_JP )
	{
		int len01=lenlist[0]+lenlist[1];
		int len23=lenlist[2]+lenlist[3];
		// 如果较长切分的第一个单元不是单字，取长切分
		// 主要解决"香|港の人|気"vs"香港|の|人気"这样的case
		if( len01>len23 && lenlist[0]!=2 ){
			return 1;
		}
		else if( len01<len23 && lenlist[2]!=2 ){
			return 0;
		}
		double w[4]={};
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

			/* [经过对比发现全局归一化比局部归一化好，所以下面局部归一化屏蔽掉！]
			   double freq01 = lmlist[0]->m_weight + lmlist[1]->m_weight; // 构建辞典的时候已经加1平滑了！
			   double freq23 = lmlist[2]->m_weight + lmlist[3]->m_weight;
			   w0 = (lmlist[0]->m_weight*lenlist[0])/(len*freq01);
			   w1 = (lmlist[1]->m_weight*lenlist[1])/(len*freq01);
			   w2 = (lmlist[2]->m_weight*lenlist[2])/(len*freq23);
			   w3 = (lmlist[3]->m_weight*lenlist[3])/(len*freq23);*/

			// 如果有一个片段频率为0，则舍弃含此片段的切分，这样可以通过在辞典中设置0频率来灵活处理！
			if( (lmlist[0]->m_weight < 2) || (lmlist[1]->m_weight < 2) ) 
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
		

// get_lgt_scw_seg
/*========================================================================================
 * function : get logistic result of scw_seg_fmm  **Jap wordseg only**
 * argu     : pwdict, the worddict
 *          : pir, the inner seg result structure
 * return   : 1 if success, <0 if failed.
 ========================================================================================*/
int get_lgt_scw_seg_gbk_chinese(scw_worddict_t * pwdict,scw_inner_t * pir)
{
	scw_lemma_t *plm = NULL;
	scw_lemma_t *plm4=NULL;
	scw_lemma_t* plm3 = NULL;
	scw_property_t ppt;
	int len=0;
	int phlen=0;
	int cnt=0;
	int pos=0;
	int bwcnt=0;
	int cnt1=0;
	scw_lemma_t* plm2 = NULL;
	scw_lemma_t * ppm2 = NULL;
	u_int bpos = 0;
	char word[256]={};
	bool flag = false;
	int type = 0;
	int spcnt = 0;
	int offset = 0;
	int newoff = 0;

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
					default:
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

int humanname_recg_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir)
{  
	int step=0;
	int pos=0;
	u_int i=0;
	scw_lemma_t* plm=NULL;

	for( i=0; i<pir->m_ppseg_cnt; )
	{
		plm = pir->m_ppseg[i];
		if(plm == NULL || pir->m_ppseg_flag[i] == 1)
		{
			i++;
			continue;
		}
		step = 1;

		// if it's a two word phrase
		if( IS_2WHPREFIX(plm->m_property) ) 
		{
			if( canbe_long_name(pwdict, pir, i) ){
				step = 2;
			}
			else{
				step = 1;
			}
		}
		// 当前词为姓，获取姓氏左边界
		else if( is_real_surname(pwdict, pir, i) )
		{
			if(is_3word_cname(pwdict, pir, i) == 1) // 三个连续单字的情况
			{
				step = 3;
			}
			else if(is_2word_cname(pwdict, pir, i) == 1) // 两个连续切分片段
			{
				step = 2;
			}
			else{
				step = 1;
			}
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

// 
/*========================================================================================
 * function : judge if the lmlist is a foreign name.
 * argu     : pdict, the dictionary
 *                      : pir, bpos lmcnt
 *                      
 *========================================================================================*/

int judge_fname_gbk_chinese(scw_worddict_t * pwdict,scw_inner_t* pir,u_int bpos,int lmcnt)
{
	int namepower = 0;
	int condition = 0;
	int condition1 = 0;
	int condition2 = 0;
	int * ppb = NULL;
	scw_lemma_t * plm = NULL;
	scw_lemma_t * pblm = NULL;
	char namestr[MAX_WORD_LEN]={};
	int i = 0;
	int len = 0;
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
	if(m_nonull == 2/*lmcnt == 2*/
			&& IS_ONEWORD(pir->m_ppseg[bpos]->m_property)
	  )
	{

		for(int m = 1; m < lmcnt; m ++)
		{
			if(pir->m_ppseg[m+bpos] == NULL){
				continue;
			}
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


	if(unvalid || len > MAX_FNAME_LEN_GBK || len >= (int)MAX_LEMMA_LENGTH)
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
		if(len < 0)
		{
			return 0;
		}
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
	if(ds_op1(rdict, &snode, SEEK) == 1){
		return 0;
	}

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

//中文相关

int fhumanname_recg_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir)
{
	//  char* where="fhumanname_recg";
	int step = 0;
	u_int i = 0;
	u_int j = 0;
	scw_lemma_t* plm = NULL;
	int ret = 0;

	for(i=0;i< pir->m_ppseg_cnt; )
	{
		plm = pir->m_ppseg[i];
		if(plm == NULL || pir->m_ppseg_flag[i] == 1)
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

				if(pir->m_ppseg_flag[i+j] == 1)
				{
					step = 1;
					break;
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
//中文相关

int canbe_long_name_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i) 
{  
	int pos1, pos2;
	int lmpos=0;
	scw_lemma_t *plm=NULL;
	scw_lemma_t *pllm=NULL;
	scw_lemma_t *plm1=NULL;
	scw_lemma_t *plm2=NULL;
	char buff[4*MAX_WORD_LEN]={};
	int len=0;
	int len1=0;
	int len2=0;
	int len3=0;
	int llen=0;
	u_int value=0;
	char tmpstr[3]={};
	Sdict_snode   snode;
	Sdict_search* ldict  = pwdict->pdict_lsn_rule;
	Sdict_search* rdict = pwdict->pdict_rsn_rule;

	plm= pir->m_ppseg[i];

	if(i==pir->m_ppseg_cnt-1){
		return false;
	}
	scw_lemma_t* pnlm= pir->m_ppseg[i+1];
	if(!pnlm || !IS_1W_NAMEEND(pnlm->m_property) || pir->m_ppseg_flag[i+1] == 1){
		return 0;
	}

	if( i > 0 )
	{
		pllm = pir->m_ppseg[i-1];
		if(pllm){
			llen = pllm->m_length;
		}
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
		{
			return 0;
		}

		len1=plm1->m_length;
		len2=plm2->m_length;

		len = get_lmstr(pwdict, pir, plm, buff+llen, sizeof(buff)-llen);
		if(len < 0)
		{
			return 0;
		}
	}
	else//is basic word
	{  
		len=get_lmstr(pwdict, pir, plm, buff+llen, sizeof(buff)-llen);
		if(len < 0 )
		{
			return 0;
		}
		value = scw_get_word(buff+llen, (int&)len1,len, "");
		plm1 = get_oneword_lemma(pwdict,value);
		value = scw_get_word(buff+llen+len1, (int&)len2,len, "");
		plm2 = get_oneword_lemma(pwdict,value);

		// judge if the first one is a real surneme
		if(!IS_SURNAME(plm1->m_property)|| !IS_NAMEMID(plm2->m_property))
		{
			return 0;
		}
	}

	len3 = get_lmstr(pwdict, pir, pnlm, buff+llen+len1+len2, sizeof(buff)-llen-len1-len2);
	if(len3 < 0)
	{
		return 0;
	}
	buff[llen+len1+len2+len3]=0;

	creat_sign_fs64(buff+llen,len+len3,&(snode.sign1),&(snode.sign2));
	if(ds_op1(rdict, &snode, SEEK) == 1)
	{
		return 0; // satisfy a right rule
	}

	if(pllm)
	{
		memcpy(tmpstr,buff+llen,2);
		llen=get_lmstr(pwdict,pir,pllm, buff,llen);
		if(llen < 0)
		{
			return 0;
		}
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

//中文相关
int is_3word_cname_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos)
{
	int len = 0;
	int lmpos = 0;
	char gname[2*MAX_WORD_LEN];

	if(pos+2 >= pir->m_ppseg_cnt)
	{
		return 0;
	}

	if(pir->m_ppseg[pos]==NULL
			|| pir->m_ppseg[pos+1]==NULL
			|| pir->m_ppseg[pos+2]==NULL)
	{
		return 0;
	}

	if(!IS_NAMEMID(pir->m_ppseg[pos+1]->m_property) 
			|| !IS_1W_NAMEEND(pir->m_ppseg[pos+2]->m_property) 
			|| pir->m_ppseg_flag[pos+1] == 1
			|| pir->m_ppseg_flag[pos+2] == 1)
	{
		return 0;
	}

	if(judge_3cname(pwdict,pir->m_ppseg[pos],pir->m_ppseg[pos+1],pir->m_ppseg[pos+2],0x1))
	{
		len = get_lmstr(pwdict, pir, pir->m_ppseg[pos+1], gname, sizeof(gname));
		if(len < 0)
		{
			return 0;
		}
		len = get_lmstr(pwdict, pir, pir->m_ppseg[pos+2], &gname[len], sizeof(gname)-len);
		if( len < 0)
		{
			return 0;
		}

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

//中文相关
int judge_3cname_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t* plm1,scw_lemma_t* plm2,scw_lemma_t* plm3,int flag)
{
	int *ppb1=NULL;
	int *ppb2=NULL;
	int *ppb3 = NULL;
	int name_power = 0;
	int condition = 0;
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
	{
		tmp=ppb3[NAME_BGN];
	}

	condition=ppb1[NAME_OTH]+ppb2[NAME_OTH]+tmp;
	if(name_power < condition){
		return 0;
	}
	condition=ppb1[NAME_OTH]+ppb2[NAME_BGN]+ppb3[NAME_END];
	if(name_power < condition){
		return 0;
	}

	if( (flag & 0x1) && IS_NAME_STICK(plm3->m_property)) // 如果最后一个字是基本只能单独使用的字的话
	{
		return 1;
	}

	condition=ppb1[NAME_BGN]+ppb2[NAME_END]+tmp;
	if(name_power < condition){
		return 0;
	}

	return 1;
}
/*Comments?
 *
 * */
int is_2word_cname_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos)
{
	int name_power=0;
	int condition=0;
	int tmp=0;
	int len=0;
	int lmpos = 0;
	scw_lemma_t * plm1=NULL;
	scw_lemma_t *plm2=NULL;
	int* ppb1=NULL;
	int *ppb2=NULL;
	char buff[2*MAX_WORD_LEN] = {0};

	if( pos+1 >= pir->m_ppseg_cnt ){
		return 0;
	}

	if( (plm1 = pir->m_ppseg[pos])==NULL ||
			(plm2 = pir->m_ppseg[pos+1])==NULL )
	{
		return 0;  
	}

	// 姓 + 单字或者两字词的情况（只有带有-8属性的两字词语才检查！）
	if( !IS_NAMEEND(plm2->m_property) || IS_HOUZHUI(plm2->m_property) || pir->m_ppseg_flag[pos+1] == 1){
		return 0;
	}

	ppb1 = scw_get_nameprob(pwdict,plm1,DATA_CNAME);
	ppb2 = scw_get_nameprob(pwdict,plm2,DATA_CNAME);

	if( !ppb1 || !ppb2 )
	{
		return 0;
	}

	name_power = ppb1[NAME_BGN] + ppb2[NAME_END];

	tmp = ppb2[NAME_OTH];
	if(tmp < ppb2[NAME_BGN]){
		tmp = ppb2[NAME_BGN];
	}

	condition = ppb1[NAME_OTH] + tmp;

	if(name_power < condition){
		return 0;
	}

	if( IS_PHRASE(plm2->m_property) )
	{
		len = get_lmstr(pwdict,pir,plm2,buff,sizeof(buff));
		if(len < 0)
		{
			return 0;
		}
		if( (lmpos=create_dynword(pir,buff)) < 0 )
		{
			WARNING_LOG("error: create_dynword() failed");
			return -1;
		}
		pir->m_ppseg[pos+1]=&pir->m_dynlm[lmpos];
	}

	return 1;
}

// is_real_surname
/*========================================================================================
 * function : judge if a lemma is a real surname 
 * argu     : pdict, the dictionary
 *           : pir, the output struct
 *           : pos, the postion in pir->m_ppseg
 * return   : 1 is OK, or return 0;
 ========================================================================================*/
int is_real_surname_gbk_chinese(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i)
{  
	int len =0, len1 = 0;
	scw_lemma_t *plm=NULL;
	scw_lemma_t* pllm=NULL;
	scw_lemma_t* prlm=NULL;
	char buff[512]={};
	Sdict_snode   snode;
	Sdict_search* ldict  = pwdict->pdict_lsn_rule;
	Sdict_search* rdict = pwdict->pdict_rsn_rule;

	plm = pir->m_ppseg[i];  
	if( !IS_SURNAME(plm->m_property) ){
		return 0;
	}

	if( i > 0 )
	{
		pllm = pir->m_ppseg[i-1];
		if(pllm == NULL)
		{
			len = 0;
		}
		else
		{
			len = get_lmstr(pwdict, pir, pllm, buff, sizeof(buff));
			if(len < 0)
			{
				return 0;
			}
		}
		len1 = get_lmstr(pwdict, pir, plm, &buff[len], sizeof(buff)-len);
		if(len1 < 0)
		{
			return 0;
		}
		len += len1;
		creat_sign_fs64(buff,len,&(snode.sign1),&(snode.sign2));
		if(ds_op1(ldict, &snode, SEEK) == 1)
		{
			return 0; // satisfy a left rule
		}
	}

	if( i < pir->m_ppseg_cnt-1 )
	{
		prlm = pir->m_ppseg[i+1];
		len = get_lmstr(pwdict, pir, plm, buff, sizeof(buff));
		if(len < 0)
		{
			return 0;
		}
		if(prlm == NULL)
		{
			len1 = 0;
		}
		else
		{
			len1 = get_lmstr(pwdict, pir, prlm, &buff[len], sizeof(buff)-len);
			if(len1 < 0)
			{
				return 0;
			}
		}
		len += len1;
		creat_sign_fs64(buff,len,&(snode.sign1),&(snode.sign2));
		if(ds_op1(rdict, &snode, SEEK) == 1)
		{
			return 0; // satisfy a right rule
		}
	}

	return 1;
}
//中文相关
//
int * scw_get_nameprob_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t * plm,int type)
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



//segutil.cpp

//编码相关
int is_my_alnum_gbk_chinese(char * p)
{
	int ret = 0;
	if(*p > 0 && al_num_map[(int)*p])
	{
		ret = 1;
	}
	else if(IS_GB_1234(p) || IS_GB_ABCD(p))
	{
		ret = 2;
	}

	return ret;
}

int is_my_num_gbk_chinese(char * p)
{
	int ret = 0;
	if(*p > 0 && (al_num_map[(int)*p] & 2))
	{
		ret = 1;
	}
	else if(IS_GB_1234(p))
	{
		ret = 2;
	}

	return ret;
}
int is_my_alpha_gbk_chinese(char * p)
{
	int ret = 0;
	if(*p > 0 && (al_num_map[(int)*p] & 1))
	{
		ret = 1;
	}
	else if(IS_GB_ABCD((u_char*)p))
	{
		ret = 2;
	}
	return ret;
}
//语言相关
//  set_prop_by_str
/*========================================================================================
 * function : convert the string property to an integer. 
 * argu    : pstr, the string property
 * return  : the integer property.
 ========================================================================================*/
void ch_set_prop_by_str_gbk_chinese(char*pstr,scw_property_t& property)
{
	unsigned i;

	for(i=0;i<strlen(pstr);i++)
	{
		switch(pstr[i])
		{
			case 'S' :
				SET_SPECIAL(property);
				break;
			case 'Z':
				SET_TEMP(property);
				break;
			case 'P' :
				SET_PHRASE(property);
				break;
			case 'O' :
				SET_ONEWORD(property);
				break;
			case '0' :
				SET_STOPWORD(property);
				break;
			case '1' :
				SET_SURNAME(property);
				break;
			case '2' :
				SET_QIANZHUI(property);
				break;
			case '3' :
				SET_HOUZHUI(property);
				break;
			case '4' :
				SET_QIANDAO(property);
				break;
			case '5' :
				SET_HOUDAO(property);
				break;
			case '6' :
				SET_CFOREIGN(property);
				break;
			case '7' :
				SET_NAMEMID(property);
				break;
			case '8' :
				SET_NAMEEND(property);
				break;
			case '9' :
				SET_NAME_STICK(property);
				break;
			case 'm' :
				SET_CNUMBER(property);
				break;
			case 't' :
				SET_TIME(property);
				break;
			case 'M' :
				SET_MARK(property);
				break;
			case 'H' :
				SET_HUMAN_NAME(property);
				break;
			case 'q':
				SET_QUANTITY(property);
				break;
			case 'Q':
				SET_QYPD(property);
				break;
			case 'B':
				SET_BOOKNAME(property);
				break;
			case 'b':
				SET_BOOKNAME_START(property);
				break;
			case 'e':
				SET_BOOKNAME_END(property);
				break;
			case 'A':
				SET_2WHPREFIX(property);
				break;
			case 'd':
				SET_DI(property);
				break;
			case 'C' : 
				SET_CH_NAME(property);
				break;    
			case 'R':
				SET_FMNAME(property);
				break;
			case 'u':
				SET_UNIVERSITY(property);
				break;
			case 'F':
				SET_SOFTNAME(property);
				break; 
			case 'D':
				SET_LOC_SMALL(property);
				break;
			case 'n':
				SET_BRAND(property);
				break;
			case 'J':
				SET_ORGNAME(property);
				break;
			case 'a':
				SET_ASCIIWORD(property);
				break;
			case 'E':
				SET_SPEC_ENSTR(property);
				break;
			case 'f':
				SET_LOC_FOLLOW(property);
				break;
			case 'l':
				SET_LOC(property);
				break;
			case 'x':
				SET_FNAMEBGN(property);
				break;
			case 'y':
				SET_FNAMEMID(property);
				break;
			case 'z':
				SET_FNAMEEND(property);
				break;
			case 'w':
				SET_2FNAMEEND(property);
				break;
			case 'N':
				SET_1W_SUFFIX(property);
				break;
			case 'U':
				SET_VIDEO(property);
				break;
			case 'V':
				SET_GAME(property);
				break;
			case 'W':
				SET_SONG(property);
				break;
			case 'c':
				SET_SCW_PRODUCT(property);
				break;
			case 'p':
				SET_PLACE_SUFFIX(property);
				break;
			case 'v':
				SET_STATIC_BOOK(property);
				break;
			default:
				break;
		}
	}

	return;
}

//语言相关
void ch_write_prop_to_str_gbk_chinese(scw_property_t property, char* buffer)
{
	buffer[0] = 0;

	if(IS_NEWWORD(property))
	{
		strncat(buffer, "-NW",3);
	}
	if(IS_SURNAME(property))
	{
		strncat(buffer, "-1",2);
	}
	if(IS_TEMP(property))
	{
		strncat(buffer,"-Z",2);
	}
	if(IS_NAMEMID(property))
	{
		strncat(buffer, "-7",2);
	}
	if(IS_NAMEEND(property))
	{
		strncat(buffer, "-8",2);
	}
	if(IS_NAME_STICK(property))
	{
		strncat(buffer, "-9",2);
	}
	if(IS_QIANZHUI(property))
	{
		strncat(buffer, "-2",2);
	}
	if(IS_HOUZHUI(property))
	{
		strncat(buffer, "-3",2);
	}
	if(IS_QIANDAO(property))
	{
		strncat(buffer, "-4",2);
	}
	if(IS_HOUDAO(property))
	{
		strncat(buffer, "-5",2);
	}
	if(IS_CFOREIGN(property))
	{
		strncat(buffer, "-6",2);
	}
	if(IS_STOPWORD(property))
	{
		strncat(buffer, "-0",2);
	}
	if(IS_MARK(property))
	{
		strncat(buffer, "-M",2);
	}
	if(IS_PHRASE(property))
	{
		strncat(buffer, "-P",2);
	}
	if(IS_SPECIAL(property))
	{
		strncat(buffer, "-S",2);
	}
	if(IS_ONEWORD(property))
	{
		strncat(buffer, "-O",2);
	}
	if(IS_CNUMBER(property))
	{
		strncat(buffer, "-m",2);
	}
	if(IS_DI(property))
	{
		strncat(buffer, "-d",2);
	}
	if(IS_TIME(property))
	{
		strncat(buffer, "-t",2);
	}
	if(IS_HUMAN_NAME(property))
	{
		strncat(buffer, "-H",2);
	}
	if(IS_QUANTITY(property))
	{
		strncat(buffer, "-q",2);
	}
	if(IS_QYPD(property))
	{
		strncat(buffer, "-Q",2);
	}
	if(IS_BOOKNAME(property))
	{
		strncat(buffer, "-B",2);
	}
	if(IS_BOOKNAME_START(property))
	{
		strncat(buffer, "-b",2);
	}
	if(IS_BOOKNAME_END(property))
	{
		strncat(buffer, "-e",2);
	}
	if(IS_2WHPREFIX(property))
	{
		strncat(buffer, "-A",2);
	}
	if(IS_CH_NAME(property))
	{
		strncat(buffer, "-C",2);
	}
	if(IS_UNIVERSITY(property))
	{
		strncat(buffer,"-u",2);
	}
	if(IS_LOC_SMALL(property))
	{
		strncat(buffer,"-D",2);
	}
	if(IS_SOFTNAME(property))
	{
		strncat(buffer,"-F",2);
	}
	if(IS_FMNAME(property))
	{
		strncat(buffer,"-R",2);
	}

	if(IS_ORGNAME(property))
	{
		strncat(buffer,"-J",2);
	}
	if(IS_ASCIIWORD(property))
	{
		strncat(buffer,"-a",2);
	}
	// asc串是否为数词
	if(IS_ASCIINUM(property))
	{
		strncat(buffer,"-g",sizeof("-g"));
	}
	if(IS_SPEC_ENSTR(property))
	{
		strncat(buffer,"-E",2);
	}
	if(IS_LOC(property))
	{
		strncat(buffer,"-l",2);
	}
	if(IS_LOC_FOLLOW(property))
	{
		strncat(buffer,"-f",2);
	}
	if(IS_FNAMEBGN(property))
	{
		strncat(buffer,"-x",2);
	}
	if(IS_FNAMEMID(property))
	{
		strncat(buffer,"-y",2);
	}
	if(IS_FNAMEEND(property))
	{
		strncat(buffer,"-z",2);
	}
	if(IS_2FNAMEEND(property))
	{
		strncat(buffer,"-w",2);
	}
	if(IS_1W_SUFFIX(property))
	{
		strncat(buffer, "-N",2); 
	}
	if(IS_VIDEO(property))
	{
		strncat(buffer, "-U",2);
	}
	if(IS_GAME(property))
	{
		strncat(buffer, "-V",2);
	}
	if(IS_SONG(property))
	{
		strncat(buffer, "-W",2);
	}
	if(IS_SCW_PRODUCT(property))
	{
		strncat(buffer, "-c",2);
	}
	if(IS_BRAND(property))
	{
		strncat(buffer, "-n",2);
	}
	if(IS_PLACE_SUFFIX(property))
	{
		strncat(buffer, "-p",2);
	}
	if(IS_STATIC_BOOK(property))
	{
		strncat(buffer, "-v",2);
	}
}

//编码相关
int is_gbk_hz_gbk_chinese(unsigned char* pstr)
{
	char *where = "is_gbk_hz";
	//assert(pstr);
	if(!pstr)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	if(pstr[0]==0 || pstr[1]==0)
		return 0;

	if(pstr[1] == 0x7f)
		return 0;

	// if it is a gbk/2 word
	if ((pstr[0]>=0xB0) && (pstr[0]<=0xF7) &&
			(pstr[1]>=0xA1) && (pstr[1]<=0xFE))
	{
		return 1;
	}
	// if it is a gbk/3 word
	else if ((pstr[0]>=0x81) && (pstr[0]<=0xa0) &&
			(pstr[1]>=0x40) && (pstr[1]<=0xfe))
	{
		return 1;
	}
	// if it is a gbk/4 word
	else if ((pstr[0]>=0xaa) && (pstr[0]<=0xfe) &&
			(pstr[1]>=0x40) && (pstr[1]<=0xa0))
	{
		return 1;
	}
	else{
		return 0;
	}
}

//编码相关
int is_my_ascii_gbk_chinese(char * pstr)
{
	int ret=0;
	u_char ch = 0;
	u_char * p = (u_char*)pstr;

	if(*p > 0x80)
	{
		if(IS_GB_1234(p) || IS_GB_ABCD(p))
		{
			ret = 2;
		}
		else if(p[0] == 0xa3 && (p[1] == 0xab || p[1] == 0xad || p[1] == 0xa5 || p[1] == 0xa6))
		{
			ret = 2;
		}
		else if(p[0] == 0xa3 && (p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ch = *(p+2);
			if(ch!='\0' && ch!=' ' && ch!='\n' && ch!='\r' && (ch < 0x80 || ch == 0xa3))
			{
				ret = 2;
			}  
		}
	}
	else if(al_num_map[*p]!=0)
		ret = 1;
	else if(*p=='+'||*p=='-'||*p=='&'||*p=='%')
		ret = 1;
	else if(*p=='.'||*p=='@'||*p=='_')
	{
		ch = *(p+1);
		if(ch!='\0' && ch!=' ' && ch!='\n' && ch!='\r'&& (ch < 0x80 || ch == 0xa3))
			ret = 1;
	}

	return ret;
}


// is_mid_mark
/*========================================================================================
 * function : if the current char is a middle mark. the term of 'middle mark' is a 
 *       : definition used in proc_ascii.
 * argu     : ch, the char
 ========================================================================================*/
int is_mid_mark_gbk_chinese(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80){
		if(*p == '@' || *p == '.'
				|| *p == '-' || *p == '+' 
				|| *p == '&' || *p == '_'
				|| *p == '%')
		{
			ret = 1;
		}
	}
	else{
		if(p[0] == 0xa3 &&
				(p[1] == 0xab || p[1] == 0xad || p[1] == 0xa5 || p[1] == 0xa6
				 || p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ret = 2;
		}
	}

	return ret;
}
int is_split_headmark_gbk_chinese(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80)
	{
		if( *p == '@' || *p == '.'
				|| *p == '-' || *p == '+' 
				|| *p == '%' || *p == '_')
		{
			ret = 1;
		}
	}
	else{
		if(p[0] == 0xa3 &&
				(p[1] == 0xab || p[1] == 0xad || p[1] == 0xa5
				 || p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ret = 2;
		}
	}

	return ret;
}
/*
 * comments*/
int is_split_tailmark_gbk_chinese(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80)
	{
		if(*p == '@' || *p == '.'
				|| *p == '-' || *p == '_' 
				|| *p == '&')
		{
			ret = 1;
		}
	}
	else{
		if(p[0] == 0xa3 &&
				(p[1] == 0xad || p[1] == 0xa6
				 || p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ret = 2;
		}
	}

	return ret;
}

/*
 *Comments
 * */
int is_radix_point_gbk_chinese(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80 && *p == '.')
	{
		ret = 1;
	}
	else if(p[0] == 0xa3 && p[1] == 0xae)
	{
		ret = 2;
	}

	return ret;
}

/*
 *Comments
 * */
int is_fourbit_point_gbk_chinese(const char * word)
{
	u_char * p = (u_char*)word;
	if((p[0] == 0x81 && p[1] ==0x39 && p[2] ==0xA7 && p[3] ==0x39))
		return 1;
	return 0;
}

//编码相关
u_int get_value_gb_gbk_chinese(const char * buf, int& pos, const int len)
{
	if(pos >= len)
		return 0;

	unsigned char *tmpbuf=(unsigned char*)(buf + pos);
	u_int value = tmpbuf[0];
	int off = 0;  
	off = 1;

	if((pos+1 < len) && (tmpbuf[0] >= 0x81 && tmpbuf[0] <= 0xfe))
	{
		// hanzi with 2 char
		if(tmpbuf[1] >= 0x40 && tmpbuf[1] <= 0xfe)
		{
			off = 2;
			value = tmpbuf[0] *256 + tmpbuf[1];
		}       
		// hanzi with 4 char
		else if((tmpbuf[1] >= 0x30 && tmpbuf[1] <= 0x39))
		{
			if( pos + 3 < len 
					&&(tmpbuf[2] >= 0x81 && tmpbuf[2] <= 0xfe)
					&&(tmpbuf[3] >= 0x30 && tmpbuf[3] <= 0x39))
			{
				value = tmpbuf[0]*16777216 + tmpbuf[1]*65536 + tmpbuf[2]*256 + tmpbuf[3];
				off = 4;
			}
		}
	}

	pos += off;
	return value;
}


/*
 *Comments
 * */
int is_oneword_gb_gbk_chinese(char * word)
{
	u_char *tmpbuf=(u_char *)word;
	int len = strlen(word);

	if(len == 1)
		return 1;
	if(len == 2 && (tmpbuf[0] >= 0x81 && tmpbuf[0] <= 0xfe)
			&& (tmpbuf[1] >= 0x40 && tmpbuf[1] <= 0xfe))
		return 2;
	if(len == 4 && (tmpbuf[0] >= 0x81 && tmpbuf[0] <= 0xfe)
			&& (tmpbuf[1] >= 0x30 && tmpbuf[1] <= 0x39)
			&& (tmpbuf[2] >= 0x81 && tmpbuf[2] <= 0xfe)
			&& (tmpbuf[3] >= 0x30 && tmpbuf[3] <= 0x39))
		return 4;

	return 0;
}


//segout.cpp相关

//segdict.cpp
//编码相关，构建词典
int is_3human_need_reseg_gbk_chinese(scw_worddict_t* pwdict,scw_inlemma_t* pilemma,char* src)
{
	const char* where="is_3human_need_reseg";
	int offset;
	char* nextword;
	char* left;
	char* right;
	int remlen=0,slen=0;
	u_int lmpos;
	scw_lemma_t *plm=NULL;

	nextword=src;
	remlen=strlen(src);
	if(nextword==NULL||*nextword==0)
		return 0;

	offset=strtol(nextword,&left,10);
	if( (left==nextword) || (left==NULL) || ( *left!='(') || (offset > (int)MAX_LEMMA_LENGTH) )
	{
		ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
		return -1;
	}

	// get the sublemma position
	right=strchr(left+1,')');
	if((right== NULL)||(remlen -2 < right-left) ||(right-left >(int) MAX_LEMMA_LENGTH)||(right-left==1))
	{  
		ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
		return -1;
	}

	char ch = *right;
	*right = 0;

	lmpos = scw_seek_lemma(pwdict, left+1, strlen(left+1));
	if(lmpos == LEMMA_NULL)
	{
		ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
		return -1;
	}

	*right=ch;
	plm = &pwdict->m_lemmalist[lmpos];
	slen=plm->m_length;

	if(IS_SURNAME(plm->m_property) || IS_ONEWORD(plm->m_property) || (remlen == 8 && slen == 4))
		return 0;// not need reseg  
	else 
		return 1;//need reseg
}

//编码相关
int reseg_3human_phrase_gbk_chinese(scw_worddict_t* pwdict,scw_inlemma_t *pilemma)
{
	//const char* where="reseg_3human_phrase";
	char * phr=NULL;
	int len=0;
	int slen=2;
	int len22=2;
	int off=0;
	u_int lmpos=0;
	u_int lmpos1=0;
	scw_lemma_t *plm=NULL;
	int i=0;

	phr=pilemma->m_lmstr;
	len=strlen(phr);

	if(len!=6 && len!=8){
		return 0;
	}

	if(len==8){
		slen=4;
	}

	while(i<len) //check if include ascii
	{
		if(*(phr+i)<0)
			i+=2;
		else if(*(phr+i)>0)
			return 0;
		else 
			i++;
	}

	i=0;

	pilemma->m_wordlist[0]=0;
	pilemma->m_subphlist[0]=0;
	pilemma->m_bwcount=0;
	pilemma->m_sphcount=0;

	lmpos=scw_seek_lemma(pwdict,phr+slen,len-slen); //check the given name
	if(lmpos==LEMMA_NULL)
	{
		pilemma->m_bwcount=3;

		lmpos=scw_seek_lemma(pwdict,phr,slen);
		if(lmpos==LEMMA_NULL)
			return -1;

		pilemma->m_wordlist[0]=0;
		pilemma->m_wordlist[1]=lmpos;
		off=slen;

		for(i=0;i<pilemma->m_bwcount-1;i++)
		{
			lmpos=scw_seek_lemma(pwdict,phr+slen+2*i,2);
			if(lmpos==LEMMA_NULL)   
				return -1;

			pilemma->m_wordlist[2*i+2]=off;
			pilemma->m_wordlist[2*i+3]=lmpos;
			off+=2;
		}

		lmpos=scw_seek_lemma(pwdict,phr,len-len22);
		if(lmpos==LEMMA_NULL || !IS_PHRASE(pwdict->m_lemmalist[lmpos].m_property))
			return 1;// have not sub phrase
		pilemma->m_sphcount=1;
		pilemma->m_subphlist[0]=0;
		pilemma->m_subphlist[1]=lmpos;
	}
	else
	{
		plm=&pwdict->m_lemmalist[lmpos];
		if(IS_PHRASE(plm->m_property))
		{
			pilemma->m_bwcount=3;

			lmpos1=scw_seek_lemma(pwdict,phr,slen);
			if(lmpos1==LEMMA_NULL)   
				return -1;
			pilemma->m_wordlist[0]=0;
			pilemma->m_wordlist[1]=lmpos1;  
			off=slen;    

			for(i=0;i<pilemma->m_bwcount-1;i++)
			{
				lmpos1=scw_seek_lemma(pwdict,phr+slen+2*i,2);
				if(lmpos1==LEMMA_NULL)   
					return -1;
				pilemma->m_wordlist[2*i+2]=off;
				pilemma->m_wordlist[2*i+3]=lmpos1;
				off+=2;
			}

			pilemma->m_sphcount=1;
			pilemma->m_subphlist[0]=1;
			pilemma->m_subphlist[1]=lmpos;
		}
		else
		{
			pilemma->m_bwcount=2;

			pilemma->m_wordlist[2]=slen;
			pilemma->m_wordlist[3]=lmpos;

			lmpos=scw_seek_lemma(pwdict,phr,slen);
			if(lmpos==LEMMA_NULL)   
				return -1;
			pilemma->m_wordlist[0]=0;
			pilemma->m_wordlist[1]=lmpos;
		}
	}

	return 1;
}

//语言相关
int update_multichr_lmprob_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t * plm,int type)
{
	const char* where="update_multichr_lmprob";
	int len=0;
	int len1 = 0;
	u_int value = 0;
	scw_lemma_t * psblm=NULL;
	int *ppb = NULL;
	char lemma[MAX_LEMMA_LENGTH]={};
	int prob[NAME_PROP_COUNT]={};
	int cnt = 0;
	int const_smooth_val = (int)(NORMALIZE_INTEGER*log(LEMMA_SMOOTH_PROB));
	int const_default_val = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB));

	memset(prob,0,NAME_PROP_COUNT*sizeof(int));

	if(type == DATA_CNAME )
	{
		cnt = 0;
		if(IS_NAMEEND(plm->m_property)
				|| IS_SURNAME(plm->m_property))
		{
			len = get_static_lmstr(pwdict,plm, lemma, sizeof(lemma));

			while((value = scw_get_word(lemma,(int&)len1,len,NULL)) != 0)
			{
				psblm = get_oneword_lemma(pwdict,value);

				if(psblm == NULL)
				{
					ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
					return -1;
				}

				ppb = scw_get_nameprob(pwdict,psblm,type);

				if(IS_SURNAME(plm->m_property))
				{
					prob[NAME_BGN] += const_default_val;
				}
				else
				{
					prob[NAME_BGN] += const_smooth_val;
				}

				if(IS_NAMEEND(plm->m_property))
				{
					if(ppb)
					{
						if(len == len1)
						{
							prob[NAME_END] += ppb[NAME_END];
						}
						else
						{
							prob[NAME_END] += ppb[NAME_MID];
						}
					}
					else
					{
						prob[NAME_END] += const_default_val;
					}
				}
				else
				{
					prob[NAME_END] += const_smooth_val;
				}

				if(ppb)
				{
					prob[NAME_OTH] += ppb[NAME_OTH];
				}
				else
				{
					prob[NAME_OTH] += const_default_val;
				}

				prob[NAME_MID] += const_smooth_val;

				cnt++;
			}

			if(cnt != 2)
			{
				ul_writelog(UL_LOG_NOTICE, "unvalid multichar lemma %s!\n", lemma);
				return -1;
			}

			if(IS_NAMEEND(plm->m_property))
			{
				prob[NAME_END] += (int)(NORMALIZE_INTEGER*3*log((double)cnt));
			}

			scw_add_lmprob(pwdict,plm,prob,NAME_PROP_COUNT,type);
		}
	}
	else if(type == DATA_FNAME)
	{
		cnt = 0;
		if( IS_FNAMEEND(plm->m_property)
				|| IS_FNAMEBGN(plm->m_property)
				|| IS_FNAMEMID(plm->m_property) )
		{
			len = get_static_lmstr(pwdict,plm, lemma, sizeof(lemma));

			while((value = scw_get_word(lemma,(int&)len1,len,NULL)) != 0)
			{
				psblm = get_oneword_lemma(pwdict,value);

				if(psblm == NULL)
				{
					ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
					return -1;
				}

				ppb = scw_get_nameprob(pwdict,psblm,type);

				if(IS_FNAMEBGN(plm->m_property))
				{
					if(ppb)
					{
						if(cnt == 0)
						{
							prob[NAME_BGN] += ppb[NAME_BGN];  
						}
						else
						{
							prob[NAME_BGN] += ppb[NAME_MID];
						}
					}
					else
					{  
						prob[NAME_BGN] += const_default_val;  
					}
				}
				else
				{
					prob[NAME_BGN] += const_smooth_val;
				}

				if(IS_FNAMEMID(plm->m_property))
				{
					if(ppb)
					{
						prob[NAME_MID] += ppb[NAME_MID];
					}
					else
					{
						prob[NAME_MID] += const_default_val; 
					}
				}
				else
				{
					prob[NAME_MID] += const_smooth_val;
				}

				if(IS_FNAMEEND(plm->m_property))
				{
					if(ppb)
					{
						if(len1 == len)
						{
							prob[NAME_END] += ppb[NAME_END];
						}
						else
						{
							prob[NAME_END] += ppb[NAME_MID];
						}
					}
					else
					{
						prob[NAME_END] += const_default_val;
					}
				}
				else
				{
					prob[NAME_END] += const_smooth_val;
				}

				if(ppb)
				{
					if(cnt == 0)
					{
						prob[NAME_OTH] += ppb[NAME_BGN];
					}
					else if(len1 == len)
					{
						prob[NAME_OTH] += ppb[NAME_END];
					}
					else
					{
						prob[NAME_OTH] += ppb[NAME_MID];
					}
				}
				else
				{
					prob[NAME_OTH] += const_default_val;
				}

				cnt++;    
			}

			/*if(IS_FNAMEBGN(plm->m_property)){
			  prob[NAME_BGN] -= (int)(NORMALIZE_INTEGER*log((double)cnt));
			  }                 

			  if(IS_FNAMEMID(plm->m_property)){
			  prob[NAME_MID] += (int)(NORMALIZE_INTEGER*log((double)cnt));
			  }                 

			  if(IS_FNAMEEND(plm->m_property)){
			  prob[NAME_END] -= (int)(NORMALIZE_INTEGER*log((double)cnt));
			  }

			  prob[NAME_OTH] -= (int)(NORMALIZE_INTEGER*log((double)cnt));*/

			scw_add_lmprob(pwdict,plm,prob,NAME_PROP_COUNT,type);  
		}    
	}
	else if(type == DATA_JNAME)
	{
		cnt = 0;
		if( IS_NAMEEND(plm->m_property)
				|| IS_SURNAME(plm->m_property)
				|| IS_NAMEMID(plm->m_property)
				|| IS_GIVENNAME(plm->m_property) )
		{
			len = get_static_lmstr(pwdict,plm, lemma, sizeof(lemma));

			while((value = scw_get_word(lemma,(int&)len1,len,NULL)) != 0)
			{
				psblm = get_oneword_lemma(pwdict,value);

				if(psblm == NULL)
				{
					ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
					return -1;
				}

				ppb = scw_get_nameprob(pwdict,psblm,type);

				if(IS_SURNAME(plm->m_property))
				{
					if(ppb)
					{
						if(cnt == 0)
						{
							prob[NAME_BGN] += ppb[NAME_BGN];
						}
						else
						{
							prob[NAME_BGN] += ppb[NAME_MID];
						}
					}
					else
					{
						prob[NAME_BGN] += const_default_val;
					}
				}
				else
				{
					prob[NAME_BGN] += const_smooth_val;
				}  

				if(IS_NAMEEND(plm->m_property) || IS_GIVENNAME(plm->m_property)){
					if(ppb)
					{
						if(len == len1)
						{
							prob[NAME_END] += ppb[NAME_END];
						}
						else
						{
							prob[NAME_END] += ppb[NAME_MID];
						}
					}
					else
					{
						prob[NAME_END] += const_default_val;
					}
				}
				else
				{
					prob[NAME_END] += const_smooth_val;
				}

				if(IS_NAMEMID(plm->m_property))
				{
					if(ppb)
					{
						prob[NAME_MID] +=  ppb[NAME_MID];
					}
					else
					{
						prob[NAME_MID] +=  const_default_val;
					}
				}
				else
				{
					prob[NAME_MID] += const_smooth_val;
				}

				if(ppb)
				{
					prob[NAME_OTH] += ppb[NAME_OTH];
				}
				else
				{
					prob[NAME_OTH] += const_default_val;
				}

				cnt++;
			}  

			scw_add_lmprob(pwdict,plm,prob,NAME_PROP_COUNT,type);
		}
	}

	return 0;   
}

//语言相关
void scw_init_utilinfo_gbk_chinese(scw_utilinfo_t * puti,int wdtype)
{
	puti->m_ambsuffix_pos = COMMON_NULL;
	puti->m_fnameprob_pos = COMMON_NULL;

	if(wdtype & SCW_WD_JP)
		puti->m_jnameprob_pos = COMMON_NULL;
	else if(wdtype & SCW_WD_CH)
		puti->m_cnameprob_pos = COMMON_NULL;
	return;
}

//语言相关
int scw_seek_ambfrag_gbk_chinese(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos)
{
	u_int tmppos  = 0;
	u_int utilpos = 0;
	u_int * pinfo = NULL;

	if((utilpos = pflm->m_utilinfo_pos) != COMMON_NULL
			&& pwdict->m_utilinfo[utilpos].m_ambsuffix_pos != COMMON_NULL)
	{
		pinfo = pwdict->m_ambinfo + pwdict->m_utilinfo[utilpos].m_ambsuffix_pos;

		tmppos = (slmpos+1) % pinfo[0];

		if(pinfo[tmppos+1] == slmpos)
			return 1;
	}

	if((pwdict->m_wdtype & SCW_WD_CH))
	{
		if(IS_LOC(pflm->m_property) && IS_LOC_FOLLOW(pslm->m_property))
			return 1;
	}

	return 0;
}

//语言相关
int scw_add_lmprob_gbk_chinese(scw_worddict_t * pwdict,scw_lemma_t * plm,int * prob,int pbsize,int type)
{
	const char * where = "scw_add_lmprob";
	int * plm_prob = NULL;
	scw_utilinfo_t * puti = NULL;
	u_int * ppbpos =NULL;

	if(plm == NULL || prob == NULL || pwdict == NULL)
		return 0;

	if(plm->m_utilinfo_pos == COMMON_NULL)
	{
		if(pwdict->m_utilipos + 1 >= pwdict->m_utilisize)
		{
			if(scw_resize_utilinfo(pwdict,pwdict->m_utilisize*2)!=1)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
		}
		puti = pwdict->m_utilinfo + pwdict->m_utilipos;
		scw_init_utilinfo(puti,pwdict->m_wdtype);
		plm->m_utilinfo_pos = pwdict->m_utilipos;
		pwdict->m_utilipos++;
	}
	else
	{
		puti = pwdict->m_utilinfo + plm->m_utilinfo_pos;
	}

	if(type == DATA_CNAME)
	{
		ppbpos = &puti->m_cnameprob_pos;  
	}
	else if(type == DATA_FNAME)
	{
		ppbpos = &puti->m_fnameprob_pos;
	}
	else if(type == DATA_JNAME)
	{
		ppbpos = &puti->m_jnameprob_pos;
	}

	if(*ppbpos != COMMON_NULL)//lemma prob exist
	{
		plm_prob = pwdict->m_probbuf + (*ppbpos);
		memcpy(plm_prob,prob,pbsize*sizeof(int));
		return 0;
	}

	if(pwdict->m_pbpos + pbsize >= pwdict->m_pbsize)
	{
		if(scw_resize_probbuf(pwdict,pwdict->m_pbsize*2) != 1)
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
	}

	plm_prob = pwdict->m_probbuf + pwdict->m_pbpos;
	memcpy(plm_prob,prob,pbsize*sizeof(int));
	*ppbpos = pwdict->m_pbpos;
	pwdict->m_pbpos += pbsize;

	return 0;
}

//语言相关
int scw_build_nameinfo_gbk_chinese(const char * namefile,int type,scw_worddict_t * pwdict)
{
	FILE * fp = fopen(namefile,"r");
	if(fp == NULL)   
		return 0;

	char line[MAX_IL_LEN];
	char lemma[MAX_LEMMA_LENGTH];
	double prob[NAME_PROP_COUNT];
	int lnprob[NAME_PROP_COUNT];
	int len = 0;
	int i = 0,multibgn = 0;
	u_int lmpos = LEMMA_NULL;
	scw_lemma_t * plm = NULL;

	for(i=0;i<(int)pwdict->m_lmpos;i++)
	{
		plm = &(pwdict->m_lemmalist[i]);
		if(type == DATA_CNAME)
		{
			if(IS_ONEWORD(plm->m_property))
			{
				if(!IS_HZ(plm->m_property))
					continue;  

				for(int j=0;j<(int)NAME_PROP_COUNT;j++){
					lnprob[j] = (int)(NORMALIZE_INTEGER*log(LEMMA_SMOOTH_PROB));
				}

				lnprob[NAME_MID]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB));
				lnprob[NAME_END]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB));

				scw_add_lmprob(pwdict,plm,lnprob,NAME_PROP_COUNT,type);
			}
			else
			{
				multibgn = i;
				break;
			}
		}
		else if(type == DATA_FNAME)
		{
			if(IS_ONEWORD(plm->m_property))
			{
				if(!IS_HZ(plm->m_property))
					continue;
			}
			else
			{
				multibgn = i;
				break;
			}
		}
		else if(type == DATA_JNAME)
		{
			if(IS_SURNAME(plm->m_property) || IS_GIVENNAME(plm->m_property) 
					|| IS_NAMEEND(plm->m_property) || IS_NAMEMID(plm->m_property))
			{
				for(int j=0;j<(int)NAME_PROP_COUNT;j++)
				{
					lnprob[j] = (int)(NORMALIZE_INTEGER*log(LEMMA_SMOOTH_PROB_JP));
				}
				lnprob[NAME_MID]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB_JP));
				lnprob[NAME_END]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB_JP));

				scw_add_lmprob(pwdict,plm,lnprob,NAME_PROP_COUNT,type);
			}
		}
	}

	while(fgets(line,MAX_IL_LEN,fp))
	{
		len = strlen(line);
		while(line[len-1] == '\n'
				|| line[len-1] =='\r')
			len--;
		line[len] = 0;

		if(sscanf(line,"%s%lf%lf%lf%lf",lemma,&prob[NAME_BGN],&prob[NAME_MID],
					&prob[NAME_END],&prob[NAME_OTH]) != 5)
		{
			ul_writelog(UL_LOG_NOTICE,"add nameinfo line %s failed!\n",line);
			continue;
		}

		if((lmpos = scw_seek_lemma(pwdict,lemma,strlen(lemma))) == LEMMA_NULL)
			continue;

		plm = &(pwdict->m_lemmalist[lmpos]);

		for(int j=0;j<(int)NAME_PROP_COUNT;j++){
			lnprob[j] = (int)(NORMALIZE_INTEGER*log(prob[j]+LEMMA_SMOOTH_PROB));
		}

		scw_add_lmprob(pwdict,plm,lnprob,NAME_PROP_COUNT,type);
	}

	if(type == DATA_CNAME || type == DATA_FNAME)
	{
		for(i = multibgn;i<(int)pwdict->m_lmpos;i++)
		{
			plm = &(pwdict->m_lemmalist[i]);
			update_multichr_lmprob(pwdict,plm,type);
		}
	}

	fclose(fp);

	return 0;
}
//编码相关
/*
int seg_split_gbk_chinese( Sdict_search* phrasedict, scw_out_t* pout, token_t subtokens[], int tcnt )
{
	int ret = 0;
	Sdict_snode snode;    
	int ds_ret = 0;

	if (pout == NULL  || phrasedict == NULL)
	{      
		WARNING_LOG("error: pout or phrasedict is null!");
		return ret;
	}

	//混排结果
	for (u_int i = 0; i < pout->wpbtermcount; i++)
	{
		u_int pos = GET_TERM_POS( pout->wpbtermpos[i] );
		u_int len = GET_TERM_LEN( pout->wpbtermpos[i] );
		u_int off = pout->wpbtermoffsets[i];

		char term[256] = "\0";
		if(len >= 256)
		{
			WARNING_LOG("error: term length longer than 256!");
			return ret;
		}
		strncpy( term, pout->wpcompbuf+pos, len );
		term[len] = '\0';

		//book name combine
		//        char *p = pout->wpcompbuf+pos;
		//       if (strncmp( p, "《", 2 ) == 0 &&  strncmp( p+len-2, "》", 2 ) == 0)
		if(IS_BOOKNAME(pout->wpbtermprop[i]))
		{          
			u_int bsccnt = 0;        
			u_int boff = off;
			u_int rlen = len;

			while (rlen > 0)
			{
				bsccnt += 1;//basic个数
				rlen -= GET_TERM_LEN( pout->wsbtermpos[boff] );
				boff += 1;
			}

			if(IS_BOOKNAME_START(pout->wsbtermprop[off]) && IS_BOOKNAME_END(pout->wsbtermprop[off + bsccnt - 1]))
			{
				if (bsccnt == 3)
				{
					if (ret + 3 > tcnt)
					{
						WARNING_LOG("error : term result number more than max term count!");
						return ret;
					}    

					boff = off;
					while (boff < off+3)
					{
						u_int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
						u_int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

						strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
						subtokens[ret].buffer[blen] = '\0';
						subtokens[ret].offset = boff;
						subtokens[ret].length = blen;
						subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
						subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
						subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
						subtokens[ret].index = 0;

						boff += 1;
						ret += 1;
					}
					continue;
				}

				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}  
				boff = off;
				u_int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
				u_int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

				strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
				subtokens[ret].buffer[blen] = '\0';
				subtokens[ret].offset = boff;
				subtokens[ret].length = blen;
				subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
				subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
				subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
				subtokens[ret].index = 0;

				ret += 1;

				//build complete seg from subphrase buffer        
				rlen = len - 4;
				boff = off + 1;
				while (rlen > 0)
				{
					u_int lentemp = len;
					int maxsubphridx = -1;
					unsigned int maxsubphrlen = 0;
					while(1)
					{
						for (int j = 0; j < (int)pout->spbtermcount; j++)
						{
							//u_int spos = GET_TERM_POS( pout->spbtermpos[j] );
							u_int slen = GET_TERM_LEN( pout->spbtermpos[j] );

							//pass not aligned subphrase
							if (pout->spbtermoffsets[j] != boff)
							{
								continue;
							}

							//pass itself,or longer than it 
							if (slen >= len || slen >= lentemp)
							{    
								continue;    
							}
							if (maxsubphrlen < slen)
							{
								maxsubphridx = j;
								maxsubphrlen = slen;
							}                
						}
						if (maxsubphridx >= 0)
						{
							u_int spos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
							u_int slen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );

							char sterm[256] = "\0";
							if(slen >= 256)
							{
								WARNING_LOG("error: term length longer than 256!");
								return ret;
							}
							strncpy( sterm, pout->subphrbuf+spos, slen );
							sterm[slen] = '\0';

							//pass subphrase can split 
							creat_sign_f64 (sterm, slen, &(snode.sign1), &(snode.sign2));
							ds_ret = ds_op1 (phrasedict, &snode, SEEK);
							if ( ds_ret == 1)
							{
								lentemp = slen;
								//   j = -1;
								maxsubphridx = -1;
								maxsubphrlen = 0;
								continue;
							}
							else if(ds_ret != 0)
							{
								WARNING_LOG("error: ds_op1 result error!");
								return ret;
							}
							else
							{
								break;
							}
						}
						break;
					}

					//copy max subphrase
					if (maxsubphridx >= 0)
					{					
						if(ret + 1 > tcnt)
						{
							WARNING_LOG("error : term result number more than max term count!");
							return ret;
						}
						u_int mpos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
						u_int mlen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );
						u_int moff = pout->spbtermoffsets[maxsubphridx];

						strncpy( subtokens[ret].buffer, pout->subphrbuf+mpos, mlen );
						subtokens[ret].buffer[mlen] = '\0';
						subtokens[ret].offset = moff;
						subtokens[ret].length = mlen;
						subtokens[ret].prop.m_hprop = pout->spbtermprop[maxsubphridx].m_hprop;
						subtokens[ret].prop.m_lprop = pout->spbtermprop[maxsubphridx].m_lprop;
						subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
						subtokens[ret].index = 0;

						ret += 1;									
						rlen -= mlen;
						int bcnt = 1;
						mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );
						while (mlen > 0)
						{
							bcnt += 1;
							moff += 1;
							mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );
						}

						boff += bcnt;
					}
					//copy basic
					else
					{		
						if(ret + 1 > tcnt)
						{
							WARNING_LOG("error : term result number more than max term count!");
							return ret;
						}
						u_int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
						u_int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

						strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
						subtokens[ret].buffer[blen] = '\0';
						subtokens[ret].offset = boff;
						subtokens[ret].length = blen;
						subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
						subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
						subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
						subtokens[ret].index = 0;

						ret += 1;				
						rlen -= blen;
						boff += 1;
					}
				}

				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
				blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

				strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
				subtokens[ret].buffer[blen] = '\0';
				subtokens[ret].offset = boff;
				subtokens[ret].length = blen;
				subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
				subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
				subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
				subtokens[ret].index = 0;

				ret += 1;
				continue;
			}
		}

		creat_sign_f64 (term, len, &(snode.sign1), &(snode.sign2));
		if (ds_op1 (phrasedict, &snode, SEEK) != 1)
		{
			if(ret + 1 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}

			strncpy( subtokens[ret].buffer, term, len );
			subtokens[ret].buffer[len] = '\0';
			subtokens[ret].offset = off;
			subtokens[ret].length = len;
			subtokens[ret].prop.m_hprop = pout->wpbtermprop[i].m_hprop;
			subtokens[ret].prop.m_lprop = pout->wpbtermprop[i].m_lprop;
			subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
			subtokens[ret].index = 0;

			ret += 1;	  	
			continue;
		}

		u_int rlen = len;
		u_int boff = off;
		while (rlen > 0)
		{
			u_int lentemp = len;//phrase itself length
			int maxsubphridx = -1;
			unsigned int maxsubphrlen = 0;
			while(1)
			{
				for (int j = 0; j < (int)pout->spbtermcount; j++)
				{
					//u_int spos = GET_TERM_POS( pout->spbtermpos[j] );
					u_int slen = GET_TERM_LEN( pout->spbtermpos[j] );

					//pass not aligned subphrase
					if (pout->spbtermoffsets[j] != boff)
					{
						continue;
					}

					//pass itself	
					if (slen >= len || slen >= lentemp)
					{
						continue;	
					}
					if (maxsubphrlen < slen)
					{
						maxsubphridx = j;
						maxsubphrlen = slen;
					}		
				}
				if (maxsubphridx >= 0)
				{
					u_int spos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
					u_int slen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );

					char sterm[256] = "\0";
					if(slen >= 256)
					{
						WARNING_LOG("error: term length longer than 256!");
						return ret;
					}
					strncpy( sterm, pout->subphrbuf+spos, slen );
					sterm[slen] = '\0';

					//pass subphrase can split 
					creat_sign_f64 (sterm, slen, &(snode.sign1), &(snode.sign2));
					ds_ret = ds_op1 (phrasedict, &snode, SEEK);
					if ( ds_ret == 1)
					{
						lentemp = slen;
						//j = -1;
						maxsubphridx = -1;
						maxsubphrlen = 0;
						continue;
					}
					else if(ds_ret != 0)
					{
						WARNING_LOG("error: ds_op1 result error!");
						return ret;
					}
					else
					{
						break;
					}
				}
				break;
			}

			//copy max subphrase
			if (maxsubphridx >= 0)
			{					              
				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				int mpos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
				int mlen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );
				int moff = pout->spbtermoffsets[maxsubphridx];

				strncpy( subtokens[ret].buffer, pout->subphrbuf+mpos, mlen );
				subtokens[ret].buffer[mlen] = '\0';
				subtokens[ret].offset = moff;
				subtokens[ret].length = mlen;
				subtokens[ret].prop.m_hprop = pout->spbtermprop[maxsubphridx].m_hprop;
				subtokens[ret].prop.m_lprop = pout->spbtermprop[maxsubphridx].m_lprop;
				subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
				subtokens[ret].index = 0;
				ret += 1;				
				rlen -= mlen;

				int sbcnt = 1;
				mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );//获得basic个数
				while (mlen > 0)
				{
					sbcnt += 1;
					moff += 1;
					mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );
				}

				boff += sbcnt;
			}
			//copy basic
			else
			{			
				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
				int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

				strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
				subtokens[ret].buffer[blen] = '\0';
				subtokens[ret].offset = boff;
				subtokens[ret].length = blen;
				subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
				subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
				subtokens[ret].weight = 0;//pout没有weight和index字段,赋初值为0
				subtokens[ret].index = 0;

				ret += 1;				
				rlen -= blen;
				boff += 1;
			}
		}
	}

	return ret;
}

//编码相关
int seg_split_tokenize_gbk_chinese( Sdict_search* phrasedict, handle_t handle, token_t token, token_t subtokens[], int tcnt)
{
	int ret = 0;
	Sdict_snode snode;
	token_t subphrase[256];//这样的话,用户还怎么指定啊?
	token_t basic[256];
	int ds_ret = 0;

	if (token.length <= 2)
	{
		memcpy( subtokens, &token, sizeof( token_t ) );
		ret = 1;
		return ret;
	}

	//book name combine
	//    char *p = token.buffer;
	//   if (strncmp( p, "《", 2 ) == 0 && strncmp( p+token.length-2, "》", 2 ) == 0)
	int subcnt = seg_tokenize(handle, token, TOK_SUBPHR,subphrase,256);
	int bsccnt = seg_tokenize(handle, token, TOK_BASIC, basic, 256);

	if(IS_BOOKNAME(token.prop) && IS_BOOKNAME_START(basic[0].prop) && IS_BOOKNAME_END(basic[bsccnt - 1].prop))
	{
		//     int subcnt = seg_tokenize(handle, token, TOK_SUBPHR,subphrase,256);
		//     int bsccnt = seg_tokenize(handle, token, TOK_BASIC, basic, 256);

		if (bsccnt == 3)
		{
			if (ret + 3 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}    
			memcpy( subtokens+ret, basic, sizeof( token_t ) );
			ret += 1;
			memcpy( subtokens+ret, basic+1, sizeof( token_t ) );
			ret += 1;
			memcpy( subtokens+ret, basic+2, sizeof( token_t ) );
			ret += 1;

			return ret;
		}

		if (ret + 1 > tcnt)
		{
			WARNING_LOG("error : term result number more than max term count!");
			return ret;
		}
		memcpy( subtokens+ret, basic, sizeof( token_t ) );
		ret += 1;

		//build complete seg from subphrase buffer        
		unsigned int len = token.length-4;
		unsigned int off = 2;
		while (len > 0)
		{
			u_int splitlen = token.length;//最大拆分phrase的长度
			u_int tmplen = token.length;
			int maxsubphridx = -1;
			unsigned int maxsubphrlen = 0;
			while(1)
			{
				for (int i = 0; i < subcnt; i++)
				{
					//pass not aligned subphrase
					if (subphrase[i].offset != off)
						continue;

					//pass itself    
					if (subphrase[i].length >= tmplen || subphrase[i].length >= splitlen)
						continue;    

					if (maxsubphrlen < subphrase[i].length)
					{
						maxsubphridx = i;
						maxsubphrlen = subphrase[i].length;
					}
				}
				if(maxsubphridx >= 0)
				{
					//pass subphrase can split 
					creat_sign_f64 (subphrase[maxsubphridx].buffer, subphrase[maxsubphridx].length, &(snode.sign1), &(snode.sign2));
					ds_ret = ds_op1 (phrasedict, &snode, SEEK);
					if (ds_ret == 1)
					{
						splitlen = maxsubphrlen;
						maxsubphridx = -1;
						maxsubphrlen = 0;
						continue;
					}
					else if(ds_ret != 0)
					{
						WARNING_LOG("error: ds_op1 result error!");
						return ret;
					}
					else
					{
						break;
					}	
				}
				break;
			}
			//copy max subphrase
			if (maxsubphridx >= 0)
			{
				if (ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				memcpy( subtokens+ret, subphrase+maxsubphridx, sizeof( token_t ) );
				ret += 1;                               
				len -= subphrase[maxsubphridx].length;
				off += subphrase[maxsubphridx].length;
			}
			//copy basic
			else
			{
				if (ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}

				int boff = 0;
				while (boff < bsccnt && basic[boff].offset != off)
				{
					boff += 1;
				}       

				if((boff >= bsccnt) || (basic[boff].offset != off))
				{
					WARNING_LOG("basic offset exceeds basic count");
					return ret;
				}
				//assert (boff < bsccnt );
				//assert (basic[boff].offset == off);                

				memcpy( subtokens+ret, basic+boff, sizeof( token_t ) );

				ret += 1;                
				len -= basic[boff].length;
				off += basic[boff].length;
			}    
		}
		if (ret + 1 > tcnt)
		{
			WARNING_LOG("error : term result number more than max term count!");
			return ret;
		}
		memcpy( subtokens+ret, basic+bsccnt-1, sizeof( token_t ) );
		ret += 1;

		return ret;
	}

	creat_sign_f64 (token.buffer, token.length, &(snode.sign1), &(snode.sign2));
	if (ds_op1 (phrasedict, &snode, SEEK) != 1)
	{
		memcpy( subtokens, &token, sizeof( token_t ) );
		ret = 1;
		return ret;
	}

	//  int subcnt = seg_tokenize(handle, token,TOK_SUBPHR,subphrase,32);
	//  int bsccnt = seg_tokenize(handle, token, TOK_BASIC, basic, 32);

	if (bsccnt == 1)//unexpect : phrasedict error
	{
		memcpy( subtokens, &token, sizeof( token_t ) );
		ret = 1;
		return ret;
	}

	//split phrase
	unsigned int len = token.length;
	unsigned int off = 0;
	while (len > 0)
	{
		u_int splitlen = token.length;
		u_int tmplen = token.length;
		int maxsubphridx = -1;
		unsigned int maxsubphrlen = 0;
		while(1)
		{
			for (int i = 0; i < subcnt; i++)
			{
				//pass not aligned subphrase
				if (subphrase[i].offset != off)
					continue;

				//pass itself    
				if (subphrase[i].length >= tmplen || subphrase[i].length >= splitlen)
					continue;     

				if (maxsubphrlen < subphrase[i].length)
				{
					maxsubphridx = i;
					maxsubphrlen = subphrase[i].length;
				}
			}
			if(maxsubphridx >= 0)
			{
				//pass subphrase can split 
				creat_sign_f64 (subphrase[maxsubphridx].buffer, subphrase[maxsubphridx].length, &(snode.sign1), &(snode.sign2));
				ds_ret = ds_op1 (phrasedict, &snode, SEEK);
				if ( ds_ret == 1)    
				{
					splitlen = maxsubphrlen;
					maxsubphridx = -1;
					maxsubphrlen = 0;
					continue;
				} 
				else if(ds_ret != 0)
				{
					WARNING_LOG("error: ds_op1 result error!");
					return ret;
				}
				else
				{
					break;
				}	
			}
			break;
		}

		//copy max subphrase
		if (maxsubphridx >= 0)
		{         
			if (ret + 1 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}
			memcpy( subtokens+ret, subphrase+maxsubphridx, sizeof( token_t ) );
			ret += 1;                
			len -= subphrase[maxsubphridx].length;
			off += subphrase[maxsubphridx].length;
		}
		//copy basic
		else
		{
			if (ret + 1 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}
			int boff = 0;
			while (boff < bsccnt && basic[boff].offset != off)
			{
				boff += 1;
			}       
			if((boff >= bsccnt) || (basic[boff].offset != off))
			{
				WARNING_LOG("basic offset exceeds basic count");
				return ret;
			}
			//assert (boff < bsccnt );
			//assert (basic[boff].offset == off);                

			memcpy( subtokens+ret, basic+boff, sizeof( token_t ) );
			ret += 1;

			len -= basic[boff].length;
			off += basic[boff].length;
		}    
	}

	return ret;
}*/
