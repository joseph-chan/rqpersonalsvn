#ifndef CTYPETOOL_H
#define CTYPETOOL_H

///////////////////////////////////////////////////////////
// 数据类型的工具类，主要用于各种数据类型与字符串之间的转换
//
// 作者：开江
//
// 日期：2011-06-22
//
// 版本：
//		增加：数值型转换为字符串时可以设置最终的字符串位数，当数字不足时则在字符串前面用0补齐
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
		// 字符串转换为其他类型
		//
		static Type StrTo(string str)
		{
			stringstream stream;
			stream<<str;

			Type t ;
			stream>>t;

			return t;
		}

		// 其他类型转换为字符串
		//
		// 参数：t -- 其他类型
		//		 length -- 转换的字符串的长度，默认为-1，表示保持原来的长度，
		//		 如果转换后的字符串长度小于length，则在其前面补0。
		//		 例如要转换的是数字3，如果length设置为4，则转换结果为0003
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
