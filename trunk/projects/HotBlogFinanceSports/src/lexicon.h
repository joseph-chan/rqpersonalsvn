/*
* Copyright (c) 2004, 新浪网研发中心
* All rights reserved.
* 
* 文件名称：lexicon.h
* 摘    要：用于词典的数据和功能定义
* 作    者：彭中华 2004/07/20
*           孙健 2006.04.12
* 版    本：1.0
* $Id: lexicon.h,v 1.3 2005/04/04 10:26:46 zhonghua Exp $
*/


#ifndef LEXICON_H
#define LEXICON_H

#include "searchdef.h"

#include "BaseDef.h"
#include "WordSegment.h" //分词
#include "POSTagger_new.h" //词性标注
#include "phraseterm.h"  //短语合并
#include "ExKeywordsInterface.h" //关键词提取
//#include "classifydomain.h"  //文本分类1

#include "classify.h" //文本分类2
#include "classifyknn.h" //文本分类2

#include "dllcall.h"
#include "wordprop.h"

/* 词典词库词属性信息数据结构定义 */
typedef struct 
{
	BYTE type;					//词类别 缺省为0
	BYTE stat;					//相关状态 缺省为0
	WORD IDF;					  //相关属性 缺省为0
}WORD_PROPERTY;


/* 词典描述信息数据结构定义 */
typedef struct {
	char rdir[256];				//根目录，缺省为0
	char fn_config[100];		//词典配置文件

	int lexdata;				//对应的词典数据编号
	int ws_sel;					//词典当前选择的语种
}LEXICON;


/* 分词结果数据结构定义 */
#define DOMAIN_SEMANTIC 1
typedef struct {
	unsigned int wordPos;		    // Position in source string, 在unicode串中的位置
	unsigned long long wordID;	// WORDID
	
	unsigned char wordLen;	    // Length of this word，      unicode的长度
	unsigned char postagid;     // 该词的词性id，请见POSTagger_new.h
	unsigned short wordCat;		  // 从分词系统带过来的类别属性，Attribute of word item, such as personal name, corporation name, and so on.
		
	WORD_PROPERTY wordprop;		  // word property
	
	unsigned short childnodes; //2007.7.13 sunjian
											 //该字段只有在机构名时有效；其它情况暂时空闲；
											 //16位分为4组 1111/1111/1111/1111
											 //如果机构名为AD型，那么取左边8位，每4位分别对应a和d
											 //如果机构名为ABD型，那么取左边12位，每4位分别对应a b d
											 //如果机构名为ACD型，那么取左边12位，每4位分别对应a c d
											 //如果机构名为ABCD型，那么取16位，每4位分别对应a b c d
											 //a b c d的最大值为16
											 //a b c d的值分别表示在wordlist中其对应的WORD_ITEM节点相对于上一个WORD_ITEM的偏移
											 
											 //如：北京市公交公司为 ACD类型的机构名
											 
											 // 0                                          1                        2           3             4               5
											 //北京市公交公司(0,7) 北京市(0,3)  北京(0,2) 市(2,1) 公交(3,2) 公司(5,2)
											 //64                                      192                 128            128         128          128
											 
											 //北京市公交公司(0,7)的3个孩子分别A=北京市(0,3), C=公交(3,2), D=公司(5,2)
											 //a =1-0=1, c=4-1=3, d=5-4=1
											 
											 //因此，"北京市公交公司"的childnodes = a<<12 | c<<8 | d<<4

#ifdef DOMAIN_SEMANTIC //输出分词结果节点的领域属性和语义属性 20070310 
   unsigned char domainnum; //该term有几个领域
   unsigned short domainid[TERM_MAX_DOMAIN_NUM]; //领域属性
	
   unsigned char semanticnum; //该term有几个语义属性
   unsigned short semanticid[TERM_MAX_SEMANTIC_NUM]; //语义属性
#endif

   unsigned char wordvalue;
   //当对query分析时用, wordvalue的说明:
   //高4位表示该词单元的类型 低4位表示该类型中词的权重
   // (0000 00) 疑问词  十进制范围(0)
   // (0001 00**) 关键名词 十进制范围(4-7) 
   // (0010 00**) 关键动词 十进制范围(8-11)
   // (0011 00**) 其他关键词 十进制范围12-15)
   // (0100 00**) 其他词 十进制范围(16-19)
   // (0101 00**) 停用词 十进制范围(20-23)
   //当对文本分析时，wordvalue字段的值另外解析 20070418
  
   unsigned char idf;          //idf * 10, 取值范围在0-255，用的时候需要再除以10 
   //unsigned long long reserved;

}WORD_ITEM;

