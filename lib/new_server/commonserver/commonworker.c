#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include "commonworker.h"
#include "transclient.h"
#include "parsejson.h"

static char const rcsid[] ="$commonworker:  2008/04/03 $ ";

typedef struct{	
	JsonTree_t *ptree;
	JsonText_t *ptext;
	char *title;
	char *url;
	int time;
}WorkData_t;

int CreateGlobalData(char **argv, int argc)
{
	int i;
	printf("more paras:");
	for(i=0; i<argc; i++)
		printf("%s ",argv[i]);
	printf("\n");
	return 0;
}

int ReleaseGlobalData()
{
}

void* CreateWorkingData()
{
	WorkData_t *workData = (WorkData_t*)calloc(sizeof(WorkData_t),1) ;
	assert(workData!=NULL);
	workData->ptree = CreateJsonTree();
	assert(workData->ptree!=NULL);
	workData->ptext = CreateJsonText(1000); 
	assert(workData->ptext != NULL);
	
	return workData;
}

void ReleaseWorkingData(void *working_data)
{	
	WorkData_t *workData = (WorkData_t*)working_data;
	if(workData->ptree != NULL)
	{
		FreeJsonTree(workData->ptree);
		workData->ptree = NULL;
		FreeJsonText(workData->ptext);
		workData->ptext = NULL;
	}
	free(working_data);
}

int InitWorkData(WorkData_t *workData)
{
	workData->title=NULL;
	workData->url = NULL;
	workData->time = 0;
	return 0;
}

int ParseJsonInput(WorkData_t *workData, BYTE *pSrcData, unsigned int srcLen)
{
	int ret;
	JsonTree_t *ptree = workData->ptree;
	PairNode_t *ppair;
	ret = ParseJson(pSrcData, srcLen, ptree);
	if(ret<=0)
		return -1;

	if(ptree->rootType!=V_PAIR_ARRAY)
		return -2;
	
	ForEachPairNode(ptree, 0, ppair)
	{
		if((strcmp(ppair->keyStr,"title")==0))
		{
			if(ppair->v_type != V_STR)
			{
				return -1;
			}
			workData->title = ppair->pStr;
		}
		else if((strcmp(ppair->keyStr,"url")==0))
		{
			if(ppair->v_type != V_STR)
			{
				return -1;
			}
			workData->url = ppair->pStr;
		}
		else if((strcmp(ppair->keyStr,"time")==0))
		{
			if(ppair->v_type != V_VALUE_STR)
			{
				return -1;
			}
			workData->time = atoi(ppair->pStr);
		}
	}
	
	return 0;
}

int GenJsonResult(WorkData_t *workData,BYTE *pResultData, unsigned int * resultLen)
{
	workData->ptext->curLen=0;
	if(workData->title)
		AddPairValue(workData->ptext,"title",workData->title,V_STR);
	if(workData->url)
		AddPairValue(workData->ptext,"url",workData->url,V_STR);
	if(workData->time>0)
	{
		char timeStr[32];
		sprintf(timeStr,"%d",workData->time);
		AddPairValue(workData->ptext,"time",timeStr,V_VALUE_STR);
	}
	memcpy(pResultData, workData->ptext->text, workData->ptext->curLen);
	resultLen[0] =  workData->ptext->curLen;
	return 0;
}

int DoWork(unsigned short command, DOCID_T docID,  BYTE *pSrcData, unsigned int srcLen, BYTE *pResultData, unsigned int * resultLen, void *working_data)
{
	char *text = (char *)pSrcData;
	WorkData_t *workData = (WorkData_t*)working_data;
	InitWorkData(workData);
	int ret = ParseJsonInput(workData, pSrcData, srcLen);
	if(ret<0)
	{
		resultLen[0]=sprintf(pResultData,"{\"ret\":%d}",srcLen);
		return 0;
	}
	ret = GenJsonResult(workData,pResultData,resultLen);
	return 0;
}

