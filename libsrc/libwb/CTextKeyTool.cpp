#include "CTextKeyTool.h"
#include "CMBTextTool.h"
#include <iostream>

using namespace std;
// 单例
CTextKeyTool* CTextKeyTool::_instance = NULL;

// 获取单例
CTextKeyTool* CTextKeyTool::GetInstance()
{
	if(_instance == NULL)
	  _instance = new CTextKeyTool;
	return _instance;
}

// 释放单例
void CTextKeyTool::ReleaseInstance()
{
	if(NULL != _instance)
	  delete _instance;
	_instance = NULL;
}

// 构造&西沟
CTextKeyTool::CTextKeyTool()
{
	m_spTokenizer = NULL;
}

// 西沟
CTextKeyTool::~CTextKeyTool()
{
	CTokenizer::ReleaseInstance();
	m_spTokenizer = NULL;
}

// 初始化
bool CTextKeyTool::InitInstance(const char* cSegData, unsigned long unFlag)
{
	m_unFlag = unFlag;

	CMBTextTool::InitTool();
	m_spTokenizer =  CTokenizer::GetInstance();
	return m_spTokenizer->InitInstance(cSegData, 0);
}

// 获取key
bool CTextKeyTool::GetKey(const char* cText, unsigned long long& ullKey, int nMinTermCount)
{
	if(NULL == cText || strlen(cText) == 0)
	  return false;
	if(NULL == m_spTokenizer)
	{
		cout<<"instance has not been initialized."<<endl;
		return false;
	}
	string strText(cText);
	static map<string, int> mapNames;
	if(m_unFlag & 0x0001)
	{
		mapNames.clear();
		CMBTextTool::ExtractNickName(strText, mapNames, true);
	}
	if(m_unFlag & 0x0002)
	{
		mapNames.clear();
		//CMBTextTool::KillURL(strText, mapNames);
		CMBTextTool::KillURL(strText);
	}
	if(m_unFlag & 0x0004)
	{
		static vector<string> vecTopic;
		vecTopic.clear();
		CMBTextTool::KillTopic(strText, vecTopic);
	}
	if(m_unFlag & 0x0008)
	{
		CMBTextTool::KillEmotion(strText);
	}
	if(m_unFlag & 16)
	{
		CMBTextTool::KillIntrInfo(strText);
	}
	int ret = m_spTokenizer->DoSegment(strText.c_str());
	if(ret <= 0)
	  return false;
	if(m_spTokenizer->GetResult() == NULL)
	{
		cout<<"word segement is failed."<<endl;
		return false;
	}
	ullKey = 0l;
	ret = textHash(m_spTokenizer->GetResult(), ullKey, 2, nMinTermCount);
	if(ullKey == 0l)
	  return false;
	return ret == 0;
}

// 获取key
bool CTextKeyTool::GetKey(const char* cText, vector<unsigned long long>& vecKeys, int nMinTermCount)
{
	if(NULL == cText || strlen(cText) == 0)
	  return false;
	if(NULL == m_spTokenizer)
	  return false;
	string strText(cText);
	if(m_unFlag & 0x0001)
	{
		map<string, int> mapNames;
		CMBTextTool::ExtractNickName(strText, mapNames, true);
	}
	if(m_unFlag & 0x0002)
	{
		map<string, int> mapNames;
		//CMBTextTool::KillURL(strText, mapNames);
		CMBTextTool::KillURL(strText);
	}
	if(m_unFlag & 0x0004)
	{
		vector<string> vecTopic;
		CMBTextTool::KillTopic(strText, vecTopic);
	}
	if(m_unFlag & 0x0008)
	{
		CMBTextTool::KillEmotion(strText);
	}
//	cout<<strText<<endl;
	int ret = m_spTokenizer->DoSegment(strText.c_str());
	if(ret <= 0)
	  return false;
	ret = textHash2(m_spTokenizer->GetResult(), vecKeys, 2, nMinTermCount);
	if(vecKeys.size() == 0)
	  return false;
	
	return ret == 0;
}

