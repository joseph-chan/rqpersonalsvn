
#include "tag.h"
//#define PATH_COST(y1,y2)   \
//	int( (*(b_templs_alpha_p + y1*ysize + y2)) * cost_factor_ ) 
#define PATH_COST(y1, y2) (*(b_templs_alpha_p + y1 * ysize + y2))
namespace CRFPP {

	static const char *BOS[4] = { "_B-1", "_B-2", "_B-3", "_B-4"};
	static const char *EOS[4] = { "_B+1", "_B+2", "_B+3", "_B+4"};

	//static struct timeval start, end;
	//获取相对于当前位置pos作p的偏移后对应位置的字符                                                                                 
	void CrfTag::calcAlpha(int x_num, int y_num) 
	{
		double alpha = 0.0;	
		for(size_t i = 0; x_num && i < ysize; i++)
		{
			alpha = logsumexp(alpha,
					float(PATH_COST(i,y_num))/CONV_INT_FACTOR +node[x_num-1][i].alpha,
					(i == 0));
		}
		alpha += float(node[x_num][y_num].cost)/CONV_INT_FACTOR;
		node[x_num][y_num].alpha = alpha;
		//fprintf(stderr, " alpha(%f)cost(%f)", alpha, cost);
	}

	void CrfTag::calcBeta(int x_num, int y_num) 
	{
		double beta = 0.0;
		for(size_t i = 0; x_num != (int) word_num- 1 && i < ysize; i++)
		{
			beta = logsumexp(beta,
					float(PATH_COST(y_num,i))/CONV_INT_FACTOR +node[x_num+1][i].beta,
					(i == 0));
		}
		beta += float(node[x_num][y_num].cost)/CONV_INT_FACTOR;
		node[x_num][y_num].beta = beta;
		//fprintf(stderr, " beta(%f)cost(%f)", beta, cost);
	}

	//返回当前位置pos作p的偏移后对应位置的字符
	const char* CrfTag::get_index(int idx) {
		if (idx < 0)
			return BOS[-idx-1];
		if (idx >= static_cast<int>(word_num)){
			return EOS[idx - word_num];
		}
		return x_[idx];
	}
	//根据特征模板p和当前位置pos，拼合出特征函数feature_function                                                                     
	//正常返回为0， 错误返回<0
	int CrfTag::apply_rule( const char *p,bool flag,size_t *in_pos,int *id){ 
		*id = -1;
		int b;
		unsigned int tmp_p = 0;
		for (; *p; p++) {
			b = (model->da_).array_[*in_pos].base;
			tmp_p = b +(unsigned char)(*p) + 1;
			if (static_cast<unsigned int>(b) == (model->da_).array_[tmp_p].check)
				b = (model->da_).array_[tmp_p].base;
			else
				return -1;
			*in_pos = tmp_p;
			if( flag && *(p+1) == 0 )
			{
				tmp_p = b;
				int n = (model->da_).array_[tmp_p].base;
				if (static_cast<unsigned int>(b) == (model->da_).array_[tmp_p].check && n < 0)
				{
					*id = -n - 1;
				}
				return 0;
			}
		}
		return 0;
	}

