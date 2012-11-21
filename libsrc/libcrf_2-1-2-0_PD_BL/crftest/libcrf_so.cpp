#include <stdio.h>
#include "lib_crf.h"

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
	SegCrfTag* tagger;
	crf_out_t* crf_out;
}thread_src;

CrfModel crf_model;
	

char model_file[256] = "\0";
	unsigned int nbest = 1;
	unsigned int crf_max_word_num = 1024;
	unsigned int crf_max_word_len = 100;
	float path_factor = 1.0;
	
void sub_time(struct timeval *tv1, struct timeval *tv2, struct timeval *ret);
//void output(string RES[], crf_out_t* crf_out);

void output(string RES[], crf_out_t* crf_out, char* query);

//主线程初始化
int main_thread_init(Ul_confdata* conf)
{

	if(ul_getconfstr(conf,"model_dir",model_file) != 1)
	{
		fprintf(stderr,"read model_dir error!\n");
		return -1;
	}
	char nb[5];
	if(ul_getconfstr(conf,"nbest",nb) != 1)	{
		fprintf(stderr,"Cannot get nbest! nbest default is 0;\n");
	}else	{
		nbest = atoi(nb);
		if(nbest < 1)
			nbest = 1;
		else if(nbest>10)
			nbest = 10;
	}

	if(ul_getconfstr(conf,"crf_max_word_num",nb) != 1)	{
		fprintf(stderr,"Cannot get crf max word num! default is 1000;\n");
	}else	{
		crf_max_word_num = atoi(nb);
	}

	if(ul_getconfstr(conf,"crf_max_word_len",nb) != 1)	{
		fprintf(stderr,"Cannot get crf max word len! default is 100;\n");
	}else	{
		crf_max_word_len = atoi(nb);
	}

	if(ul_getconfstr(conf, "path_factor", nb) != 1) {
		fprintf(stderr, "Cannot get path factor! default is 1.0;\n");
	}else{
		path_factor = atof(nb);
	}


	//crf_model = new CrfModel();

	//3. 加载model	
	int ret = crf_load_model(&crf_model, model_file, path_factor);
	if (0 != ret) {
		fprintf(stderr, "crf_load_model error\n");
		exit(0);
	//	return NULL;
	}
	
	return 0;
}



//主线程资源销毁
void main_thread_des(void * arg)
{	//--
	
	
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
	
	p->crf_out = crf_create_out(256);
	if(p->crf_out == NULL)
	{
		fprintf(stderr, "error: crf_create_out.\n");
		return NULL;
	}
	//p->tagger = new CRFTaggerImpl();
	//p->tagger = new SegCrfTag(1, 100, 100);
	p->tagger = new SegCrfTag(nbest, crf_max_word_num, crf_max_word_len);
	if(NULL==p->tagger){
		fprintf(stderr,"Error: create tagger error\n");
	}
	p->tagger->seg_init_by_model(&crf_model);
	return p;
}

//子线程资源销毁
void thread_resource_des(void * arg)
{
	thread_src* p = NULL;
	p = (thread_src*)arg;
	
	if(NULL == p)
		return ;

	if(p->tagger)
	{
		delete p->tagger;
		p->tagger = NULL;
	}
	crf_out_t* crf_out = p->crf_out;
	if(crf_out)
		crf_create_des(crf_out);
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
		return p;
	
	struct timeval t1;
	struct timeval t2;    
	(*t3).tv_sec = 0;    
	(*t3).tv_usec = 0;
	
	p->query = line;
	int len = strlen( line );

	gettimeofday(&t1, NULL);

	int ret = crf_segment_words(p->tagger, p->crf_out, line, len, nbest);
	if (0 != ret) {		
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
	output(RES,p->crf_out,p->query);
	//使用hmap后的输出
	
	//写hmap * result
	stringstream ss;

	ss << p->query;
	(*result)["query"]=ss.str();
	ss.str("");


	char nb[7] = {'n','b','e','s','t','0','\0'};
	for(int i = 0 ; i < p->crf_out->nbest; i++)
	{
		nb[5]='0'+i;
		(*result)[nb] = RES[i];
	}
	return 0;
}


int filter_query(char* query)
{
	int len = 0;
	len = strlen(query);
	if(len < DEFAULT_CRF_MAX_WORD_NUM )
		return 0;
	else
		return -1;

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


void output(string RES[], crf_out_t* crf_out, char* query)
{

	stringstream ss;


	int term_pos = 0;
	int term_len = 0;
	//char *term = NULL;
	char term[256];
	char qryres[256] ;

	for (int i = 0; i < crf_out->nbest; i++) {

		memset(qryres,0,sizeof(qryres));
		crf_term_out_t *pout = &crf_out->term_buf[i];

		for (u_int j = 0; j < pout->crftermcount; j++) {

			term_pos = CRF_GET_TERM_POS(pout->crftermpos[j]);
			term_len = CRF_GET_TERM_LEN(pout->crftermpos[j]);

			//term = pout->crfbuf + term_pos;	
			strncpy(term, query+term_pos, term_len);
			term[term_len] = 0;
			
			strncat(qryres,term,strlen(term));
			strcat(qryres,"|");
		}
		ss << qryres;
		RES[i]=ss.str();
		ss.str("");
	}
}
}


#endif
