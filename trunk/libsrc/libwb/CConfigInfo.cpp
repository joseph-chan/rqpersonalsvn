#include "CConfigInfo.h"
#include "CStringTool.h"
#include <fstream>
#include <iostream>

using namespace std;

CConfigInfo::CConfigInfo()
{
}

CConfigInfo::CConfigInfo(const char* cConfigFile,char cSplitChar)
{
	m_strConfigFile = string(cConfigFile);
	//
	m_nRecurseCount = 0;

	LoadConfig(cConfigFile,cSplitChar);
}

CConfigInfo::~CConfigInfo()
{
}

string CConfigInfo::GetValue(string strValue)
{
	map<string, string>::iterator iter = m_mapConfig.find(strValue);
	if(iter == m_mapConfig.end())
	  return "";
	return iter->second;
}

void CConfigInfo::SetValue(string strKey, string strValue)
{
	map<string, string>::iterator iter = m_mapConfig.find(strValue);
	if(iter != m_mapConfig.end())
	  iter->second = strValue;
}

// 保存
void CConfigInfo::SaveConfig()
{
	ofstream fout(m_strConfigFile.c_str());
	if(!fout.is_open())
	{
		cout<<m_strConfigFile<<" can not be opened."<<endl;
		return;
	}
	for(map<string, string>::iterator iter = m_mapConfig.begin(); iter != m_mapConfig.end(); iter++)
	{
		fout<<iter->first<<"="<<iter->second<<endl;
	}
	fout.close();
}

// 装载
void CConfigInfo::LoadConfig(const char* cConfigFile,char cSplitChar)
{
	ifstream fin(cConfigFile);
	if(!fin.is_open())
	{
		cout<<cConfigFile<<" can not be opened."<<endl;
		return;
	}
	m_nRecurseCount++;
	string line;
	vector<string> vecData;
	while(!fin.eof())
	{
		getline(fin,line);
		// 配置文件中的注释和包含配置文件
		unsigned int pos = line.find('#');
		if(pos != line.npos)
		{
			// 检查是否有包含其他配置文件的情况,以#include开头
			unsigned int pos1 = line.find("include", pos);
			if(pos1 != line.npos && pos1 == pos + 1)
			{
				string include_file = line.substr(pos1+7, line.size()-pos1-7);
				CStringTool::Trim(include_file);
				if(m_nRecurseCount > 10 )
				{
					cout<<"超出最大配置文件包含数！"<<include_file<<"无法读取。"<<endl;
				}
				else
				{
					LoadConfig(include_file.c_str(),cSplitChar);
				}
			}
			// 所有以#被处理的#include
			line = line.substr(0, pos);
		}
		if(line.size() == 0)
		  continue;
		vecData = CStringTool::SpliteByChar(line, cSplitChar);
		if(vecData.size() < 2)
		  continue;
		CStringTool::Trim(vecData[0]);
		CStringTool::Trim(vecData[1]);
		map<string, string>::iterator iter = m_mapConfig.find(vecData[0]);
		if(iter != m_mapConfig.end())
		{
			cout<<"key:"<<iter->first<<"已存在,value为:"<<iter->second<<endl;
			cout<<"key:"<<iter->first<<"的value将被"<<vecData[1]<<"覆盖."<<endl;
		}
		m_mapConfig[vecData[0]] = vecData[1];
	}
	fin.close();
	m_nRecurseCount--;
}
