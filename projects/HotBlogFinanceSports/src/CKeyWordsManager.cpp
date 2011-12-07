#include "CKeyWordsManager.h"
#include "CStringTool.h"
#include "CTypeTool.h"
#include <fstream>
#include <iostream>

using namespace std;

CKeyWordsManager* CKeyWordsManager::_instance = NULL;

CKeyWordsManager* CKeyWordsManager::GetInstance()
{
	if(_instance == NULL)
	  _instance = new CKeyWordsManager;
	return _instance;
}

void CKeyWordsManager::ReleaseInstance()
{
	if(_instance != NULL)
	  delete _instance;
	_instance = NULL;
}

CKeyWordsManager::CKeyWordsManager()
{
}

CKeyWordsManager::~CKeyWordsManager()
{
	for(map<string, KeyWords>::iterator iter = m_mapKeyWords.begin(); iter != m_mapKeyWords.end(); iter++)
	{
		RemoveEntry(iter->second.machine);
	}
}


int CKeyWordsManager::AddKeyWords(string strName, const char* cWordFile)
{
	if(strName.size() == 0)
	{
		cout<<"name is null."<<endl;
		return -1;
	}
	map<string, KeyWords>::iterator iter = m_mapKeyWords.find(strName);
	if(iter != m_mapKeyWords.end())
	{
		cout<<strName<<":this name is so popular that you must change another one."<<endl;
		return -1;
	}
	map<string, int> mapScore;
	MATCHENTRY* machine = LoadKeyWords(cWordFile, mapScore);
	if(machine == NULL)
	{
		cout<<"load "<<cWordFile<<" keywords failed."<<endl;
		return -2;
	}
	KeyWords keywords;
	keywords.machine = machine;
	keywords.mapScore = mapScore;
	m_mapKeyWords[strName] = keywords;
	cout<<strName<<"的关键词装载完毕，成功装载"<<machine->wordsnum<<"个关键词。"<<endl;
	return machine->wordsnum;
}

// 
int CKeyWordsManager::AddKeyIDs(string strName, const char* cWordFile)
{
	if(strName.size() == 0)
	{
		cout<<"name is null."<<endl;
		return -1;
	}
	map<string, map<unsigned long long, int> >::iterator iter = m_mapKeyIDs.find(strName);
	if(iter != m_mapKeyIDs.end())
	{
		cout<<strName<<":this name is so popular that you must change another one."<<endl;
		return -1;
	}
	
	LoadKeyIDs(cWordFile, m_mapKeyIDs[strName]);
	cout<<strName<<"的ID装载完毕，成功装载"<<m_mapKeyIDs[strName].size()<<"个ID。"<<endl;
}

// 给输入的文本用指定的关键词集合打分
int CKeyWordsManager::GetTextScore(string& strText, string strName, bool bSum)
{
	map<string, KeyWords>::iterator iter = m_mapKeyWords.find(strName);
	if(iter == m_mapKeyWords.end())
	  return 0;

	MATCHENTRY* machine = iter->second.machine;
	
	return ScoreText(iter->second.machine, iter->second.mapScore, strText, bSum);
}

int CKeyWordsManager::GetIDScore(unsigned long long id, string strName)
{
	map<string, map<unsigned long long, int> >::iterator iter = m_mapKeyIDs.find(strName);
	if(iter == m_mapKeyIDs.end())
	{
		return 0;
	}
	map<unsigned long long, int>::iterator iter1 = iter->second.find(id);
	if(iter1 == iter->second.end())
	  return 0;

	return iter1->second;
}

// 判断输入文本您是否具有某集合的关键词
int CKeyWordsManager::HaveKeyWords(string& strText, string strName)
{
	map<string, KeyWords>::iterator iter = m_mapKeyWords.find(strName);
	if(iter == m_mapKeyWords.end())
	  return -1;
	static vector<pair<string, int> > vecWatchWords;
	int watchNum = 0;
	FindKeyWord(strText.c_str(), watchNum, iter->second.machine, vecWatchWords);

	return watchNum;
}

int CKeyWordsManager::HaveKeyWords(string& strText, string strName, vector< pair<string, int> >& vecKeyWords)
{
	map<string, KeyWords>::iterator iter = m_mapKeyWords.find(strName);
	if(iter == m_mapKeyWords.end())
	  return -1;
	vecKeyWords.clear();
	int watchNum = 0;
	FindKeyWord(strText.c_str(), watchNum, iter->second.machine, vecKeyWords);

	return watchNum;
}

int CKeyWordsManager::HaveKeyWords(string& strText, string strName, map<string, int>& mapKeyWords)
{
	map<string, KeyWords>::iterator iter = m_mapKeyWords.find(strName);
	if(iter == m_mapKeyWords.end())
	  return -1;
	mapKeyWords.clear();
	vector< pair<string, int> > vecKeyWords;
	int watchNum = 0;
	FindKeyWord(strText.c_str(), watchNum, iter->second.machine, vecKeyWords);
	for(unsigned int index = 0 ; index < vecKeyWords.size(); index++)
	{
		mapKeyWords[vecKeyWords[index].first] = iter->second.mapScore[vecKeyWords[index].first];
	}
	return watchNum;
}

