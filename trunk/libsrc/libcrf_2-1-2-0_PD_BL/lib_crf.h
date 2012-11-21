/**************************************************************************
 *
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * $Id: lib_crf.h,v 1.4 2010/05/19 07:29:02 zhangchao01 Exp $ 
 * 
 **************************************************************************/
 
/**
 * @file lib_crf.h
 * @author xianjian(xianjian@baidu.com)
 * @date 2010/06/24 19:17:57
 * @version $Revision: 1.4 $ 
 * @brief 
 */

#ifndef  __LIB_CRF_H_
#define  __LIB_CRF_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <pthread.h>

#include "lib_crf_def.h"
#include "model.h"
#include "seg_tag.h"

#define COST_FACTOR 1.0
#define DEFAULT_NBEST 1
#define DEFAULT_VLEVEL 0

using namespace CRFPP;

extern CrfModel global_seg_crf_model;

/**
 * @brief Ϊcrf_out_t�����ڴ棬Ϊÿ��crf_term_out_t����tsize�ڴ�
 *
 * @param [in/out] tsize   : u_int
 * @return  crf_out_t* 
 * @retval   
 * @see 
 * @note 
**/
crf_out_t* crf_create_out(u_int tsize);


/**
 * @brief 
 *
 * @param [in/out] pout   :
 * @return  void
 * @retval   
 * @see 
 * @note 
**/
void crf_create_des(crf_out_t* pout);

/**
 * @brief ����ѵ��ģ�ͣ����ҽ�ģ�͸�ֵ��tagger
 *
 * @param [in/out] model   : CrfTag* ������ȫ�ֱ������ڵ���ǰ��Ҫ������ʵ��
 * @param [in/out] model_dir   : const char*
 * @param [in/out] model_file   : const char*
 * @return  int 
 * @retval   
 * @see 
 * @note 
**/
int crf_load_model(CrfModel* model, const char* model_name, float this_path_factor = DEFAULT_PATH_FACTOR);






/**
 * @brief ��inbuf����crf�д�
 *
 * @param [in/out] tagger   : CRFTaggerImpl*
 * @param [in/out] pout   : crf_out_t* �������crf_create_out()���г�ʼ��
 * @param [in/out] inbuf   : const char*
 * @param [in/out] inlen   : const int crf�ִʳ��ȣ����Ϊ256
 * @param [in/out] nbest   : int crf���ŵĽ���Ŀ�����Ϊ10, Ĭ��Ϊ1����ֻ������Ž��
 * @param [in/out] vlevel   : �з�term��weight�����0Ϊ�������1Ϊȫ�����2Ϊֻ��NE��ص�term���
 * @return  int 
 * @retval   
 * @see 
 * @note 
**/
int crf_segment_words(SegCrfTag* tagger, crf_out_t* pout, const char* inbuf, const int inlen, int nbest_value = DEFAULT_NBEST, int vlevel_value = DEFAULT_VLEVEL);


int crf_segment_words_seg(SegCrfTag* tagger, crf_out_t* pout, char* pcInBuf, int niGroup, int nbest_value = DEFAULT_NBEST, int vlevel_value = DEFAULT_VLEVEL);



/**
 * @brief ��crf_term_out��scw_outת��
 *
 * @param [in/out] p_scw_out   : crf_scw_out_t*
 * @param [in/out] p_crf_term_out   : crf_term_out_t*
 * @return  int 
 * @retval   
 * @see 
 * @note 
**/

//int crf_to_scwout(crf_scw_out_t* p_scw_out, crf_term_out_t* p_crf_term_out);




class CRF_t: public SegCrfTag {

	
		int queryformat(const char* inbuf, const int inlen);
	public:
		// ����Ľṹ��
		crf_out_t* crf_out;
		int load_model(char* model_name, float this_path_factor = DEFAULT_PATH_FACTOR);
		int create_out(u_int tsize);
		int create_out_des();
		int segment_words(const char* inbuf, const int inlen, int nbest_value = DEFAULT_NBEST, int vlevel_value = DEFAULT_VLEVEL);


};


#endif  //__LIB_CRF_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