typedef struct {
	WORD_ITEM word_item;
	unsigned int appwordPos;		  //Position:在分词结果数组的位置, 第几个词，起始值 = 0
	//unsigned short appwordnum;	 //Length:  包含的分词结果的词个数.该字段暂时一直=0
	
} APP_WORD_ITEM;//应用层的term

#define LIST_MAX_CHILD_NUM 10

typedef struct{
	WORD_ITEM *word;
	unsigned int childnum;
	unsigned int children[LIST_MAX_CHILD_NUM];
}LIST_ITEM;


/* 分词请求与结果数据结构定义 */
typedef struct {
	int buflen;					//对象分配空间大小
	char textfmt[10];		//请求分词的文字格式，"GBK","UNICODE"
	
	int len_uni;				//Unicode格式的文字长度
	unsigned short* uni;//Unicode格式的文字内容
	
	int word_num;				//得到的分词结果个数
	WORD_ITEM *wordlist;		//分词结果列表
	
	int appword_num;				//得到的应用层 term个数
	APP_WORD_ITEM *appwordlist;		//应用层 term列表
	
	int listitem_num;
	LIST_ITEM *list;

	DocInfo  docprop;    //应用层分析后　得到的对这篇文档的领域描述信息
	KeyWordStore keywords; //应用层分析后　得到的对这篇文档的关键词，人名 地名 事件 专指
	
}WORD_SEGMENT;


