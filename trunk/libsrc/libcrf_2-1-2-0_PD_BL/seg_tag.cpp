/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * $Id: seg_tag.cpp,v 1.0 2010/07/01 05:44:51 xianjian Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file seg_tag.cpp
 * @author xianjian(xianjian@baidu.com)
 * @date 2010/07/01 17:15:03
 * @version $Revision: 1.6 $ 
 * @brief 
 */

#include "seg_tag.h"


int SegCrfTag::seg_add(char *row_p, unsigned int row_len) {
	return add(row_p, row_len);
}

int SegCrfTag::seg_parse() {
	return parse(); 
}

int SegCrfTag::seg_init_by_model(const CrfModel *this_model) {
	int ret = init_by_model(this_model);
	if(ret < 0){
		ul_writelog(UL_LOG_FATAL, "[%s]: init by model fail", __func__);
		return -1;
	}
	use_ne = false;
	char *p = NULL;
	for(size_t i = 0; i < ysize; i++){
		yname_str[0] = 0;
		snprintf(yname_str, sizeof(yname_str), "%s", yname(i));
		p = strchr(yname_str, '_');
		if(p != NULL){
			use_ne = true;
			break;
		}
	}
	ul_writelog(UL_LOG_DEBUG, "[%s]: set use ne=%d", __func__, use_ne);
	return ret;
}

int SegCrfTag::seg_tag_clear(){
	return clear();
}

/*
int SegCrfTag::seg_initout(crf_term_out_t *pout) {
	pout->crf_curpos = 0;
	pout->wordtotallen = 0;
	pout->crftermcount = 0;
	return 0;
}

int SegCrfTag::seg_buildres(crf_term_out_t *pout, const char* buff, int num, float weight, char*
		nename_str) {

	int len = strlen(buff);

	if(pout->crf_curpos + len + 1 >= pout->crfbufsize){
		ul_writelog(UL_LOG_WARNING, "[%s]: term buff(%s) is too long to put into crfbuf", __func__, buff);
		return -1;
	}
	pout->crftermpos[pout->crftermcount] = ((len) << 24 | pout->crf_curpos & 0x00FFFFFF);
	strncpy(pout->crfbuf + pout->crf_curpos, buff, len);

	pout->crf_curpos += len;
	pout->crfbuf[pout->crf_curpos++] = '\t';
	pout->crfbuf[pout->crf_curpos] = 0;
	pout->crftermoffsets[pout->crftermcount] = pout->wordtotallen;
	pout->wordtotallen += len;

	//对crfprop赋值
	//pout->crfprop[pout->crftermcount].weight = int((weight/num)*1000);

	//对neprop赋值
	//pout->crfneprop[pout->crftermcount].iNeNameId = nename_id;
	snprintf(pout->crfneprop[pout->crftermcount].iNeName,
			sizeof(pout->crfneprop[pout->crftermcount].iNeName), "%s", nename_str);
	pout->crfneprop[pout->crftermcount].iWeight = int((weight/num)*1000);

	pout->crftermcount++;
	return 0;
}

int SegCrfTag::seg_print(crf_term_out_t *pout) {

	//std::string str;
	char term[CRF_MAX_WORD_LEN];
	size_t term_len = 0;

	char yname_str[16];
	char nename_str[11];
	int num = 0;
	char* p = NULL;
	float weight = 0.0;
	int ret = 0;
	char t = 0;
	
	for (size_t i = 0; i < get_word_num(); ++i) {

		//for (std::vector<const char*>::iterator it = x_[i].begin(); it != x_[i].end(); ++it) {
		//	std::cout << *it << '\t';
		//}

		//str = str + std::string(x_[i][0]);
		//fprintf(stderr, "[%d]%s\n", i, x_[i][0]);
		//chr = yname(y(i));
		strncpy(term+term_len, x_[i][0], x_row_size[i][0]);
		term_len += x_row_size[i][0];
		if(term_len > CRF_MAX_WORD_LEN){
			ul_writelog(UL_LOG_WARNING, "[%s]: term(%s) too long.", __func__, term);
		}
		
		snprintf(yname_str, sizeof(yname_str), "%s", yname(y(i)));
		
		nename_str[0] = 0;

		p = strchr(yname_str, '_');
		if (p != NULL) {
			
			snprintf(nename_str, sizeof(nename_str), "%s", p+1);
		}
		weight += prob(i);
		num ++;
		t = yname_str[0];

		if ('E' == t || 'S' == t) {
			//new term
			ret = seg_buildres(pout, term, num, weight, nename_str);
			if(ret < 0){
				ul_writelog(UL_LOG_WARNING, "[%s]: segment buildres error.", __func__);
				return -1;
			}

			//初始化
			//str.clear();
			
			weight = 0.0;
			num = 0;
			term[0] = 0;
			term_len = 0;

		} 

		
		//   if (vlevel_ >=1 ) std::cout << '/' << prob(i);
		//   if (vlevel_ >=2 ) {
		//   for (size_t j = 0; j < ysize_; ++j) {
		//   std::cout << '\t' << yname(j) << '/' << prob(i,j);
		//   }

		//   }

		//   std::cout << '\n';
	}
	//std::cout << '\n';
	return 0;
}
*/

