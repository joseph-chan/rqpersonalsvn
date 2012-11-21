#ifndef CRF_MODEL_H_
#define CRF_MODEL_H_

#include "common.h"
#include "mmap.h"
#include "darts.h"

namespace CRFPP {

	class CrfModel {
		protected:
			unsigned int        maxid_;  //alpha��������ID
			//float              *alpha_float_; //alpha����
			int *alpha_int_;  //alpha����(Ϊ�ӿ������ٶȣ�ת��Ϊint)
			double              cost_factor_; //����costʱ���������
			unsigned int        xsize_;  //ģ����������size 

			char **unigram_templs_; //unigramģ��
			int unigram_templs_num;  //unigramģ�������
			char **bigram_templs_;  //bigramģ��
			int bigram_templs_num;  //bigramģ�������
			int *b_templs_alpha_p;    //Bģ����alpha��������ڴ���ʼλ��		
			//Mmap <char> mmap_; //ʹ��mmap��ʽ���ģ���ļ�

			void *da_mem_;  //����˫������ڴ�ָ��
			//��ȡb_templs_pָ��
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
			size_t ysize_; //�����
			//char y_[MAX_Y_SIZE][CRF_MAX_WORD_LEN]; //����ַ���ʾ����
			char **y_; //����ַ���ʾ����
			unsigned int max_yname_len; //����yname����
			float path_factor;  //��Ȩֵ����ϵ��

			int model_init_stat;

		public:
			unsigned int version; //ģ�Ͱ汾��,��ȡģ��ʱ����
			int dan_size;  //��һģ��ĸ��������С
			int zu_size;   //���ģ��ĸ��������С 
			int *dan_array; //��һģ��ĸ�������
			int *zu_array;   //���ģ��ĸ�������
			Darts::DoubleArray da_; //˫���飬����������ӦID�Ĳ���
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

			//�ͷ�modelռ�õ���Դ
			void clear();
			//����model�ļ�
			int load_model(const char *filename, float path_factor = DEFAULT_PATH_FACTOR);
			//��ȡmaxid_
			size_t size() const  { return maxid_; }
			//��ȡxsize
			size_t xsize() const { return xsize_; }
			//��ȡ������
			size_t ysize() const { return ysize_; }
			//��ȡ���i���ַ���ʾ
			const char* y(size_t i) const { return y_[i]; }
			//��ȡalpha�����ַ(float����)
			//const float *alpha_float() const { return const_cast<float *>(alpha_float_); }
			//��ȡalpha�����ַ(int����)
			const int *alpha_int() const {
				return const_cast<int *>(alpha_int_);
			}
			//����cost factor
			void set_cost_factor(double cost_factor_value) { cost_factor_ = cost_factor_value; }
			//��ȡcost factor
			double cost_factor() const { return cost_factor_; }
			//��ȡkey��Ӧ������id
			int get_id(const char *key,size_t *node_pos,int *id) const {			
				return da_.exactMatchSearch(key,node_pos,id);
			}
			//��ȡunigramģ��ָ��
			const char** get_unigram_templs() const{			
				return const_cast<const char**>(unigram_templs_);
			}
			//��ȡBģ���Ӧalpha�������ʼ��ַָ��
			int* get_b_templs_alpha() const
			{
				return b_templs_alpha_p;
			}
			//��ȡbigramģ��ָ��
			const char** get_bigram_templs() const{
				return const_cast<const char**>(bigram_templs_);
			}
			//��ȡunigramģ����Ŀ
			int get_unigram_templs_num() const{
				return unigram_templs_num;
			}		
			//��ȡbigramģ����Ŀ
			int get_bigram_templs_num() const{
				return bigram_templs_num;
			}
			//��ȡģ�ͳ�ʼ��״̬
			//����MODEL_INIT_STAT_FINISHΪ��ʼ����ɣ�����MODEL_INIT_STAT_UNFINISHΪ��ʼ��δ���
			int get_model_init_stat() const{
				return model_init_stat;
			}
	};
}
#endif
