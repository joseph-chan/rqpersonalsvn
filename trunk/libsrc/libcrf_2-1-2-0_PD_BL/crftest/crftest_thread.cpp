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

#include "pub.h"
#include "lib_crf.h"
#include <sys/time.h>

#define AUTHOR "xianjian"
#define README "crftest util tool"
#define DESCR  "CRF分词测试工具"
#define SYNOPSIS "[options]... [files]..."

#define MAX_LINE_LEN 10240

options_data_t opt_data;


int nbest = 10;

CrfModel crf_model;
pthread_mutex_t g_read_lock;

FILE *g_fp = NULL;

int num = 0;

void output(crf_out_t* crf_out)
{
	
	int term_pos = 0;
	int term_len = 0;
	char* term = NULL;
	for (int i = 0; i < crf_out->nbest; i++) {
		crf_term_out_t *pout = &crf_out->term_buf[i];
		for (u_int j = 0; j < pout->crftermcount; j++) {
			
			term_pos = CRF_GET_TERM_POS(pout->crftermpos[j]);
			term_len = CRF_GET_TERM_LEN(pout->crftermpos[j]);

			//term = pout->crfbuf + term_pos;
			term[term_len] = 0;
			//printf("%d:%15s%10s%5d\n", i, term, pout->crfneprop[j].iNeName, pout->crfneprop[j].iWeight);
			if (pout->crfneprop[j].iNeName[0]==0)
				//printf("%s[NOR] ", term);
				printf("%s ", term);
			else
				printf("%s[%s] ", term, pout->crfneprop[j].iNeName);

		}
		printf("\n");
	}
}

/*
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

	gettimeofday(&s, NULL);	
	int num = 0;
	while (gets(line)) {
		

		//printf("=====================================\n");
	}

}
*/



int 
load_options( int argc, char** argv )
{
	//定义参数
	myopt_def_t defs[] = 
	{
		/* 这里添加命令行参数 */
		/* name,comment,absname,type,imin,imax */
		{"input","数据输入选择：1(键盘) 0(文件)，默认为键盘输入. ",'i',myoptInt,0,1},
		{"infile","选择文件输入时的输入数据文件，键盘输入时忽略此参数. ",'f',myoptString,0,0},
		{"threadnum","多线程线程数，默认为1",'n',myoptInt,1,10},
		{"modelfilename", "模型文件名",'m', myoptString, 0,0},
		{ NULL,NULL,0,0,0,0}
	};
    //解析参数
	MYOPT_PARSE(defs)
    //获取参数
	MYOPT_GETINT_DEFAULT("input",opt_data.input, 1)
	MYOPT_GETSTR_DEFAULT("infile",opt_data.infile,"sample.txt")
    MYOPT_GETINT_DEFAULT("threadnum",opt_data.threadnum,1)
	MYOPT_GETSTR_DEFAULT("modelfilename",opt_data.modelfilename,"crfdict/model_test")		

	//获取getopt_long以外的参数
	for (int i=optind;i<argc;++i)
	{
		printf("ex_va[%d] : %s\n",i,argv[i]);	
	}

	return 0;
}

int erase_tail_enter(char* line)
{
	int i=0;
	if(line == NULL)
		return 0;
	for(i=0; (line[i] != 0)&&(line[i] != '\r')&&(line[i] != '\n'); i++);
	line[i] = 0;
	return 1;
}

void* work_main(void *)
{
	//初始化线程数据

	/*
	stextend_thread_data_t stextend_thread_data;
	int ret = 0;
	ret = init_stextend_thread_data(&stextend_conf, &stextend_thread_data);
	if(ret < 0)
	{
		return NULL;
	}
	struct timeval start_time, end_time;
	
	char line[MAX_LINE_LEN];
	line[MAX_LINE_LEN - 1] = 0;
	char output[2048];
	*/
	struct timeval start_time, end_time;

	char line[MAX_LINE_LEN];
	line[MAX_LINE_LEN - 1] = 0;
	//char output[2048];

	int ret = 0;

	//2. 为crf_out分配内存
	crf_out_t* crf_out = crf_create_out(256);
	if (NULL == crf_out) {
		fprintf(stderr, "crf_create_out error\n");
		return NULL;
	}

	SegCrfTag crf_tag;
	crf_tag.seg_init_by_model(&crf_model);

	while(1)
	{
		pthread_mutex_lock(&g_read_lock);
		if(fgets(line, MAX_LINE_LEN -1, g_fp) != NULL)
		{
			num++;
			pthread_mutex_unlock(&g_read_lock);
		}
		else
		{
			pthread_mutex_unlock(&g_read_lock);
			break;
		}
		erase_tail_enter(line);
		gettimeofday(&start_time, NULL);
		ret = crf_segment_words(&crf_tag, crf_out, line, strlen(line), nbest);
		if (ret < 0) {
			fprintf(stderr, "FATAL: (%s) crf_segment_words error\n", line);
			//return -1;
			continue;
		}
		output(crf_out);

		gettimeofday(&end_time, NULL);
		//fprintf(stderr, "time[%ld]\n", (end_time.tv_sec - start_time.tv_sec)*1000000+end_time.tv_usec-start_time.tv_usec);
		if(ret < 0)
		{
			//return -1;
			fprintf(stderr, "return code:%d\n", ret);
		}
		else
		{
			;
		//	fprintf(stderr, "%s\n", output);
		}
	}
	crf_create_des(crf_out);

	return NULL;
}

