/* Copyright (c) 2005-2008
* All rights reserved.
* 
* �ļ����ƣ�
* ժ    Ҫ���ؼ�����ȡ
* ��    �ߣ��｡ 2006/11/13
* ��    ����1.0
* 
*/

#ifndef _IASK_NLP_KeyInterface_H
#define _IASK_NLP_KeyInterface_H

#include "BaseDef.h"
#include "WordSegment.h"
#include "POSTagger_new.h"
#include "phraseterm.h"
#include "docprop.h"

#define DEFAULT_KEYWORDS_NUM 5
#define AOP_EXTRACTKEYWORDS_DEFAULT 0x0010
#define AOP_EXTRACTKEYWORDS_SORT_BY_IDF 0x0020
#define AOP_EXTRACTKEYWORDS_ADV 0x0030
#define MAX_KEYWORDS_NUM 20

//�����ı���һ���ؼ��ʵ�����
typedef struct
{				
	unsigned short key[WORD_MAX_LEN_SHORT+1];

	unsigned int wordPos; //���ı���һ�γ��ֵ�λ�� ���ǲ��ٶ�ÿһ���ؼ��ʶ����ı��еĴʣ���˿���Ϊ0xFFFFFFFF
	unsigned short len;
	char *pTerm;				//����Term��Ϣ

	float fWeight;
	unsigned short tf;
	unsigned short tf_title;//�ڱ����г���Ƶ��
	unsigned short tf_context;//�������г���Ƶ��
	float idf;

	unsigned short wordCat; // �ӷִ�ϵͳ��������������ԣ�Attribute of word item, such as personal name, corporation name, and so on
	unsigned char postagid; // �ôʵĴ���id�����POSTagger_new.h
	char postag[6];
} KeywordItem;

typedef struct{
	unsigned short key_num;
	unsigned short name_num;
	unsigned short locate_num;
	unsigned short event_num;
	unsigned short n_z_num;
	KeywordItem KeyWords[MAX_KEYWORDS_NUM];
	KeywordItem NameList[MAX_KEYWORDS_NUM];
	KeywordItem LocateList[MAX_KEYWORDS_NUM];
	KeywordItem EventList[MAX_KEYWORDS_NUM];
	KeywordItem N_ZList[MAX_KEYWORDS_NUM];
	struct DocFeatVec DocFeat;
}KeyWordStore;

#ifdef __cplusplus
class IKeywords {
	public:

		/**
		 * �������ܣ���ʼ��
		 * ���������
		 * ���������
		 * ����ֵ  ��	0		�ɹ�
		 *				      <0	ʧ��
		 */
		virtual int InitKeywords(IWordSegment *pWordSegment, IPOSTagger *pPOSTagger, IPhraseTerm *pPhraseTerm) = 0;

		/**
		 * �������ܣ���ȡwebpage�Ĺؼ���
		 * ��������� KeyWordStore *keystore ,��Ҫ�û��ṩ�洢�ռ� ����Ϊ��
		 * ���������
		 * ����ֵ  ��	0		�ɹ�
		 *				      <0	ʧ��
		 */		
		virtual int ExtractKeywords(unsigned short* szSrc, unsigned int nLen, KeyWordStore *keystore, unsigned short nOption) = 0;

		virtual KeywordItem * GetNodeListHead() = 0;

		virtual KeywordItem * GetNextNode(KeywordItem *pCurKeyword) = 0;

		virtual  KeywordItem * GetPNNodeListHead()=0;//����
		virtual  KeywordItem * GetPNNextNode(KeywordItem *pCurKeyword)=0;

		virtual  KeywordItem * GetLNNodeListHead()=0;//�ص�
		virtual  KeywordItem * GetLNNextNode(KeywordItem *pCurKeyword)=0;

		virtual  KeywordItem * GetTimeNodeListHead()=0;//ʱ��
		virtual  KeywordItem * GetTimeNextNode(KeywordItem *pCurKeyword)=0;

		virtual  KeywordItem * GetEventNodeListHead()=0;//�¼�
		virtual  KeywordItem * GetEventNextNode(KeywordItem *pCurKeyword)=0;

};

extern "C"  IKeywords* CreateIAskExKeywordsInstance(void);
typedef IKeywords* (* CREATEIASKEXKEYWORDSINSTANCE)(void);

extern "C"  int ReleaseIAskExKeywords(IKeywords *);
typedef int (*RELEASEIASKEXKEYWORDS)(IKeywords*);

#endif //end __cplusplus

#endif
