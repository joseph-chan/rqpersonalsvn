/**
 * @file WeiboProcessor.h
 * @brief  
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

#ifndef WEIBOPROCESSOR_H
#define WEIBOPROCESSOR_H

#include "CConfigInfo.h"
#include "CResource.h"
#include "lexicon.h"
#include "fromuni.h"
#include "touni.h"
#include "ustring.h"
#include "checkArticle.h"
#include <map>
#include <set>
#include <vector>

extern "C"
{
	#include "nx_log.h"
}
extern ngx_log_t	*gNxlog;
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
		int Init(const char* cResource);

		/**
		 * @brief ��������ļ�
		 *
		 * @Param cWeiboFile
		 * @Param cProcessedWeiboFile
		 */
		int ProcessClassifiedData();

		/**
		 * @brief ��������ļ���һ������
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		int ParseLineClassified(string szLine);

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
		int ProcessForwardData();

		/**
		 * @brief ����һ��ת������
		 *
		 * @Param strLine
		 *
		 * @Returns   
		 */
		int ParseLineForward(string strLine);

		/**
		 * @brief ������������ 
		 *
		 * @Param szInputFile
		 */
		int ProcessCommentData();


		/**
		 * @brief ����һ���������� 
		 *
		 * @Param szInputFile
		 */
		int ParseLineComment(string strLine);


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


	private:
		typedef struct BASEINFO
		{
			bool has_video;//�Ƿ�����Ƶ
			bool has_image;//�Ƿ���ͼƬ
			int nType;//����
			int nGarbageScore;//��������
			int nYCScore;//����Դ �ͻ��˵÷�
			int nFilterScore;//filt�ֶε÷�
			int nForwardCount;//ת����
			int nValidCmtCount;//��Ч������
			int nContentLen;//΢������
			int nAtNum;//΢���������@����
			//int nClassifyScore;//����÷�
			//int nKeyWordsScore;//�ؼ��ʵ÷�
			//int nWords;//΢���Ĵ���
			//int nUidScore;//���ߵķ���
			//int nRubScore;//������������
			unsigned long long nUid;//�û�id
			float fTextScore;//΢���ı�����
			string strKeywords;//�ؼ���
			string strDebugContent;//΢�����ݣ���������
			string strDebugUrl;//url(�����)
		}BaseInfo;

		typedef struct WEIBOINFO
		{
			BaseInfo base_info;
			int nWeight;//����Ȩ��
		}WeiboInfo;

		/**
		 * @brief ������Դ
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int LoadResource(const char* cResource);

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

		// ��Ҫ����ķ���
		vector<int> m_types;

		// ΢��������ָ��
		set<unsigned long long> m_setContentKey;

		// ����������Ϣ
		CConfigInfo* m_pConfigInfo;

		// �ͻ���������Ϣ
		CConfigInfo* m_pYCConfigInfo;

		//������Դ
		CResource* m_pDarenResource;

		//vip��Դ
		CResource* m_pVipResource;

		//type��ϵӳ���(����һ���÷���)
		CResource* m_pTypeRelation;

		//IT�������
		CResource* m_pITWhiteList;

		//�ƾ��������
		CResource* m_pFinanceWhiteList;

		//�����������
		CResource* m_pSportsWhiteList;

		// ������Ϣ�ĸ���
		//int m_nAttachInfoCount;

		int nMaxForwardCount;//ת����
		int nMaxValidCmtCount;//��Ч������

		// �ؼ������ƣ�boolֵtrue��ʾ�ؼ��ʾ��з�ֵ
		vector<pair<string, bool> > m_vecKeyWords;

		map<unsigned long long, WeiboInfo*> m_mapWeiboInfo;
};
#endif
