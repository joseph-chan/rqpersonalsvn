#ifndef CTYPETOOL_H
#define CTYPETOOL_H

///////////////////////////////////////////////////////////
// �������͵Ĺ����࣬��Ҫ���ڸ��������������ַ���֮���ת��
//
// ���ߣ�����
//
// ���ڣ�2011-06-22
//
// �汾��
//		���ӣ���ֵ��ת��Ϊ�ַ���ʱ�����������յ��ַ���λ���������ֲ���ʱ�����ַ���ǰ����0����
//
//

#include <stdlib.h>
#include <sstream>
#include <string>

using namespace std;

template<class Type>

class CTypeTool
{
	public:
		// �ַ���ת��Ϊ��������
		//
		static Type StrTo(string str)
		{
			stringstream stream;
			stream<<str;

			Type t ;
			stream>>t;

			return t;
		}

		// ��������ת��Ϊ�ַ���
		//
		// ������t -- ��������
		//		 length -- ת�����ַ����ĳ��ȣ�Ĭ��Ϊ-1����ʾ����ԭ���ĳ��ȣ�
		//		 ���ת������ַ�������С��length��������ǰ�油0��
		//		 ����Ҫת����������3�����length����Ϊ4����ת�����Ϊ0003
		static string ToStr(Type t, int length = -1)
		{
			stringstream stream;
			stream<<t;
			if(stream.str().size() >= length)
				return stream.str();
			else
			{
				int num = length - stream.str().size();
				string result = stream.str();
				while(num > 0)
				{
					result = "0" + result;
					num--;
				}
				return result;
			}
		}
};

#endif
