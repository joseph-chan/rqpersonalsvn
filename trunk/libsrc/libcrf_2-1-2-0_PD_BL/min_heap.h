#ifndef __MIN_HEAP_H_
#define __MIN_HEAP_H_

#include <iostream>
#include "node.h"
#include "ul_log.h"

namespace CRFPP{
	struct QueueElement{
		Node *node;
		QueueElement *next;
		int fx;
		int gx;
	};
	
	struct Heap{
		int capacity;
		int elem_size; //size of elem_list
		int size;	// size of elem_ptr_list
		QueueElement **elem_ptr_list;
		QueueElement *elem_list;
	};

    //最小堆的初始化
	Heap *heap_init(int max_size);
	
	QueueElement *allc_from_heap(Heap *H);
	
	int heap_insert(QueueElement *qe, Heap *H);
	
	QueueElement *heap_delete_min(Heap *H);
	
	bool is_heap_empty(Heap *H);
	
	QueueElement *find_min(Heap *H);
	
	void heap_clear(Heap *H);
	
	void heap_reset(Heap *H);
	
}

#endif