#ifdef __cplusplus
extern "C" {
#endif
/*	对词典操作的相关函数 */

/**
* 函数介绍：	打开一个词典库
* 输入参数：	rdir		词典库根目录
* 输出参数： 
* 返回值  ：	lexicon		词典库描述信息，NULL表示打开失败
*/
LEXICON *OpenLexicon(const char *rdir);

/**
* 函数介绍：	打开一个词典库
* 输入参数：	rdir		词典库根目录，带选择的load资源
* 输出参数： 
* 返回值  ：	lexicon		词典库描述信息，NULL表示打开失败
* option      决定load哪些资源
*                 16位
*                       分类   关键词提取   短语合并   词性标注   分词
* 00 000 000 00         1          1             1         1       1
* 
* 注意事项:
* Load资源的参数选项与Analyze的参数选项是相对独立的。但是如果只load了分词的资源，然后要求既分词又词性标注时就有问题
* 
*/
LEXICON *OpenLexicon_Opt(const char *rdir, unsigned short option);

/**
* 函数介绍：	保存词典库属性数据
* 输入参数：	lexicon		词典库描述信息，指针
* 输出参数： 
* 返回值  ：	0			成功
*				<0			失败
*/
int SaveLexicon(LEXICON *lexicon);

/**
* 函数介绍：	关闭一个词典库
* 输入参数：	lexicon		词典库描述信息，指针
* 输出参数： 
* 返回值  ：	0			成功
*				<0			失败
*/
int CloseLexicon(LEXICON *lexicon);

/**
* 函数介绍：	从词典中修改指定缺省使用的语种
* 输入参数：	lexicon		词典库描述信息，指针
*				ws_sel		0为简体，1为繁体
* 输出参数： 	
* 返回值  ：	0			成功
*				<0			失败
*/
int setLexiconWS(LEXICON *lexicon, int ws_sel);

/**
* 函数介绍：	分配并初始化一个保存分词结果的数据对象
* 输入参数：	buflen		预分配的空间大小
* 输出参数： 
* 返回值  ：	word_segment			成功
*				NULL					失败
*/
WORD_SEGMENT*  InitWordSeg(int buflen);

/**
* 函数介绍：	释放一个分词结果对象
* 输入参数：	word_segment		分词结果对象
* 输出参数： 
* 返回值  ：	0			成功
*				<0			失败
*/
int FreeWordSeg(WORD_SEGMENT *word_segment);

/**
* 函数介绍：	利用词典分析文本串，根据需要得到分词结果数据结构（可以包括分词接口结果、分词结果列表）
* 输入参数：	lexicon		词典库描述信息，指针

				text		被分析的文本，'\0'结束的字符串
				
				textfmt	文本的格式
				目前输入文本可以是"GBK"或者"UNICODE"编码
				爱问分词系统不支持BIG5 编码。也不支持其它编码。
				
				8字节的flags (unsigned long long)
				0xFFFF 　　　　　 FFFF 　　　　　　FFFF 　　　　　　　FFFF
				lexicon.h选项     应用层选项       词性标注选项　　   分词选项
				
				分词参数选项请见BaseDef.h               (#define SEG_OPT_ALL 0x7fff)
				词性标注参数选项请见POSTagger_new.h     (#define POSTAGGER_OPT_ALL 0x7fff)
				应用层参数选项请见phraseterm.h          如果领域分类，应用层参数可以设置为　0x0003
				
				lexicon.h参数选项
				0x0001 分词
				0x0002 分词 + 词性标注
				0x0004 分词 + 词性标注 + 应用层
				0x0008 分词 + 应用层
				
				0x0100 对存在结构信息的词生成细粒度分词结果
				0x0010 对生成的细粒度分词结果 给出wordcat posid等信息(缺省情况下，没有这些信息)
				
				只有在lexicon.h参数选项中包含了某一模块时，该模块对应的参数才起作用
				
				为了简单起见，当低６字节为 空 时，就是默认值．只需要设定最高的二字节即可
								
* 输出参数： 	word_segment	保存分词结果的数据对象，需要预先初始化
*　　　　　　　docprop　　　输出该文档的属性信息（领域　时效性）
* 返回值  ：	0	成功
*				         <0	失败
*/
//第一个函数接受的输入为字符串，不支持unicode;该函数是为了兼容老版本
int AnalyTextWithLex(LEXICON *lexicon, const char *text, char *textfmt, WORD_SEGMENT* word_segment, unsigned long long flags);

//第二个函数接受的输入为字符串或unicode串
//如果是UNICODE串，调用者转换为char*，长度为字节长度
//以后就维护该函数
//获取文本分析以后的领域分类，看phraseterm.h

//textsrcid 
//0-未知 1-新闻 2-博客 3-知识人 4-论坛 5-广告
int AnalyTextWithLex2(LEXICON *lexicon, const char *text, int iByteLen, char *textfmt, WORD_SEGMENT* word_segment,
 unsigned long long flags, unsigned char textsrcid);

//对文本进行分词/短语合并/并进行文本类型的判定(是否是招聘类型页面，是否是公司简介页面)
//例如 用于招聘搜索中页面的判定
//2007.11.18 sunjian@staff.sina.com.cn
//获取文本分析以后的页面类型，看classifiy.h
//unsigned long long flags暂时没有用，为以后预留
//不记录分词结果
//* 返回值  ：	0	     成功
//*				     <0	失败

#define PAGE_CLASS_NUM 10
//result[0] - 职位招聘 判别结果 
//result[1] - 公司简介 判别结果
int AnalyTextAndClassify(LEXICON *lexicon, 
										const char *text,
										int iByteLen,
										char *textfmt,
										WORD_SEGMENT* word_segment,
										unsigned long long flags,
										PAGE_CLASS_RESULT result[]);

//对query进行切分 //2006.12.31 sunjian@staff.sina.com.cn
int AnalyQueryWithLex2(LEXICON *lexicon, const char *text, int iByteLen, char *textfmt, WORD_SEGMENT* word_segment, unsigned long long flags);


/**
* 函数介绍：	从词典中修改指定词的stat
* 输入参数：	lexicon		词典库描述信息，指针
*				wordID		词ID
*				stat		词状态
* 输出参数： 	
* 返回值  ：	0			成功
*				1			词不存在
*				<0			失败
*/
int setWordStat(LEXICON *lexicon, unsigned long long wordID, BYTE stat);

/**
* 函数介绍：	从词典中修改指定词的type
* 输入参数：	lexicon		词典库描述信息，指针
*				wordID		词ID
*				type		词类型
* 输出参数： 	
* 返回值  ：	0			成功
*				1			词不存在
*				<0			失败
*/
int setWordType(LEXICON *lexicon, unsigned long long wordID, BYTE type);

/**
* 函数介绍：	从词典中修改指定词的IDF
* 输入参数：	lexicon		词典库描述信息，指针
*				wordID		词ID
*				IDF			词的IDF值
* 输出参数： 	
* 返回值  ：	0			成功
*				1			词不存在
*				<0			失败
*/
int setWordIDF(LEXICON *lexicon, unsigned long long wordID, WORD IDF);

/**
* 函数介绍：	从词典中查找一个词是否存在，如果存在，返回词ID
* 输入参数：	lexicon		词典库描述信息，指针
*				      word		词
* 输出参数： 	wordID		词ID，当词不存在时为随机值
* 返回值  ：	0			词存在
*				1			词不存在
*				<0			失败
*/
int getWordID(LEXICON *lexicon, char *word, unsigned long long *wordID);

/**
* 函数介绍：	从词典中查找一个词是否存在，如果存在，返回词ID
* 输入参数：	lexicon		词典库描述信息，指针
*				word		词
*				wordlen		词长
* 输出参数： 	wordID		词ID，当词不存在时为随机值
* 返回值  ：	0			词存在
*				1			词不存在
*				<0			失败
*/
int getWordID_U(LEXICON *lexicon, unsigned short *word, int wordlen, unsigned long long *wordID);


/**
* 函数介绍：	从词典中查找一个wordID的汉字词是否存在，如果存在，返回词串内容
* 输入参数：	lexicon		词典库描述信息，指针
*				wordID		词ID
* 输出参数： 	word		词串内容，由调用者提供足够大的空间，返回字符串以'\0'结束
* 返回值  ：	0			该ID词不存在
*				>0			该ID词GBK长度
*				<0			失败
*/
int getWordStr(LEXICON *lexicon, unsigned long long wordID, char *word);

/**
* 函数介绍：	从词典中取指定ID的词的IDF值
* 输入参数：	lexicon		词典库描述信息，指针
*				wordID		词ID
* 输出参数： 	
* 返回值  ：	>=0			IDF值
*				<0			失败
*/
int getWordIDF(LEXICON *lexicon, unsigned long long wordID);

/**
* 函数介绍：	从词典中取指定ID的词的stat值
* 输入参数：	lexicon		词典库描述信息，指针
*				wordID		词ID
* 输出参数： 	
* 返回值  ：	>=0			stat值
*				<0			失败
*/
int getWordStat(LEXICON *lexicon, unsigned long long wordID);

/**
* 函数介绍：	从词典中取指定ID的词的type值
* 输入参数：	lexicon		词典库描述信息，指针
*				wordID		词ID
* 输出参数： 	
* 返回值  ：	>=0			type值
*				<0			失败
*/
int getWordType(LEXICON *lexicon, unsigned long long wordID);


/**2006/06/03
* 函数介绍：	根据词性id，得到词性对应的标记
* 返回值  ：	0		成功
*						<0		失败
*/
int getPOSTag(LEXICON *lexicon, unsigned char posid, char *postag);

/**2007/12/09
* 函数介绍：	得到该term
* 返回值  ：	>=0			成功
*							<0			失败
*             TermInfo_touser空间由用户提供
*/
int getTermInfo(LEXICON *lexicon, unsigned short *word, int wordlen, TermInfo_touser *pTerm);

#ifdef __cplusplus
}
#endif

