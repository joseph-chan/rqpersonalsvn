/**
 * @file WeiboProcessor.h
 * @brief  
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

#ifndef WEIBOPROCESSOR_H
#define WEIBOPROCESSOR_H


#include "WordNetInclude.h"

using namespace std;

class WeiboProcessor
{
	public:

		WeiboProcessor();
		~WeiboProcessor();

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
		int LoadWeiboToIndex();


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
		int ProcessOriginalData();
		/**
		 * @brief ����һ��jsonԭʼ����
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		int ParseLineOriginal(string strLine);

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

		int RecordTerm(char * term,WORD_ITEM *seg_info );

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
		int SortOutput();
		int DumpIndex();


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

		//hash map to record term in one weibo
		__gnu_cxx::hash_map<u_int,u_int> hmInWeibo;
		//vector<string> vTermInWeibo;

		u_int m_term_num;
		u_int m_relate_term_num;
		unsigned long long m_weibo_num;

		__gnu_cxx::hash_map<u_int,u_int> mTermIndex;
		vector<term_info_t> mTermInfo;

		vector<relate_term_info_t> mRelateInfo;

};
#endif
