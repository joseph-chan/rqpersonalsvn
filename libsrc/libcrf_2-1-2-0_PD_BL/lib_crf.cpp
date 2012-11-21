/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * $Id: lib_crf.cpp,v 1.10 2010/05/19 07:29:02 zhangchao01 Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file lib_crf.cpp
 * @author zhangwei01(zhangwei01@baidu.com)
 * @date 2010/02/24 19:17:13
 * @version $Revision: 1.10 $ 
 * @brief 
 */

#include "lib_crf.h"


std::string EngPunct = "\\,./?><;':[]{}=_+!$%^&*()|";


inline bool IsChnChar(char ch)
{
	if ((ch & 0x80) == 0x80)  return true;
	else return false;
}


inline bool IsEngPunct(char ch)
{
	if (IsChnChar(ch)) return false;
	if (!ispunct(ch)) return false;
	if (EngPunct.find(ch) == std::string::npos) return false;
	else return true;
}


/**
 * @brief 将输入inbuf转成crf可以识别的输入
 *
 * @param [in/out] tagger   : CRFTaggerImpl*
 * @param [in/out] inbuf   : const char*
 * @param [in/out] inlen   : const int
 * @return  int 
 * @retval   
 * @see 
 * @note 
**/
//int crf_queryformat(CrfTag* tagger, const char* inbuf, const int inlen);


/**
 * @brief 加载训练模型，并且将模型赋值给tagger
 *
 * @param [in/out] tagger   : TaggerImpl*   必须是全局变量，在调用前需要创建该实例
 * @param [in/out] model_dir   : const char*
 * @param [in/out] model_file   : const char*
 * @return  int 
 * @retval   
 * @see 
 * @note 
**/
int crf_load_model(CrfModel* crf_model,  const char* model_name, float this_path_factor)
{
	int ret = 0;
	ret = crf_model->load_model(model_name, this_path_factor);	
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: fail to load model '%s'.", __func__, model_name);
		return -1;
	}

	//设置costfactor
	crf_model->set_cost_factor(COST_FACTOR);

	return 0;
}



/**
 * @brief 为crf_out_t分配内存，为每个crf_term_out_t分配tsize内存
 *
 * @param [in/out] tsize   : u_int
 * @return  crf_out_t* 
 * @retval   
 * @see 
 * @note 
**/
crf_out_t* crf_create_out(u_int tsize )
{
	crf_out_t* crf_out = NULL;
	//1.为crf_out分配内存
	crf_out = (crf_out_t*) malloc(sizeof(crf_out_t));
	if (NULL == crf_out) {
		//fprintf(stderr,"malloc for crf_out error");
		ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_out error", __func__);		
		return NULL;
	}


	//2.为crf_term_out分配内存
	for (int i=0; i<MAX_NBEST_NUM; i++) {
		crf_term_out_t* crf_term = &crf_out->term_buf[i];
//		crf_term->crfbuf = (char *) malloc(sizeof(char) * (tsize + 1));
//		if (NULL == crf_term->crfbuf) {
//			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crfbuf error", __func__, i);
//			return NULL;
//		}
		//赋值crfbufsize
//		crf_term->crfbufsize = tsize;

		//3.为crftermoffsets分配内存
		crf_term->crftermoffsets = (u_int *)malloc(sizeof(u_int) * (tsize + 1));
		if (NULL == crf_term->crftermoffsets) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crftermoffsets error", __func__, i);
			return NULL;
			
		}
			
		//4.为crftermpos分配内存
		crf_term->crftermpos = (u_int *)malloc(sizeof(u_int) * (tsize + 1));
		if (NULL == crf_term->crftermpos) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crftermpos error",__func__, i);
			return NULL;
		}

		//5.为crftermprop分配内存
		crf_term->crftermprop = (crf_scw_property_t *)malloc(sizeof(crf_scw_property_t) * (tsize + 1));
		if (NULL == crf_term->crftermprop) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crftermprop error", __func__, i);
			return NULL;
			
		}

		//6.为crfprop分配内存
		/*crf_term->crfprop = (crfprop_t *)malloc(sizeof(crfprop_t) * tsize);
		if (NULL == crf_term->crfprop) {
			fprintf(stderr,"malloc for crf_term:%d crfprop error", i);
			return NULL;
			
		}
		*/
		//7.为neprop分配内存
		crf_term->crfneprop = (neprop_t *)malloc(sizeof(neprop_t) * (tsize + 1));
		if (NULL == crf_term->crfneprop) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crfneprop error",__func__,  i);
			return NULL;
			
		}

		//初始化
		//crf_term->crf_curpos = 0;
		crf_term->wordtotallen = 0;
		crf_term->crftermcount = 0;

		for (u_int j=0; j < tsize; j++) {
			crf_term->crftermoffsets[j] = 0;
			crf_term->crftermpos[j] = 0;
			crf_term->crftermprop[j].m_hprop = crf_term->crftermprop[j].m_lprop = 0;
			//crf_term->crfprop[j].weight = 0;
			crf_term->crfneprop[j].iNeName[0] = 0;
			crf_term->crfneprop[j].iWeight = 0;

		}
	}



	return crf_out;
}


