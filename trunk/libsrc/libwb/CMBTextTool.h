/**
 * ΢���ı�����
 *  
 * ���ߣ� ����
 *
 * ���ڣ�2011-07-15
 *
 * �汾��0.4
 *
 * �����޸ģ�
 *		���ӷ�����Ĩ��΢���е��ǳ�
 *		���ӷ�����Ĩ��΢���е�URL
 *		���ӷ�����Ĩ��΢���е�##��ǵĻ���
 *		���ӷ�����Ĩ��΢���еı���
 *		���ӷ�����Ĩ��΢���е�˵�����֣��������Ű�Χ��
 */

#ifndef WEIBOTEXTTOOL_H
#define WEIBOTEXTTOOL_H
#include <map>
#include <string>
#include <set>
#include <vector>
#include <cctype>

using namespace std;

class CMBTextTool
{
	public:
		// ��ʼ������
		//
		static void InitTool();

		// ��ȡ΢���ı���@�û��ǳ��б�
		//
		// ������
		//		text -- �ı�
		//		mapAt -- ��ȡ���ǳƼ������ӳ���
		//
		// ���أ�
		//		�ǳƸ���
		//
		static int ExtractNickName(const char* text, map<string, int>& mapAt);

		// ��ȡ΢���ı���@�û��ǳ��б�
		// ������
		//		text -- ΢���ı�
		//		mapAt -- ��ȡ���ǳƼ������ӳ���
		//		bRemoveAt -- �Ƿ�Ĩ����ȡ���ǳƣ�Ĭ��Ϊ��
		// ���أ�
		//		�ܹ����ǳƸ���
		//
		static int ExtractNickName(string& text, map<string, int>& mapAt, bool bRemoveAt = false);
		static int KillNickName(string& text);

		// Ĩ��΢���е�url
		// ������
		//		text -- ΢���ı�
		//		mapURL -- ��Ĩ����url
		// ���أ�
		//		��Ĩ����url�ܸ���
		//
		static int GetURL(string& text, map<string, int>& mapURL);
		static int KillURL(string& text);

		// Ĩ��΢���еĻ���
		// ������
		//		text -- ΢��ԭ��
		//		vecTopics[OUT] -- ��Ĩ���Ļ���
		//
		//	���أ�
		//		��Ĩ���Ļ�������
		//
		static int KillTopic(string& text, vector<string>& vecTopics);

		// Ĩ��΢���еı���
		// ������
		//		text -- ΢��ԭ��
		//
		// ���أ�
		//		��Ĩ���ı�������
		//
		static int KillEmotion(string& text);

		// Ĩ��΢���е�˵������
		// ������
		//		text -- ΢��ԭ��
		//
		// ���أ�
		//		��
		static void KillIntrInfo(string& text);

	private:
		// �ж��Ƿ�Ϊ���ı��
		//
		// ������
		//		�����ֽڵ������ַ�
		//
		// ���أ�
		//      true -- ��
		//      false -- ��
		//
		static bool IsPunction(const char* ch);


		// ������ֹ���ţ�Ĩ�����е�����
		// ������
		//		strText -- ��������
		//		strBegin -- ��ʼ���Ŵ�
		//		strEnd -- �������Ŵ�
		//		vecElements[OUT] -- ��Ĩ���Ĵ�
		// ���أ�
		//		��Ĩ���Ĵ�����
		//
		static int KillElements(string& strText, string strBegin, string strEnd, vector<string>& vecELements);

	private:

		static set<string> m_setChPunct;		//�����еı��
};
#endif
