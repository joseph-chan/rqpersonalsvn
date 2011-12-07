/**
 * ����ؼ��ʣ����ؼ��ʼ���Ȩ�ط���ͳһ����
 *
 * ���ߣ�����
 *
 * ���ڣ�2011 08 10
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
		// ���һ���ؼ��ʼ���
		// ������
		//		strName -- �ؼ��ʼ���Ψһ������
		//		cWordFile -- �ؼ������ڵ��ļ�
		// ���أ�
		//		-1 -- ͬ���ļ����Ѵ���
		//		-2 -- �ؼ����ļ������Ϲ淶
		//		>=0 -- ���صĹؼ��ʸ���
		int AddKeyWords(string strName, const char* cWordFile);

		// ���һ��ID����,�൱����ֵ�͵Ĺؼ��ʼ���
		// ������
		//		strName -- �ؼ��ʼ���Ψһ������
		//		cWordFile -- �ؼ������ڵ��ļ�
		// ���أ�
		//		-1 -- ͬ���ļ����Ѵ���
		//		-2 -- �ؼ����ļ������Ϲ淶
		//		>=0 -- ���صĹؼ��ʸ���
		int AddKeyIDs(string strName, const char* cWordFile);

		// ��������ı����
		// ������
		//		strText -- ������ı�
		//		strName -- ʹ�õĹؼ��ʼ���
		//		bSum -- �Ƿ�ȡ��ֵ����Ϊ�÷֣�Ĭ��ȡ���ֵ��Ϊ�÷�
		// ���أ�
		//		�÷֣����ڳ����쳣����û�ҶԸ������ֵĹؼ��ʣ�����С��0�ķ���
		int GetTextScore(string& strText, string strName, bool bSum = false);
		int GetIDScore(unsigned long long id, string strName);

		// �жϸ����ı��Ƿ����ָ�����ƹؼ��ʼ��ϵĹؼ���
		// ������
		//		strText -- �ı�
		//		strName -- �ؼ�������
		//		vecKeyWords[OUT] -- �ı��д��ڵĹؼ��ʼ�����ֵĴ���
		//		mapKeyWords -- �ı��д��ڵĹؼ��ʼ������
		// ���أ�
		//		ƥ��Ĺؼ��ʸ���
		int HaveKeyWords(string& strText, string strName);
		int HaveKeyWords(string& strText, string strName, vector< pair<string, int> >& vecKeyWords);
		int HaveKeyWords(string& strText, string strName, map<string, int>& mapKeyWords);
		int HaveKeyIDs(unsigned long long id, string strName);

		// ��ȡָ�����ƵĹؼ��ʼ�����Ŀ
		// ������
		//		strName -- �ؼ��ʼ�������
		// ���أ�
		//		�ؼ�����Ŀ
		int GetKeyWordsCount(string strName);

		// ��ȡָ������ָ��λ�õĹؼ���
		// ������
		//		strName -- �ؼ��ʼ�������
		//		index -- ����
		// ���أ��ؼ���
		string GetKeyWord(string strName, int index);
	private:
		typedef struct KEYWORDS
		{
			MATCHENTRY* machine;
			map<string, int> mapScore;
		}KeyWords;

		// ���ݹؼ��ʶ����۽�������
		//
		int ScoreText(MATCHENTRY* pKeyWords, map<string, int>& mapScore, string& strText, bool bSum);

		// ���ҹؼ���
		//
		int FindKeyWord(const char *text,int &watchNum,MATCHENTRY *machine_ww,vector<pair<string,int> >& watchWord);

		// װ�عؼ���
		//
		MATCHENTRY* LoadKeyWords(const char* cFile, map<string, int>& mapScore);

		void LoadKeyIDs(const char* cFile, map<unsigned long long, int>& mapScore);
		
	private:
		map<string, KeyWords> m_mapKeyWords;
		map<string, map<unsigned long long, int> > m_mapKeyIDs;
};
#endif
