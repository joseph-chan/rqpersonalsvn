#include "CStringTool.h"
#include <algorithm> 
#include <iostream>
#include <functional>


//#include <iostream>
//#include <functional>
//#include <algorithm>
//#include <cstdlib>
//#include <numeric>

//#include <cctype>

using namespace std;

// ���ַ�������ָ�����ַ���ֳ��ַ�������
vector<string> CStringTool::SpliteByChar(string& input, char ch)
{
	vector<string> result;
	unsigned int pos = 0;
	unsigned int prepos = 0;
	do
	{
		pos = input.find(ch, pos);
		if(pos != string::npos && pos == input.size() -1)
		{// found but at tail
		  string tmp = input.substr(prepos, pos - prepos);
		  result.push_back(tmp);
		  result.push_back("");
		  break;
		}
		if(pos == string::npos)
		{ // not found
		  pos = input.size()-1;
		}
	    string tmp = input.substr(prepos, pos - prepos);
	    result.push_back(tmp);
		if(pos >= input.size() - 1)
		{
		  break;
		}
		pos++;
		prepos = pos;
	}while(pos != input.npos);

	return result;
}

// �����ַ������ַ������в��
vector<string> CStringTool::SpliteByStr(string& input, string str)
{
	vector<string> result;
	size_t pos = 0;
	size_t prepos = 0;
	if(str.size() > input.size())
	{
		return result;
	}

	do
	{
		pos = input.find(str, pos);
		if(pos == string::npos)
		{
		  pos = input.size();
		}
		
		string tmp = input.substr(prepos, pos - prepos);
		result.push_back(tmp);
		if(pos >= input.size() - str.size())
		  break;
		pos += str.size();
		prepos = pos;
	}while(pos != string::npos);

	return result;
}

//ȥ����β�Ŀհ��ַ�
void CStringTool::Trim(string& str)
{
	LeftTrim(str);
	RightTrim(str);
}

// ȥ����ߵĿհ��ַ�
void CStringTool::LeftTrim(string& ss)
{
	string::iterator p=find_if(ss.begin(),ss.end(),not1(ptr_fun(::isspace))); 
	ss.erase(ss.begin(),p);
}

// ȥ���ұߵĿհ��ַ�
void CStringTool::RightTrim(string& ss)
{
	string::reverse_iterator p=find_if(ss.rbegin(),ss.rend(),not1(ptr_fun(::isspace))); 
	ss.erase(p.base(),ss.end()); 
}
