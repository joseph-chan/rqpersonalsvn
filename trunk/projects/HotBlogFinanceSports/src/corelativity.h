#ifndef CORELATIVITY_H
#define CORELATIVITY_H
#include "checkArticle.h"
#include "get_keyword2.h"
#include <vector>
#include "lexicon.h"
struct Core_Information
{
	double topic_score;//正文得分
	double reply_score;//回复有意义程度得分
	double corelativity;//正文和回复相关度,待完善
	unsigned short topic_value;//正文评价
	//topic_value=0;正文为一般状态
	//topic_value=1;正文值得推荐
	//topic_value=2;此正文应过滤掉
	unsigned short reply_value;
	//reply_value=0;一般评论
	//reply_value=1;推荐,显示在第一页
	//reply_value=2;垃圾,应过滤掉
};
typedef struct hdict_t Hash;
int get_corelativity(WORD_SEGMENT *wordseg_t,WORD_SEGMENT *wordseg_r,char *topic,char * reply,     
Core_Information &ans);
int Init_CoreInfor(Core_Information &ans);
int mvFileInHash_Value(char *filename);
void hash_map_clear();//相关词hash_map的清空
double test_hash(char *keyword);
#endif

