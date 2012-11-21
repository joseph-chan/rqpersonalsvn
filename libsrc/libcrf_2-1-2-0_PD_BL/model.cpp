


#include "model.h"

namespace CRFPP {

	static inline char *read_ptr(char **ptr, size_t size) {
		char *r = *ptr;
		*ptr += size;
		return r;
	}

	template <class T> static inline void read_static(char **ptr,
			T *value) {
		char *r = read_ptr(ptr, sizeof(T));
		memcpy(value, r, sizeof(T));
	}
	/*
	int CrfModel::getID(const char *key) {
		return da_.exactMatchSearch<Darts::DoubleArray::result_type>(key);
	}
	*/
	
	//加载model文件
	//返回值<0 为出错，=0为正常
  int CrfModel::load_model(const char *filename, float this_path_factor) {
		//CHECK_FALSE(mmap_.open(filename1)) << mmap_.what();
		//使用mmap读入模型文件
		//原来使用mmap加载双数组和alpha数组，现在改为全部复制到model的内存区域中
		Mmap <char> mmap_;
		FILE* fp = fopen(filename, "r");
		if(fp == NULL) {
			ul_writelog(UL_LOG_FATAL, "[%s]: open model file[%s] failed! Error[%m]", __func__, filename);
			return -1;
		}
		fclose(fp);
		
		if (!mmap_.open(filename)) {
			ul_writelog(UL_LOG_FATAL, "[%s]: open filename[%s] failed", __func__, filename);
			return -1;
		}

		char *ptr = mmap_.begin();
                //读入版本号
		read_static<unsigned int>(&ptr, &version);
		
		ul_writelog(UL_LOG_TRACE, "[%s]: VERSION = %d", __func__,version);
		//CHECK_FALSE(version_ / 100 == version / 100)
		//	<< "model version is different: " << version_
		//	<< " vs " << version << " : " << filename1;
		
		int type = 0;
		read_static<int>(&ptr, &type);
		read_static<double>(&ptr, &cost_factor_);
		read_static<unsigned int>(&ptr, &maxid_);
		read_static<unsigned int>(&ptr, &xsize_);

		unsigned int dsize = 0;
		read_static<unsigned int>(&ptr, &dsize);

		//使用字符串数组代替vector<string>保存类别的字符表示y_
		unsigned int y_str_size;
		read_static<unsigned int>(&ptr, &y_str_size);
		char *y_str = read_ptr(&ptr, y_str_size);
		size_t pos = 0;
		
		//减少y_占用内存
		/*  
		ysize_ = 0;
		while (pos < y_str_size) {
			//y_.push_back(y_str + pos);
			strncpy(y_[ysize_], y_str + pos, CRF_MAX_WORD_LEN);
			ysize_++;
			while (y_str[pos++] != '\0') {}
		}
		*/

		//先计算YSIZE和最大yname的长度
		ysize_ = 0;
		max_yname_len = 0;
		while(pos < y_str_size){
			ysize_++;
			unsigned int yname_len = 0;
			while(y_str[pos++] != '\0'){
				yname_len++;
			}
			if(yname_len > max_yname_len){
				max_yname_len = yname_len;
			}
		}
		//分配y_用来存放yname
		y_ = (char **)calloc(ysize_, sizeof(char *));
		if(NULL == y_){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for yname", __func__);
			return -1;
		}		
		for(unsigned int i = 0; i < ysize_; i++){
			y_[i] = (char*)calloc(max_yname_len + 1, sizeof(char));
			if(NULL == y_[i]){
				ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for yname[%d]", __func__, i);
				return -1;
			}			
		}
		//重新扫描复制yname
		pos = 0;
		unsigned int ycount = 0;
		while(pos < y_str_size){
			bzero(y_[ycount], max_yname_len + 1);
			strncpy(y_[ycount], y_str + pos, max_yname_len);
			while(y_str[pos++] != '\0'){
			}
			ycount++;
		}
		
		//debug
		/*
		for(unsigned int i = 0; i < ysize_; i++){
		fprintf(stderr, "y[%d]=[%s]\n", i, y_[i]);
			}
		*/

	//	fprintf(stderr,"ysize = %d\n",ysize_);
		// load unigram templs and bigram templs
		//模型使用字符串数组代替vector<string>存储，并为减少函数调用开销，CrfTag在标注中获得模板指针
		unigram_templs_num = 0;
		unigram_templs_ = (char **)calloc(MAX_TEMPLS_NUM, sizeof(char *));
		if(NULL == unigram_templs_){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for unigram templs", __func__);
			return -1;
		}
		bigram_templs_num = 0;
		bigram_templs_ = (char **)calloc(MAX_TEMPLS_NUM, sizeof(char *));
		if(NULL == bigram_templs_){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for bigram templs", __func__);
			return -1;
		}

		unsigned int tmpl_str_size;
		read_static<unsigned int>(&ptr, &tmpl_str_size);
		char *tmpl_str = read_ptr(&ptr, tmpl_str_size);
		pos = 0;
		while (pos < tmpl_str_size) {
			char *v = tmpl_str + pos;
			if (v[0] == '\0') {
				++pos;
			} else if (v[0] == 'U') {
				if(unigram_templs_num > MAX_TEMPLS_NUM){
					ul_writelog(UL_LOG_FATAL, "[%s]: too many unigram templs, unigram templs num=%d", __func__, unigram_templs_num);
					return -1;					
				}
				unigram_templs_[unigram_templs_num] = (char *)calloc(MAX_TEMPLS_LEN, sizeof(char));
				if(NULL == unigram_templs_[unigram_templs_num]){	
					ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for unigram templs", __func__);
					return -1;
				}
				strncpy(unigram_templs_[unigram_templs_num], v, MAX_TEMPLS_LEN);
				ul_writelog(UL_LOG_TRACE, "[%s]: unigram_templs[%d](%s)", __func__, unigram_templs_num, unigram_templs_[unigram_templs_num]);
				unigram_templs_num++;
			} else if (v[0] == 'B') {
				if(bigram_templs_num > MAX_TEMPLS_NUM){
					ul_writelog(UL_LOG_FATAL, "[%s]: too many bigram templs, bigram templs num=%d", __func__, bigram_templs_num);
					return -1;					
				}
				bigram_templs_[bigram_templs_num] = (char *)calloc(MAX_TEMPLS_LEN, sizeof(char));
				if(NULL == bigram_templs_[bigram_templs_num]){	
					ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for bigram templs", __func__);
					return -1;
				}
				strncpy(bigram_templs_[bigram_templs_num], v, MAX_TEMPLS_LEN);
				ul_writelog(UL_LOG_TRACE, "[%s]: bigram_templs[%d](%s)", __func__, bigram_templs_num, bigram_templs_[bigram_templs_num]);
				bigram_templs_num++;
			} else {
				//CHECK_FALSE(true) << "unknown type: " << v;
			}
			while (tmpl_str[pos++] != '\0') {}
		}
		
		//检查是否为单一B模板
		if( bigram_templs_num != 1 || strcmp(bigram_templs_[0],"B") !=0 )
	    {
		    ul_writelog(UL_LOG_WARNING,"this is not a single bigram templetes.\n");
		    return -1;	
		}
		//复制双数组da
		da_mem_ = (void*)malloc(dsize);
		if(da_mem_ == NULL){
			ul_writelog(UL_LOG_WARNING, "[%s]: malloc for da memory fail.", __func__);
			return -1;
		}	
		memcpy(da_mem_, ptr, dsize);
		ul_writelog(UL_LOG_DEBUG, "[%s]: copy double array to memory, dsize=%d", __func__, dsize);
		
		da_.set_array(da_mem_);
		ptr += dsize;
		//复制alpha数组，从float转为int，乘上CONV_INT_FACTOR。该因子较大时基本无精度损失
		float *alpha_float_ = reinterpret_cast<float *>(ptr);
		ptr += sizeof(alpha_float_[0]) * maxid_;
		
		alpha_int_ = (int*)calloc(maxid_ + 1, sizeof(int));
		if(alpha_int_ == NULL){
			ul_writelog(UL_LOG_WARNING, "[%s]: malloc for alpha int array fail.maxid=%d", __func__, maxid_);
			return -1;
		}
		int conv_alpha_int;
		for(unsigned int i = 0; i < maxid_; i++){
			conv_alpha_int = (int)(alpha_float_[i] * CONV_INT_FACTOR);
			alpha_int_[i] = conv_alpha_int;
	//		fprintf(stderr,"<<>>%d\n",alpha_int_[i]);
		}
		ul_writelog(UL_LOG_DEBUG, "[%s]: conv alpha from float to int, maxid=%d", __func__, maxid_);
		if( version == 100 )
		{
		    if (ptr != mmap_.end()) 
		    {
				ul_writelog(UL_LOG_FATAL, "[%s]: model file is broken: %s", __func__, filename);
				return -1;
		    }
		}
		else
		{
		  //my add
		    read_static<int>(&ptr, &dan_size); //读取单一模板的辅助数据结构的size
		    read_static<int>(&ptr, &zu_size);   //读取组合模板的辅助数据结构的size
		    dan_array = (int*)calloc(dan_size,sizeof(int)); 
		    zu_array = (int*)calloc(zu_size,sizeof(int));
		    if( dan_array == NULL || zu_array == NULL)
		    {
		        ul_writelog(UL_LOG_FATAL, "dan_array or zu_array is NULL");
				return -1;
		    }
		    ul_writelog(UL_LOG_DEBUG, "%d %d\n",dan_size,zu_size);	
		    memcpy(dan_array,ptr,dan_size*sizeof(int));
		    ptr+=dan_size*sizeof(int);
		    memcpy(zu_array,ptr,zu_size*sizeof(int));
		    ptr+=zu_size*sizeof(int);
		    if (ptr != mmap_.end()) 
		    {
				ul_writelog(UL_LOG_FATAL, "[%s]: model file is broken: %s", __func__, filename);
				return -1;
		    }
		}
		mmap_.close(); //释放mmap占用内存
		b_templs_alpha_p = get_b_templs_start(); //获取B指针
		if( NULL == b_templs_alpha_p )
		{
		    ul_writelog(UL_LOG_WARNING,"b_templs_alpha_p is NULL.\n");
			return -1;
		}
		
		path_factor = this_path_factor;
		for(unsigned int i = 0; i < ysize_ * ysize_; i++){
			*(b_templs_alpha_p + i) = int(*(b_templs_alpha_p + i) * path_factor * cost_factor_);
			/*
			fprintf(stderr, "%d ", *(b_templs_alpha_p + i));
			if((i+1) % ysize_ == 0){
				fprintf(stderr, "\n");	
			}
			*/
		}
		ul_writelog(UL_LOG_DEBUG, "[%s]: set path factor to %f", __func__, path_factor);
		model_init_stat = MODEL_INIT_FINISH;
		return 0;
	}