int main(int argc,char** argv)
{
	signal(SIGPIPE, SIG_IGN);

	load_options( argc, argv );
	//输入选择
	int flag = opt_data.input;
	if(flag == 0){
		g_fp = fopen(opt_data.infile, "r");
		if(g_fp == NULL){
			printf("open [%s] failed, exit\nplease input filename [-f]\n", opt_data.infile);
			return -1;
		}
	}
	
	pthread_t *work_thrds = NULL;
	int thread_num = opt_data.threadnum;

	int ret = 0;


	struct timeval s = {0, 0};
	struct timeval e = {0, 0};
	//struct timeval total = {0, 0};

	unsigned int t = 0;

	/*
	//2. 为crf_out分配内存
	crf_out_t* crf_out = crf_create_out(256);
	if (NULL == crf_out) {
		fprintf(stderr, "crf_create_out error\n");
		return -1;
	}
	*/

	//3. 加载model
	ret = crf_load_model(&crf_model, opt_data.modelfilename);
	if (ret < 0) {
		fprintf(stderr, "crf_load_model error\n");
		return -1;
	}
	
	/*
	ret = load_stextend_conf_by_file("conf/", "stextend.conf", &stextend_conf);
	if(ret < 0)
	{
		return -1;
	}

	//初始化全局数据
	ret = init_stextend_global(&stextend_conf, &stextend_env);
	if(ret < 0)
	{
		return -1;
	}
	*/
	
	work_thrds = (pthread_t *)calloc(thread_num, sizeof(pthread_t));
	if (NULL == work_thrds)
    {	
        ul_writelog(UL_LOG_FATAL, "[%s]: fail to calloc pthreads",__func__);
        return -1;
    }

	pthread_mutex_init(&g_read_lock, NULL);

	for(int i = 0; i < thread_num; i++)
	{
		pthread_create(&work_thrds[i], NULL, work_main, NULL);
	}

    for (int i = 0; i < thread_num; i++)
    {
        pthread_join(work_thrds[i], NULL);
    }



    fclose(g_fp);
    g_fp = NULL;

    free(work_thrds);
    work_thrds = NULL;




	gettimeofday(&e, NULL);
	t = (e.tv_sec - s.tv_sec)*1000000 + (e.tv_usec - s.tv_usec);
	printf("total time: %u(us)\ttime per query: %lf(us)\n", t, double(t*1.0)/num);

	return 0;


	/*
	while(1){
		if(flag == 0){
			if(!fgets(line_buffer, sizeof(line_buffer), fp)){
				break;
			}
		}
		else{
			fprintf(stdout, "\nplease input query:\n");
			fgets(line_buffer, sizeof(line_buffer), stdin);
		}
		int end = strlen(line_buffer);
		end--;line_buffer[end] = 0;//cut return
		end--;
		while(line_buffer[end] == ' '){
			line_buffer[end] = 0;
			end --;
		}
		gettimeofday(&start_time, NULL);
		ret = extend_text(&stextend_env, &stextend_thread_data, line_buffer, output);
		gettimeofday(&end_time, NULL);
		fprintf(stderr, "time[%ld]\n", (end_time.tv_sec - start_time.tv_sec)*1000000+end_time.tv_usec-start_time.tv_usec);
		if(ret < 0)
		{
			//return -1;
			fprintf(stderr, "return code:%d", ret);
		}
		else
		{
			fprintf(stderr, "output: %s", output);
		}
		
	}
	*/
	return 0;
}






















/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
