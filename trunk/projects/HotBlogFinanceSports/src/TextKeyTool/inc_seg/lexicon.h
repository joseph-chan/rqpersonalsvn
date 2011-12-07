/*
* Copyright (c) 2004, �������з�����
* All rights reserved.
* 
* �ļ����ƣ�lexicon.h
* ժ    Ҫ�����ڴʵ�����ݺ͹��ܶ���
* ��    �ߣ����л� 2004/07/20
*           �｡ 2006.04.12
* ��    ����1.0
* $Id: lexicon.h,v 1.3 2005/04/04 10:26:46 zhonghua Exp $
*/


#ifndef LEXICON_H
#define LEXICON_H

#include "searchdef.h"

#include "BaseDef.h"
#include "WordSegment.h" //�ִ�
#include "POSTagger_new.h" //���Ա�ע
#include "phraseterm.h"  //����ϲ�
#include "ExKeywordsInterface.h" //�ؼ�����ȡ
//#include "classifydomain.h"  //�ı�����1

#include "classify.h" //�ı�����2
#include "classifyknn.h" //�ı�����2

#include "dllcall.h"
#include "wordprop.h"

/* �ʵ�ʿ��������Ϣ���ݽṹ���� */
typedef struct 
{
	BYTE type;					//����� ȱʡΪ0
	BYTE stat;					//���״̬ ȱʡΪ0
	WORD IDF;					  //������� ȱʡΪ0
}WORD_PROPERTY;


/* �ʵ�������Ϣ���ݽṹ���� */
typedef struct {
	char rdir[256];				//��Ŀ¼��ȱʡΪ0
	char fn_config[100];		//�ʵ������ļ�

	int lexdata;				//��Ӧ�Ĵʵ����ݱ��
	int ws_sel;					//�ʵ䵱ǰѡ�������
}LEXICON;


/* �ִʽ�����ݽṹ���� */
#define DOMAIN_SEMANTIC 1
typedef struct {
	unsigned int wordPos;		    // Position in source string, ��unicode���е�λ��
	unsigned long long wordID;	// WORDID
	
	unsigned char wordLen;	    // Length of this word��      unicode�ĳ���
	unsigned char postagid;     // �ôʵĴ���id�����POSTagger_new.h
	unsigned short wordCat;		  // �ӷִ�ϵͳ��������������ԣ�Attribute of word item, such as personal name, corporation name, and so on.
		
	WORD_PROPERTY wordprop;		  // word property
	
	unsigned short childnodes; //2007.7.13 sunjian
											 //���ֶ�ֻ���ڻ�����ʱ��Ч�����������ʱ���У�
											 //16λ��Ϊ4�� 1111/1111/1111/1111
											 //���������ΪAD�ͣ���ôȡ���8λ��ÿ4λ�ֱ��Ӧa��d
											 //���������ΪABD�ͣ���ôȡ���12λ��ÿ4λ�ֱ��Ӧa b d
											 //���������ΪACD�ͣ���ôȡ���12λ��ÿ4λ�ֱ��Ӧa c d
											 //���������ΪABCD�ͣ���ôȡ16λ��ÿ4λ�ֱ��Ӧa b c d
											 //a b c d�����ֵΪ16
											 //a b c d��ֵ�ֱ��ʾ��wordlist�����Ӧ��WORD_ITEM�ڵ��������һ��WORD_ITEM��ƫ��
											 
											 //�磺�����й�����˾Ϊ ACD���͵Ļ�����
											 
											 // 0                                          1                        2           3             4               5
											 //�����й�����˾(0,7) ������(0,3)  ����(0,2) ��(2,1) ����(3,2) ��˾(5,2)
											 //64                                      192                 128            128         128          128
											 
											 //�����й�����˾(0,7)��3�����ӷֱ�A=������(0,3), C=����(3,2), D=��˾(5,2)
											 //a =1-0=1, c=4-1=3, d=5-4=1
											 
											 //��ˣ�"�����й�����˾"��childnodes = a<<12 | c<<8 | d<<4

#ifdef DOMAIN_SEMANTIC //����ִʽ���ڵ���������Ժ��������� 20070310 
   unsigned char domainnum; //��term�м�������
   unsigned short domainid[TERM_MAX_DOMAIN_NUM]; //��������
	
   unsigned char semanticnum; //��term�м�����������
   unsigned short semanticid[TERM_MAX_SEMANTIC_NUM]; //��������
#endif

   unsigned char wordvalue;
   //����query����ʱ��, wordvalue��˵��:
   //��4λ��ʾ�ôʵ�Ԫ������ ��4λ��ʾ�������дʵ�Ȩ��
   // (0000 00) ���ʴ�  ʮ���Ʒ�Χ(0)
   // (0001 00**) �ؼ����� ʮ���Ʒ�Χ(4-7) 
   // (0010 00**) �ؼ����� ʮ���Ʒ�Χ(8-11)
   // (0011 00**) �����ؼ��� ʮ���Ʒ�Χ12-15)
   // (0100 00**) ������ ʮ���Ʒ�Χ(16-19)
   // (0101 00**) ͣ�ô� ʮ���Ʒ�Χ(20-23)
   //�����ı�����ʱ��wordvalue�ֶε�ֵ������� 20070418
  
   unsigned char idf;          //idf * 10, ȡֵ��Χ��0-255���õ�ʱ����Ҫ�ٳ���10 
   //unsigned long long reserved;

}WORD_ITEM;

