/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * $Id: crftest.cpp,v 1.6 2010/05/19 07:29:02 zhangchao01 Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file crftest.cpp
 * @author zhangwei01(zhangwei01@baidu.com)
 * @date 2010/03/03 12:23:39
 * @version $Revision: 1.6 $ 
 * @brief 
 */

#include "lib_crf.h"
#include <sys/time.h>
#include <iostream>



int nbest = 1;




void output(crf_out_t* crf_out, char *input_line)
{
	
	int term_pos = 0;
	int term_len = 0;
	char term[CRF_MAX_WORD_LEN];

	for (int i = 0; i < crf_out->nbest; i++) {
		float sum_weight = 0.0;
		crf_term_out_t *pout = &crf_out->term_buf[i];
		for (u_int j = 0; j < pout->crftermcount; j++) {
			
			term_pos = CRF_GET_TERM_POS(pout->crftermpos[j]);
			term_len = CRF_GET_TERM_LEN(pout->crftermpos[j]);

			//term = pout->crfbuf + term_pos;
			strncpy(term, input_line + term_pos, term_len);
			//term = input_line + term_pos;
			term[term_len] = 0;
			//printf("%d:%15s%10s%5d\n", i, term, pout->crfneprop[j].iNeName, pout->crfneprop[j].iWeight);
			//printf("%d:%s %s %d\n", i, term, pout->crfneprop[j].iNeName, pout->crfneprop[j].iWeight);
			sum_weight += pout->crfneprop[j].iWeight;
			if (pout->crfneprop[j].iNeName[0]==0)
				//printf("%s[NOR] ", term);
				printf("%s[%d] ", term, pout->crfneprop[j].iWeight);
			else
				printf("%s[%s,%d] ", term, pout->crfneprop[j].iNeName, pout->crfneprop[j].iWeight);
	
//			printf("%s|",term);
				}
		printf("\t--[%f][%f]", sum_weight, sum_weight/pout->crftermcount);
		printf("\n");
	}
}
int main(int argc, char* argv[])
{
	if(argc < 2){
		printf("Error: Please run with:\n./crftest model_file \n or \n./crftest model_file < your_query_list\n");
		return -1;
	}
	int ret = 0;
	struct timeval s = {0, 0};
	struct timeval e = {0, 0};
	//struct timeval total = {0, 0};

	unsigned int t = 0;

	CrfModel crf_model;

	//2. 为crf_out分配内存

	crf_out_t* crf_out = crf_create_out(256);
	if (NULL == crf_out) {
		fprintf(stderr, "crf_create_out error\n");
		return -1;
	}

	//3. 加载model
	ret = crf_load_model(&crf_model, argv[1]);
	if (ret < 0) {
		fprintf(stderr, "crf_load_model error\n");
		return -1;
	}
	//4.循环读入
	char line[10240];

	SegCrfTag crf_tag;
	crf_tag.seg_init_by_model(&crf_model);
	
	gettimeofday(&s, NULL);	
	int num = 0;
	while (std::cin.getline(line, 10240)) {

		
		ret = crf_segment_words(&crf_tag, crf_out, line, strlen(line), nbest,1 );
		if (ret < 0) {
			fprintf(stderr, "FATAL: (%s) crf_segment_words error\n", line);
			//return -1;
			continue;
		}
		num++;

		output(crf_out, line);

		//printf("=====================================\n");
	}

	crf_create_des(crf_out);


	gettimeofday(&e, NULL);
	t = (e.tv_sec - s.tv_sec)*1000000 + (e.tv_usec - s.tv_usec);
	printf("total time: %u(us)\ttime per query: %lf(us)\n", t, double(t*1.0)/num);

	return 0;
}





















/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
