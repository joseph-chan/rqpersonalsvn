#ifndef WORDSEGSIMPLE_H
#define WORDSEGSIMPLE_H
#include <vector>
#include <string>
#include <map>
#include "lexicon.h"
#include "fromuni.h"
#include "touni.h"
using namespace std;
struct wordItem
{
	char word[64];//词
	int pid;//词性
	int idf;//重要性
	int tf;//词频
	int wordLen;//词的字符数量
	int semantic;//语义属性
	bool isErase;//该词是否需要抹掉
	bool isEngOrDigit;//是否是英文和数字 
};
int wordsegSimple(const WORD_SEGMENT *wordseg,vector<wordItem> &wordvec);
int get_wordtf(vector<wordItem> &wSimple,map<string,wordItem>&wordtf);
bool great_second(const pair<string,double> &m1,const pair<string,double> &m2);
#endif
