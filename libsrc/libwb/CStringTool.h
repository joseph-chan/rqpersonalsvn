
//////////////////////////////////////////////////
//字符串工具
//
//
//日期：2011-08-10
//		增加：按照字符串拆分字符串的方法
//		增加：去掉字符串中首尾部位的空白字符
//版本：0.3
//
//
#ifndef _STRINGTOOL_H
#define _STRINGTOOL_H

#include <vector>
#include <string>
#include <ctype.h>
//#include <iostream>

using namespace std;

	class CStringTool
	{
		public:
		// 用指定的字符拆分字符串
		static vector<string> SpliteByChar(string& input, char ch);

		// 用指定字符串拆分字符串
		static vector<string> SpliteByStr(string& input, string str);

		// 去掉首尾的空格和table
		static void Trim(string& str);

		// 去掉首部的空白字符
		static void LeftTrim(string& str);

		// 去掉尾部的空白字符
		static void RightTrim(string& str);
	};
#endif
