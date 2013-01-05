/**
 * @file WeiboProcessor.h
 * @brief  
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

#ifndef TERMSPROCESSOR_H
#define TERMSPROCESSOR_H


#include "WordNetInclude.h"
#include "WordNetIndexor.h"
#include "lookup.h"

using namespace std;

typedef struct _sort_item_t
{
	int sort_int;
	int appendix;
}sort_item_t;

class TermsProcessor
{
	public:

		TermsProcessor();
		~TermsProcessor();

	public:
		/**
		 * @brief ��ʼ��ʵ�� 
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int WBProcessorInit(const char  *pConfigFile);
		/**
		 * @brief ������������
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int LoadTermsToIndex();


		/**
		 * @brief ��������ļ�
		 *
		 * @Param cWeiboFile
		 * @Param cProcessedWeiboFile
		 */
		//int ProcessClassifiedData();

		/**
		 * @brief ��������ļ���һ������
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		//int ParseLineClassified(string szLine);

		/**
		 * @brief ����ԭʼjson����
		 *
		 * @Param szInputFile
		 */
		int ProcessTerms();
		/**
		 * @brief ����һ��jsonԭʼ����
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		int ParseLineTerms(string strLine);

		/**
		 * @brief ����ת������
		 *
		 * @Param 
		 */
		//int ProcessForwardData();

		/**
		 * @brief ����һ��ת������
		 *
		 * @Param strLine
		 *
		 * @Returns   
		 */
		//int ParseLineForward(string strLine);

		/**
		 * @brief ������������ 
		 *
		 * @Param szInputFile
		 */
		//int ProcessCommentData();

		int RecordTerm(char * term);

		bool IsGoodTerm(int postagid);

		int AddCooc(vector<u_int> &hash_values);

		/**
		 * @brief ����һ���������� 
		 *
		 * @Param szInputFile
		 */
		//int ParseLineComment(string strLine);


		/**
		 * @brief ��������΢��Ȩ��
		 *
		 * @Returns   
		 */
		int CalcWeight();

		/**
		 * @brief �������
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
		 * @brief �����дʿ�
		 *
		 * @Param cLexiconPath
		 *
		 * @Returns   
		 */
		int LoadLexicon(const char* cLexiconPath);

		void ReleaseWordSeg();


		/**
		 * @brief ΢����������
		 *
		 * @Param strContent
		 *
		 * @Returns   
		 */
		bool HaveDuplicates(string& strContent);


		void Clear();

/****************************����Ϊ���ݳ�Ա��**************************************/
	private:
		LEXICON * m_Lexicon; 
		WORD_SEGMENT *m_Wordseg ;
		unsigned long long m_WordSegFlags;

		// ΢��������ָ��
		set<unsigned long long> m_setContentKey;

		// ����������Ϣ
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