#define MAX_LEXICON_NUM 10					//系统可以使用的辞典数目

#ifndef MAX_WORDSEGMENT_NUM
#define MAX_WORDSEGMENT_NUM 1			//分词库最大个数
#endif

#ifndef MAX_WORDLIB_NUM
#define MAX_WORDLIB_NUM 8				//一个分词库中最大载入词库个数
#endif

#ifndef MAX_IWORDSEGMENT
#define MAX_IWORDSEGMENT 1			//最大可用分词接口个数，即线程数
#endif

#ifndef MAX_WORDNUM
#define MAX_WORDNUM 16777216			//一个分词库中最大词数
#endif

#ifndef MAX_WORD_ID
#define MAX_WORD_ID 16777215			//词典中的最大wordID
#endif

#ifndef STATIC_DIC_MAX_WORD_ID
#define STATIC_DIC_MAX_WORD_ID 2621439	//词库中的词条分为两个区:[0~2621440); [2621440~16777216)
#endif


#ifdef __cplusplus

/* 分词系统单一字体分析数据的数据结构定义 */
typedef struct {
	int exist;					//是否存在，0--不存在，1--存在
	char fn_libgpws[100];		//分词接口函数库
	HINSTANCE hWS;				//DLL句柄
	ENDWORDSEGMENT EndWordSegment_d;
	COINITWORDSEGMENT CoInitWordSegment_d;
	
	CREATEWORDLIBINSTANCE CreateWordLibInstance_d;
	CREATEWORDSEGMENTINSTANCE CreateWordSegmentInstance_d;
	CREATEPOSTAGGERINSTANCE   CreatePOSTaggerInstance_d;
	CREATEPHRASETERMINSTANCE  CreatePhraseTermInstance_d;
	CREATEIASKEXKEYWORDSINSTANCE  CreateKeywordsInstance_d;

	CREATECLASSIFYKNNINSTANCE CreateClassifyKnnInstance_d;
	//CREATECLASSIFYDOMAININSTANCE  CreateClassifydomainInstance_d;	
	CREATECLASSIFYINSTANCE  CreateClassifierInstance_d;
	
	RELEASEWORDLIB ReleaseWordLib_d;
	RELEASESEGMENT ReleaseSegment_d;
	RELEASEPOSTAGGER ReleasePOSTagger_d;
	RELEASEPHRASETERM ReleasePhraseTerm_d;
	RELEASEIASKEXKEYWORDS ReleaseKeywords_d;
	//RELEASECLASSIFYDOMAIN ReleaseClassifydomain_d;
	RELEASECLASSIFYKNN ReleaseClassifyKnn_d;
	RELEASECLASSIFY ReleaseClassifier_d;
	
	char dir_segdata[100];	//词库位置
	int wordlibnum;				//词库个数
	
	IWordLib* wl[MAX_WORDLIB_NUM];	//词库列表
	IWordSegment* ws[MAX_IWORDSEGMENT];	//分词接口
	IPOSTagger* postagger[MAX_IWORDSEGMENT];	//词性标注接口
	IPhraseTerm* chunker[MAX_IWORDSEGMENT];	//短语合并接口
	IKeywords* keyworder[MAX_IWORDSEGMENT];	//关键词提取接口
	Classify_KNN *pknnclassifier[MAX_IWORDSEGMENT];

	//ClassifyDomain* classifydomainer[MAX_IWORDSEGMENT];	//网页领域分类接口
	Classify* classifier[MAX_IWORDSEGMENT];	//网页分类的接口
	
	int ws_stat[MAX_IWORDSEGMENT];		//分词接口状态 0:表示没有被占用；１表示已经被占用
	
	WORD_PROPERTY *wordprop;		//词属性
	
	IWordLib* lexicon_level_wordLib;
	//20070327 用于屏蔽分词结果的id，所以在这一层维护了一个词->wordid的映射, 这个wordid就用于索引中
	
} WORD_SEGMENT_LIB;

/* 词典描述信息数据部分数据结构定义 */
typedef struct {
	int wordsegment_num;			//分词系统语种数目
	WORD_SEGMENT_LIB wordsegment[MAX_WORDSEGMENT_NUM];	//简体（0）和繁体（1）分析系统数据
}LEXICON_DATA;

extern LEXICON_DATA *gp_lexdata[MAX_LEXICON_NUM];		//词典数据指针

#endif //end __cplusplus

#endif //end LEXICON_H
