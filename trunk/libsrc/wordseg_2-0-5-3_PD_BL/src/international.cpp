#include "international.h"
#include "chinese_gbk.h"

int (*scw_judge_backtrack)(scw_worddict_t * pwdict,u_int flmpos,u_int ftlmpos,u_int slmpos,u_int stlmpos);
int (*get_lgt_scw_seg)(scw_worddict_t * pwdict,scw_inner_t * pir);
int (*humanname_recg)(scw_worddict_t* pwdict, scw_inner_t* pir);
int (*judge_fname)(scw_worddict_t * pwdict,scw_inner_t* pir,u_int bpos,int lmcnt);
int (*fhumanname_recg)(scw_worddict_t* pwdict, scw_inner_t* pir);
int (*canbe_long_name)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
int (*is_3word_cname)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
int (*judge_3cname)(scw_worddict_t* pwdict,scw_lemma_t* plm1,scw_lemma_t* plm2,scw_lemma_t* plm3,int flag);
int (*is_2word_cname)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int pos);
int (*is_real_surname)(scw_worddict_t* pwdict, scw_inner_t* pir, u_int i);
int *(*scw_get_nameprob)(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);
int (*is_my_alnum)(char * p);
int (*is_my_num)(char * p);
int (*is_my_alpha)(char * p);
void (*ch_set_prop_by_str)(char*pstr,scw_property_t& property);
void (*ch_write_prop_to_str)(scw_property_t property, char* buffer);
int (*is_gbk_hz)(unsigned char* pstr);
int (*is_my_ascii)(char * pstr);
int (*is_mid_mark)(char * pstr);
int (*is_split_headmark)(char * pstr);
int (*is_split_tailmark)(char * pstr);
int (*is_radix_point)(char * pstr);
int (*is_fourbit_point)(const char * word);
u_int (*get_value_gb)(const char * buf, int& pos, const int len);
int (*is_oneword_gb)(char * word);                                                                  
int (*is_3human_need_reseg)(scw_worddict_t* pwdict,scw_inlemma_t* pilemma,char* src);               
int (*reseg_3human_phrase)(scw_worddict_t* pwdict,scw_inlemma_t *pilemma);                          
int (*update_multichr_lmprob)(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);                   
void (*scw_init_utilinfo)(scw_utilinfo_t * puti,int wdtype);                                        
int (*scw_seek_ambfrag)(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos); 
int (*scw_add_lmprob)(scw_worddict_t * pwdict,scw_lemma_t * plm,int * prob,int pbsize,int type);    
int (*scw_build_nameinfo)(const char * namefile,int type,scw_worddict_t * pwdict);                  
//int (*seg_split)( Sdict_search* phrasedict, scw_out_t* pout, token_t subtokens[], int tcnt );       
//int (*seg_split_tokenize)( Sdict_search* phrasedict, handle_t handle, token_t token, token_t subtokens[], int tcnt);
                                                                                                           
