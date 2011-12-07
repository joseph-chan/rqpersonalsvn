/**
 * 管理关键词，将关键词及其权重分数统一管理
 *
 * 作者：开江
 *
 * 日期：2011 08 10
 *
 */

#ifndef CKEYWORDSMANAGER_H
#define CKEYWORDSMANAGER_H
#include "strmatch.h"
#include <map>
#include <string>
#include <vector>

using namespace std;

class CKeyWordsManager
{
	public:
		static CKeyWordsManager* GetInstance();
		static void ReleaseInstance();

	private:
		static CKeyWordsManager* _instance;
		CKeyWordsManager();
		~CKeyWordsManager();

	public:
		// 添加一个关键词集合
		// 参数：
		//		strName -- 关键词集合唯一的名字
		//		cWordFile -- 关键词所在的文件
		// 返回：
		//		-1 -- 同名的集合已存在
		//		-2 -- 关键词文件不符合规范
		//		>=0 -- 加载的关键词个数
		int AddKeyWords(string strName, const char* cWordFile);

		// 添加一个ID集合,相当于数值型的关键词集合
		// 参数：
		//		strName -- 关键词集合唯一的名字
		//		cWordFile -- 关键词所在的文件
		// 返回：
		//		-1 -- 同名的集合已存在
		//		-2 -- 关键词文件不符合规范
		//		>=0 -- 加载的关键词个数
		int AddKeyIDs(string strName, const char* cWordFile);

		// 给输入的文本打分
		// 参数：
		//		strText -- 输入的文本
		//		strName -- 使用的关键词集合
		//		bSum -- 是否取分值和作为得分，默认取最大值作为得分
		// 返回：
		//		得分，对于出现异常，如没找对给定名字的关键词，返回小于0的分数
		int GetTextScore(string& strText, string strName, bool bSum = false);
		int GetIDScore(unsigned long long id, string strName);

		// 判断给定文本是否存在指定名称关键词集合的关键词
		// 参数：
		//		strText -- 文本
		//		strName -- 关键词名称
		//		vecKeyWords[OUT] -- 文本中存在的关键词及其出现的次数
		//		mapKeyWords -- 文本中存在的关键词及其分数
		// 返回：
		//		匹配的关键词个数
		int HaveKeyWords(string& strText, string strName);
		int HaveKeyWords(string& strText, string strName, vector< pair<string, int> >& vecKeyWords);
		int HaveKeyWords(string& strText, string strName, map<string, int>& mapKeyWords);
		int HaveKeyIDs(unsigned long long id, string strName);

		// 获取指定名称的关键词集合数目
		// 参数：
		//		strName -- 关键词集合名称
		// 返回：
		//		关键词数目
		int GetKeyWordsCount(string strName);

		// 获取指定名称指定位置的关键词
		// 参数：
		//		strName -- 关键词集合名称
		//		index -- 索引
		// 返回：关键词
		string GetKeyWord(string strName, int index);
	private:
		typedef struct KEYWORDS
		{
			MATCHENTRY* machine;
			map<string, int> mapScore;
		}KeyWords;

		// 根据关键词对评论进行评分
		//
		int ScoreText(MATCHENTRY* pKeyWords, map<string, int>& mapScore, string& strText, bool bSum);

		// 查找关键词
		//
		int FindKeyWord(const char *text,int &watchNum,MATCHENTRY *machine_ww,vector<pair<string,int> >& watchWord);

		// 装载关键词
		//
		MATCHENTRY* LoadKeyWords(const char* cFile, map<string, int>& mapScore);

		void LoadKeyIDs(const char* cFile, map<unsigned long long, int>& mapScore);
		
	private:
		map<string, KeyWords> m_mapKeyWords;
		map<string, map<unsigned long long, int> > m_mapKeyIDs;
};
#endif
