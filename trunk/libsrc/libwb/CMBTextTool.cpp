#include "CMBTextTool.h"
#include <iostream>
using namespace std;

// static����Ҫ����
set<string> CMBTextTool::m_setChPunct;

// ��ʼ��
void CMBTextTool::InitTool()
{
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert("��");
	m_setChPunct.insert(" ");
	m_setChPunct.insert("?");
}

//
int CMBTextTool::ExtractNickName(const char* text, map<string, int>& mapAt)
{
	mapAt.clear();
	char* atPos = strstr(text, "@");
	int atnum = 0;
	while(atPos != NULL)
	{
		//cout<<atPos<<endl;
		int pos = 0;
		atPos++;
		if(*atPos == '\0')
		  break;
		// ����΢���ǳƵĹ淶�����ܳ���20���ֽ�
		bool chn = false;//�Ժ��ֱ��������ǳ�
		bool enp = false;//��ascii��������ǳ�
		while(pos <= 20 && atPos[pos] != '\0')
		{
			// �����ascii�������cctype�еĺ���ispunct�ж��Ƿ�Ӣ�ı����߿ո�
			if(atPos[pos] >= 0 && atPos[pos] <= 127)
			{
				if('-' != atPos[pos] && '_' != atPos[pos] && (ispunct(atPos[pos]) || ' ' == atPos[pos] ))
				{
					enp = true;
					break;
				}
			}
			// ����Ǻ�����ȡ�����ֽ�ƴ��һ���ַ����ж��ǲ������ı��
			else
			{
				char tmp[3];
				tmp[0] = atPos[pos++];
				tmp[1] = atPos[pos];
				tmp[2] = '\0';
				if(IsPunction(tmp))
				{
					chn = true;
					break;
				}
			}
			pos++;
		}
		if(chn || enp || atPos[pos] == '\0')
		{
			char* cuser = NULL;
			int len  = pos;
			if(chn)
			  len--;
			cuser = new char[len+1];
			strncpy(cuser, atPos, len);
			cuser[len] = '\0';
			string user(cuser);
			if(mapAt.count(user))
			  mapAt[user]++;
			else
			  mapAt[user] = 1;
			delete []  cuser;
			atnum++;
		}
		atPos = strstr(atPos+pos, "@");
	}
	return atnum;
}

int CMBTextTool::KillNickName(string& text)
{
	unsigned int atPos = text.find_first_of("@", 0);
	int atnum = 0;
	while(atPos != text.npos)
	{
		int pos = 0;
		text[atPos] = ' ';
		atPos++;
		if(atPos == text.npos)
		  break;
		// ����΢���ǳƵĹ淶�����ܳ���20���ֽ�
		bool chn = false;//�Ժ��ֱ��������ǳ�
		bool enp = false;//��ascii��������ǳ�
		while(pos <= 20 && text[atPos+pos] != '\0')
		{
			// �����ascii�������cctype�еĺ���ispunct�ж��Ƿ�Ӣ�ı����߿ո�
			if(' ' == text[atPos+pos])
			{
				enp = true;
				break;
			}
			else if(text[atPos+pos] >= 0 && text[atPos+pos] <= 127)
			{
				if(('-' != text[atPos+pos] && '_' != text[atPos+pos] && 
							(ispunct(text[atPos+pos])  )) )
				{
					enp = true;
					break;
				}
			}
			// ����Ǻ�����ȡ�����ֽ�ƴ��һ���ַ����ж��ǲ������ı��
			else
			{
				int postmp = atPos+pos;
				pos++;
				if(IsPunction(text.substr(postmp, 2).c_str()))
				{
					chn = true;
					break;
				}
			}
			pos++;
		}
		if(chn || enp || atPos+pos == text.size())
		{
			int len  = pos;
			if(chn)
			{
			  len--;
			}
			text.replace(atPos, len, " ");
		}
		atPos = text.find_first_of('@', atPos);
	}
	return atnum;
}
// Ϊ�˼���֮ǰ�ķ��������ò��ظ�д���������
int CMBTextTool::ExtractNickName(string& text, map<string, int>& mapAt, bool bRemoveAt)
{
	unsigned int atPos = text.find_first_of("@", 0);
	int atnum = 0;
	mapAt.clear();
	while(atPos != text.npos)
	{
		int pos = 0;
		text[atPos] = ' ';
		atPos++;
		if(atPos == text.npos)
		  break;
		// ����΢���ǳƵĹ淶�����ܳ���20���ֽ�
		bool chn = false;//�Ժ��ֱ��������ǳ�
		bool enp = false;//��ascii��������ǳ�
		while(pos <= 20 && text[atPos+pos] != '\0')
		{
			// �����ascii�������cctype�еĺ���ispunct�ж��Ƿ�Ӣ�ı����߿ո�
			if(text[atPos+pos] >= 0 && text[atPos+pos] <= 127)
			{
				if(('-' != text[atPos+pos] && '_' != text[atPos+pos] && 
							(ispunct(text[atPos+pos]) || ' ' == text[atPos+pos] )) )
				{
					enp = true;
					break;
				}
			}
			// ����Ǻ�����ȡ�����ֽ�ƴ��һ���ַ����ж��ǲ������ı��
			else
			{
				int postmp = atPos+pos;
				pos++;
				if(IsPunction(text.substr(postmp, 2).c_str()))
				{
					chn = true;
					break;
				}
			}
			pos++;
		}
		if(chn || enp || atPos+pos == text.size())
		{
			int len  = pos;
			if(chn)
			  len--;
			string user = text.substr(atPos, len);
			if(mapAt.count(user))
			  mapAt[user]++;
			else
			  mapAt[user] = 1;
			atnum++;
			// �����ҪĨ���ǳƣ���ô�ͽ��ǳƴ����Ϊ�հ�
			if(bRemoveAt)
			{
				text.replace(atPos, len, " ");
			}
		}
		atPos = text.find_first_of('@', atPos);
	}
	return atnum;
}

