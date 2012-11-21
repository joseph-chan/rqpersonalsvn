#include "min_heap.h"

namespace CRFPP {
	Heap *heap_init(int max_size){
		Heap* H;
		H = (Heap*)malloc(sizeof(Heap));
		if(H == NULL){
			ul_writelog(UL_LOG_WARNING, "[%s]: malloc for heap fail", __func__);
			return NULL;
		}
		H->capacity = max_size;
		H->size = 0;
		H->elem_size = 0;
		H->elem_ptr_list = (QueueElement **)malloc((max_size+1) * sizeof(QueueElement*));
		if(H->elem_ptr_list == NULL){
			ul_writelog(UL_LOG_WARNING, "[%s]: malloc for heap element ptr list fail", __func__);
			return NULL;
		}
		H->elem_list = (QueueElement *)malloc((max_size+1) * sizeof(QueueElement));
		if(H->elem_list == NULL){
			ul_writelog(UL_LOG_WARNING, "[%s]: malloc for heap element list fail", __func__);
			return NULL;
		}
		H->elem_list[0].fx = INT_MIN;
		H->elem_ptr_list[0] = &(H->elem_list[0]);
		return H;
	}
	
	QueueElement *allc_from_heap(Heap *H){
		if(H->elem_size >= H->capacity){
			ul_writelog(UL_LOG_WARNING, "[%s]: heap full, can't allocate from heap", __func__);
			return NULL;
		}else{
			return &(H->elem_list[++H->elem_size]);
		}
	}

 	int heap_insert(QueueElement *qe, Heap *H){
		if(H->size >= H->capacity){
			ul_writelog(UL_LOG_WARNING, "[%s]: heap size full", __func__);		
			return -1;
		}
		int i = ++H->size;
		while(i != 1 && H->elem_ptr_list[i/2]->fx > qe->fx){
			H->elem_ptr_list[i] = H->elem_ptr_list[i/2];  //此时i还没有进行i/2操作		
			i /= 2;
		}
		H->elem_ptr_list[i] = qe;
		return 0;
	}
	
	QueueElement *heap_delete_min(Heap *H){
		QueueElement *min_elem = H->elem_ptr_list[1];  //堆是从第1号元素开始的
		QueueElement *last_elem = H->elem_ptr_list[H->size--];
		int i = 1, ci = 2;
		while(ci <= H->size){
			if(ci < H->size && H->elem_ptr_list[ci]->fx > H->elem_ptr_list[ci+1]->fx){
				ci++;
			}
			if(last_elem->fx <= H->elem_ptr_list[ci]->fx){
				break;
			}
			H->elem_ptr_list[i] = H->elem_ptr_list[ci];
			i = ci;
			ci *= 2;
		}
		H->elem_ptr_list[i] = last_elem;
		return min_elem;
	}

	bool is_heap_empty(Heap *H){
		return H->size==0;
	}
	
	QueueElement *find_min(Heap *H){
		return H->elem_ptr_list[1];
	}
	
	void heap_clear(Heap *H){
		if(H == NULL){
			return;
		}
		if(H->elem_list != NULL){
			free(H->elem_list);
			H->elem_list = NULL;
		}
		if(H->elem_ptr_list != NULL){
			free(H->elem_ptr_list);
			H->elem_ptr_list = NULL;
		}
		if(H != NULL){
			free(H);
			H = NULL;
		}
	}
	
	void heap_reset(Heap *H){
		if(H != NULL){
			H->size = 0;
			H->elem_size = 0;
		}
	}
	
}