	//使用模型初始化tag，必须先使用该函数初始化才能使用add和parse                                                                    
	//正常返回为0， 错误返回<0
	//int CrfTag::init_by_model(const CrfModel *model_p, bool this_use_next){
	int CrfTag::init_by_model(const CrfModel *model_p){

		//ul_writelog(UL_LOG_TRACE, "[%s]: init by model", __func__);
		if(model_p == NULL){
			ul_writelog(UL_LOG_FATAL, "[%s]: init by model fail, model NULL", __func__);
			return -1;
		}
		if(model_p->get_model_init_stat() != MODEL_INIT_FINISH){
			ul_writelog(UL_LOG_FATAL, "[%s]: init by model fail, model init unfinish", __func__);
			return -1;			
		}

		model = model_p;
		unigram_templs_ = model_p->get_unigram_templs();
		unigram_templs_num = model_p->get_unigram_templs_num();
		xsize_ = model_p->xsize();
		ysize = model_p->ysize();
		b_templs_alpha_p = model_p->get_b_templs_alpha();
		alpha_int_ = model_p->alpha_int();
		cost_factor_ = model_p->cost_factor();
		dan_array = model_p->dan_array;
		zu_array = model_p->zu_array;
		version = model_p->version; // 从模型中获得版本号

		// 2.1.1 start
		/*
		char_freelist_ = (char **)calloc(crf_max_word_num, sizeof(char *));
		if(char_freelist_ == NULL){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for char_freelist_", __func__);
			return -1;
		}
		for(unsigned int i = 0; i < crf_max_word_num; i++){
			char_freelist_[i] = (char *)calloc(crf_max_word_len, sizeof(char));
			if(char_freelist_[i] == NULL){
				ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for char_freelist_[%d]", __func__, i);
				return -1;
			}
		}
		*/
		
		node = (Node **)calloc(crf_max_word_num, sizeof(Node *));
		if(node == NULL){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for node", __func__);
			return -1;			
		}
		for(unsigned int i = 0; i < crf_max_word_num; i++){
			node[i] = (Node *)calloc(ysize, sizeof(Node));
			if(node[i] == NULL){
				ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for node[%d]", __func__, i);
				return -1;
			}
		}
		
		result_ = (int *)calloc(crf_max_word_num, sizeof(int));
		if(result_ == NULL){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for result", __func__);
			return -1;
		}
		
		//node中的x和y是不会变化的
		for(unsigned int i = 0; i < crf_max_word_num; i++){
			for(unsigned int j = 0; j < ysize; j++){
				node[i][j].x = i;
				node[i][j].y = j;
			}
		}

		x_ = (char **)calloc(crf_max_word_num, sizeof(char *));
		if(x_ == NULL){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for x_", __func__);
			return -1;
		}
		for(unsigned int i = 0; i < crf_max_word_num; i++){
			x_[i] = (char *)calloc(crf_max_word_len, sizeof(char));
			if(x_[i] == NULL){
				ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for x_[%d]", __func__, i);
				return -1;
			}
		}

		x_row_size = (size_t *)calloc(crf_max_word_num, sizeof(size_t));
		if(x_row_size == NULL){
			ul_writelog(UL_LOG_FATAL, "[%s]: fail to malloc for x_row_size", __func__);
			return -1;
		}
		// 2.1.1 end

		if(max_nbest > 1){
			heap_queue = heap_init(crf_max_word_num * ysize * ysize);
			if(heap_queue == NULL){
				ul_writelog(UL_LOG_FATAL, "[%s]: init heap fail.", __func__);
				return -1;			
			}
		}

		ul_writelog(UL_LOG_DEBUG, "[%s]: max_nbest=%d, max_word_num=%d, max_word_len=%d, max_y_size=%d", __func__, max_nbest, crf_max_word_num, crf_max_word_len, ysize);
		tag_init_stat = TAG_INIT_FINISH;
		forward_backward_stat = FORWARD_BACKWARD_UNDO;
		return 0;
	}

