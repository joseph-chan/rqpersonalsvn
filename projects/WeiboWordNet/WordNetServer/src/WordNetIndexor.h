/**
* @file WordNetIndexor.h
* @brief 
* @author RuQiang (ruqiang@staff.sina.com.cn)
* @version 
* @date 2011-12-30
*/

#ifndef WORDNETINDEXOR_H
#define WORDNETINDEXOR_H


#include "WordNetInclude.h"

using namespace std;

class WordNetIndexor
{
	public:

		WordNetIndexor();
		~WordNetIndexor();

	public:


		//int DumpIndex();
	int LoadIndex(const char* word_index,const char* term_info,const char * relate_info,__gnu_cxx::hash_map<u_int,u_int> & TermIndex, vector<term_info_t> &TermInfo,vector<relate_term_info_t> &RelateInfo,total_index_info_t *total_info);


	private:

		//void Clear();

/****************************以下为数据成员区**************************************/
	private:


};
#endif
