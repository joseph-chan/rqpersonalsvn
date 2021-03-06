#ifndef _IASK_NLP_DOCPROP_H
#define _IASK_NLP_DOCPROP_H

#include <stdio.h>

//文本向量的属性
#define MAX_FEATWORDS_NUM 64
#define FEAT_WORD_LEN 17

//应用层分析得到的关于该文档的属性
#define DOC_MAX_DOMAIN_NUM  3  //一个文档最多可能属于多少个领域
#define DOC_DEFAULT_DOMAIN_NUM  2  //缺省情况下取32隽煊�

typedef struct {		
	  unsigned short domainid;
	  char domain[128];
	  float weight;  //属于该domain的权重
	  float trust;
} DomainInfo;
 
typedef struct
{	
	unsigned char domainnum; //该文档属于几个领域(第二层)
	DomainInfo domains[DOC_MAX_DOMAIN_NUM]; //该文档的领域(第二层)	
	DomainInfo domain_level1;//第一层是属于哪个领域
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
