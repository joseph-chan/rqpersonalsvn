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

//ÿ��term����Ϣ
typedef struct term_info_t
{
	//int hs;
	char termstr[MAX_TERM_LENGTH];
	int postagid;
	int nTotalTF;
	int nTotalTFBin;
	int is_bad_num; //���Ե�ͳ�ơ�����ֻҪ���ִ��ԣ����������Ҫ�Ĵ��Եı����������ƣ���Ҫ���term
	__gnu_cxx::hash_map<int,int> TermFreq; //first int: type name hash, second int :term freq
	__gnu_cxx::hash_map<int,int> TermFreqBin;//same as above,but term freq is binary in same weibo 
	__gnu_cxx::hash_map<int,int> TermScore; //first :type name hash, second: term score,tf-idf
}term_info;

//������ÿһ�������µ�term��Ϣ
typedef struct type_terminfo_t
{
	char termstr[MAX_TERM_LENGTH];
	int term_num;//�����������棬���term���������˶����΢����,ͬһ��΢�����ظ�����
	int term_weight;//���term��Ȩ��
	int ChiSquare;//����ֵ
	int output_weight;//������������Ȩ��,��һ�������Ǵӽ�����Ƶ�
	int postagid;//����
}type_terminfo;

typedef struct wbtype_info_t
{
	int weibo_num; //����������weibo����
	map<string, type_terminfo> type_term;//������µ�term��Ϣ
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
		// ����������Ϣ
		CConfigInfo* m_pConfigInfo;
		CTokenizer* m_spTokenizer;

		LEXICON * m_Lexicon; 
		WORD_SEGMENT *m_Wordseg ;
		unsigned long long m_WordSegFlags;

		//hash map to record term in one weibo
		__gnu_cxx::hash_map<int,int> hm_local;

		WeiboProcessor *pWBProcessor;

		int m_term_num;
		int m_weibo_num; //ԭʼ΢������-��������+���������������ᱻ��Ϊ����΢��
		__gnu_cxx::hash_map<int,int> mTermIndex;
		vector<term_info> vTermInfo;
		map<int, string> m_mapTypeInfo;//type �����ֺ�hashֵ
		//map<string,vector<map<string, type_terminfo> > > mTypeTermWeight;
		map<string, wbtype_info> mTypeTermWeight;
		map<string, vector< type_terminfo*> > m_voutput;
};

#endif
