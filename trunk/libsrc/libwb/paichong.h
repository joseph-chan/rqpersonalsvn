#ifndef PAICHONG_H
#define PAICHONG_H

#include "lexicon.h"
#include "wordsegSimple.h"
//#include "analysis.h"
#include <map>
#include <vector>
using namespace std;


#ifdef __cplusplus
extern "C"
{
#endif

struct ResData
{
	map<string,int> white_swatch;//文本白名单
	map<string,string> simplified_complex;//繁体到简体对照表
	map<string,int> white_nickname;//如 精彩语录 类白名单昵称
};

// 产生文本key
int textHash(WORD_SEGMENT *wordseg,unsigned long long &textkey,int level,int nMinTermCount = 15);
int textHash2(WORD_SEGMENT *wordseg,vector<unsigned long long> &textkeys,int level, int nMinTermCount = 15);

int chooseWord(WORD_SEGMENT *wordseg,vector<wordItem> &wordvec,int wordLevel);

//通用型string map的装载函数
int loadStringMap(const char *file,map<string,string> &smap);

//资源装载
int loadResource(const char *path,ResData &rdata);
#ifdef __cplusplus
}
#endif

#endif

