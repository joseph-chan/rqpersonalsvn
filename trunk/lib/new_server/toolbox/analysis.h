#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "lexicon.h"
#include "maxentmodel.hpp"
#include "strmatch.h"
#include "bayesHM.h"
#include <vector>
using namespace std;
#define testbit(x,y) x&(1<<y)  //����x�ĵ�YλΪ1
#define min(a,b) a<b?a:b //ȡ��Сֵ����

#ifdef __cplusplus
extern "C"
{
#endif
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

	int wordsegSimple(WORD_SEGMENT *wordseg,vector<wordItem> &wordvec,double &engDigitRate,int wordLevel);

	int get_wordtf(vector<wordItem> &wSimple,map<string,wordItem>&wordtf);

	struct coreData
	{
		map<string,int> hanzi;//���庺�ֱ�

		map<string,int> englishWord;//Ӣ�Ĵʱ�

		map<string,string> chTodigit;//�ַ������������ֱ�

		map<string,int> interpunction;//�����ű�

		MATCHENTRY *blackList;//������

		MATCHENTRY *whiteList;//��������Ӧ����ɱ������

		maxent::MaxentModel *maxent_variationAdv;//���ֹ��ģ��

		BAYES_HM_MODEL *bayesHM_adv;//��Ҷ˹���ģ��
	};
	int loadCoreData(const char *path,coreData *data);

	/*****************************************************************
	 * date:2010.05.27
	 * author:guibin
	 * describe:�������ı��������˵��ı���һ������ӿ� �����ı�����
	 * src:�����ı�
	 * srcLen:�����ı�����
	 * out:����ı�
	 * outLen:����ı�����
	 * labelRate:�����ַ�ռ�ı��ı���
	 * spiteRate:�����ʣ�����������㷽ʽ
	 * urlMap:�ı�����������URL
	 * flag:��һ������ѡ��
	 *      00000001 Ӣ�Ĵ�Сдת�� 
	 *      00000010 ���Ź�һ������==ȫ���ת��
	 *      00000100 Ĩ�����еı�����
	 *      00001000 �������ֱ�﷽ʽ��һ��
	 *      00010000 Ĩ��html��ǩ
	 *      00100000 ȥ���������
	 *      01000000 �Ƿ����ˢ������
	 *      10000000 ֻ�����ı��еĺ���
	 * return:==0 �ӿ�ִ������
	 *        <0 �ӿڵ���ʧ��
	 * **************************************************************/
	int TextNormalization(char *src,int srcLen,char *out,int outLen,coreData *data,int flag);

	//�ж��Ƿ��������ظ��ʹ��ߵ��ı� compressRate  Խ�� �ı����ظ���Խ��
	int IS_overlap(const char *text,double &compressRate);

	//Ĩ���ı��еı�����
	int rubout(const char *src,char *out,coreData *data,int flags);

	//ͨ�õ������ģ�͸���ʶ��ӿ�
	int MaxentModeRate(map<string,wordItem> &wordtf,double &badRate,maxent::MaxentModel *model);

	//��ģʽװ�غ���
	MATCHENTRY * loadMachine(const char *file);

	//Ĩ���ı��е�HTML���ŷ� + �������
	int eraseHtmlEmoticons(const char *src,char *text,int &htmlLen,map<string,int>  &urlMap,coreData *cdata,int flags);

	//map<string,int>����Դ��ͨ��װ�غ���
	int loadMapResource(const char *file,map<string,int> &rMap,int type);

	//URL������װ��
	int loadBlackURL(const char *file,map<string,int> &black_url);

	//�ı����Ƿ������ϵ��ʽ
	int ExistContactWay(WORD_SEGMENT *wordseg, double &badrate);

	//���ַ��ָ��ı�
	int splitTextbyCh(const char *src,vector<string> &chVec);

	//������Ҷ˹ģ������Ҫ�������ռ�ģ�͵�����
	int buildBayesData(map<string,wordItem> &wordtf,vector<pair<string,double> > &bayes_word);
	
	//ͨ������ֵ���㺯��
	int eigenvalueCompute(map<string,wordItem> &wordtf,vector<pair<string,double> > &eigenvalueVec,int *oneGramNum);
	//�ַ����ķ�����ת��
	int transComplex2simple(char *src,int slen,char *out,int olen);

	//���ط����ֵ������ֵĶ�Ӧ��unicode��ʽ
	int loadComplex2simple(const char *path);

	int findBlackWord(char *text,int &watchNum,MATCHENTRY *machine_ww,vector<pair<string,int> > &watchWord);
	
	//20110310
	int loadStrIntMap(const char *file,map<string,int> &wmap,int value);
#ifdef __cplusplus
}
#endif

#endif
