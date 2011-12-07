#ifndef GET_KEYWORD_H
#define GET_KEYWORD_H
#include <string>
#include <map>
#include <vector>
#include "hdict.h"
#include "lexicon.h"
#include <math.h>
using namespace std;
struct keyword
{
	string word;
	int tf;
	double fWeight;
	char yuyi;
};
typedef struct hdict_t Hash;
double get_keyword(WORD_SEGMENT *wordseg,vector<keyword> &keyseg,int keynum);
//返回最后一个关键词的权重值
#endif
