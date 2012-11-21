/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * $Id: tag.h,v 1.5 2010/06/24 05:44:51 xianjian Exp $ 
 * 
 **************************************************************************/
/**
 * @file tag.h
 * @author xianjian(xianjian@baidu.com)
 * @date 2010/10/07 12:00:00
 * @version $Revision: 1.5 $ 
 * @brief tagger
 */

#ifndef  __TAG_H_
#define  __TAG_H_

#include "min_heap.h"
#include <sys/time.h>
#include <cmath>
#include "lib_crf_def.h"
#include "common.h"
#include "node.h"
#include "mmap.h"
#include "darts.h"
#include "model.h"

#define FORWARD_BACKWARD_UNDO 0
#define FORWARD_BACKWARD_DONE 1

namespace CRFPP {
	
	class CrfTag {
	protected:
		//���ʼ��㺯��
		//static inline double toprob(Node *n, double Z) {
		double toprob(Node *n, double Z){
			//fprintf(stderr, "alpha(%f)beta(%f)cost(%d)Z(%f)\n", n->alpha,n->beta,n->cost,Z)
			//fprintf(stderr, "forwardbackward");
			return std::exp(n->alpha + n->beta - float(n->cost)/CONV_INT_FACTOR - Z);
			//return 0.0f;
		};
		
		//��vlevel=0����nbest=1ʱ����ֻ��Ҫ������ŵĽ�����Ҳ���Ҫ�õ�����ֵʱ��forward-backward�㷨��A*�㷨�����У�viterbi�㷨��ֱ�ӵõ��������ʱ�ٶ�������
		unsigned int vlevel_; //�Ƿ���Ҫ���ʼ���, 0Ϊ����Ҫ���ʼ��㣬>0Ϊ��Ҫ
		unsigned int nbest_; //��Ҫ������ٸ��������
		
		double cost_;  //·���÷�
		double Z_;  //���ڼ������ֵ
		
		unsigned int max_xsize_; 
		
		//���²���Ϊ��ģ���л�ȡ��ָ��ͱ������Լ����������
		const CrfModel *model;
		unsigned int xsize_;
		const char **unigram_templs_;
		int unigram_templs_num;
		int *b_templs_alpha_p;    //Bģ����alpha��������ڴ���ʼλ��
		size_t ysize;
		const int *alpha_int_;
		double cost_factor_;
		unsigned int version; //ģ�Ͱ汾��
		//��ģ�ͻ�ȡ���ֽ����ڴ�

		size_t word_num; //Ҫ��ע�Ĵ���
		unsigned int next_time;  //ִ��next�Ĵ���
		
		//Node node[CRF_MAX_WORD_NUM][ MAX_Y_SIZE ];  //״̬�ڵ㡣����ԭ��������ͼ   
		Node **node; //״̬�ڵ㡣����ԭ��������ͼ, size = crf_max_word_num * ysize
		
		//char char_freelist_[CRF_MAX_WORD_NUM][CRF_MAX_WORD_LEN]; //����ʱ��������
		//char **char_freelist_; //����ʱ��������, size = crf_max_word_num * crf_max_word_len
		
		//int result_[CRF_MAX_WORD_NUM]; //���ȡֵ
		int *result_; //���ȡֵ, size = crf_max_word_num

		Heap *heap_queue; //ʹ����С����ȡnext result
		
		int tag_init_stat; //tag��ʼ��״̬λ
		int forward_backward_stat; //ǰ������������״̬��0Ϊδ���У�1Ϊ�Ѿ�����
        //my add ���ݽṹ

		unsigned int crf_max_word_num;
		unsigned int crf_max_word_len;
		unsigned int max_nbest;

		const int* dan_array;
		const int* zu_array;
		//���ýڵ�
		void set_node( size_t i, size_t j) { node[i][j].x = i;
		   	node[i][j].y = j; }
		
		//��ȡ����ڵ�ǰλ��pos��p��ƫ�ƺ��Ӧλ�õ��ַ�
		//���ص�ǰλ��pos��p��ƫ�ƺ��Ӧλ�õ��ַ�
		const char *get_index(int in);
		
