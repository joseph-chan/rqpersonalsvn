/* 
包装类，把通用的CrfTag包装为分词专用的SegCrfTag类，主要作分词buffer和crf tag之间的数据转换
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

	//填充termbuf，vlevel=0不计算weight，=1计算所有weight，=2只对ne属性的term计算weight
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