	//释放model占用空间
	//无返回值
	void CrfModel::clear() {
		if(da_mem_ != NULL){
			free(da_mem_);
			da_mem_ = NULL;
		}
		if(alpha_int_ != NULL){
			free(alpha_int_);
			alpha_int_ = NULL;
		}
		if(y_ != NULL){
			for(unsigned int i = 0; i < ysize_; i++){
				if(y_[i] != NULL){
					free(y_[i]);					
				}
				y_[i] = NULL;
			}
			free(y_);
			y_ = NULL;
		}
		if(unigram_templs_ != NULL){
			for(int i = 0; i < unigram_templs_num; i++){
				if(unigram_templs_[i] != NULL){
					free(unigram_templs_[i]);
				}
				unigram_templs_[i] = NULL;
			}
			free(unigram_templs_);
			unigram_templs_ = NULL;
		}
		if(bigram_templs_ != NULL){
			for(int i = 0; i < bigram_templs_num; i++){
				if(bigram_templs_[i] != NULL){
					free(bigram_templs_[i]);
				}
				bigram_templs_[i] = NULL;
			}
			free(bigram_templs_);
			bigram_templs_ = NULL;
			b_templs_alpha_p = NULL;
			//释放辅助结构
			if(dan_array != NULL)
			{
			   free(dan_array);
			}
			if(zu_array != NULL)
		        {
			  free(zu_array);
			}
			dan_array = NULL;
			zu_array = NULL;
		}
	}

}