	static char* KEY[7] = {"U01:","U02:","U03:","U08:","U09:","U10:"} ;
	int CrfTag::buildFeatures() {
		if(word_num <= 0){
			ul_writelog(UL_LOG_WARNING, "[%s]: tagger to get feature matrix is empty.", __func__);
			return -1;
		}
		//特征函数
		int id = -1;
		if( version == 200 ) //单一B模板优化和模板固有优化以及固定模板查询优化
		{
			for (int cur = -1; cur <= (int) word_num; ++cur) 
			{	
				size_t pos = 0;
				//把BOS的cost加到第一个单词上
				if(cur == -1)
				{
					apply_rule(":",false,&pos,&id);
					apply_rule(BOS[0],true,&pos,&id);
					if( id != -1)
					{
						int id2 = dan_array[id];
						if( id2 != -1)
						{
							for(size_t j = 0; j < ysize; j++)
							{
								node[0][j].cost += alpha_int_[id2+j];
							}
						}
					}
					//把BOS/x_[0][0]的cost加到第一个单词上
					pos = 0;
					apply_rule(":",false,&pos,&id);
					if(apply_rule(BOS[0],false,&pos,&id) == -1)
					{
						continue;
					}
					if(apply_rule("/",false,&pos,&id) == -1)
					{
						continue;
					}
					apply_rule(x_[0],true,&pos,&id);
					if( id != -1)
					{
						int id2 = zu_array[id];
						if( id2 != -1)
						{
							for(size_t j = 0; j < ysize; j++)
							{
								node[0][j].cost += alpha_int_[id2+j];
							}  
						}
					}
				}
				else if( cur == (int) word_num)
				{
					//把EOS的cost加到最后一个单词上
					pos = 0;
					apply_rule(":",false,&pos,&id);
					apply_rule(EOS[0],true,&pos,&id);
					if( id != -1)
					{
						int id2 = dan_array[id+2];
						if( id2 != -1)
						{
							for(size_t j = 0; j < ysize; j++)
							{
								node[cur-1][j].cost += alpha_int_[id2+j];
							}
						}
					}
				}
				else
				{
					//把x_[cur][0]的cost加到前一个，本身，后一个上
					pos = 0;
					apply_rule(":",false,&pos,&id);
					apply_rule(x_[cur],true,&pos,&id);
					if( id != -1)
					{
						for( int k = 0; k < 3; k ++)
						{
							int id2 = dan_array[id+k];
							int now = cur-k+1;
							if( id2 != -1 && now >= 0 && now <(int) word_num )
							{  
								for(size_t j = 0; j < ysize; j++)
								{
									node[now][j].cost += alpha_int_[id2+j];
								}
							}
						}
					}
					//把x_[cur][0]/x_[cur+1][0]加到本身，后一个上
					pos = 0; 
					apply_rule(":",false,&pos,&id);
					if(apply_rule(x_[cur],false,&pos,&id) == -1)
					{
						goto DOWN;
					}
					if(apply_rule("/",false,&pos,&id) == -1)
					{
						goto DOWN;
					}
					if( cur + 1 == (int)word_num)
					{
						apply_rule(EOS[0],true,&pos,&id);
					}
					else
					{
						apply_rule(x_[cur+1],true,&pos,&id);
					}
					if( id != -1 )
					{
						for( int k = 0; k < 2; k++)
						{
							int id2 = zu_array[id+k];
							int now = cur -k + 1;
							if( id2 != -1 && now >= 0 && now < (int)word_num )
							{
								for(size_t j = 0; j < ysize; j++)
								{
									node[now][j].cost += alpha_int_[id2+j];
								}
							}
						}
					}
DOWN:
					//处理模板x[-1,0]/x[1,0]
					pos = 0;
					apply_rule("U10:",false,&pos,&id);
					if(apply_rule(get_index(cur-1),false,&pos,&id) == -1)
					{
						continue;
					}
					if(apply_rule("/",false,&pos,&id) == -1)
					{
						continue ;
					}
					apply_rule(get_index(cur+1),true,&pos,&id);
					if(id != -1)
					{
						for(size_t j = 0; j < ysize; j++)
						{
							node[cur][j].cost += alpha_int_[id+j];
						}
					} 
				}
			}
			for( size_t cur = 0; cur < word_num; cur++)
			{
				for(size_t j = 0; j < ysize; j++)
				{
					node[cur][j].cost = int(cost_factor_ * node[cur][j].cost);
				}
			}
		}
		else if( version == 100 ) //单一B模板优化和模板固有优化
		{
			for (size_t cur = 0; cur < word_num; ++cur)
			{
				for(size_t j = 0; j < ysize; j++)
				{
					node[cur][j].cost = 0;
				}
				for(int i = 0; i < unigram_templs_num; i++)
				{
					size_t pos = 0;
					apply_rule(KEY[i],false,&pos,&id);
					if(i <= 2 )
					{
						apply_rule(get_index(cur+i-1),true,&pos,&id);
					}
					else if ( i <= 4 )
					{
						if(apply_rule(get_index(cur+i-4),false,&pos,&id) ==-1)
						{
							goto DOWN2;
						}
						if( apply_rule("/",false,&pos,&id) == -1 )
						{
							goto DOWN2;
						}
						apply_rule(get_index(cur+i-3),true,&pos,&id);
					}
					else
					{
						if(apply_rule(get_index(cur-1),false,&pos,&id) ==-1)
						{
							goto DOWN2;
						} 
						if( apply_rule("/",false,&pos,&id) == -1)
						{
							goto DOWN2;
						}
						apply_rule(get_index(cur+1),true,&pos,&id);
					}
DOWN2:
					if(id != -1)
					{
						for(size_t j = 0; j < ysize; j++)
						{
							node[cur][j].cost += alpha_int_[id+j];
						}
					}
				} 
				for(size_t j = 0; j < ysize; j++)
				{
					node[cur][j].cost = int(cost_factor_ * node[cur][j].cost);
				}
			}
		}		  
		else  //模型版本错误
		{
		    ul_writelog(UL_LOG_FATAL, "[%s]:you use version=%d, please use 100 or 200", __func__,version);
		    return -1;
		}
		return 0;
	}