		//��������ģ��p�͵�ǰλ��pos��ƴ�ϳ���������feature_function
        //��������Ϊ0�� ���󷵻�<0
		int apply_rule( const char *p,bool flag,size_t *in_pos,int *id); 
		
		//����ڵ�node[x][y]��alphaֵ
	    void calcAlpha(int x_num, int y_num);
		
    	//����ڵ�node[x][y]��betaֵ
	     void calcBeta(int x_num, int y_num);


		//�������봮������ID
        //��������Ϊ0�� ���󷵻�<0
		int buildFeatures();

		//��ʼ����nbest�����ݽṹ
        //��������Ϊ0�� ���󷵻�<0
		int initNbest();

		//ǰ������㷨
        //��������Ϊ0�� ���󷵻�<0
		int forwardbackward();

		//viterbi�㷨����������·��
        //��������Ϊ0�� ���󷵻�<0
		int viterbi();

		//char *x_[CRF_MAX_WORD_NUM][CRF_MAX_WORD_ATTRIBUTE]; //�����������ַ���ʾ

		//size_t x_row_size[CRF_MAX_WORD_NUM][CRF_MAX_WORD_ATTRIBUTE]; //���������ַ���ʾ�ĳ��ȡ����ڼ������ʱ��strlen����

		
		// 2.1.1
		//�򵥻�Ϊ���б�ע��ȥ����char_freelist_��ֱ�Ӵ�ŵ�x_�У�����x_�Ĳ㼶
		char **x_; //����ʱ��������, size = crf_max_word_num * crf_max_word_len
		size_t *x_row_size; //���������ַ���ʾ�ĳ��ȡ����ڼ������ʱ��strlen����, size = crf_max_word_num

	public:
		/*
		explicit CrfTag(int this_crf_max_word_num = DEFAULT_CRF_MAX_WORD_NUM): 
		crf_max_word_num(this_crf_max_word_num),
		vlevel_(0), nbest_(0),
			cost_(0.0), Z_(0), max_xsize_(0), 
			model(NULL), xsize_(0), unigram_templs_(NULL), unigram_templs_num(0),  ysize(0), alpha_int_(NULL), cost_factor_(0.0), 
			word_num(0), next_time(0),
			heap_queue(NULL),
			tag_init_stat(TAG_INIT_UNFINISH),
			forward_backward_stat(FORWARD_BACKWARD_UNDO)
		{}
		*/
		explicit CrfTag(unsigned int this_max_nbest = DEFAULT_MAX_NBEST, unsigned int this_crf_max_word_num = DEFAULT_CRF_MAX_WORD_NUM, unsigned int this_crf_max_word_len = DEFAULT_CRF_MAX_WORD_LEN) {
			crf_max_word_num = this_crf_max_word_num;
			crf_max_word_len = this_crf_max_word_len;
			vlevel_ = 0;
			nbest_ = 0;
			cost_ = 0.0;
			Z_ = 0;
			max_xsize_ = 0;
			model = NULL;
			xsize_ = 0;
			unigram_templs_ = NULL;
			unigram_templs_num = 0;
			b_templs_alpha_p = NULL;
			ysize = 0;
			alpha_int_ = NULL;
			cost_factor_ = 0.0;
			word_num = 0;
			next_time = 0;
			heap_queue = NULL;
			node = NULL;
			x_ = NULL;
			x_row_size = NULL;
			result_ = NULL;
			max_nbest = this_max_nbest;
			tag_init_stat = TAG_INIT_UNFINISH;
			forward_backward_stat = FORWARD_BACKWARD_UNDO;
		}
		~CrfTag() { clear(); }

		void set_crf_max_word_num(unsigned int this_crf_max_word_num){
			crf_max_word_num = this_crf_max_word_num;
		}

		void set_this_crf_max_word_len(unsigned int this_crf_max_word_len){
			crf_max_word_len = this_crf_max_word_len;
		}

		//��ȡģ�͵�������
		size_t xsize() const {
			return xsize_;
		}  

		//�жϸ������Ƿ�Ϊ��
		bool empty() const {
			if(word_num == 0)
				return true;
			else
				return false;
		}

		//��ȡ������
		size_t get_ysize() const { 
			return ysize; 
		}

		//��ȡ���еĴ���
		size_t get_word_num() const {
			return word_num;	
		}