int SegCrfTag::seg_termbuf_build(crf_term_out_t *term_buf, int vlevel) {
	char t = 0;
	char* p = NULL;
	int num = 0;
	float weight = 0.0;
	int term_len = 0;

	int crf_curpos = 0;
	term_buf->wordtotallen = 0;
	term_buf->crftermcount = 0;
	size_t this_word_num = get_word_num();
	for (size_t i = 0; i < this_word_num; ++i) {			
		if(term_len + x_row_size[i] > CRF_MAX_WORD_LEN){
			ul_writelog(UL_LOG_WARNING, "[%s]: term(%s) too long.", __func__, term);
			return -1;
		}
		
		strncpy(term+term_len, x_[i], x_row_size[i] + 1);
		term_len += x_row_size[i];
		//y_ptr = yname(y[i]);
		snprintf(yname_str, sizeof(yname_str), "%s", yname(y(i)));
		//debug start
		//fprintf(stderr, "%s(%s) ", x_[i][0], yname_str);
		//if(i == this_word_num - 1) fprintf(stderr, "\n");
		//debug end
		nename_str[0] = 0;
		if(use_ne){
			p = strchr(yname_str, '_');
			if (p != NULL) {
				snprintf(nename_str, sizeof(nename_str), "%s", p+1);
			}
		}
		t = yname_str[0];
		switch(vlevel){
		case 0:
			break;
		case 1:
			//对所有词均计算权值
			weight += prob(i);
			num++;
			break;
		case 2:
			//只对专名计算权值
			if(p != NULL){
				weight += prob(i);
				num++;
			}else{
				weight = 0.0;
				num = 0;
			}
			break;
		}
		/*
		if(vlevel){
			if(p != NULL)
				weight += prob(i);
			//fprintf(stderr, "prob(%d)=%f\n", i, prob(i));
			num ++;
		}
		*/
		if ('E' == t || 'S' == t || i == this_word_num - 1) {
		//	if(term_buf->crf_curpos + term_len + 1 >= term_buf->crfbufsize){
		//		ul_writelog(UL_LOG_WARNING, "[%s]: term buff(%s) is too long to put into crfbuf", __func__, term);
		//		return -1;
		//	}
			term_buf->crftermpos[term_buf->crftermcount] = ((term_len) << 24 | crf_curpos & 0x00FFFFFF);
			//strncpy(term_buf->crfbuf + term_buf->crf_curpos, term, term_len);
			
			crf_curpos += term_len;
			//term_buf->crfbuf[term_buf->crf_curpos++] = '\t';
			//term_buf->crfbuf[term_buf->crf_curpos] = 0;
			term_buf->crftermoffsets[term_buf->crftermcount] = term_buf->wordtotallen;
			term_buf->wordtotallen += term_len;
					
			//对crfprop赋值
			//pout->crfprop[pout->crftermcount].weight = int((weight/num)*1000);
			
			//对neprop赋值
			//pout->crfneprop[pout->crftermcount].iNeNameId = nename_id;
			if(nename_str[0] == 0){
				term_buf->crfneprop[term_buf->crftermcount].iNeName[0] = 0;
			}else{
				snprintf(term_buf->crfneprop[term_buf->crftermcount].iNeName, sizeof(term_buf->crfneprop[term_buf->crftermcount].iNeName), "%s", nename_str);
			}
			switch(vlevel){
			case 0:
				term_buf->crfneprop[term_buf->crftermcount].iWeight = 0;
				break;
			case 1:
				term_buf->crfneprop[term_buf->crftermcount].iWeight = int(weight*1000/num);
				weight = 0.0;
				num = 0;
				break;
			case 2:
				if(nename_str[0] == 0){
					term_buf->crfneprop[term_buf->crftermcount].iWeight = 0;
					weight = 0.0;
					num = 0;
				}else{
					term_buf->crfneprop[term_buf->crftermcount].iWeight = int(weight*1000/num);
					weight = 0.0;
					num = 0;
				}
				break;
			}
			/*
			if(has_weight){
				term_buf->crfneprop[term_buf->crftermcount].iWeight = int(weight*1000/num);
				weight = 0.0;
				num = 0;
			}else{
				term_buf->crfneprop[term_buf->crftermcount].iWeight = 0;
			}
			*/
			term_buf->crftermcount++;
			term[0] = 0;
			term_len = 0;
		}
	}
	return 0;
}