	int CrfTag::initNbest() {
		size_t k = word_num - 1;
		for (size_t i = 0; i < ysize; ++i) {
			//QueueElement *eos = &nbest_freelist_ptr[nbest_freelist_size++];
			QueueElement *eos = allc_from_heap(heap_queue);
			if(eos == NULL){
				ul_writelog(UL_LOG_WARNING, "[%s]: allocate from heap error", __func__);
				return -1;
			}
			eos->node = &node[k][i];
			eos->fx = -node[k][i].bestCost;
			eos->gx = -node[k][i].cost;
			eos->next = 0;
			if(heap_insert(eos, heap_queue) < 0){
				ul_writelog(UL_LOG_WARNING, "[%s]: can't push agenda", __func__);
				return -1;
			}
		}		
		return 0;
	}

	int CrfTag::next() {
		//		gettimeofday(&start, NULL);
		if(tag_init_stat != TAG_INIT_FINISH){
			ul_writelog(UL_LOG_FATAL, "[%s]: tag init stat error", __func__);
			return -1;			
		}
		if(max_nbest == 1){
			ul_writelog(UL_LOG_WARNING, "[%s]: set max nbest == 1 at process init_by_model, can't use next()", __func__);
			return -1;			
		}

		while(!is_heap_empty(heap_queue)){
			QueueElement *top = heap_delete_min(heap_queue);
			Node *rnode = top->node;

			if (rnode->x == 0) {
				for (QueueElement *n = top; n; n = n->next){
					result_[n->node->x] = n->node->y;
				}
				cost_ = top->gx;
				//		gettimeofday(&end, NULL);
				//		ul_writelog(UL_LOG_TRACE, "[%s]: [TIME]next[%ld]", __func__, (end.tv_sec - start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
				//ul_writelog(UL_LOG_TRACE, "[%s]: RESULT COST:%f", __func__, cost_);
				//fprintf(stderr, "next gx(%d)fx(%d)\n", top->gx, top->fx);
				next_time++;
				return 0;
			}

			for (size_t i = 0; rnode->x  && i < ysize; i++) {
				QueueElement *n = allc_from_heap(heap_queue);
				if(n == NULL){
					ul_writelog(UL_LOG_WARNING, "[%s]: allocate from heap error", __func__);
					return -1;
				}
				int x_num = (rnode->x) - 1;
				n->node = &node[x_num][i];
				n->gx   = -node[x_num][i].cost - PATH_COST(i,rnode->y) +  top->gx;
				n->fx   = -node[x_num][i].bestCost - PATH_COST(i,rnode->y) +  top->gx;
				//          |              h(x)                 |  |  g(x)  |
				n->next = top;
				if(heap_insert(n, heap_queue) < 0){
					ul_writelog(UL_LOG_WARNING, "[%s]: can't push agenda", __func__);
					return -1;
				}

			}
		}

		//		gettimeofday(&end, NULL);
		//		ul_writelog(UL_LOG_TRACE, "[%s]: [TIME]next[%ld]", __func__, (end.tv_sec - start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
		return 0;
	}


	//清空tag, 释放占用的资源                                                                                                        
	//正常返回为0， 错误返回<0
	int CrfTag::clear() {
		if(tag_init_stat != TAG_INIT_FINISH){
			ul_writelog(UL_LOG_TRACE, "[%s]: tag init stat error", __func__);
			return -1;			
		}

		// 2.1.1 start
		/*
		if(char_freelist_ != NULL){
			for(unsigned int i = 0; i < crf_max_word_num; i++){
				if(char_freelist_[i] != NULL){
					free(char_freelist_[i]);
					char_freelist_[i] = NULL;
				}
			}
			free(char_freelist_);
			char_freelist_ = NULL;
		}
		*/
		model = NULL;
		unigram_templs_ = NULL;
		alpha_int_ = NULL;
		

		if(node != NULL){
			for(unsigned int i = 0; i < crf_max_word_num; i++){
				if(node[i] != NULL){
					free(node[i]);
					node[i] = NULL;
				}
			}
			free(node);
			node = NULL;
		}

		if(result_ != NULL){
			free(result_);
			result_ = NULL;
		}

		if(x_ != NULL){
			for(unsigned int i = 0; i < crf_max_word_num; i++){
				if(x_[i] != NULL){
					free(x_[i]);
					x_[i] = NULL;
				}
			}
			free(x_);
			x_ = NULL;
		}
		
		if(x_row_size != NULL){
			free(x_row_size);
			x_row_size = NULL;
		}

		// 2.1.1 end

		word_num = 0;
		Z_ = cost_ = 0.0;
		
		heap_clear(heap_queue);
		heap_queue = NULL;
		
		tag_init_stat = TAG_INIT_UNFINISH;
		
		return 0;
	}

