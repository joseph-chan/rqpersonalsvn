#ifndef _PARSE_JSON_H_
#define _PARSE_JSON_H_

/*
	json�ı��Ľ�������������
	
	����������������ݽṹ���json�е�dict,array, string int ..
	
	�����﷨���� http://json.org/json-zh.html �еĶ��塣
		object����	"PairNode_t+����" ��ʾ��
		array ���� "ValueNode_t+����" ��ʾ��
	
*/

#ifdef __cplusplus
extern "C"
{
#endif

//��������
typedef enum{V_STR,V_VALUE_STR,V_VALUE_ARRAY,V_PAIR_ARRAY} VALUE_TYPE;

//ֵ��Ϣ
typedef struct{
	VALUE_TYPE v_type;
	union{
		char *pStr;
		int startPos;//-1����ʾ��ֵ����ֵ����
	};
	int nextPos;//>0����ʾ���ں��
}ValueNode_t;

//ֵ����Ϣ
typedef struct{
	char *keyStr;
	VALUE_TYPE v_type;
	union{
		char *pStr;
		int startPos;//-1����ʾ��ֵ����ֵ����
	};
	int nextPos;//>0����ʾ���ں��
}PairNode_t;

//����������
#define MAX_TREE_DEEP_LEVEL 255

//��֧��Ϣ
typedef struct{	
	int starti:24;
	int valueType:8;
	int startPos;
}BranchInfo_t;

//����json����������
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

//��ʾjson�����ı��ռ�
typedef struct{
	int textLen;
	int curLen;
	char *text;
	
	int errNo;
	char errInfo[255];
}JsonText_t;

JsonTree_t * CreateJsonTree();

void FreeJsonTree(JsonTree_t *ptree);

//����json�ı������������
int ParseJson(char *text, int textLen,JsonTree_t *ptree);

//����json�ı������������趨�����ĸ߶�
int ParseJsonTop(char *text, int textLen , JsonTree_t *ptree, int top);

//��ӡ����ʱ�ķ�������Ϣ
int PrintBreakInfo(JsonTree_t *ptree);

JsonText_t *CreateJsonText(int initTextLen);

void FreeJsonText(JsonText_t *ptext);

//����json��ʽ���ı�
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