typedef struct {
	WORD_ITEM word_item;
	unsigned int appwordPos;		  //��ʼ��wordlist����ĵڼ����ʣ������ǵڼ����ַ�
	//unsigned short appwordnum;	 //Length:  �����ķִʽ���Ĵʸ���.���ֶ���ʱһֱ=0
	
} APP_WORD_ITEM;//Ӧ�ò��term

#define LIST_MAX_CHILD_NUM 10

typedef struct{
	WORD_ITEM *word;
	unsigned int childnum;
	unsigned int children[LIST_MAX_CHILD_NUM];//�ӽ��Ĵ���
}LIST_ITEM;


/* �ִ������������ݽṹ���� */
typedef struct {
	int buflen;					//�������ռ��С
	char textfmt[10];		//����ִʵ����ָ�ʽ��"GBK","UNICODE"
	
	int len_uni;				//Unicode��ʽ�����ֳ���
	unsigned short* uni;//Unicode��ʽ����������
	
	int word_num;				//�õ��ķִʽ������
	WORD_ITEM *wordlist;		//�ִʽ���б�
	
	int appword_num;				//�õ���Ӧ�ò� term����
	APP_WORD_ITEM *appwordlist;		//Ӧ�ò� term�б�
	
	int listitem_num;
	LIST_ITEM *list;

	DocInfo  docprop;    //Ӧ�ò�����󡡵õ��Ķ���ƪ�ĵ�������������Ϣ
	KeyWordStore keywords; //Ӧ�ò�����󡡵õ��Ķ���ƪ�ĵ��Ĺؼ��ʣ����� ���� �¼� רָ
	
}WORD_SEGMENT;


