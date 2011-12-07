/*
* Copyright (c) 2005-2008
* All rights reserved.
* 
* �ļ����ƣ�phraseterm.h
* ժ    Ҫ���ִ�֮�ϵ�Ӧ�ò�
* ��    �ߣ��｡ 2006/04/03
* ��    ����1.0
* 
*/

#ifndef _IASK_NLP_PhraseTerm_H
#define _IASK_NLP_PhraseTerm_H

#include "BaseDef.h"
#include "WordSegment.h"
#include "POSTagger_new.h"

#define IN
#define OUT

//�ύ���û���term����Ϣ
typedef struct
{
	//unsigned short concept[CONCEPT_MAX_LEN+1];//��term�Ĵ���
	unsigned int   termid;             //��term��id ��id����������id��ִʽ����idû���κι�ϵ

	unsigned char domainnum; //��term�м�������
	unsigned char semanticnum; //��term�м�����������
	unsigned char postagid; //��Ӧ�ò�Ĵ���id
	unsigned char idf;

	unsigned short domainid[TERM_MAX_DOMAIN_NUM]; //��������
	unsigned short semanticid[TERM_MAX_SEMANTIC_NUM]; //��������

}TermInfo_touser;

//App Option Tags
#define APP_MARK_PHRASETERM 0x000f

#define AOP_TERM_MM  0x0001 //���Ҹ���ķ�ʽ����������з� 
#define AOP_TERM_ALL 0x0002 //���Ҹ���ķ�ʽ��ȫ�з� (ȱʡֵ)
#define AOP_TERM_NOPHRASE 0x0000 //ֻ�ѷִʽ��copy����Ӧ�����ݽṹ����������ĺϲ� 

//���ѡ����ǰ��������ֽڽ��С��򡱲���

#ifdef __cplusplus
class IPhraseTerm {
public:
	
	/**
	* �������ܣ���ʼ��
	* ���������
	* ���������
	* ����ֵ  ��	0		�ɹ�
	*				      <0	ʧ��
	*/
	virtual int InitIPhraseTerm(IWordSegment *pWordSegment, IPOSTagger *pPOSTagger) = 0;
	
	/**
	* �������ܣ�ʶ�����
	* ���������
	* ���������
	* ����ֵ  ��	0		�ɹ�
	*				      <0	ʧ��
	*/
	//�ӿں���1
	//�ı�->WordSegment->PhraseTerm
	//�밮�ʷִ�ϵͳ�ӿ�
	virtual int PhraseTerm(unsigned short* szSrc, unsigned int nLen, _PWordNodeU pHeadNode, unsigned short nOption) = 0;//�ִ�+Ӧ�ò�
	virtual int PhraseTerm(unsigned short* szSrc, unsigned int nLen, _PWordPOSNode pHeadNode, unsigned short nOption) = 0;//�ִ�+���Ա�ע+Ӧ�ò�
	
	/**
	* �������ܣ�Traversing node list method
	* ���������
	* ���������
	* ����ֵ  ��	0		�ɹ�
	*				      <0	ʧ��
	*
	*/	
	virtual int GetListHeadState(int *iNextTime, int *NodeNum) = 0;
	
	/**
	* �������ܣ���ȡ��һ��ʱ�� ��ȡ���ʱ�̵Ķ���������
	* ���������
	* ���������
	* ����ֵ  ��	0		�ɹ�
	*				      <0	ʧ��
	*
	*/
	virtual int GetNextState(int iCurTime, int *iNextTime, int *NodeNum) = 0;
	virtual int GetMaxLenNextState(int iCurTime, int *iNextTime, int *NodeNum) = 0;
	
	/**
	* �������ܣ���ȡ����ʱ�̵�ĳһ������
	* ���������
	* ���������
	* ����ֵ  ��	0		�ɹ�
	*				      <0	ʧ��
	*
	*/	
	virtual _PWordPOSNode GetNode(int iCurTime, int nIndex) = 0;

	/**
	* �������ܣ���ȡ����ʱ�̵�ĳһ������
	* ���������
	* ���������
	* ����ֵ  ��    0               �ɹ�
	*                                     <0        ʧ��
	*
	*/

	virtual _PWordPOSNode GetNextNode(int *NextIdx) = 0;
	virtual _PWordPOSNode GetHeadNode(int *NextIdx) = 0;
	
	/**
	* �������ܣ���ȡ����������Ϣ
	* ���������
	* ���������
	* ����ֵ  ��	0		�ɹ�
	*				      <0	û��
	*
	*/
	virtual int GetNodeInfo(int iCurTime, int nIndex, TermInfo_touser *info) = 0;
	
	/**
	* 2007/12/09
	* �������ܣ�	���ݵ��ʵõ���term��IDF
	* pIDF ��ַ�ռ��ɵ������ṩ
	* ����ֵ  ��	0		�ɹ�
	*						  <0	ʧ��
	*/
	virtual int GetTermIDF(unsigned short *word, int wordlen,  unsigned char *pIDF) = 0;
	
	/**
	* 2007/12/09
	* �������ܣ�	
	* pTerm ��ַ�ռ��ɵ������ṩ
	* ����ֵ  ��	0		�ɹ�
	*							<0	ʧ�ܻ򲻴���
	*/
	virtual int GetTerm(unsigned short *word, int wordlen,  TermInfo_touser *pTerm) = 0;
	
};

// Create IPhraseTerm instance, return pointer to the interface of it(Call Release to destroy it):
extern "C"  IPhraseTerm* CreatePhraseTermInstance(void);
typedef IPhraseTerm* (* CREATEPHRASETERMINSTANCE)(void);

//Call it to release IPhraseTerm instance
extern "C"  int ReleasePhraseTerm(IPhraseTerm *);
typedef int (*RELEASEPHRASETERM)(IPhraseTerm*);

#endif //end __cplusplus

#endif
