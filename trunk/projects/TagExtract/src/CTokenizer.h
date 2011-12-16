/*
* 最新封装了公司的分词器接口，以简单好用为宗旨，只封装了基本功能
*
* 作者：kaijiang
*
* 日期：2011-07-13
*
*/

#ifndef CTOKENIZER_H_
#define CTOKENIZER_H_

#include "lexicon.h"
#include "fromuni.h"
#include "touni.h"
#include <set>
#include <string>

using namespace std;

class CTokenizer
{
	public:
		// 获取单例
		static CTokenizer* GetInstance();

		// 释放单例
		static void ReleaseInstance();

	public:
		// 初始化分词器
		// 参数：
		//		cDicData -- 词典数据路径
		//		nLang -- 语言：0 -- 中文简体;1 -- 繁体
		// 返回：
		//		true -- 初始化成功
		//		false -- 初始化失败
		//
		bool InitInstance(const char* cDicData, int nLang);

		// 执行分词
		// 参数：
		//		cText -- 文字
		// 返回：
		//		-1 -- 输入文字为空
		//		-2 -- 分词器未初始化
		//		-3 -- 其他错误
		//		>=0 -- 分词结果数
		int DoSegment(const char* cText);

		// 获取分词结果
		inline WORD_SEGMENT* GetResult() const {return m_pResult;}

		// 获取分词结果数目
		int GetWordCount() const;

		// 获取指定位置上的词，位置不对则返回空字符串
		string GetWord(int index)const;

		// 获取指定位置上的词的词性，位置不对则返回空字符串
		string GetPosTag(int index)const;

		// 判断是否停用词
		bool IsStopWord(int index) const;

		// 获取IDF值
		int GetIDF(int index) const;

	private: 
		// 初始化停用词表
		void InitStopWordSet();

		// 构造函数和西沟函数
		CTokenizer();

		~CTokenizer();
		// 单例
		static CTokenizer* _instance;
	private:
		WORD_SEGMENT* m_pResult;				// 分词结果结构体指针
		set<unsigned char> m_setStopWord;		// 停用词表
		LEXICON * m_pLexicon;					// 词典
		unsigned long long  m_unAnalyserFlag;	// 分析分析器的设置
};
#endif
