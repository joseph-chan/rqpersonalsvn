/**
 * @file WeiboProcessor.h
 * @brief  
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

#ifndef WORDNETINCLUDE_H
#define WORDNETINCLUDE_H

#include "CJParser.h"
#include "CStringTool.h"
#include "CTypeTool.h"
#include "CTextKeyTool.h"
#include "CMBTextTool.h"
#include "CConfigInfo.h"
#include "CResource.h"
#include "lexicon.h"
#include "fromuni.h"
#include "touni.h"
#include "ustring.h"
#include "checkArticle.h"
#include "nx_log.h"
#include <map>
#include <set>
#include <vector>
#include <math.h>
#include <fstream>
#include <iostream>
#include <ext/hash_fun.h>
#include <ext/hash_map>

using namespace std;

#define MAX_TERM_LENGTH 64

//每个term的信息
typedef struct _term_info_t
{
	//int hs;
	char termstr[MAX_TERM_LENGTH];
	u_int df_history; //only record good_term
	u_int df_now;
	u_int relate_term_num;
	//map<int,int> cooc_term; //first: term hash value, second: cooc_freq
	__gnu_cxx::hash_map<u_int,u_int> cooc_term; //first: term hash value, second: cooc_freq
}term_info_t;

typedef struct _relate_term_info_t
{
	u_int num_history; //only record good_term
	u_int num_now;
	u_int relation;
}relate_term_info_t;

typedef struct _total_index_info_t
{
	u_int term_num;
	u_int relate_term_num;
	unsigned long long weibo_num;
}total_index_info_t;
#endif
