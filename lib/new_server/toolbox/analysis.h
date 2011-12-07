#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "lexicon.h"
#include "maxentmodel.hpp"
#include "strmatch.h"
#include "bayesHM.h"
#include <vector>
using namespace std;
#define testbit(x,y) x&(1<<y)  //测试x的第Y位为1
#define min(a,b) a<b?a:b //取最小值函数

#ifdef __cplusplus
extern "C"
{
#endif
	struct wordItem
	{
		char word[64];//词
		int pid;//词性
		int idf;//重要性
		int tf;//词频
		int wordLen;//词的字符数量
		int semantic;//语义属性
		bool isErase;//该词是否需要抹掉
		bool isEngOrDigit;//是否是英文和数字 
	};

	int wordsegSimple(WORD_SEGMENT *wordseg,vector<wordItem> &wordvec,double &engDigitRate,int wordLevel);

	int get_wordtf(vector<wordItem> &wSimple,map<string,wordItem>&wordtf);

	struct coreData
	{
		map<string,int> hanzi;//简体汉字表

		map<string,int> englishWord;//英文词表

		map<string,string> chTodigit;//字符到阿拉伯数字表

		map<string,int> interpunction;//标点符号表

		MATCHENTRY *blackList;//黑名单

		MATCHENTRY *whiteList;//黑名单对应的免杀白名单

		maxent::MaxentModel *maxent_variationAdv;//变种广告模型

		BAYES_HM_MODEL *bayesHM_adv;//贝叶斯广告模型
	};
	int loadCoreData(const char *path,coreData *data);

	/*****************************************************************
	 * date:2010.05.27
	 * author:guibin
	 * describe:适用于文本垃圾过滤的文本归一化处理接口 并对文本评分
	 * src:输入文本
	 * srcLen:输入文本长度
	 * out:输出文本
	 * outLen:输出文本长度
	 * labelRate:干扰字符占文本的比例
	 * spiteRate:恶意率，还需调整计算方式
	 * urlMap:文本中所包含的URL
	 * flag:归一化功能选项
	 *      00000001 英文大小写转化 
	 *      00000010 符号归一化处理==全半角转化
	 *      00000100 抹掉文中的标点符号
	 *      00001000 多种数字表达方式归一化
	 *      00010000 抹掉html标签
	 *      00100000 去掉表情符号
	 *      01000000 是否进行刷屏测试
	 *      10000000 只保留文本中的汉字
	 * return:==0 接口执行正常
	 *        <0 接口调用失败
	 * **************************************************************/
	int TextNormalization(char *src,int srcLen,char *out,int outLen,coreData *data,int flag);

	//判断是否是文字重复率过高的文本 compressRate  越低 文本的重复率越高
	int IS_overlap(const char *text,double &compressRate);

	//抹掉文本中的标点符号
	int rubout(const char *src,char *out,coreData *data,int flags);

	//通用的最大熵模型概率识别接口
	int MaxentModeRate(map<string,wordItem> &wordtf,double &badRate,maxent::MaxentModel *model);

	//多模式装载函数
	MATCHENTRY * loadMachine(const char *file);

	//抹掉文本中的HTML干扰符 + 表情符号
	int eraseHtmlEmoticons(const char *src,char *text,int &htmlLen,map<string,int>  &urlMap,coreData *cdata,int flags);

	//map<string,int>型资源的通用装载函数
	int loadMapResource(const char *file,map<string,int> &rMap,int type);

	//URL黑名单装载
	int loadBlackURL(const char *file,map<string,int> &black_url);

	//文本中是否存在联系方式
	int ExistContactWay(WORD_SEGMENT *wordseg, double &badrate);

	//以字符分割文本
	int splitTextbyCh(const char *src,vector<string> &chVec);

	//生产贝叶斯模型所需要的项梁空间模型的数据
	int buildBayesData(map<string,wordItem> &wordtf,vector<pair<string,double> > &bayes_word);
	
	//通用特征值计算函数
	int eigenvalueCompute(map<string,wordItem> &wordtf,vector<pair<string,double> > &eigenvalueVec,int *oneGramNum);
	//字符串的繁简体转化
	int transComplex2simple(char *src,int slen,char *out,int olen);

	//下载繁体字到简体字的对应表unicode形式
	int loadComplex2simple(const char *path);

	int findBlackWord(char *text,int &watchNum,MATCHENTRY *machine_ww,vector<pair<string,int> > &watchWord);
	
	//20110310
	int loadStrIntMap(const char *file,map<string,int> &wmap,int value);
#ifdef __cplusplus
}
#endif

#endif