#ifdef __cplusplus
extern "C" {
#endif
/*	�Դʵ��������غ��� */

/**
* �������ܣ�	��һ���ʵ��
* ���������	rdir		�ʵ���Ŀ¼
* ��������� 
* ����ֵ  ��	lexicon		�ʵ��������Ϣ��NULL��ʾ��ʧ��
*/
LEXICON *OpenLexicon(const char *rdir);

/**
* �������ܣ�	��һ���ʵ��
* ���������	rdir		�ʵ���Ŀ¼����ѡ���load��Դ
* ��������� 
* ����ֵ  ��	lexicon		�ʵ��������Ϣ��NULL��ʾ��ʧ��
* option      ����load��Щ��Դ
*                 16λ
*                       ����   �ؼ�����ȡ   ����ϲ�   ���Ա�ע   �ִ�
* 00 000 000 00         1          1             1         1       1
* 
* ע������:
* Load��Դ�Ĳ���ѡ����Analyze�Ĳ���ѡ������Զ����ġ��������ֻload�˷ִʵ���Դ��Ȼ��Ҫ��ȷִ��ִ��Ա�עʱ��������
* 
*/
LEXICON *OpenLexicon_Opt(const char *rdir, unsigned short option);

/**
* �������ܣ�	����ʵ����������
* ���������	lexicon		�ʵ��������Ϣ��ָ��
* ��������� 
* ����ֵ  ��	0			�ɹ�
*				<0			ʧ��
*/
int SaveLexicon(LEXICON *lexicon);

/**
* �������ܣ�	�ر�һ���ʵ��
* ���������	lexicon		�ʵ��������Ϣ��ָ��
* ��������� 
* ����ֵ  ��	0			�ɹ�
*				<0			ʧ��
*/
int CloseLexicon(LEXICON *lexicon);

/**
* �������ܣ�	�Ӵʵ����޸�ָ��ȱʡʹ�õ�����
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				ws_sel		0Ϊ���壬1Ϊ����
* ��������� 	
* ����ֵ  ��	0			�ɹ�
*				<0			ʧ��
*/
int setLexiconWS(LEXICON *lexicon, int ws_sel);

/**
* �������ܣ�	���䲢��ʼ��һ������ִʽ�������ݶ���
* ���������	buflen		Ԥ����Ŀռ��С
* ��������� 
* ����ֵ  ��	word_segment			�ɹ�
*				NULL					ʧ��
*/
WORD_SEGMENT*  InitWordSeg(int buflen);

/**
* �������ܣ�	�ͷ�һ���ִʽ������
* ���������	word_segment		�ִʽ������
* ��������� 
* ����ֵ  ��	0			�ɹ�
*				<0			ʧ��
*/
int FreeWordSeg(WORD_SEGMENT *word_segment);

/**
* �������ܣ�	���ôʵ�����ı�����������Ҫ�õ��ִʽ�����ݽṹ�����԰����ִʽӿڽ�����ִʽ���б�
* ���������	lexicon		�ʵ��������Ϣ��ָ��

				text		���������ı���'\0'�������ַ���
				
				textfmt	�ı��ĸ�ʽ
				Ŀǰ�����ı�������"GBK"����"UNICODE"����
				���ʷִ�ϵͳ��֧��BIG5 ���롣Ҳ��֧���������롣
				
				8�ֽڵ�flags (unsigned long long)
				0xFFFF ���������� FFFF ������������FFFF ��������������FFFF
				lexicon.hѡ��     Ӧ�ò�ѡ��       ���Ա�עѡ���   �ִ�ѡ��
				
				�ִʲ���ѡ�����BaseDef.h               (#define SEG_OPT_ALL 0x7fff)
				���Ա�ע����ѡ�����POSTagger_new.h     (#define POSTAGGER_OPT_ALL 0x7fff)
				Ӧ�ò����ѡ�����phraseterm.h          ���������࣬Ӧ�ò������������Ϊ��0x0003
				
				lexicon.h����ѡ��
				0x0001 �ִ�
				0x0002 �ִ� + ���Ա�ע
				0x0004 �ִ� + ���Ա�ע + Ӧ�ò�
				0x0008 �ִ� + Ӧ�ò�
				
				0x0100 �Դ��ڽṹ��Ϣ�Ĵ�����ϸ���ȷִʽ��
				0x0010 �����ɵ�ϸ���ȷִʽ�� ����wordcat posid����Ϣ(ȱʡ����£�û����Щ��Ϣ)
				
				ֻ����lexicon.h����ѡ���а�����ĳһģ��ʱ����ģ���Ӧ�Ĳ�����������
				
				Ϊ�˼���������ͣ��ֽ�Ϊ �� ʱ������Ĭ��ֵ��ֻ��Ҫ�趨��ߵĶ��ֽڼ���
								
* ��������� 	word_segment	����ִʽ�������ݶ�����ҪԤ�ȳ�ʼ��
*��������������docprop������������ĵ���������Ϣ������ʱЧ�ԣ�
* ����ֵ  ��	0	�ɹ�
*				         <0	ʧ��
*/
//��һ���������ܵ�����Ϊ�ַ�������֧��unicode;�ú�����Ϊ�˼����ϰ汾
int AnalyTextWithLex(LEXICON *lexicon, const char *text, char *textfmt, WORD_SEGMENT* word_segment, unsigned long long flags);

//�ڶ����������ܵ�����Ϊ�ַ�����unicode��
//�����UNICODE����������ת��Ϊchar*������Ϊ�ֽڳ���
//�Ժ��ά���ú���
//��ȡ�ı������Ժ��������࣬��phraseterm.h

//textsrcid 
//0-δ֪ 1-���� 2-���� 3-֪ʶ�� 4-��̳ 5-���
int AnalyTextWithLex2(LEXICON *lexicon, const char *text, int iByteLen, char *textfmt, WORD_SEGMENT* word_segment, unsigned long long flags, unsigned char textsrcid);

//�˺�����������������ֱ��ʹ��K�������������
//texttype 0-���������� 1-��������
//classifier "KNN" "SVM" "ME"
//vec "feat:val\tfeat:val\t ... feat:val";
//int ClsVecWithModel(LEXICON *lexicon, const char *vec, int veclen, int texttype, const char *classifier, DocInfo *docprop);

//���ı����зִ�/����ϲ�/�������ı����͵��ж�(�Ƿ�����Ƹ����ҳ�棬�Ƿ��ǹ�˾���ҳ��)
//���� ������Ƹ������ҳ����ж�
//2007.11.18 sunjian@staff.sina.com.cn
//��ȡ�ı������Ժ��ҳ�����ͣ���classifiy.h
//unsigned long long flags��ʱû���ã�Ϊ�Ժ�Ԥ��
//����¼�ִʽ��
//* ����ֵ  ��	0	     �ɹ�
//*				     <0	ʧ��

#define PAGE_CLASS_NUM 10
//result[0] - ְλ��Ƹ �б��� 
//result[1] - ��˾��� �б���
int AnalyTextAndClassify(LEXICON *lexicon, 
										const char *text,
										int iByteLen,
										char *textfmt,
										WORD_SEGMENT* word_segment,
										unsigned long long flags,
										PAGE_CLASS_RESULT result[]);

//��query�����з� //2006.12.31 sunjian@staff.sina.com.cn
int AnalyQueryWithLex2(LEXICON *lexicon, const char *text, int iByteLen, char *textfmt, WORD_SEGMENT* word_segment, unsigned long long flags);


/**
* �������ܣ�	�Ӵʵ����޸�ָ���ʵ�stat
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				wordID		��ID
*				stat		��״̬
* ��������� 	
* ����ֵ  ��	0			�ɹ�
*				1			�ʲ�����
*				<0			ʧ��
*/
int setWordStat(LEXICON *lexicon, unsigned long long wordID, BYTE stat);

/**
* �������ܣ�	�Ӵʵ����޸�ָ���ʵ�type
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				wordID		��ID
*				type		������
* ��������� 	
* ����ֵ  ��	0			�ɹ�
*				1			�ʲ�����
*				<0			ʧ��
*/
int setWordType(LEXICON *lexicon, unsigned long long wordID, BYTE type);

/**
* �������ܣ�	�Ӵʵ����޸�ָ���ʵ�IDF
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				wordID		��ID
*				IDF			�ʵ�IDFֵ
* ��������� 	
* ����ֵ  ��	0			�ɹ�
*				1			�ʲ�����
*				<0			ʧ��
*/
int setWordIDF(LEXICON *lexicon, unsigned long long wordID, WORD IDF);

/**
* �������ܣ�	�Ӵʵ��в���һ�����Ƿ���ڣ�������ڣ����ش�ID
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				      word		��
* ��������� 	wordID		��ID�����ʲ�����ʱΪ���ֵ
* ����ֵ  ��	0			�ʴ���
*				1			�ʲ�����
*				<0			ʧ��
*/
int getWordID(LEXICON *lexicon, char *word, unsigned long long *wordID);

/**
* �������ܣ�	�Ӵʵ��в���һ�����Ƿ���ڣ�������ڣ����ش�ID
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				word		��
*				wordlen		�ʳ�
* ��������� 	wordID		��ID�����ʲ�����ʱΪ���ֵ
* ����ֵ  ��	0			�ʴ���
*				1			�ʲ�����
*				<0			ʧ��
*/
int getWordID_U(LEXICON *lexicon, unsigned short *word, int wordlen, unsigned long long *wordID);


/**
* �������ܣ�	�Ӵʵ��в���һ��wordID�ĺ��ִ��Ƿ���ڣ�������ڣ����شʴ�����
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				wordID		��ID
* ��������� 	word		�ʴ����ݣ��ɵ������ṩ�㹻��Ŀռ䣬�����ַ�����'\0'����
* ����ֵ  ��	0			��ID�ʲ�����
*				>0			��ID��GBK����
*				<0			ʧ��
*/
int getWordStr(LEXICON *lexicon, unsigned long long wordID, char *word);

/**
* �������ܣ�	�Ӵʵ���ȡָ��ID�Ĵʵ�IDFֵ
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				wordID		��ID
* ��������� 	
* ����ֵ  ��	>=0			IDFֵ
*				<0			ʧ��
*/
int getWordIDF(LEXICON *lexicon, unsigned long long wordID);

/**
* �������ܣ�	�Ӵʵ���ȡָ��ID�Ĵʵ�statֵ
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				wordID		��ID
* ��������� 	
* ����ֵ  ��	>=0			statֵ
*				<0			ʧ��
*/
int getWordStat(LEXICON *lexicon, unsigned long long wordID);

/**
* �������ܣ�	�Ӵʵ���ȡָ��ID�Ĵʵ�typeֵ
* ���������	lexicon		�ʵ��������Ϣ��ָ��
*				wordID		��ID
* ��������� 	
* ����ֵ  ��	>=0			typeֵ
*				<0			ʧ��
*/
int getWordType(LEXICON *lexicon, unsigned long long wordID);


/**2006/06/03
* �������ܣ�	���ݴ���id���õ����Զ�Ӧ�ı��
* ����ֵ  ��	0		�ɹ�
*						<0		ʧ��
*/
int getPOSTag(LEXICON *lexicon, unsigned char posid, char *postag);

/**2007/12/09
* �������ܣ�	�õ���term
* ����ֵ  ��	>=0			�ɹ�
*							<0			ʧ��
*             TermInfo_touser�ռ����û��ṩ
*/
int getTermInfo(LEXICON *lexicon, unsigned short *word, int wordlen, TermInfo_touser *pTerm);

#ifdef __cplusplus
}
#endif