int SegCrfTag::seg_output(crf_out_t* pout) {
	//struct timeval start, end;	
	int n = 0;
	int ret = 0;
	
	if(max_nbest == 1 || (get_nbest() == 1)){  //当只需要最优结果并且无需概率计算时,并不使用next
		pout->nbest = 1;

		//term_buf = &pout->term_buf[0];
		ret = seg_termbuf_build(&pout->term_buf[0], get_vlevel());
		if(ret < 0){
			ul_writelog(UL_LOG_WARNING, "[%s]: termbuf build fail", __func__);
			return -1;
		}
		
		/*
		term_buf->crf_curpos = 0;
		term_buf->wordtotallen = 0;
		term_buf->crftermcount = 0;		
		for (size_t i = 0; i < get_word_num(); ++i) {			
			if(term_len + x_row_size[i][0] > CRF_MAX_WORD_LEN){
				ul_writelog(UL_LOG_WARNING, "[%s]: term(%s) too long.", __func__, term);
				return -1;
			}
			//fprintf(stderr, "i(%d)termlen(%d)row(%d)\t", i, term_len, x_row_size[i][0]);

			strncpy(term+term_len, x_[i][0], x_row_size[i][0]);
			term_len += x_row_size[i][0];
			snprintf(yname_str, sizeof(yname_str), "%s", yname(y(i)));
			nename_str[0] = 0;
			if(use_ne){
				p = strchr(yname_str, '_');
				if (p != NULL) {
					snprintf(nename_str, sizeof(nename_str), "%s", p+1);
				}
			}
			t = yname_str[0];
			if ('E' == t || 'S' == t) {
				if(term_buf->crf_curpos + term_len + 1 >= term_buf->crfbufsize){
					ul_writelog(UL_LOG_WARNING, "[%s]: term buff(%s) is too long to put into crfbuf", __func__, term);
					return -1;
				}
				term_buf->crftermpos[term_buf->crftermcount] = ((term_len) << 24 | term_buf->crf_curpos & 0x00FFFFFF);
				strncpy(term_buf->crfbuf + term_buf->crf_curpos, term, term_len);
				
				term_buf->crf_curpos += term_len;
				term_buf->crfbuf[term_buf->crf_curpos++] = '\t';
				term_buf->crfbuf[term_buf->crf_curpos] = 0;
				term_buf->crftermoffsets[term_buf->crftermcount] = term_buf->wordtotallen;
				term_buf->wordtotallen += term_len;
		
				//pout->crfneprop[pout->crftermcount].iNeNameId = nename_id;
				if(nename_str[0] == 0){
					term_buf->crfneprop[term_buf->crftermcount].iNeName[0] = 0;
				}else{
					snprintf(term_buf->crfneprop[term_buf->crftermcount].iNeName, sizeof(term_buf->crfneprop[term_buf->crftermcount].iNeName), "%s", nename_str);
				}
				//pout->crfneprop[pout->crftermcount].iWeight = int((weight/num)*1000);
				term_buf->crfneprop[term_buf->crftermcount].iWeight = 0;	
				term_buf->crftermcount++;
				term[0] = 0;
				term_len = 0;
			}
		}
		*/
	}else{
		int max_next_time = get_nbest();
		if(!use_ne && get_word_num() < 5){
			int pow_next_time = int(pow(2, (get_word_num() - 1)));
			if(pow_next_time < max_next_time){
				max_next_time = pow_next_time;
			}
		}
		for (n = 0; n < max_next_time; ++n) {
			//gettimeofday(&start, NULL);
			ret = next();
			if (ret < 0){
				ul_writelog(UL_LOG_WARNING, "[%s]: get next%d error", __func__, n);
				break;
			}
			//term_buf = &pout->term_buf[n];
			ret = seg_termbuf_build(&pout->term_buf[n], get_vlevel());
			if(ret < 0){
				ul_writelog(UL_LOG_WARNING, "[%s]: termbuf build fail", __func__);
				return -1;
			}
			//fprintf(stderr, "next time %d prob[%f]\n", n, prob());
			
			//std::cout << "# " << n << " " << prob() << '\n';
			/*
			term_buf->crf_curpos = 0;
			term_buf->wordtotallen = 0;
			term_buf->crftermcount = 0;

			for (size_t i = 0; i < get_word_num(); ++i) {			
				if(term_len + x_row_size[i][0] > CRF_MAX_WORD_LEN){
					ul_writelog(UL_LOG_WARNING, "[%s]: term(%s) too long.", __func__, term);
					return -1;
				}
				fprintf(stderr, "i(%d)termlen(%d)row(%d)\t", i, term_len, x_row_size[i][0]);

				strncpy(term+term_len, x_[i][0], x_row_size[i][0]);
				term_len += x_row_size[i][0];
				//y_ptr = yname(y[i]);
				snprintf(yname_str, sizeof(yname_str), "%s", yname(y(i)));
				nename_str[0] = 0;
				if(use_ne){
					p = strchr(yname_str, '_');
					if (p != NULL) {
						snprintf(nename_str, sizeof(nename_str), "%s", p+1);
					}
				}
				t = yname_str[0];
				weight += prob(i);
				num ++;
				if ('E' == t || 'S' == t) {
					if(term_buf->crf_curpos + term_len + 1 >= term_buf->crfbufsize){
						ul_writelog(UL_LOG_WARNING, "[%s]: term buff(%s) is too long to put into crfbuf", __func__, term);
						return -1;
					}
					term_buf->crftermpos[term_buf->crftermcount] = ((term_len) << 24 | term_buf->crf_curpos & 0x00FFFFFF);
					strncpy(term_buf->crfbuf + term_buf->crf_curpos, term, term_len);
					
					term_buf->crf_curpos += term_len;
					term_buf->crfbuf[term_buf->crf_curpos++] = '\t';
					term_buf->crfbuf[term_buf->crf_curpos] = 0;
					term_buf->crftermoffsets[term_buf->crftermcount] = term_buf->wordtotallen;
					term_buf->wordtotallen += term_len;
					
					//对crfprop赋值
					//pout->crfprop[pout->crftermcount].weight = int((weight/num)*1000);
					
					//对neprop赋值
					//pout->crfneprop[pout->crftermcount].iNeNameId = nename_id;
					if(nename_str[0] == 0){
						term_buf->crfneprop[term_buf->crftermcount].iNeName[0] = 0;
					}else{
						snprintf(term_buf->crfneprop[term_buf->crftermcount].iNeName, sizeof(term_buf->crfneprop[term_buf->crftermcount].iNeName), "%s", nename_str);
					}
					term_buf->crfneprop[term_buf->crftermcount].iWeight = int((weight/num)*1000);
					term_buf->crftermcount++;
					term[0] = 0;
					term_len = 0;
					weight = 0.0;
					num = 0;
				}
			}
			//gettimeofday(&end, NULL);
			//ul_writelog(UL_LOG_TRACE, "[%s]: [TIME]next%d[%ld]", __func__, int(n), (end.tv_sec - start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
			*/
		}
			
		pout->nbest = n;
	}

	return 0;
}

