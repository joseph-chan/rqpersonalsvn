/* $XJG: xslib/hdict.h,v 1.7 2006/09/22 03:51:15 jiagui Exp $ */
/*
   Author: XIONG Jiagui
   Date: 2006-07-05
 */
#ifndef HDICT_H_
#define HDICT_H_ 1


#ifdef __cplusplus
extern "C" {
#endif


typedef struct hdict_t hdict_t;


/*
   The item and the key are stored in memory as following figure.
        +----------+----------------------+
        |  item    |  key                 |
        +----------+----------------------+
   The item must be fix-sized, the key can be a string or fix-sized binary 
   block.  The item is stored aligned to sizeof(long) bytes in memory.  

   NB: There is no memory gap between the item and the key. The alignment 
   of the key depends on the size of the item.
 */


/*
   If /key_size/ == 0, the /key/ is treated as a string when hdict_get() 
   or hdict_test(). Otherwise, the /key/ is as an opaque binary block
   whose size is /key_size/.
 */
hdict_t *hdict_create(unsigned int slot_num, unsigned int key_size, unsigned int item_size);

void hdict_destroy(hdict_t *hd);


/* 
   Clear the hdict_t for reuse. The slot_num, key_size, item_size 
   remain unchanged.
 */
void hdict_clear(hdict_t *hd);


/*
   If the 'key' is not in the hdict_t already, insert it and return a new 
   item associated with the inserted key. Otherwise, the associated item
   of the key is returned.
   If the key is new (so is the item), the content of the new item will be
   memset() to zero before the function returns.
   This function should return a non-NULL pointer, except memory exhausted.
 */
void *hdict_insert(hdict_t *hd, const char *key);


/*
   Return a pointer to the item with the /key/. If no item with the
   specified /key/, NULL is returned.
 */
void *hdict_test(hdict_t *hd, const char *key);


/*
   Return the total number of items in the hdict_t.
 */
unsigned int hdict_total(hdict_t *hd);


/*
   The allocated memory can't be free()ed. When hdict_destroy() 
   or hdict_clear(), the hdict_alloc()ed memories are released then.
 */
void *hdict_alloc(hdict_t *hd, unsigned int size);



typedef struct {
	hdict_t *hdict;
	void *data;
} hdict_iter_t;


void hdict_iter_init(hdict_t *hd, hdict_iter_t *iter);

void *hdict_iter_next(hdict_iter_t *iter, char **p_key);


char *hdict_item_key(hdict_t *hd, void *item);


#ifdef __cplusplus
}
#endif

#endif

