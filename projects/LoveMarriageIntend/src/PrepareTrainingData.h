/**
 * @file PrepareTrainingData.h
 * @brief  
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

#ifndef WEIBOPROCESSOR_H
#define WEIBOPROCESSOR_H


#include "CJParser.h"
#include "CStringTool.h"
#include "CTypeTool.h"
#include "CTextKeyTool.h"
#include "CMBTextTool.h"
#include "CConfigInfo.h"
#include "CResource.h"
#include "lexicon.h"
#include "fromuni.h"
#include "touni.h"
#include "ustring.h"
#include "checkArticle.h"
#include "nx_log.h"
#include "lookup.h"
#include <map>
#include <set>
#include <vector>
#include <math.h>
#include <fstream>
#include <iostream>
#include <ext/hash_fun.h>
#include <ext/hash_map>
//#include "WordNetIndexor.h"

using namespace std;

typedef struct _term_info_t
{
	int seg_idf;
	int is_good_term;
	int positive;
	int negative;
}term_info_t;

typedef struct _NBtotal_info_t
{
	int positive;
	int negative;
}NBtotal_info_t;

class PrepareTrainingData
{
	public:

		PrepareTrainingData();
		~PrepareTrainingData();

	public:
		/**
		 * @brief 初始化实例 
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int WBProcessorInit(const char  *pConfigFile);
		/**
		 * @brief 读入所有数据
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int LoadWeiboToTerm();
		int WeiboToTrainingData();
		int GetTopTerms();
		int GetTermFrmOriginalLine(string strLine);
		int SortTerms();


		int NBPredict();
		int NBPredictLine(string strLine,ofstream &fout_predict);
		int GetNBModel();
		int GetNBFrmOriginalLine(string strLine,bool if_positive);

		int ProcessOriginalData();
		/**
		 * @brief 处理一行json原始数据
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		int ParseLineOriginal(string strLine,ofstream &output);

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

		int RecordTerm(char * term,WORD_ITEM *seg_info );

		bool IsGoodTerm(int postagid);
		bool IsGoodTermTmp(int postagid);



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
		CResource* m_pDarenResource;
		CResource* m_pVipResource;
		CResource* m_pYcResource;

		map<string,u_int> mTopTerms;
		map<string,term_info_t> mNBTerms;
		NBtotal_info_t mNBTotalInfo;


};
#endif
