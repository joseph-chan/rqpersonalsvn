#ifndef _IASK_NLP_CLASSIFIER_H
#define _IASK_NLP_CLASSIFIER_H

#include <stdio.h>
#include "docprop.h"

#define CLASSIFY_USING_DOMAIN 0x1000     //ʹ�õ÷����� ֪ʶ���ͶƱ��
#define CLASSIFY_USING_KNN 0x2000        //knn������
#define CLASSIFY_USING_MAXENT 0X4000	 //maxent classifier

#define CLASSIFY_LOAD_DOMAIN 0X01
#define CLASSIFY_LOAD_KNN 0X02
#define CLASSIFY_LOAD_MAXENT 0X04

#ifdef __cplusplus

class Classifier
{
public:

	/*
	 *	˵��:�����������ʼ��������ײ��ָ��
	 */
	virtual int InitClsResource(const char *path) = 0;
	
	virtual int GetDocPropInfo(const char *src, const char seg, DocInfo *docinfo, unsigned char pagetype) = 0;
	virtual int GetDocPropInfo(DocFeatVec *docfeat, DocInfo *docinfo, unsigned char pagetype) = 0;
};
#endif

#endif
