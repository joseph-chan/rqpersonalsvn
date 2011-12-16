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
#include "WeiboProcessor.h"
#include <ext/hash_fun.h>
#include <ext/hash_map>

#define MAX_TERM_LENGTH 64

//每个term的信息
typedef struct term_info_t
{
	//int hs;
	char termstr[MAX_TERM_LENGTH];
	int postagid;
	int nTotalTF;
	int nTotalTFBin;
	int is_bad_num; //词性的统计。我们只要几种词性，如果不是需要的词性的比例超过限制，则不要这个term
	__gnu_cxx::hash_map<int,int> TermFreq; //first int: type name hash, second int :term freq
	__gnu_cxx::hash_map<int,int> TermFreqBin;//same as above,but term freq is binary in same weibo 
	__gnu_cxx::hash_map<int,int> TermScore; //first :type name hash, second: term score,tf-idf
}term_info;

//归属于每一个类型下的term信息
typedef struct type_terminfo_t
{
	char termstr[MAX_TERM_LENGTH];
	int term_num;//在这个类别下面，这个term出现了在了多个条微博中,同一条微博不重复计数
	int term_weight;//这个term的权重
	int ChiSquare;//卡方值
	int output_weight;//最后输出的排序权重,不一定合理，是从结果反推的
	int postagid;//词性
}type_terminfo;

typedef struct wbtype_info_t
{
	int weibo_num; //此类别下面的weibo数量
	map<string, type_terminfo> type_term;//此类别下的term信息
}wbtype_info;


typedef struct original_weibo_line_t
{
	int filter;
	int type;
	string content;
}original_weibo_line;

class TagExtractor
{
	public:
		TagExtractor();
		~TagExtractor();

		int TagExtractInit(const char *pConfFile );

		int LoadWeibo();

		int ExtractTag();

		int GetTermChiSquare();

		int AnalyzeGrammer();

		int LoadLexicon(const char* cLexiconPath);
		
		int RecordTerm(char* term,unsigned long long mid, vector<string> vStrType ,WORD_ITEM seg_info);

		bool IsBadTerm(int postagid);

		void ReleaseWordSeg();

	private:
		// 基本配置信息
		CConfigInfo* m_pConfigInfo;
		CTokenizer* m_spTokenizer;

		LEXICON * m_Lexicon; 
		WORD_SEGMENT *m_Wordseg ;
		unsigned long long m_WordSegFlags;

		//hash map to record term in one weibo
		__gnu_cxx::hash_map<int,int> hm_local;

		WeiboProcessor *pWBProcessor;

		int m_term_num;
		int m_weibo_num; //原始微博数量-判重数量+兼类的数量。兼类会被记为两条微博
		__gnu_cxx::hash_map<int,int> mTermIndex;
		vector<term_info> vTermInfo;
		map<int, string> m_mapTypeInfo;//type 的名字和hash值
		//map<string,vector<map<string, type_terminfo> > > mTypeTermWeight;
		map<string, wbtype_info> mTypeTermWeight;
		map<string, vector< type_terminfo*> > m_voutput;
};

#endif