int CKeyWordsManager::HaveKeyIDs(unsigned long long id, string strName)
{
	map<string, map<unsigned long long, int> >::iterator iter = m_mapKeyIDs.find(strName);
	if(iter == m_mapKeyIDs.end())
		return 0;
	
	map<unsigned long long, int>::iterator iter1 = iter->second.find(id);
	if(iter1 == iter->second.end())
		return 0;

	return 1;
}
// 获取关键词数目
int CKeyWordsManager::GetKeyWordsCount(string strName)
{
	map<string, KeyWords>::iterator iter = m_mapKeyWords.find(strName);
	if(iter == m_mapKeyWords.end())
	  return 0;
	return iter->second.machine->wordsnum;
}

// 返回关键词
string CKeyWordsManager::GetKeyWord(string strName, int index)
{
	map<string, KeyWords>::iterator iter = m_mapKeyWords.find(strName);
	if(iter == m_mapKeyWords.end())
	  return "";

	MATCHENTRY* machine_ww = iter->second.machine;
	if(index < 0 || index >= machine_ww->wordsnum)
	  return "";
	for(int i = 0; i< machine_ww->wordsnum; i++)
	{
		if(index == i)
		  return (char*)machine_ww->wordlist[i].word; 
	}
	return "";
}

// 查找关键词
int CKeyWordsManager::FindKeyWord(const char *text,int &watchNum,MATCHENTRY *machine_ww,vector<pair<string,int> >& watchWord)
{
	if(text == NULL || machine_ww == NULL || machine_ww->wordsnum == 0)
		return -1;
	FindAllUntilNull(text,0,machine_ww);
	watchNum=0;
	watchWord.clear();
	pair<string,int> pp;


	for(int i = 0; i< machine_ww->wordsnum; i++)
	{
		if (machine_ww->wordlist[i].findnum>0)
		{
			pp.first = (char*)machine_ww->wordlist[i].word;
			pp.second = machine_ww->wordlist[i].findnum;
			watchWord.push_back(pp);
			watchNum += machine_ww->wordlist[i].findnum;
			machine_ww->wordlist[i].findnum = 0;
		}
	}
	return machine_ww->wordsnum;
}

// 打分
int CKeyWordsManager::ScoreText(MATCHENTRY* pKeyWords, map<string, int>& mapScore, string& strText, bool bSum)
{
	vector<pair<string, int> > vecWatchWords;
	int watchNum = 0;
	FindKeyWord(strText.c_str(), watchNum, pKeyWords, vecWatchWords);
	// 取最大的分值
	int nScore = 0;
	for(unsigned int i=0; i!=vecWatchWords.size(); i++)
	{
		map<string, int>::iterator iter = mapScore.find(vecWatchWords[i].first);
		if(iter == mapScore.end())
		  continue;
		if(iter->second > nScore && (!bSum))
		  nScore = iter->second;
		else
		  nScore += iter->second;
	}

	return nScore;
}

//装载关键词
MATCHENTRY* CKeyWordsManager::LoadKeyWords(const char* cFile, map<string, int>& mapScore)
{
	if(cFile == NULL)
		return NULL;
	ifstream fin;
	fin.open(cFile);
	if(!fin.is_open())
	{
		cout<<cFile<<" can not be opened."<<endl;
		return NULL;
	}
	MATCHENTRY *machine = NULL;
	if(!(machine = strMatchInit(2)))
	{
		fprintf(stderr,"Init machine %s err!\n",cFile);
		return NULL;
	}

	char buf[1024]="";
	vector<string> vecTmp;

	while(!fin.eof())
	{
		if(fin.getline(buf,1024).good()==false)
			break;

		string line(buf);	
		if(line.size() == 0)
		  continue;
		vecTmp = CStringTool::SpliteByChar(line, '\t');
		if(vecTmp.size() == 2)
		{
			mapScore[vecTmp[0]] = CTypeTool<int>::StrTo(vecTmp[1]);
		}

		Addword(vecTmp[0].c_str(),0,machine,0);
	}
	fin.close();
	Prepare(0, machine);
	return machine;
}

void CKeyWordsManager::LoadKeyIDs(const char* cFile, map<unsigned long long, int>& mapScore)
{
	if(cFile == NULL)
		return ;
	ifstream fin;
	fin.open(cFile);
	if(!fin.is_open())
	{
		cout<<cFile<<" can not be opened."<<endl;
		return ;
	}

	char buf[1024]="";
	vector<string> vecTmp;
	string line;
	while(!fin.eof())
	{
		getline(fin,line);
		if(line.size() == 0)
		  continue;

		vecTmp = CStringTool::SpliteByChar(line, '\t');
		unsigned long long id = CTypeTool<unsigned long long>::StrTo(vecTmp[0]);
		if(vecTmp.size() == 2)
		{
			mapScore[id] = CTypeTool<int>::StrTo(vecTmp[1]);
		}
		else
		{
			mapScore[id] = 0;
		}
	}
	fin.close();
}