/**
 * @param [in/out] pout   :
 * @return  void
 * @retval   
 * @see 
 * @note 
**/

void crf_create_des(crf_out_t* pout)
{

	//释放每个term_out的term_buf的内存
	for (int i=0; i<MAX_NBEST_NUM; i++) {
		crf_term_out_t* crf_term = &pout->term_buf[i];
		//if(crf_term->crfbuf != NULL){
		//	free(crf_term->crfbuf);
		//	crf_term->crfbuf = NULL;
		//}
		if(crf_term->crftermoffsets != NULL){
			free(crf_term->crftermoffsets);	
			crf_term->crftermoffsets = NULL;
		}
		if(crf_term->crftermpos != NULL){
			free(crf_term->crftermpos);
			crf_term->crftermpos = NULL;
		}
		if(crf_term->crftermprop != NULL){
			free(crf_term->crftermprop);
			crf_term->crftermprop = NULL;
		}
		if(crf_term->crfneprop != NULL){
			free(crf_term->crfneprop);
			crf_term->crfneprop = NULL;
		}
	}
	if(pout != NULL){
		free(pout);
		pout = NULL;
	}
}








int crf_segment_words(SegCrfTag* tagger, crf_out_t* pout, const char* inbuf, const int inlen, int nbest_value, int vlevel_value)
{
	int ret = 0;

	if ( NULL == tagger || NULL == pout || NULL == inbuf || inlen <0 || nbest_value <0 || vlevel_value < 0 || vlevel_value > 2) {
		ul_writelog(UL_LOG_FATAL, "[%s]: some of argument is error", __func__);
		return -1;
	}
	if(inlen >= CRF_MAX_QUERY_LEN){
		ul_writelog(UL_LOG_WARNING, "[%s]: the word to segment is too long(%d).", __func__, inlen);
		return -1;
	}

	ret = tagger->reset();
	if(ret < 0){
		ul_writelog(UL_LOG_WARNING, "[%s]: reset error", __func__);
		return -1;		
	}

	//3.格式化输入query

    //保存每一个字
	char row[CRF_MAX_QUERY_LEN * 2];
	int row_len = 0;


	//2.handle all characters
	int m, n;
	m = n = 0;
	int len = strlen(inbuf);

    while (m < len) {	
        if ((inbuf[m] & 0x80) == 0x80) {
            m += 2;
			row[n + row_len] = inbuf[n];
			row[n + row_len + 1] = inbuf[n + 1];
			row[n + row_len + 2] = 0;
            row_len++;
            n = m;
        } else {
            m++;
			row[n + row_len] = inbuf[n];
			row[n + row_len + 1] = 0;
            row_len++;
            n = m;
        }
    }

	ret = tagger->seg_add(row, row_len);
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: add word to tagger error", __func__);
		//fprintf(stderr,"tagger->my_add error");
		return -1;
	}

	//设置nbest，vlevel
	if (nbest_value > MAX_NBEST_NUM) {
		nbest_value = MAX_NBEST_NUM;
	}

	tagger->set_nbest(nbest_value);
	tagger->set_vlevel(vlevel_value);
	//4.parse
	ret = tagger->seg_parse();
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: segment parse error", __func__);
		return -1;
	}

	//5.my_output
	ret = tagger->seg_output(pout);
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: segment output error", __func__);
		return -1;
	}

	return 0;

}

int crf_segment_words_seg(SegCrfTag* tagger, crf_out_t* pout, char* pcInBuf, int niGroup, int nbest_value, int vlevel_value)
{
    int ret = 0;

    if ( NULL == tagger || NULL == pout || NULL == pcInBuf || niGroup <0 || nbest_value <0 || vlevel_value < 0) {
        ul_writelog(UL_LOG_FATAL, "[%s]: some of argument is error", __func__);
        return -1;
    }
    if(niGroup >= CRF_MAX_QUERY_LEN){
        ul_writelog(UL_LOG_WARNING, "[%s]: the word to segment is too long(%d).", __func__, niGroup);
        return -1;
    }

    ret = tagger->reset();
    if(ret < 0){
        ul_writelog(UL_LOG_WARNING, "[%s]: reset error", __func__);
        return -1;      
    }


    char *row;
    int row_len = 0;

    row = pcInBuf;
    row_len = niGroup;

    ret = tagger->seg_add(row, row_len);
    if (ret < 0) {
        ul_writelog(UL_LOG_FATAL, "[%s]: add word to tagger error", __func__);
        //fprintf(stderr,"tagger->my_add error");
        return -1;
    }

    //设置nbest，vlevel
    if (nbest_value > MAX_NBEST_NUM) {
        nbest_value = MAX_NBEST_NUM;
    }

    tagger->set_nbest(nbest_value);
    tagger->set_vlevel(vlevel_value);
    //4.parse
    ret = tagger->seg_parse();
    if (ret < 0) {
        ul_writelog(UL_LOG_FATAL, "[%s]: segment parse error", __func__);
        return -1;
    }

    //5.my_output
    ret = tagger->seg_output(pout);
    if (ret < 0) {
        ul_writelog(UL_LOG_FATAL, "[%s]: segment output error", __func__);
        return -1;
    }

    return 0;

}



