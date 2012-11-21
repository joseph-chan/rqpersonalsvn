#include <stdio.h>
#include "scwdef.h"

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "ul_conf.h"
#include <sys/time.h>


using namespace std;
#ifdef __cplusplus
extern "C" {

	typedef map <string,string> hmap;

	typedef struct _thread_src{
		char* query;
		scw_out_t* pout;
	}thread_src;

	scw_worddict_t* g_pwdict=NULL;

	char g_model_file[256] = "\0";
	void sub_time(struct timeval *tv1, struct timeval *tv2, struct timeval *ret);

	void output(string RES[], scw_out_t* pout);
	//主线程初始化
	int main_thread_init(Ul_confdata* conf)
	{

		if(ul_getconfstr(conf,"model_dir",g_model_file) != 1)
		{
			fprintf(stderr,"read model_dir error!\n");
			return -1;
		}

		fprintf(stderr,"Model_Dir=%s\n",g_model_file);
		// 加载model	
		if((g_pwdict=scw_load_worddict(g_model_file))==NULL)
		{
			fprintf(stderr,"Load worddict failed. Filename=%s\n",g_model_file);
			exit(-1);
			//	return NULL;
		}

		return 0;
	}



	//主线程资源销毁
	void main_thread_des(void * arg)
	{	

		scw_destroy_worddict( g_pwdict);
		return;
	}

	//子线程初始化
	void * thread_resource_init(int count , int arg[])
	{
		thread_src* p = NULL;
		p = (thread_src*)calloc(1, sizeof(thread_src));
		if(p == NULL)
		{
			fprintf(stderr,"error: thread_src* calloc error.\n");
			return NULL;
		}
		memset(p, 0, sizeof(thread_src));

		u_int scw_out_flag = SCW_OUT_ALL | SCW_OUT_PROP;
		if((p->pout = scw_create_out(10000, scw_out_flag))==NULL)
		{
			fprintf(stderr,"Init the output buffer error.\n");
			return NULL;
		}
		return p;
	}

	//子线程资源销毁
	void thread_resource_des(void * arg)
	{
		thread_src* p = NULL;
		p = (thread_src*)arg;

		if(NULL == p)
			return ;

		scw_destroy_out( p->pout);
		if(p)
		{
			free (p);
		}
		p = NULL;

		return;
	}

	//线程运行接口
	void * thread_run(char* line,void * parg, int count, void *rarg[], int * status, struct timeval* t3)
	{
		thread_src* p = NULL;
		p = (thread_src*)parg;
		if(NULL == p)
		{
			return p;
		}

		struct timeval t1;
		struct timeval t2;    
		(*t3).tv_sec = 0;    
		(*t3).tv_usec = 0;

		p->query = line;
		int len = strlen( line );

		gettimeofday(&t1, NULL);


		int ret = scw_segment_words(g_pwdict,p->pout,line,len);
		if ( ret < 0 ) {		
			fprintf(stderr, "crf_segment_words error\n");		
			return NULL;
		}

		gettimeofday(&t2, NULL);
		sub_time(&t2,&t1,&t3[0]);

		return p;
	}

	//打印处理结果接口
	int process_result(void * arg, hmap * result,void * selfswitch )
	{
		thread_src* p = NULL;
		p = (thread_src*)arg;

		string RES[10];
		output(RES,p->pout);
		//使用hmap后的输出

		//写hmap * result
		stringstream ss;

		ss.str("");
		ss << p->query;
		(*result)["query"]=ss.str();


		(*result)["1.basic"] = RES[0];
		(*result)["2.comp"] = RES[1];
		(*result)["3.subph"] = RES[2];
		(*result)["4.newword"] = RES[3];
		(*result)["5.name"] = RES[4];
		(*result)["6.book"] = RES[5];

		return 0;
	}


	int filter_query(char* query)
	{
		int len = 0;
		len = strlen(query);
		//		if(len < 256 )
		return 0;
		//		else
		//			return -1;

	}

	void sub_time(struct timeval *tv1, struct timeval *tv2, struct timeval *ret) 
	{  
		if (tv1->tv_usec >= tv2->tv_usec)   
		{       
			ret->tv_sec = tv1->tv_sec - tv2->tv_sec;        
			ret->tv_usec = tv1->tv_usec - tv2->tv_usec; 
		}   
		else    
		{       
			ret->tv_sec = tv1->tv_sec - tv2->tv_sec - 1;        
			ret->tv_usec = 1000000 + tv1->tv_usec - tv2->tv_usec;   
		}
	}


	void output(string RES[], scw_out_t* pout)
	{

		stringstream ss;
		int pos = 0;
		int len = 0;
		u_int i = 0;

		char qryres[102400]={} ;
		int res_len = 0;
		qryres[0]=0;

		//		Basic Word Sep Result

		qryres[0]=0;

		res_len = 0;
		for(i=0;i<pout->wsbtermcount; i++)
		{
			
			pos = GET_TERM_POS(pout->wsbtermpos[i]);
			len = GET_TERM_LEN(pout->wsbtermpos[i]);
			
			strncat(qryres, pout->wordsepbuf+pos, len);
			res_len += len;
			strncat(qryres, "|", 1);
			res_len ++;
		}
		qryres[res_len]=0;
		ss.str("");
		ss << qryres;
		RES[0]=ss.str();


		// view word phrase compond result
		qryres[0]=0;
		res_len = 0;
		for( i = 0; i<pout->wpbtermcount; i++)
		{
			pos = GET_TERM_POS(pout->wpbtermpos[i]);
			len = GET_TERM_LEN(pout->wpbtermpos[i]);
			strncat(qryres, pout->wpcompbuf + pos, len);
			res_len += len;
			strncat(qryres, "|", 1);
			res_len ++;
		}
		qryres[res_len]=0;
		ss.str("");
		ss << qryres;
		RES[1]=ss.str();

		// view sub phrase result
		qryres[0]=0;
		res_len = 0;
		for(i=0; i<pout->spbtermcount; i++)
		{
			pos = GET_TERM_POS(pout->spbtermpos[i]);
			len = GET_TERM_LEN(pout->spbtermpos[i]);

			strncat(qryres, pout->subphrbuf+pos, len);
			res_len += len;
			strncat(qryres, "|", 1);
			res_len ++;
		}
		qryres[res_len]=0;
		ss.str("");
		
		if(res_len > 0)
		{
			ss << qryres;
			RES[2]=ss.str();
		}else{
			RES[2]="-NONE-";
		}


		// view newword result
		qryres[0]=0;
		res_len = 0;
		scw_newword_t* pnewword = pout->pnewword;
		for( i = 0; i<pnewword->newwordbtermcount; i++)
		{
			pos = GET_TERM_POS(pnewword->newwordbtermpos[i]);
			len = GET_TERM_LEN(pnewword->newwordbtermpos[i]);

			strncat(qryres, pnewword->newwordbuf+pos, len);
			res_len += len;
			strncat(qryres, "|", 1);
			res_len ++;
		}
		qryres[res_len]=0;
		ss.str("");

		if(res_len > 0)
		{
			ss << qryres;
			RES[3]=ss.str();
		}else{
			RES[3]="-NONE-";
		}

		// view NER result
		qryres[0]=0;
		res_len = 0;
		for(i=0; i<pout->namebtermcount; i++)
		{
			pos = GET_TERM_POS(pout->namebtermpos[i]);
			len = GET_TERM_LEN(pout->namebtermpos[i]);

			strncat(qryres, pout->namebuf+pos, len);
			res_len += len;
			strncat(qryres, "|", 1);
			res_len ++;
		}
		qryres[res_len]=0;
		ss.str("");
		if(res_len > 0)
		{
			ss << qryres;
			RES[4]=ss.str();
		}else{
			RES[4]="-NONE-";
		}

		// view the book name result
		qryres[0]=0;
		res_len = 0;
		for(i=0; i<pout->bnbtermcount; i++)
		{
			pos = GET_TERM_POS(pout->bnbtermpos[i]);
			len = GET_TERM_LEN(pout->bnbtermpos[i]);

			strncat(qryres, pout->booknamebuf+pos, len);
			res_len += len;
			strncat(qryres, "|", 1);
			res_len ++;
		}
		qryres[res_len]=0;
		ss.str("");
		if(res_len > 0)
		{
			ss << qryres;
			RES[5]=ss.str();
		}else{
			RES[5]="-NONE-";
		}

	}
}


#endif
