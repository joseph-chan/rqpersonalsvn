#include "CJParser.h"
#include <iostream>

using namespace std;

vector<string> CJParser::m_vecFields;

void CJParser::InitParser(vector<string>& fields)
{
	m_vecFields = fields;
}

int CJParser::ParseJson(string& strJson, map<string, string>& mapJson)
{
	if(m_vecFields.size() == 0)
	{
		cout<<"parser has not been initialized."<<endl;
		return 0;
	}
	unsigned int pos = 0;
	unsigned int pos1 = 0;
	int ret = 0;
	for(unsigned int i=0; i!=m_vecFields.size(); i++)
	{
		string fields = "\"" + m_vecFields[i] + "\":";
		pos = strJson.find(fields, pos1);
		
		if(pos == strJson.npos)
		{
			mapJson[m_vecFields[i]] = "";
			continue;
		}
		pos += fields.size();
		if(pos == strJson.npos)
		{
			mapJson[m_vecFields[i]] = "";
			continue;
		}
		// valueµÄÖÕµã£º","
		pos1 = strJson.find(",\"", pos);
		if(pos1 == strJson.npos)
		{
			pos1 = strJson.find("}", pos);
			if(!(pos1 != strJson.npos && (pos1 == strJson.size() - 1)))
			{
				mapJson[m_vecFields[i]] = "";
				continue;
			}
		}
		if(strJson[pos] == '\"')
		  pos++;
		if(strJson[pos1-1] == '\"')
		  pos1--;
		string value = strJson.substr(pos, pos1 - pos);
		//cout<<fields<<"\t";
		//cout<<value<<endl;
		mapJson[m_vecFields[i]] = value;
		ret++;
	}
	return ret; 
}
