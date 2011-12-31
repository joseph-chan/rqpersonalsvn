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

// ����
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

// װ��
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
		// �����ļ��е�ע�ͺͰ��������ļ�
		unsigned int pos = line.find('#');
		if(pos != line.npos)
		{
			// ����Ƿ��а������������ļ������,��#include��ͷ
			unsigned int pos1 = line.find("include", pos);
			if(pos1 != line.npos && pos1 == pos + 1)
			{
				string include_file = line.substr(pos1+7, line.size()-pos1-7);
				CStringTool::Trim(include_file);
				if(m_nRecurseCount > 10 )
				{
					cout<<"������������ļ���������"<<include_file<<"�޷���ȡ��"<<endl;
				}
				else
				{
					LoadConfig(include_file.c_str(),cSplitChar);
				}
			}
			// ������#�������#include
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
			cout<<"key:"<<iter->first<<"�Ѵ���,valueΪ:"<<iter->second<<endl;
			cout<<"key:"<<iter->first<<"��value����"<<vecData[1]<<"����."<<endl;
		}
		m_mapConfig[vecData[0]] = vecData[1];
	}
	fin.close();
	m_nRecurseCount--;
}
