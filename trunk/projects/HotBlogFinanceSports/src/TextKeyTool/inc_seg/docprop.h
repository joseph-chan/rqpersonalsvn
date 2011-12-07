#ifndef _IASK_NLP_DOCPROP_H
#define _IASK_NLP_DOCPROP_H

#include <stdio.h>

//�ı�����������
#define MAX_FEATWORDS_NUM 64
#define FEAT_WORD_LEN 17

//Ӧ�ò�����õ��Ĺ��ڸ��ĵ�������
#define DOC_MAX_DOMAIN_NUM  3  //һ���ĵ����������ڶ��ٸ�����
#define DOC_DEFAULT_DOMAIN_NUM  2  //ȱʡ�����ȡ32�����

typedef struct {		
	  unsigned short domainid;
	  char domain[128];
	  float weight;  //���ڸ�domain��Ȩ��
	  float trust;
} DomainInfo;
 
typedef struct
{	
	unsigned char domainnum; //���ĵ����ڼ�������(�ڶ���)
	DomainInfo domains[DOC_MAX_DOMAIN_NUM]; //���ĵ�������(�ڶ���)	
	DomainInfo domain_level1;//��һ���������ĸ�����
} DocInfo;

typedef struct
{
	char featword[FEAT_WORD_LEN];
	float weight;
	unsigned char postagid;
} FeatItem;

struct DocFeatVec
{
	unsigned short feat_num;
	FeatItem featitem[MAX_FEATWORDS_NUM];
};

#endif
