/**
* @file word_net_server_pub.h
* @brief 
* @author RuQiang (ruqiang@staff.sina.com.cn)
* @version 
* @date 2011-12-30
*/

#include "WordNetInclude.h"

typedef struct _sort_item_t
{
	int sort_int;
	int appendix;
}sort_item_t;

typedef struct _global_data_t
{
	total_index_info_t total_index_info;

	__gnu_cxx::hash_map<u_int,u_int> TermIndex;

	vector<term_info_t> TermInfo;

	//vector<relate_term_info_t> RelateInfo;

	file_mmap_t file_mmap;

	sort_item_t* hot_term_buf;

}global_data_t;

