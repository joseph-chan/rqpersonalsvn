
//////////////////////////////////////////////////
//�ַ�������
//
//���ߣ�����
//
//���ڣ�2011-08-10
//		���ӣ������ַ�������ַ����ķ���
//		���ӣ�ȥ���ַ�������β��λ�Ŀհ��ַ�
//�汾��0.3
//
//
#ifndef KAIJIANG_STRINGTOOL_H
#define KAIJIANG_STRINGTOOL_H

#include <vector>
#include <string>
#include <ctype.h>
using namespace std;

	class CStringTool
	{
		public:
		// ��ָ�����ַ�����ַ���
		static vector<string> SpliteByChar(string& input, char ch);

		// ��ָ���ַ�������ַ���
		static vector<string> SpliteByStr(string& input, string str);

		// ȥ����β�Ŀո��table
		static void Trim(string& str);

		// ȥ���ײ��Ŀհ��ַ�
		static void LeftTrim(string& str);

		// ȥ��β���Ŀհ��ַ�
		static void RightTrim(string& str);
	};
#endif
