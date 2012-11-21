/* 
��װ�࣬��ͨ�õ�CrfTag��װΪ�ִ�ר�õ�SegCrfTag�࣬��Ҫ���ִ�buffer��crf tag֮�������ת��
*/


#ifndef __SEG_TAG_H_
#define __SEG_TAG_H_

#include <sys/time.h>
#include "tag.h"
#include "model.h"
#include "lib_crf_def.h"

using namespace CRFPP;

class SegCrfTag : public CRFPP::CrfTag {
protected:
	bool use_ne;

	//for build termbuf
	char term[CRF_MAX_WORD_LEN];
	char yname_str[CRF_MAX_WORD_LEN];
	char nename_str[CRF_MAX_WORD_LEN];

	//���termbuf��vlevel=0������weight��=1��������weight��=2ֻ��ne���Ե�term����weight
	int seg_termbuf_build(crf_term_out_t *term_buf, int vlevel);

public:
	explicit SegCrfTag(unsigned int this_max_nbest = DEFAULT_MAX_NBEST, unsigned int this_crf_max_word_num = DEFAULT_CRF_MAX_WORD_NUM, unsigned int this_crf_max_word_len = DEFAULT_CRF_MAX_WORD_LEN):CrfTag(max_nbest, crf_max_word_num, crf_max_word_len){
		max_nbest = this_max_nbest;
		crf_max_word_num = this_crf_max_word_num;
		crf_max_word_len = this_crf_max_word_len;
	}

	~SegCrfTag(){
		seg_tag_clear();
	}
	/*
	int clear(){
		return clear();
	}
	*/

	int seg_tag_clear();	
	int seg_init_by_model(const CrfModel *crf_model);
	int seg_add(char *row_p, unsigned int row_len);
	int seg_parse();
	int seg_output(crf_out_t* pout);
};

#endif

