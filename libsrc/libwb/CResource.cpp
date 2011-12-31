#include "CResource.h"
#include "CStringTool.h"
#include <fstream>
#include <iostream>

using namespace std;

CResource::CResource()
{
}

CResource::CResource(const char* cResourceFile,int nBlockNum,char cSplitChar)
{
	m_strResourceFile = string(cResourceFile);

	LoadResource(cResourceFile,nBlockNum,cSplitChar);
}

CResource::~CResource()
{
}

int CResource::GetValue(string strValue,vector<string> &vOutput)
{
	map<string, vector<string> >::iterator iter = m_mapResource.find(strValue);
	if(iter == m_mapResource.end())
	{
		vOutput.clear();
		return 1;
	}
	vOutput = iter->second;
	return 0;
}

const map<string, vector<string> > & CResource::GetAllValue()
{
	return m_mapResource;
}


/*
void CResource::SetValue(string strKey, string strValue)
{
	map<string, string>::iterator iter = m_mapResource.find(strValue);
	if(iter != m_mapResource.end())
	  iter->second = strValue;
}
*/

// 保存
/*
void CResource::SaveResource()
{
	ofstream fout(m_strResourceFile.c_str());
	if(!fout.is_open())
	{
		cout<<m_strResourceFile<<" can not be opened."<<endl;
		return;
	}
	for(map<string, string>::iterator iter = m_mapResource.begin(); iter != m_mapResource.end(); iter++)
	{
		fout<<iter->first<<"="<<iter->second<<endl;
	}
	fout.close();
}
*/

// 装载
void CResource::LoadResource(const char* cResourceFile,int nBlockNum, char cSplitChar)
{
	ifstream fin(cResourceFile);
	if(!fin.is_open() && nBlockNum <=1)
	{
		cout<<cResourceFile<<" can not be opened."<<endl;
		return;
	}
	string line;
	vector<string> vecData;
	while(!fin.eof())
	{
		getline(fin,line);
		//是否以#开始
		int pos = line.find('#');
		if(pos == 0)
		{
			continue;
		}
		if(line.size() == 0)
		{
		  continue;
		}
		CStringTool::Trim(line);
		vecData = CStringTool::SpliteByChar(line, cSplitChar);
		if(vecData.size() != nBlockNum)
		{
			cout << "data format error" << endl;
			continue;
		}

		map<string, vector<string> >::iterator iter = m_mapResource.find(vecData[0]);
		if(iter != m_mapResource.end())
		{
			cout<<"key:"<<iter->first<<"已存在,value为:"<<endl;
			cout<<"key:"<<iter->first<<"的value将被"<<"覆盖."<<endl;
		}
		//赋值

		string strTmp = vecData[0];
		vecData.erase(vecData.begin());
		pair <string,vector<string> > tmp_pair(strTmp,vecData);
		m_mapResource.insert(tmp_pair);
		//cout << "add to resource" << strTmp << ":" << vecData[0] << "..." <<endl;
	}
	fin.close();
}
