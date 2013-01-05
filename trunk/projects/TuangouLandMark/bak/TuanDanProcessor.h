/**
 * @file TuanDanProcessor.h
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-03-14
 */

#ifndef TUANDANPROCESSOR_H
#define TUANDANPROCESSOR_H


#include "lookup.h"

using namespace std;

typedef struct _strict_item_t
{
	int id;
	char name[MAX_NAME_LEN];
	int type;
}strict_item_t;

class TuanDanProcessor
{
	public:

		TuanDanProcessor();
		~TuanDanProcessor();

	public:
		/**
		 * @brief 初始化实例 
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int TDProcessorInit(const char  *pConfigFile);
		/**
		 * @brief 读入所有数据
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int TDProcess();


		/**
		 * @brief 处理分类文件
		 *
		 * @Param cWeiboFile
		 * @Param cProcessedWeiboFile
		 */
		//int ProcessClassifiedData();

		/**
		 * @brief 处理分类文件的一行输入
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		//int ParseLineClassified(string szLine);

		/**
		 * @brief 处理原始json数据
		 *
		 * @Param szInputFile
		 */
		int ProcessTerms();
		/**
		 * @brief 处理一行json原始数据
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		int ParseLineTerms(string strLine);

		/**
		 * @brief 处理转发数据
		 *
		 * @Param 
		 */
		//int ProcessForwardData();

		/**
		 * @brief 解析一行转发数据
		 *
		 * @Param strLine
		 *
		 * @Returns   
		 */
		//int ParseLineForward(string strLine);

		/**
		 * @brief 处理评论数据 
		 *
		 * @Param szInputFile
		 */
		//int ProcessCommentData();

		int RecordTerm(char * term);

		bool IsGoodTerm(int postagid);

		int AddCooc(vector<u_int> &hash_values);

		/**
		 * @brief 处理一行评论数据 
		 *
		 * @Param szInputFile
		 */
		//int ParseLineComment(string strLine);


		/**
		 * @brief 计算所有微博权重
		 *
		 * @Returns   
		 */
		int CalcWeight();

		/**
		 * @brief 排序输出
		 *
		 * @Returns   
		 */
		int SortIndex(float threshold);
		int SortIndex(int threshold);
		int FiltIndex(int K);

		int GetIndexSlot();

		int DumpIndex();
		int DumpIndexAsText();


	private:


		/**
		 * @brief 加载切词库
		 *
		 * @Param cLexiconPath
		 *
		 * @Returns   
		 */
		int LoadLexicon(const char* cLexiconPath);

		void ReleaseWordSeg();


		/**
		 * @brief 微博内容判重
		 *
		 * @Param strContent
		 *
		 * @Returns   
		 */
		bool HaveDuplicates(string& strContent);


		void Clear();

/****************************以下为数据成员区**************************************/
	private:
		LEXICON * m_Lexicon; 
		WORD_SEGMENT *m_Wordseg ;
		unsigned long long m_WordSegFlags;

		// 微博的内容指纹
		set<unsigned long long> m_setContentKey;

		// 基本配置信息
		CConfigInfo* m_pConfigInfo;

		int mload_history;

		//hash map to record term in one weibo
		__gnu_cxx::hash_map<u_int,u_int> hmInWeibo;
		//vector<string> vTermInWeibo;

		//u_int m_term_num;
		//u_int m_relate_term_num;
		//u_int m_sorted_relate_term_num;
		//unsigned long long m_weibo_num;

		total_index_info_t m_total_info;

		__gnu_cxx::hash_map<u_int,u_int> mTermIndex;
		vector<term_info_t> mTermInfo;

		vector<relate_term_info_t> mRelateInfo;
		
		vector<int> mRelateIndexSlot;
		bool mNoMemory;
};
#endif
