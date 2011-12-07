#ifndef WORDSEGSIMPLE_H
#define WORDSEGSIMPLE_H
#include <vector>
#include <string>
#include <map>
#include "lexicon.h"
#include "fromuni.h"
#include "touni.h"
using namespace std;
struct wordItem
{
	char word[64];//��
	int pid;//����
	int idf;//��Ҫ��
	int tf;//��Ƶ
	int wordLen;//�ʵ��ַ�����
	int semantic;//��������
	bool isErase;//�ô��Ƿ���ҪĨ��
	bool isEngOrDigit;//�Ƿ���Ӣ�ĺ����� 
};
int wordsegSimple(const WORD_SEGMENT *wordseg,vector<wordItem> &wordvec);
int get_wordtf(vector<wordItem> &wSimple,map<string,wordItem>&wordtf);
bool great_second(const pair<string,double> &m1,const pair<string,double> &m2);
#endif