#define MAX_LEXICON_NUM 10					//ϵͳ����ʹ�õĴǵ���Ŀ

#ifndef MAX_WORDSEGMENT_NUM
#define MAX_WORDSEGMENT_NUM 1			//�ִʿ�������
#endif

#ifndef MAX_WORDLIB_NUM
#define MAX_WORDLIB_NUM 8				//һ���ִʿ����������ʿ����
#endif

#ifndef MAX_IWORDSEGMENT
#define MAX_IWORDSEGMENT 1			//�����÷ִʽӿڸ��������߳���
#endif

#ifndef MAX_WORDNUM
#define MAX_WORDNUM 16777216			//һ���ִʿ���������
#endif

#ifndef MAX_WORD_ID
#define MAX_WORD_ID 16777215			//�ʵ��е����wordID
#endif

#ifndef STATIC_DIC_MAX_WORD_ID
#define STATIC_DIC_MAX_WORD_ID 2621439	//�ʿ��еĴ�����Ϊ������:[0~2621440); [2621440~16777216)
#endif


#ifdef __cplusplus

/* �ִ�ϵͳ��һ����������ݵ����ݽṹ���� */
typedef struct {
	int exist;					//�Ƿ���ڣ�0--�����ڣ�1--����
	char fn_libgpws[100];		//�ִʽӿں�����
	HINSTANCE hWS;				//DLL���
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
	
	char dir_segdata[100];	//�ʿ�λ��
	int wordlibnum;				//�ʿ����
	
	IWordLib* wl[MAX_WORDLIB_NUM];	//�ʿ��б�
	IWordSegment* ws[MAX_IWORDSEGMENT];	//�ִʽӿ�
	IPOSTagger* postagger[MAX_IWORDSEGMENT];	//���Ա�ע�ӿ�
	IPhraseTerm* chunker[MAX_IWORDSEGMENT];	//����ϲ��ӿ�
	IKeywords* keyworder[MAX_IWORDSEGMENT];	//�ؼ�����ȡ�ӿ�
	Classify_KNN *pknnclassifier[MAX_IWORDSEGMENT];

	//ClassifyDomain* classifydomainer[MAX_IWORDSEGMENT];	//��ҳ�������ӿ�
	Classify* classifier[MAX_IWORDSEGMENT];	//��ҳ����Ľӿ�
	
	int ws_stat[MAX_IWORDSEGMENT];		//�ִʽӿ�״̬ 0:��ʾû�б�ռ�ã�����ʾ�Ѿ���ռ��
	
	WORD_PROPERTY *wordprop;		//������
	
	IWordLib* lexicon_level_wordLib;
	//20070327 �������ηִʽ����id����������һ��ά����һ����->wordid��ӳ��, ���wordid������������
	
} WORD_SEGMENT_LIB;

/* �ʵ�������Ϣ���ݲ������ݽṹ���� */
typedef struct {
	int wordsegment_num;			//�ִ�ϵͳ������Ŀ
	WORD_SEGMENT_LIB wordsegment[MAX_WORDSEGMENT_NUM];	//���壨0���ͷ��壨1������ϵͳ����
}LEXICON_DATA;

extern LEXICON_DATA *gp_lexdata[MAX_LEXICON_NUM];		//�ʵ�����ָ��

#endif //end __cplusplus

#endif //end LEXICON_H
