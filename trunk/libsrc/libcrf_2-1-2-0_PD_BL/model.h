#ifndef CRF_MODEL_H_
#define CRF_MODEL_H_

#include "common.h"
#include "mmap.h"
#include "darts.h"

namespace CRFPP {

	class CrfModel {
		protected:
			unsigned int        maxid_;  //alpha数组的最大ID
			//float              *alpha_float_; //alpha数组
			int *alpha_int_;  //alpha数组(为加快运算速度，转化为int)
			double              cost_factor_; //计算cost时的相乘因子
			unsigned int        xsize_;  //模型中特征的size 

			char **unigram_templs_; //unigram模板
			int unigram_templs_num;  //unigram模板的数量
			char **bigram_templs_;  //bigram模板
			int bigram_templs_num;  //bigram模板的数量
			int *b_templs_alpha_p;    //B模板在alpha数组里的内存起始位置		
			//Mmap <char> mmap_; //使用mmap方式存放模型文件

			void *da_mem_;  //保存双数组的内存指针
			//获取b_templs_p指针
			inline int* get_b_templs_start()
			{
				int id = -1;
				size_t pos = 0;
				get_id(&bigram_templs_[0][0],&pos,&id); 
                                ul_writelog(UL_LOG_TRACE, "[%s]: B's position:%d", __func__, id);
				if( id == -1 )
				{
					return NULL;
				}
				return  alpha_int_+id;

			}	
			size_t ysize_; //类别数
			//char y_[MAX_Y_SIZE][CRF_MAX_WORD_LEN]; //类别字符表示数组
			char **y_; //类别字符表示数组
			unsigned int max_yname_len; //最大的yname长度
			float path_factor;  //边权值调整系数

			int model_init_stat;

		public:
			unsigned int version; //模型版本号,读取模型时读入
			int dan_size;  //单一模板的辅助数组大小
			int zu_size;   //组合模板的辅助数组大小 
			int *dan_array; //单一模板的辅助数组
			int *zu_array;   //组合模板的辅助数组
			Darts::DoubleArray da_; //双数组，用于特征对应ID的查找
			/*
			explicit CrfModel(): maxid_(0), alpha_int_(0),
				cost_factor_(1.0), xsize_(0), unigram_templs_num(0), bigram_templs_num(0), ysize_(0), max_yname_len(0), model_init_stat(MODEL_INIT_UNFINISH),
				dan_size(0),zu_size(0),dan_array(0),zu_array(0){y_ = NULL; unigram_templs_= NULL ;bigram_templs_ = NULL;da_mem_ = NULL; }
			*/
			explicit CrfModel(){
				maxid_ = 0;
				alpha_int_ = 0;
				cost_factor_ = 1.0;
				xsize_ = 0;
				unigram_templs_num = 0;
				unigram_templs_ = NULL;
				bigram_templs_num = 0;
				bigram_templs_ = NULL;
				ysize_ = 0;
				max_yname_len = 0;
				dan_size = 0;
				zu_size = 0;
				dan_array = NULL;
				zu_array = NULL;	
				da_mem_ = NULL;
				path_factor = 1.0;
				model_init_stat = MODEL_INIT_UNFINISH;
			}
			
			~CrfModel() {clear();}

			//释放model占用的资源
			void clear();
			//加载model文件
			int load_model(const char *filename, float path_factor = DEFAULT_PATH_FACTOR);
			//获取maxid_
			size_t size() const  { return maxid_; }
			//获取xsize
			size_t xsize() const { return xsize_; }
			//获取类别个数
			size_t ysize() const { return ysize_; }
			//获取类别i的字符表示
			const char* y(size_t i) const { return y_[i]; }
			//获取alpha数组地址(float类型)
			//const float *alpha_float() const { return const_cast<float *>(alpha_float_); }
			//获取alpha数组地址(int类型)
			const int *alpha_int() const {
				return const_cast<int *>(alpha_int_);
			}
			//设置cost factor
			void set_cost_factor(double cost_factor_value) { cost_factor_ = cost_factor_value; }
			//获取cost factor
			double cost_factor() const { return cost_factor_; }
			//获取key对应的特征id
			int get_id(const char *key,size_t *node_pos,int *id) const {			
				return da_.exactMatchSearch(key,node_pos,id);
			}
			//获取unigram模板指针
			const char** get_unigram_templs() const{			
				return const_cast<const char**>(unigram_templs_);
			}
			//获取B模板对应alpha数组的起始地址指针
			int* get_b_templs_alpha() const
			{
				return b_templs_alpha_p;
			}
			//获取bigram模板指针
			const char** get_bigram_templs() const{
				return const_cast<const char**>(bigram_templs_);
			}
			//获取unigram模板数目
			int get_unigram_templs_num() const{
				return unigram_templs_num;
			}		
			//获取bigram模板数目
			int get_bigram_templs_num() const{
				return bigram_templs_num;
			}
			//获取模型初始化状态
			//返回MODEL_INIT_STAT_FINISH为初始化完成，返回MODEL_INIT_STAT_UNFINISH为初始化未完成
			int get_model_init_stat() const{
				return model_init_stat;
			}
	};
}
#endif