// �ж��Ƿ������ı��
bool CMBTextTool::IsPunction(const char* ch)
{
	if(m_setChPunct.size() == 0)
	{
		cout<<"CMBTextTool has not been initialized!"<<endl;
		return false;
	}
	return m_setChPunct.count(string(ch))!= 0;
}

// ȥ��΢���е�url
int CMBTextTool::KillURL(string& text)
{
	unsigned int start = text.find("http://");
	int urlnum = 0;
	while(start != text.npos)
	{
		unsigned int end = text.find(" ", start);
		if(end == text.npos)
		{
			for(unsigned int i = start; i != text.size(); i++)
			{
				if(!(text[i] >= 0 && text[i] <= 127))
				{
					end = i;
					break;
				}
			}
		}
		text.replace(start, end - start, " ");
		start = text.find("http://", end);
		urlnum++;
	}
	return urlnum;
}

// ���΢���е�url
int CMBTextTool::GetURL(string& text, map<string, int>& mapURL)
{
	mapURL.clear();
	unsigned int start = text.find("http://");
	int urlnum = 0;
	while(start != text.npos)
	{
		unsigned int end = text.find(" ", start);
		if(end == text.npos)
		{
			for(unsigned int i = start; i != text.size(); i++)
			{
				if(!(text[i] >= 0 && text[i] <= 127))
				{
					end = i;
					break;
				}
			}
		}
		string url = text.substr(start, end - start);
		map<string, int>::iterator iter = mapURL.find(url);
		if(iter == mapURL.end())
		  mapURL[url] = 0;
		else
		  iter->second++;
		//text.replace(start, end - start, " ");
		start = text.find("http://", end);
		urlnum++;
	}

	return urlnum;
}

// Ĩ��΢���еĻ���
int CMBTextTool::KillTopic(string& text, vector<string>& vecTopics)
{
	return KillElements(text, "#", "#", vecTopics);
}

void CMBTextTool::KillIntrInfo(string& text)
{
	vector<string> tmp;
	KillElements(text, "(", ")", tmp);
	KillElements(text, "��", "��", tmp);
	KillElements(text, "��", "��", tmp);
}

// Ĩ��΢���еı���
int CMBTextTool::KillEmotion(string& text)
{
	vector<string> vectmp;
	return KillElements(text, "[", "]", vectmp);
}


// ���ݸ�������ֹ�ַ�����Ĩ�����е����д�
int CMBTextTool::KillElements(string& strText, string strBegin, string strEnd, vector<string>& vecElements)
{
	if(strBegin.size() == 0 || strEnd.size() == 0)
	  return 0;
	int count = 0;
	unsigned int prepos = strText.find(strBegin);
	vecElements.clear();
	while(prepos != strText.npos)
	{
		prepos += strBegin.size();
		if(prepos == strText.npos)
		  break;
		unsigned int postpos = strText.find(strEnd, prepos);
		if(postpos == strText.npos)
		  break;
		count++;
		string element = strText.substr(prepos, postpos - prepos);
		vecElements.push_back(element);
		strText.replace(prepos-strBegin.size(), postpos + strEnd.size() - prepos + strBegin.size(), " ");
		prepos = strText.find(strBegin, prepos);
	}

	return count;
}


