#ifndef _PARSE_JSON_H_
#define _PARSE_JSON_H_

/*
	json文本的解析器和生产器
	
	采用数组和链表数据结构表达json中的dict,array, string int ..
	
	解析语法基于 http://json.org/json-zh.html 中的定义。
		object采用	"PairNode_t+链表" 表示。
		array 采用 "ValueNode_t+链表" 表示。
	
*/

#ifdef __cplusplus
extern "C"
{
#endif

//基本类型
typedef enum{V_STR,V_VALUE_STR,V_VALUE_ARRAY,V_PAIR_ARRAY} VALUE_TYPE;

//值信息
typedef struct{
	VALUE_TYPE v_type;
	union{
		char *pStr;
		int startPos;//-1，表示空值，空值数组
	};
	int nextPos;//>0，表示存在后继
}ValueNode_t;

//值对信息
typedef struct{
	char *keyStr;
	VALUE_TYPE v_type;
	union{
		char *pStr;
		int startPos;//-1，表示空值，空值数组
	};
	int nextPos;//>0，表示存在后继
}PairNode_t;

//最大分析层数
#define MAX_TREE_DEEP_LEVEL 255

//分支信息
typedef struct{	
	int starti:24;
	int valueType:8;
	int startPos;
}BranchInfo_t;

//分析json树的数据体
typedef struct{	
	VALUE_TYPE rootType;
	
	PairNode_t *pairs;
	
	ValueNode_t *values;
	
	int curPairNum;
	int curValueNum;
	
	int maxPairNum;
	int maxValueNum;
	
	BranchInfo_t branchs[MAX_TREE_DEEP_LEVEL];
	int curBranchNum;
	
	int errNo;
	char errInfo[255];
}JsonTree_t;

//表示json树的文本空间
typedef struct{
	int textLen;
	int curLen;
	char *text;
	
	int errNo;
	char errInfo[255];
}JsonText_t;

JsonTree_t * CreateJsonTree();

void FreeJsonTree(JsonTree_t *ptree);

//分析json文本，生成树表达
int ParseJson(char *text, int textLen,JsonTree_t *ptree);

//分析json文本，生成树表达，设定了数的高度
int ParseJsonTop(char *text, int textLen , JsonTree_t *ptree, int top);

//打印出错时的分析层信息
int PrintBreakInfo(JsonTree_t *ptree);

JsonText_t *CreateJsonText(int initTextLen);

void FreeJsonText(JsonText_t *ptext);

//生成json格式化文本
int PrintJsonTree(JsonTree_t *ptree, JsonText_t *ptext);

int PrintJsonMarkStr(char *str, JsonText_t *ptext);

int PrintJsonMarkStrSimple(char *str, int strLen, char *dest, int destLen);

int PackedJsonText(char *text, int textLen );

int AddPairValue(JsonText_t *ptext, char *key, char *value, VALUE_TYPE v_type);

int AddArrayValue(JsonText_t *ptext, char *value, VALUE_TYPE v_type);

int AddPreRearText(JsonText_t *ptext,char *preText, char *rearText);

#ifdef __cplusplus
}
#endif

#define GetFirstPairNode(ptree, pos, ppair)\
	{	ppair = ptree->pairs+pos;\
		if(ppair->startPos==-1) ppair=NULL;\
	}

#define GetNextPairNode(ptree, ppair)\
	{	if(ppair->nextPos>0) ppair=ptree->pairs+ppair->nextPos;\
		else ppair = NULL;\
	}

#define GetFirstValueNode(ptree, pos, pvalue)\
	{	pvalue = ptree->values+pos;\
		if(pvalue->startPos==-1) pvalue=NULL;\
	}

#define GetNextValueNode(ptree, pvalue)\
	{	if(pvalue->nextPos>0) pvalue=ptree->values+pvalue->nextPos;\
		else pvalue = NULL;\
	}

#define ForEachPairNode(ptree,_startPos,ppair)\
	for(ppair=(ptree->pairs[_startPos].startPos==-1)?NULL:(ptree->pairs+_startPos);\
		ppair!=NULL;\
		ppair=(ppair->nextPos>0)?(ptree->pairs+ppair->nextPos):NULL\
		)

#define ForEachValueNode(ptree,_startPos,pvalue)\
	for(pvalue=(ptree->values[_startPos].startPos==-1)?NULL:(ptree->values+_startPos);\
		pvalue!=NULL;\
		pvalue=(pvalue->nextPos>0)?(ptree->values+pvalue->nextPos):NULL\
		)

#endif