	int CrfTag::reset() {
		if(tag_init_stat != TAG_INIT_FINISH){
			ul_writelog(UL_LOG_FATAL, "[%s]: tag init unfinish.", __func__);
			return -1;
		}
		word_num = 0;
		next_time = 0;
		Z_ = cost_ = 0.0;
		forward_backward_stat = FORWARD_BACKWARD_UNDO;
		heap_reset(heap_queue);
		return 0;
	}

	int CrfTag::forwardbackward() {
		if (word_num == 0){
			return -1;
		}

		for (int i = 0; i < static_cast<int>(word_num); ++i)
		{
			for (size_t j = 0; j < ysize; ++j)
			{
				//fprintf(stderr, "{forwardbackward}calcAlpha(%d,%d)", i, j);
				calcAlpha(i,j);
			}				
		}

		for (int i = static_cast<int>(word_num - 1); i >= 0;  --i)
		{
			for (size_t j = 0; j < ysize; ++j)
			{
				//fprintf(stderr, "{forwardbackward}calcBeta(%d,%d)", i, j);
				calcBeta(i,j);					
			}	
		}

		Z_ = 0.0;
		for (size_t j = 0; j < ysize; ++j)
		{
			//fprintf(stderr, "{forwardbackward}calc Z(0,%d)", j);
			Z_ = logsumexp(Z_, node[0][j].beta, j == 0);
		}
		forward_backward_stat = FORWARD_BACKWARD_DONE;
		return 0;
	}

	//标注输入串过程，输出存于result[]中。如果设置了nbest>1，之后可以通过next获取nbest结果                                           
	//正常返回为0， 错误返回<0
	int CrfTag::viterbi() {
		for (size_t i = 0;   i < word_num; ++i) {
			for (size_t j = 0; j < ysize; ++j) {
				int bestc = INT_MIN;
				Node *best = 0;
				for(size_t k = 0; i && k < ysize; k++){
					int node_cost = node[i-1][k].bestCost + PATH_COST(k,j) + node[i][j].cost;
					if (node_cost > bestc) {
						bestc = node_cost;
						best  = &node[i-1][k];
					}					
				}

				node[i][j].prev     = best;
				node[i][j].bestCost = best ? bestc : node[i][j].cost;
				//fprintf(stderr,"bestCost(%f)\t", node_[i][j]->bestCost);
			}
		}

		//double bestc = -1e37;
		int bestc = INT_MIN;
		Node *best = 0;
		size_t s = word_num-1;
		for (size_t j = 0; j < ysize; ++j) {
			if (bestc < node[s][j].bestCost) {
				best  = &node[s][j];
				bestc = node[s][j].bestCost;
			}
		}

		for (Node *n = best; n; n = n->prev)
			result_[n->x] = n->y;

		cost_ = -node[word_num-1][result_[word_num-1]].bestCost;
		//fprintf(stderr, "\ncost_(%f)\n", cost_);
		return 0;
	}