//CRF_t start

int CRF_t::create_out(u_int tsize)
{
	crf_out = NULL;
	//1.为crf_out分配内存
	crf_out = (crf_out_t*) malloc(sizeof(crf_out_t));
	if (NULL == crf_out) {
		ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_out error", __func__);
		return -1;
	}

	//2.为crf_term_out分配内存
	for (int i=0; i<MAX_NBEST_NUM; i++) {
		crf_term_out_t* crf_term = &crf_out->term_buf[i];
//		crf_term->crfbuf = (char *) malloc(sizeof(char) * (tsize + 1));
//		if (NULL == crf_term->crfbuf) {
//			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crfbuf error", __func__, i);
//			return -1;
//		}
//		//赋值crfbufsize
//		crf_term->crfbufsize = tsize;

		//3.为crftermoffsets分配内存
		crf_term->crftermoffsets = (u_int *)malloc(sizeof(u_int) * (tsize + 1));
		if (NULL == crf_term->crftermoffsets) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crftermoffsets error", __func__, i);
			return -1;
		}
			
		//4.为crftermpos分配内存
		crf_term->crftermpos = (u_int *)malloc(sizeof(u_int) * (tsize + 1));
		if (NULL == crf_term->crftermpos) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crftermpos error", __func__, i);
			return -1;
			
		}

		//5.为crftermprop分配内存
		crf_term->crftermprop = (crf_scw_property_t *)malloc(sizeof(crf_scw_property_t) * (tsize + 1));
		if (NULL == crf_term->crftermprop) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crftermprop error", __func__, i);
			return -1;
			
		}

		//6.为crfprop分配内存
		/*crf_term->crfprop = (crfprop_t *)malloc(sizeof(crfprop_t) * tsize);
		if (NULL == crf_term->crfprop) {
			fprintf(stderr,"malloc for crf_term:%d crfprop error", i);
			return NULL;
			
		}
		*/
		//7.为neprop分配内存
		crf_term->crfneprop = (neprop_t *)malloc(sizeof(neprop_t) * (tsize + 1));
		if (NULL == crf_term->crfneprop) {
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for crf_term:%d crfneprop error", __func__, i);
			return -1;
			
		}

		//初始化
		//crf_term->crf_curpos = 0;
		crf_term->wordtotallen = 0;
		crf_term->crftermcount = 0;

		for (u_int j=0; j < tsize; j++) {
			crf_term->crftermoffsets[j] = 0;
			crf_term->crftermpos[j] = 0;
			crf_term->crftermprop[j].m_hprop = crf_term->crftermprop[j].m_lprop = 0;
			//crf_term->crfprop[j].weight = 0;
			crf_term->crfneprop[j].iNeName[0] = 0;
			crf_term->crfneprop[j].iWeight = 0;

		}
	}

	return 0;
}

int CRF_t::queryformat(const char* inbuf, const int inlen)
{
	if (NULL == inbuf || inlen <0){
		ul_writelog(UL_LOG_FATAL, "[%s]: some of argument is error", __func__);
		return -1;
		}
    if(inlen >= CRF_MAX_QUERY_LEN){
		ul_writelog(UL_LOG_WARNING, "[%s]: the word to segment is too long(%d).", __func__, inlen);
		return -1;
	}

	int ret = 0;
	char query[CRF_MAX_QUERY_LEN];
	snprintf(query, inlen+1, "%s", inbuf);

	//保存每一个字
	//std::string row[CRF_MAX_QUERY_LEN];
	char row[CRF_MAX_QUERY_LEN * 2];

	int row_len = 0;

	//1.normalize query
//	ul_trans2bj(query, query);
//	ul_trans2bj_ext(query, query);
//	ul_trans2lower(query, query);


	//2.handle all characters
	int m, n;
	m = n = 0;
	int len = strlen(query);

    while (m < len) {	
        if (IsChnChar(query[m])) {
            m += 2;
			row[n + row_len] = query[n];
			row[n + row_len + 1] = query[n + 1];
			row[n + row_len + 2] = 0;
            row_len++;
            n = m;
        } else {
            m++;
			row[n + row_len] = query[n];
			row[n + row_len + 1] = 0;
            row_len++;
            n = m;
        }
    }

	ret = seg_add(row, row_len);
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: segment add error", __func__);
		return -1;
	}


	return 0;
}

