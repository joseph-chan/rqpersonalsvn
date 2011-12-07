/**
 * �ı���key���ɹ����࣬��keyΪ32λ�����ͣ����ڴ�����ı���
 * һ�㣬��ƪ��ͬ���ı���̫��������ͬ����key����ˣ���key
 * �����ڶ��ı��������صȡ�
 *
 * ���ߣ�����
 *
 * ���ڣ�2011.07.13
 *
 */
 
#ifndef CTEXTKEYTOOL_H
#define CTEXTKEYTOOL_H

#include "paichong.h"
#include "CTokenizer.h"

class CTextKeyTool
{
	public:
		// ����ģʽ
		static CTextKeyTool* GetInstance();

		// �ͷ�
		static void ReleaseInstance();

		// ��ʼ��
		// ������
		//		cSegData -- �ִ�����
		//		unFlag --����key��ǰ�ڴ����־λ
		//				0x0001 -- ȥ��@�ǳ�
		//				0x0002 -- ȥ��url
		//				0x0004 -- ȥ������
		//				0x0008 -- ȥ������
		//				16 -- ȥ�������Ű�Χ��˵������
		//				ps : �������
		// ���أ�
		//		true -- ��ʼ���ɹ�
		//		false -- ʧ��
		//
		bool InitInstance(const char* cSegData, unsigned long unFlag = 0x0003);

		// �ı��ȡkey��ǰ�ڴ����־λ
		//
		// ������
		//
		// ���أ�
		//		unFlag --����key��ǰ�ڴ����־λ
		//				0x0001 -- ȥ��@�ǳ�
		//				0x0002 -- ȥ��url
		//				�������
		//
		unsigned long& GetFlag(){return m_unFlag;}

		// ��������ı�����һ��key
		// ������
		//		cText -- �ı�
		//		ullKey[OUT] -- ���ɵ�key
		//		nMinTermCount -- ���Բ���key����������
		// ���أ�
		//		true -- �ɹ�
		//		false -- ����
		//
		bool GetKey(const char* cText, unsigned long long & ullKey, int nMinTermCount = 15);

		// ��������ı����ɶ��ģ����key������ĳЩģ����λ��������0������1���õ����key
		// ������
		//		cText -- �ı�
		//		vecKeys[OUT] -- ���ɵĶ��key
		//		nMinTermCount -- ���Բ���key����������
		// ���أ�
		//		true -- �ɹ�
		//		false -- ʧ��
		bool GetKey(const char* cText, vector<unsigned long long>& vecKeys, int nMinTermCount = 15);

	private:
		// ���캯��
		CTextKeyTool();

		// ��������
		~CTextKeyTool();

		static CTextKeyTool* _instance;

		// �ִʹ��ߵ�ʵ��
		CTokenizer* m_spTokenizer;

		unsigned long m_unFlag;
};

#endif
