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
		//概率计算函数
		//static inline double toprob(Node *n, double Z) {
		double toprob(Node *n, double Z){
			//fprintf(stderr, "alpha(%f)beta(%f)cost(%d)Z(%f)\n", n->alpha,n->beta,n->cost,Z)
			//fprintf(stderr, "forwardbackward");
			return std::exp(n->alpha + n->beta - float(n->cost)/CONV_INT_FACTOR - Z);
			//return 0.0f;
		};
		
		//当vlevel=0并且nbest=1时，即只需要输出最优的结果并且不需要得到概率值时，forward-backward算法和A*算法不进行，viterbi算法后直接得到结果。此时速度是最快的
		unsigned int vlevel_; //是否需要概率计算, 0为不需要概率计算，>0为需要
		unsigned int nbest_; //需要输出多少个结果序列
		
		double cost_;  //路径得分
		double Z_;  //用于计算概率值
		
		unsigned int max_xsize_; 
		
		//以下部分为从模型中获取的指针和变量，以加速运算过程
		const CrfModel *model;
		unsigned int xsize_;
		const char **unigram_templs_;
		int unigram_templs_num;
		int *b_templs_alpha_p;    //B模板在alpha数组里的内存起始位置
		size_t ysize;
		const int *alpha_int_;
		double cost_factor_;
		unsigned int version; //模型版本号
		//从模型获取部分结束于此

		size_t word_num; //要标注的词数
		unsigned int next_time;  //执行next的次数
		
		//Node node[CRF_MAX_WORD_NUM][ MAX_Y_SIZE ];  //状态节点。代替原来的无向图   
		Node **node; //状态节点。代替原来的无向图, size = crf_max_word_num * ysize
		
		//char char_freelist_[CRF_MAX_WORD_NUM][CRF_MAX_WORD_LEN]; //保存词本身的数组
		//char **char_freelist_; //保存词本身的数组, size = crf_max_word_num * crf_max_word_len
		
		//int result_[CRF_MAX_WORD_NUM]; //结果取值
		int *result_; //结果取值, size = crf_max_word_num

		Heap *heap_queue; //使用最小堆求取next result
		
		int tag_init_stat; //tag初始化状态位
		int forward_backward_stat; //前向后向过程运行状态，0为未运行，1为已经运行
        //my add 数据结构

		unsigned int crf_max_word_num;
		unsigned int crf_max_word_len;
		unsigned int max_nbest;

		const int* dan_array;
		const int* zu_array;
		//设置节点
		void set_node( size_t i, size_t j) { node[i][j].x = i;
		   	node[i][j].y = j; }
		
		//获取相对于当前位置pos作p的偏移后对应位置的字符
		//返回当前位置pos作p的偏移后对应位置的字符
		const char *get_index(int in);
		
		//根据特征模板p和当前位置pos，拼合出特征函数feature_function
        //正常返回为0， 错误返回<0
		int apply_rule( const char *p,bool flag,size_t *in_pos,int *id); 
		
		//计算节点node[x][y]的alpha值
	    void calcAlpha(int x_num, int y_num);
		
    	//计算节点node[x][y]的beta值
	     void calcBeta(int x_num, int y_num);


		//计算输入串的特征ID
        //正常返回为0， 错误返回<0
		int buildFeatures();

		//初始化求nbest的数据结构
        //正常返回为0， 错误返回<0
		int initNbest();

		//前向后向算法
        //正常返回为0， 错误返回<0
		int forwardbackward();

		//viterbi算法，计算最优路径
        //正常返回为0， 错误返回<0
		int viterbi();

		//char *x_[CRF_MAX_WORD_NUM][CRF_MAX_WORD_ATTRIBUTE]; //序列向量的字符表示

		//size_t x_row_size[CRF_MAX_WORD_NUM][CRF_MAX_WORD_ATTRIBUTE]; //序列向量字符表示的长度。用于减少输出时的strlen计算

		
		// 2.1.1
		//简单化为单列标注，去掉了char_freelist_，直接存放到x_中，减少x_的层级
		char **x_; //保存词本身的数组, size = crf_max_word_num * crf_max_word_len
		size_t *x_row_size; //序列向量字符表示的长度。用于减少输出时的strlen计算, size = crf_max_word_num

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

		//获取模型的特征数
		size_t xsize() const {
			return xsize_;
		}  

		//判断该序列是否为空
		bool empty() const {
			if(word_num == 0)
				return true;
			else
				return false;
		}

		//获取类型数
		size_t get_ysize() const { 
			return ysize; 
		}

		//获取序列的词数
		size_t get_word_num() const {
			return word_num;	
		}

		//获取next的次数
		unsigned int get_next_time() const {
			return next_time;
		}

		//获取cost
		double cost() const { return cost_; }

		//获取Z值
		double get_Z() const { return Z_; }

		//获取概率值
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
		
 		//获取第i个词的类别
		size_t result(size_t i) const { return result_[i]; }
 		//获取第i个词的类别		
		size_t y(size_t i)  const    { return result_[i]; }

		//获取第i类的字符表示
		const char* yname(size_t i) const    { return model->y(i); }

		//获取第i个词的类别(字符表示)
		const char* y2(size_t i) const      { return yname(result_[i]); }

		/*
		//获取输入串的(i,j)位置上的值
		const char*  x(size_t i, size_t j) const { return x_[i][j]; }

		//获取输入串的(i,0)位置上的值
		const char** x(size_t i) const {
			return const_cast<const char **>(&x_[i][0]);
		}
		*/

		//每次推算完后用于重置重新计算
		int reset();

		//取下一个nbest的结果
		int next();

		//获取vlevel
		unsigned int get_vlevel() const {
			return vlevel_;
		}

		//获取nbest
		size_t get_nbest() const {
			return nbest_; 
		}

		//设置vlevel
		void set_vlevel(unsigned int vlevel_value) {
			vlevel_ = vlevel_value;
		}

		//设置nbest
		void set_nbest(size_t nbest) {
			if(nbest < max_nbest){
				nbest_ = nbest;
			}else{
				nbest_ = max_nbest;
			}
		}

		//添加词串，每个词之间用\0分隔，共从输入的row_p中读入row_len个词
		//正常返回为0， 错误返回<0
		int add(char *row_p, unsigned int row_len);

		//标注输入串过程，输出存于result[]中。如果设置了nbest>1，之后可以通过next获取nbest结果
		//正常返回为0， 错误返回<0		
		int parse();

		//使用模型初始化tag，必须先使用该函数初始化才能使用add和parse
		//正常返回为0， 错误返回<0
		int init_by_model(const CrfModel *model_p);

		//返回tag的初始化状态
		//返回TAG_INIT_FINISH为已经正确初始化，返回TAG_INIT_UNFINISH为未正确初始化
		int get_tag_init_stat() const{
			return tag_init_stat;
		}

		//清空tag, 释放占用的资源
		//正常返回为0， 错误返回<0
		int clear();
	};
	
}

#endif