		//��ȡnext�Ĵ���
		unsigned int get_next_time() const {
			return next_time;
		}

		//��ȡcost
		double cost() const { return cost_; }

		//��ȡZֵ
		double get_Z() const { return Z_; }

		//��ȡ����ֵ
		/*
		double       prob() const {
			return std::exp(- cost_ - Z_);
		}
		
		double       prob(size_t i, size_t j) const {
			return toprob(node_[i][j], Z_);
		}

		double       prob(size_t i) const {
			//fprintf(stderr, "node[%d][%d]:Z_(%f)", i, result_[i], Z_);
			return toprob(node_[i][result_[i]], Z_);
		}
		*/

		double prob(){
			if(forward_backward_stat == FORWARD_BACKWARD_UNDO){
				if(forwardbackward() != 0){
					ul_writelog(UL_LOG_WARNING, "[%s]: forward-backward error", __func__);
					return -1;
				}
			}
			return std::exp(- cost_ - Z_);
		}
		
		double prob(size_t i, size_t j){
			if(forward_backward_stat == FORWARD_BACKWARD_UNDO){
				if(forwardbackward() != 0){
					ul_writelog(UL_LOG_WARNING, "[%s]: forward-backward error", __func__);
					return -1;
				}
			}
			return toprob(&node[i][j], Z_);
		}

		double prob(size_t i){
			//fprintf(stderr, "node[%d][%d]:Z_(%f)", i, result_[i], Z_);
			if(forward_backward_stat == FORWARD_BACKWARD_UNDO){
				if(forwardbackward() != 0){
					ul_writelog(UL_LOG_WARNING, "[%s]: forward-backward error", __func__);
					return -1;
				}
			}
			return toprob(&node[i][result_[i]], Z_);
		}
		
 		//��ȡ��i���ʵ����
		size_t result(size_t i) const { return result_[i]; }
 		//��ȡ��i���ʵ����		
		size_t y(size_t i)  const    { return result_[i]; }

		//��ȡ��i����ַ���ʾ
		const char* yname(size_t i) const    { return model->y(i); }

		//��ȡ��i���ʵ����(�ַ���ʾ)
		const char* y2(size_t i) const      { return yname(result_[i]); }

		/*
		//��ȡ���봮��(i,j)λ���ϵ�ֵ
		const char*  x(size_t i, size_t j) const { return x_[i][j]; }

		//��ȡ���봮��(i,0)λ���ϵ�ֵ
		const char** x(size_t i) const {
			return const_cast<const char **>(&x_[i][0]);
		}
		*/

		//ÿ��������������������¼���
		int reset();

		//ȡ��һ��nbest�Ľ��
		int next();

		//��ȡvlevel
		unsigned int get_vlevel() const {
			return vlevel_;
		}

		//��ȡnbest
		size_t get_nbest() const {
			return nbest_; 
		}

		//����vlevel
		void set_vlevel(unsigned int vlevel_value) {
			vlevel_ = vlevel_value;
		}

		//����nbest
		void set_nbest(size_t nbest) {
			if(nbest < max_nbest){
				nbest_ = nbest;
			}else{
				nbest_ = max_nbest;
			}
		}

		//��Ӵʴ���ÿ����֮����\0�ָ������������row_p�ж���row_len����
		//��������Ϊ0�� ���󷵻�<0
		int add(char *row_p, unsigned int row_len);

		//��ע���봮���̣��������result[]�С����������nbest>1��֮�����ͨ��next��ȡnbest���
		//��������Ϊ0�� ���󷵻�<0		
		int parse();

		//ʹ��ģ�ͳ�ʼ��tag��������ʹ�øú�����ʼ������ʹ��add��parse
		//��������Ϊ0�� ���󷵻�<0
		int init_by_model(const CrfModel *model_p);

		//����tag�ĳ�ʼ��״̬
		//����TAG_INIT_FINISHΪ�Ѿ���ȷ��ʼ��������TAG_INIT_UNFINISHΪδ��ȷ��ʼ��
		int get_tag_init_stat() const{
			return tag_init_stat;
		}

		//���tag, �ͷ�ռ�õ���Դ
		//��������Ϊ0�� ���󷵻�<0
		int clear();
	};
	
}

#endif
