/*
* Copyright (c) 2005-2008
* All rights reserved.
* 
* �ļ����ƣ�classifydomain.h
* ժ    Ҫ���ִ�֮�ϵ�Ӧ�ò�
* ��    �ߣ��｡ 2006/04/03
* ��    ����1.0
* 
*/

#ifndef _IASK_NLP_Domain_H
#define _IASK_NLP_Domain_H

#include "classifier.h"
#include "ExKeywordsInterface.h"
#include <stdio.h>

#define IN
#define OUT

#define CONCEPT_MAX_LEN 20

#define MAX_VOTE_NUM 100
//#define SUN_DEBUG_VOTE 1

// ****** tagoption˵�� ********** //
//unsigned short��9-16λ���ڷ��� 
//�����λ���ڷ�����ѡ�񣬵���λ���ڷ����ı���ѡ��
 
//knn��������������
#define TAG_KNN_ADSENSE 0
#define TAG_KNN_NEWS 1
#define TAG_KNN_BLOG 2

#ifdef __cplusplus

class ClassifyDomain
{
public:
	virtual int InitClsResource(IWordSegment *pWordSegment, IPOSTagger *pPOSTagger, IPhraseTerm *pPhraseTerm, IKeywords *pKeywords) = 0;

	/*ʹ�÷���: ���ݵĲ����option����������3������
	 * ����ϲ�����:AOP_PHRASETERM_XX û�оͲ�������ϲ�
	 * 
	 * featvec �ı����������������� seg �ָ�����GetDocFeat�õ��˹ؼ��ʴ�
	 * ��featvec��NULL, seg��������
	 */
	virtual int GetDocPropInfo(DocInfo *docinfo, unsigned char pagetype) = 0;
	//virtual int GetDocFeat(unsigned short *srz, unsigned int nlen, unsigned char pagetype, unsigned short Option) = 0;
};

extern "C" ClassifyDomain *CreateClassifyDomainInstance(void);
typedef ClassifyDomain* (*CREATECLASSIFYDOMAININSTANCE)(void);

extern "C" int ReleaseClassifyDomain(ClassifyDomain *);
typedef int(*RELEASECLASSIFYDOMAIN)(ClassifyDomain *);

#endif //end __cplusplus
#endif