	//添加词串，每个词之间用\0分隔，共从输入的row_p中读入row_len个词                                                                 
	//正常返回为0， 错误返回<0
	int CrfTag::add(char *row_p, unsigned int row_len) {
		if(model == NULL){
			ul_writelog(UL_LOG_FATAL, "[%s]: can't found model! Must use init_by_model() before add", __func__);
			return -1;			
		}
		if(tag_init_stat != TAG_INIT_FINISH){
			ul_writelog(UL_LOG_FATAL, "[%s]: tag init stat error, can't add", __func__);
			return -1;			
		}

		//放入x_中。如果add的词过多，取前crf_max_word_num个
		if(row_len > crf_max_word_num){
			row_len = crf_max_word_num;
			ul_writelog(UL_LOG_WARNING, "[%s]: too many terms(%d) to tag, max term num is %d", __func__, row_len, crf_max_word_num);
		}
		size_t this_row_len = 0;
		char *q;
		for (unsigned int i = 0; i < row_len; ++i) {
			this_row_len = strlen(row_p);
			q = x_[word_num];
			if(this_row_len > crf_max_word_num - 1){
				ul_writelog(UL_LOG_WARNING, "[%s]: too long row(%s)", __func__, row_p);
				this_row_len = CRF_MAX_WORD_LEN - 1;
			}
			strncpy(q, row_p, this_row_len + 1);
			//x_[i][0] = q;
			x_row_size[i] = this_row_len;
			result_[i] = 0;
			row_p += this_row_len + 1;
			word_num++;
		}
		
		for( size_t i = 0; i < word_num; i++ )
		{
			for( size_t j = 0; j < ysize; j++ )
			{
				//fprintf(stderr, "(%d,%d)%d,%d,%d\n",i, j, node[i][j].x, node[i][j].y, node[i][j].cost);
				//node[i][j].x = i;
				//node[i][j].y = j; 
				node[i][j].cost = 0;
			}
		}
		
		return 0;
	}

	//标注输入串过程，输出存于result[]中。如果设置了nbest>1，之后可以通过next获取nbest结果                                           
	//正常返回为0， 错误返回<0	
	int CrfTag::parse() {
		if(tag_init_stat != TAG_INIT_FINISH){
			ul_writelog(UL_LOG_FATAL, "[%s]: tag init stat error, can't parse", __func__);
			return -1;			
		}
		int ret = 0;
		//1.无待标注元素则返回
		if (word_num == 0){
			return 0;			
		}

		//	gettimeofday(&start, NULL);
		//2. buildFeatures
		//标注序列的特征化
		ret = buildFeatures();
		if (ret < 0) {
			ul_writelog(UL_LOG_FATAL, "[%s]: buildFeatures error", __func__);
			return -1;
		}
		//	gettimeofday(&end, NULL);
		//	ul_writelog(UL_LOG_TRACE, "[%s]: [TIME]buildFeatures[%ld]", __func__, (end.tv_sec - start.tv_sec)*1000000+end.tv_usec-start.tv_usec);	

		//4.forward-backward
		//当需要概率计算时，进行前向后想(forward-backward)算法
		if(vlevel_ == 1){
			//			gettimeofday(&start, NULL);
			ret = forwardbackward();
			if(ret < 0){
				ul_writelog(UL_LOG_WARNING, "[%s]: forward backward error", __func__);
				return -1;			
			}
			//			gettimeofday(&end, NULL);
			//			ul_writelog(UL_LOG_TRACE, "[%s]: [TIME]forwardbackward[%ld]", __func__, (end.tv_sec - start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
		}

		//	gettimeofday(&start, NULL);
		//5.viterbi
		//求取最优路径的viterbi算法
		ret = viterbi();
		if(ret < 0){
			ul_writelog(UL_LOG_FATAL, "[%s]: viterbi error", __func__);
			return -1;			
		}
		//	gettimeofday(&end, NULL);
		//	ul_writelog(UL_LOG_TRACE, "[%s]: [TIME]viterbi[%ld]", __func__, (end.tv_sec - start.tv_sec)*1000000+end.tv_usec-start.tv_usec);

		//6.initNbest
		//	求nbest(n>1)时的数据结构初始化，此后可以调用next()来获取nbest结果
		if ((max_nbest > 1) && (nbest_ > 1)){  //如果只求1-best，不需要使用initNbest()和next()获取结果
			//			gettimeofday(&start, NULL);
			ret = initNbest();
			if(ret < 0){
				ul_writelog(UL_LOG_FATAL, "[%s]: init n-best error", __func__);
				return -1;			
			}
			//			gettimeofday(&end, NULL);
			//			ul_writelog(UL_LOG_TRACE, "[%s]: [TIME]initNbest[%ld]", __func__, (end.tv_sec - start.tv_sec)*1000000+end.tv_usec-start.tv_usec);		
		}

		return 0;
	}


}
