/*
* ���·�װ�˹�˾�ķִ����ӿڣ��Լ򵥺���Ϊ��ּ��ֻ��װ�˻�������
*
* ���ߣ�kaijiang
*
* ���ڣ�2011-07-13
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
		// ��ȡ����
		static CTokenizer* GetInstance();

		// �ͷŵ���
		static void ReleaseInstance();

	public:
		// ��ʼ���ִ���
		// ������
		//		cDicData -- �ʵ�����·��
		//		nLang -- ���ԣ�0 -- ���ļ���;1 -- ����
		// ���أ�
		//		true -- ��ʼ���ɹ�
		//		false -- ��ʼ��ʧ��
		//
		bool InitInstance(const char* cDicData, int nLang);

		// ִ�зִ�
		// ������
		//		cText -- ����
		// ���أ�
		//		-1 -- ��������Ϊ��
		//		-2 -- �ִ���δ��ʼ��
		//		-3 -- ��������
		//		>=0 -- �ִʽ����
		int DoSegment(const char* cText);

		// ��ȡ�ִʽ��
		inline WORD_SEGMENT* GetResult() const {return m_pResult;}

		// ��ȡ�ִʽ����Ŀ
		int GetWordCount() const;

		// ��ȡָ��λ���ϵĴʣ�λ�ò����򷵻ؿ��ַ���
		string GetWord(int index)const;

		// ��ȡָ��λ���ϵĴʵĴ��ԣ�λ�ò����򷵻ؿ��ַ���
		string GetPosTag(int index)const;

		// �ж��Ƿ�ͣ�ô�
		bool IsStopWord(int index) const;

		// ��ȡIDFֵ
		int GetIDF(int index) const;

	private: 
		// ��ʼ��ͣ�ôʱ�
		void InitStopWordSet();

		// ���캯������������
		CTokenizer();

		~CTokenizer();
		// ����
		static CTokenizer* _instance;
	private:
		WORD_SEGMENT* m_pResult;				// �ִʽ���ṹ��ָ��
		set<unsigned char> m_setStopWord;		// ͣ�ôʱ�
		LEXICON * m_pLexicon;					// �ʵ�
		unsigned long long  m_unAnalyserFlag;	// ����������������
};
#endif
