#ifndef PAICHONG_H
#define PAICHONG_H

#include "lexicon.h"
#include "wordsegSimple.h"
//#include "analysis.h"
#include <map>
#include <vector>
using namespace std;


#ifdef __cplusplus
extern "C"
{
#endif

struct ResData
{
	map<string,int> white_swatch;//�ı�������
	map<string,string> simplified_complex;//���嵽������ձ�
	map<string,int> white_nickname;//�� ������¼ ��������ǳ�
};

// �����ı�key
int textHash(WORD_SEGMENT *wordseg,unsigned long long &textkey,int level,int nMinTermCount = 15);
int textHash2(WORD_SEGMENT *wordseg,vector<unsigned long long> &textkeys,int level, int nMinTermCount = 15);

int chooseWord(WORD_SEGMENT *wordseg,vector<wordItem> &wordvec,int wordLevel);

//ͨ����string map��װ�غ���
int loadStringMap(const char *file,map<string,string> &smap);

//��Դװ��
int loadResource(const char *path,ResData &rdata);
#ifdef __cplusplus
}
#endif

#endif

