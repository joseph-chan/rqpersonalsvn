#include<iostream>
#include<map>
#include<string>
#include<fstream>
#include"fromuni.h"
#include"get_keyword2.h"
struct tf
{
	int tf;
	double idf;
	bool flag;
	char yuyi;
};
#define FILENAME_LEN 1024
//int get_filelineCount(const char *filename);
void Init_tf(tf &xx);
void Init_key(keyword &xx);
double fill_vector(map<string,tf>&word,vector<keyword>&keyseg,int keynum);
double get_keyword(WORD_SEGMENT *wordseg,vector<keyword> &keyseg,int keynum)
{
	tf xx;//词频和idf信息结构体
	Init_tf(xx);
	char tmp[1024];
	int len=0,nlen=0,postid=0,elen=0;
	double lastfWeight=0;
	bool flag=false;
	string temp;
	map<string,tf>word;
	map<string,tf>::iterator it;
	keyseg.clear();//清空向量
	for(int i = 0; i < wordseg->word_num; i++)
	{
		memset(tmp,0,1024);
		flag=false;
		len = uniToBytes(wordseg->uni+(wordseg->wordlist[i]).wordPos, (wordseg->wordlist[i]).      wordLen, tmp, 1024, "GBK");
		tmp[len] ='\0';
		postid=(wordseg->wordlist[i]).postagid;
		nlen=0;//number
		elen=0;//english
		for(int j=0;j<len;j++)
		{
			if(isdigit(tmp[j]))
				nlen++;
			if(isalpha(tmp[j]))
				elen++;
		}
		//cout<<tmp<<"\t"<<len<<"\t"<<(int)wordseg->wordlist[i].wordLen<<endl;
		if((wordseg->wordlist[i]).wordLen<2)//单字过滤掉
			flag=true;
		if((wordseg->wordlist[i]).idf<0.0001)
			(wordseg->wordlist[i]).idf=120;
		if(postid==108||(postid>=104&&postid<=132)||(postid>=190&&postid<=202))
			flag=true;
		if(postid==0||postid==230||tmp[0]=='\t'||(postid>=172&&postid<=176))
			flag=true;
		if((postid>=140&&postid<=160)||(postid>=30&&postid<90))
			flag=true;
		if((postid<95||postid>103)&&(wordseg->wordlist[i]).idf<50)
			flag=true;
		if((postid==10||postid==20)&&(wordseg->wordlist[i]).idf<100)//增强对形容词的过滤
			flag=true;
		if((2*elen>len)&&(wordseg->wordlist[i]).idf<121)
			flag=true;
		if(!flag)
		{
			temp=tmp;
			it=word.find(temp);
			if(it==word.end())
			{
				xx.yuyi='\0';
				if((wordseg->wordlist[i]).semanticnum)
				{
					xx.yuyi=(wordseg->wordlist[i]).semanticid[0];
				}
				xx.tf=1;
				xx.idf=(wordseg->wordlist[i].idf*1.0)/10;
				word.insert(map<string,tf>::value_type(temp,xx));
			}
			else
				it->second.tf++;
		}
	}
	lastfWeight=fill_vector(word,keyseg,keynum);
	//cout<<keyseg.size()<<"\t"<<keynum<<"fill_vector"<<endl;
	return lastfWeight;
}
void Init_tf(tf &xx)
{
	xx.tf=1;
	xx.idf=12;
	xx.flag=true;
	xx.yuyi='\0';
}
double fill_vector(map<string,tf>&word,vector<keyword>&keyseg,int keynum)
{
	int wordnum=word.size();
	double lastmax=10000,fw=0;
	keyword actkey;
	map<string,tf>::iterator it;
	for( int i=0;i<keynum&&i<wordnum;i++)
	{
		Init_key(actkey);
		for(it=word.begin();it!=word.end();it++)
		{
			fw=log(1.0*(it->second.tf+1))*it->second.idf;
			if((fw>actkey.fWeight)&&(fw<=lastmax)&&(it->second.flag))
			{
				actkey.word.clear();
				actkey.word=it->first;
				actkey.fWeight=fw;
				actkey.tf=it->second.tf;
				actkey.yuyi=it->second.yuyi;
			}
		}
#ifdef DEBUG
		cout<<(int)(actkey.yuyi)<<endl;
#endif
		it=word.find(actkey.word);
		it->second.flag=false;
		keyseg.push_back(actkey);
		lastmax=actkey.fWeight;
	}
	return lastmax;//返回最后一个关键词的权重大小
}
void Init_key(keyword &xx)
{
	xx.word.clear();
	xx.fWeight=0;
	xx.tf=0;
	xx.yuyi='\0';;
}