int CRF_t::segment_words(const char* inbuf, const int inlen, int nbest_value, int vlevel_value)
{
	
	int ret = 0;

	if (NULL == crf_out ||  NULL == inbuf || inlen <0 || nbest_value <0 || vlevel_value < 0 || vlevel_value > 2) {
		ul_writelog(UL_LOG_FATAL, "[%s]: some of argument is error", __func__);
		return -1;
	}
	if(inlen >= CRF_MAX_QUERY_LEN){
		ul_writelog(UL_LOG_WARNING, "[%s]: the word to segment is too long(%d).", __func__, inlen);
		return -1;
	}

	ret = reset();
	if(ret < 0){
		ul_writelog(UL_LOG_WARNING, "[%s]: reset error", __func__);
		return -1;		
	}

	char row[CRF_MAX_QUERY_LEN * 2];
	int row_len = 0;

	//2.handle all characters
	int m, n;
	m = n = 0;
	int len = strlen(inbuf);

    while (m < len) {	
        if ((inbuf[m] & 0x80) == 0x80) {
            m += 2;
			row[n + row_len] = inbuf[n];
			row[n + row_len + 1] = inbuf[n + 1];
			row[n + row_len + 2] = 0;
            row_len++;
            n = m;
        } else {
            m++;
			row[n + row_len] = inbuf[n];
			row[n + row_len + 1] = 0;
            row_len++;
            n = m;
        }
    }

	ret = seg_add(row, row_len);
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: add word to tagger error", __func__);
		//fprintf(stderr,"tagger->my_add error");
		return -1;
	}


    //3.格式化输入query
	//设置nbest，vlevel
	if (nbest_value > MAX_NBEST_NUM) {
		nbest_value = MAX_NBEST_NUM;
	}

	set_nbest(nbest_value);
	set_vlevel(vlevel_value);
	//4.parse
	ret = seg_parse();
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: segment parse error", __func__);
		return -1;
	}

	//5.my_output
	ret = seg_output(crf_out);
	if (ret < 0) {
		ul_writelog(UL_LOG_FATAL, "[%s]: segment output error", __func__);
		return -1;
	}	


	return 0;
	
}



/**
 * @brief 为CRF_t的crf_out释放内存。
 * @param [in/out] pout   :
 * @return  void
 * @retval   
 * @see 
 * @note 
**/

int CRF_t::create_out_des()
{

	//释放每个term_out的term_buf的内存
	for (int i=0; i<MAX_NBEST_NUM; i++) {
		crf_term_out_t* crf_term = &crf_out->term_buf[i]; 
	
//		if(crf_term->crfbuf){
//			free(crf_term->crfbuf);
//			crf_term = NULL;
//		}
		if(crf_term->crftermoffsets){
			free(crf_term->crftermoffsets);
			crf_term->crftermoffsets = NULL;
		}
		if(crf_term->crftermpos){
			free(crf_term->crftermpos);
			crf_term->crftermpos = NULL;
		}
		if(crf_term->crftermprop){
			free(crf_term->crftermprop);
			crf_term->crftermprop = NULL;
		}
		if(crf_term->crfneprop){
			free(crf_term->crfneprop);
			crf_term->crfneprop = NULL;
		}
	}
	if(crf_out){
		free(crf_out);
		crf_out = NULL;
	}
	return 0;
}

/*
int crf_to_scwout(crf_scw_out_t* p_scw_out, crf_term_out_t* p_crf_term_out)
{

	if (NULL == p_scw_out || NULL == p_crf_term_out) {
		ul_writelog(UL_LOG_FATAL, "[%s]: p_scw_out or p_crf_term_out is null", __func__);
		return -1;
	}

	//填充word phrase sep result
	
	memcpy(p_scw_out->wpcompbuf, p_crf_term_out->crfbuf, p_crf_term_out->crfbufsize);

	p_scw_out->wpbsize = p_crf_term_out->crfbufsize;
	p_scw_out->wpb_curpos = p_crf_term_out->crf_curpos;
	p_scw_out->wpbtermcount = p_crf_term_out->crftermcount;
	
	memcpy(p_scw_out->wpbtermoffsets, p_crf_term_out->crftermoffsets,
			sizeof(u_int)*p_crf_term_out->crftermcount);

	memcpy(p_scw_out->wpbtermpos, p_crf_term_out->crftermpos,
			sizeof(u_int)*p_crf_term_out->crftermcount);

	memcpy(p_scw_out->spbtermprop, p_crf_term_out->crftermprop,
			sizeof(crf_scw_property_t)*p_crf_term_out->crftermcount);

	return 0;
}
*/
