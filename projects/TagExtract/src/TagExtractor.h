/**
 * @file TagExtractor.h
 * @brief 
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 
 * @date 2011-12-07
 */

#ifndef TAGEXTRACTOR_H
#define TAGEXTRACTOR_H
#include "CConfigInfo.h"

#include "nx_log.h"
#include <ext/hash_fun.h>
#include <ext/hash_map>

#define MAX_TERM_LENGTH 64

typedef struct term_info_t
{
	int hs;
	char termstr[MAX_TERM_LENGTH];
	int nTotalTF;
	int nTotalTFB;
	__gnu_cxx::hash_map<int,int> TermFreq;
	__gnu_cxx::hash_map<int,int> TermFreqBin;
}term_info;

typedef struct original_weibo_line_t
{
	int filter;
	int type;
	string content;
}

class TagExtractor
{
	public:
		TagExtractor();
		~TagExtractor();

		int TagExtractInit(const char *pConfFile );

		int LoadWeibo();

		int GetTermFreq();

		int GetTermChiSquare();

		int AnalyzeGrammer();

	private:
		// 基本配置信息
		CConfigInfo* m_pConfigInfo;

		LEXICON * m_Lexicon; 
		WORD_SEGMENT *m_Wordseg ;
		unsigned long long m_WordSegFlags;
		__gnu_cxx::hash_map<int,int> TermIndex;
		vector<term_info> vTermInfo;
		WeiboProcessor *pWBProcessor;
}

#endif