int init_function(int code, int language)
{
	if(code == SCW_UTF8 && language == SCW_CHINESE)
	{
/*		scw_judge_backtrack = scw_judge_backtrack_utf8_chinese;
		canbe_long_name = canbe_long_name_utf8_chinese;
		ch_set_prop_by_str = ch_set_prop_by_str_utf8_chinese;
		ch_write_prop_to_str = ch_write_prop_to_str_utf8_chinese;
		fhumanname_recg = fhumanname_recg_utf8_chinese;
		get_lgt_scw_seg = get_lgt_scw_seg_utf8_chinese;
		get_value_gb = get_value_gb_utf8_chinese;
		humanname_recg = humanname_recg_utf8_chinese;
		is_2word_cname = is_2word_cname_utf8_chinese;
		is_3human_need_reseg = is_3human_need_reseg_utf8_chinese;
		is_3word_cname = is_3word_cname_utf8_chinese;
		is_fourbit_point = is_fourbit_point_utf8_chinese;
		is_gbk_hz = is_gbk_hz_utf8_chinese;
		is_mid_mark = is_mid_mark_utf8_chinese;
		is_my_alnum = is_my_alnum_utf8_chinese;
		is_my_alpha = is_my_alpha_utf8_chinese;
		is_my_ascii = is_my_ascii_utf8_chinese;
		is_my_num = is_my_num_utf8_chinese;
		is_oneword_gb = is_oneword_gb_utf8_chinese;
		is_radix_point = is_radix_point_utf8_chinese;
		is_real_surname = is_real_surname_utf8_chinese;
		is_split_headmark = is_split_headmark_utf8_chinese;
		is_split_tailmark = is_split_tailmark_utf8_chinese;
		judge_3cname = judge_3cname_utf8_chinese;
		judge_fname = judge_fname_utf8_chinese;
		reseg_3human_phrase = reseg_3human_phrase_utf8_chinese;
		scw_add_lmprob = scw_add_lmprob_utf8_chinese;
		scw_build_nameinfo = scw_build_nameinfo_utf8_chinese;
		scw_get_nameprob = scw_get_nameprob_utf8_chinese;
		scw_init_utilinfo = scw_init_utilinfo_utf8_chinese;
		scw_seek_ambfrag = scw_seek_ambfrag_utf8_chinese;
		scw_seg = scw_seg_utf8_chinese;
		seg_split = seg_split_utf8_chinese;
		seg_split_tokenize = seg_split_tokenize_utf8_chinese;
		update_multichr_lmprob = update_multichr_lmprob_utf8_chinese;
*/	}
	else//È±Ê¡ÊÇgbk_chinese
	{
	    scw_judge_backtrack = scw_judge_backtrack_gbk_chinese;
		canbe_long_name = canbe_long_name_gbk_chinese;
		ch_set_prop_by_str = ch_set_prop_by_str_gbk_chinese;
		ch_write_prop_to_str = ch_write_prop_to_str_gbk_chinese;
		fhumanname_recg = fhumanname_recg_gbk_chinese;
		get_lgt_scw_seg = get_lgt_scw_seg_gbk_chinese;
		get_value_gb = get_value_gb_gbk_chinese;
		humanname_recg = humanname_recg_gbk_chinese;
		is_2word_cname = is_2word_cname_gbk_chinese;
		is_3human_need_reseg = is_3human_need_reseg_gbk_chinese;
		is_3word_cname = is_3word_cname_gbk_chinese;
		is_fourbit_point = is_fourbit_point_gbk_chinese;
		is_gbk_hz = is_gbk_hz_gbk_chinese;
		is_mid_mark = is_mid_mark_gbk_chinese;
		is_my_alnum = is_my_alnum_gbk_chinese;
		is_my_alpha = is_my_alpha_gbk_chinese;
		is_my_ascii = is_my_ascii_gbk_chinese;
		is_my_num = is_my_num_gbk_chinese;
		is_oneword_gb = is_oneword_gb_gbk_chinese;
		is_radix_point = is_radix_point_gbk_chinese;
		is_real_surname = is_real_surname_gbk_chinese;
		is_split_headmark = is_split_headmark_gbk_chinese;
		is_split_tailmark = is_split_tailmark_gbk_chinese;
		judge_3cname = judge_3cname_gbk_chinese;
		judge_fname = judge_fname_gbk_chinese;
		reseg_3human_phrase = reseg_3human_phrase_gbk_chinese;
		scw_add_lmprob = scw_add_lmprob_gbk_chinese;
		scw_build_nameinfo = scw_build_nameinfo_gbk_chinese;
		scw_get_nameprob = scw_get_nameprob_gbk_chinese;
		scw_init_utilinfo = scw_init_utilinfo_gbk_chinese;
		scw_seek_ambfrag = scw_seek_ambfrag_gbk_chinese;
	//	seg_split = seg_split_gbk_chinese;
	//	seg_split_tokenize = seg_split_tokenize_gbk_chinese;
		update_multichr_lmprob = update_multichr_lmprob_gbk_chinese;
	}
	return 1;
}







