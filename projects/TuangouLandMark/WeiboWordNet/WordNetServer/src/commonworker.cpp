#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <map>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/time.h>
#include "strmatch.h"
#include "dictmatch.h"
#include "nx_log.h"
#include "commonworker.h"
#include "parsejson.h"
using namespace std;

static char const rcsid[] ="$commonworker:  2008/04/03 $ ";

extern global_data_t global_data;

typedef struct // 线程资源结构体，可以根据自己需求 增加
{	
	JsonTree_t *ptree;
	JsonText_t *ptext;
	char *query_type;
	char *content;
	char *res;
}WorkData_t;
///////////////////////////////////////////////////////////////
bool CmpTerm(pair<string, u_int> term1, pair<string,u_int> term2)
{
    return term1.second > term2.second;
}

int heapify_r(sort_item_t* input,unsigned int len, int i)
{
    if(input==NULL)
    {
        return -1;
    }

    int min;
	sort_item_t tmp;
	min = i;
	// number of left child must smaller than the length of the heap
	if( (2*i+1) <= len -1 && input[2*i+1].sort_int<input[i].sort_int)
	{
		min = 2*i+1;
	}
	// number of right child must smaller than the length of the heap
	if( (2*i+2) <= len -1 && input[2*i+2].sort_int<input[min].sort_int)
	{
		min = 2*i +2;
	}

	if ( min != i && min <= len -1)
	{
		tmp=input[min];
		input[min]=input[i];
		input[i]=tmp;
		heapify_r(input,len,min);
	}

    return 0;

}

int build_min_heap(sort_item_t* input,unsigned int len)
{
    if(input==NULL)
    {
        return -1;
    }

    int i;

	for(i=(len-1)/2;i>=0;i--)
	{
		heapify_r(input,len,i);
	}
    return 0;
}

MATCHENTRY * loadMachine(const char *file)
{
	MATCHENTRY *machine_ww = NULL;
	if(!(machine_ww = strMatchInit(2)))
	{
		fprintf(stderr,"Init machine err!\n");
		return NULL;
	}
	ifstream fin;
	fin.open(file);
	if(!fin.is_open())
	{
		fprintf(stderr,"%s can not be opened!\n",file);
		return NULL;
	}
	char text[1024]="";
	char unitary[1024]="";
	char *pos = NULL;
	int tLen = 0,iret = 0;
	double labelLen=0,disturbRate=0;
	map<string,int> machineMap;
	map<string,int>::iterator it;
	while(!fin.eof())
	{
		if(fin.getline(text,1024).good()==false)
			break;
		tLen = strlen(text);
		if(tLen < 2)
			continue;
		it = machineMap.find(text);
		if(it == machineMap.end())
		{
			machineMap.insert(map<string,int>::value_type(text,1));
			Addword(text,0,machine_ww,0);
		}
	}
	fin.close();
	Prepare(0, machine_ww);
	return machine_ww;
}

int findBlackWord(char *text,int &watchNum,MATCHENTRY *machine_ww,vector<pair<string,int> > &watchWord)
{
	if(text == NULL || machine_ww == NULL || machine_ww->wordsnum == 0)
		return -1;
	FindAllUntilNull(text,0,machine_ww);
	watchNum=0;
	watchWord.clear();
	pair<string,int> pp;
	for(int i = 0; i< machine_ww->wordsnum; i++)
	{
		if (machine_ww->wordlist[i].findnum > 0)
		{
			pp.first = (char*)machine_ww->wordlist[i].word;
			pp.second = machine_ww->wordlist[i].findnum;
#ifdef DEBUG
			fprintf(stderr,"findBlackWord:%s--%d\n", pp.first.c_str(),machine_ww->wordlist[i].findnum);
#endif
			watchWord.push_back(pp);
			watchNum ++;
			machine_ww->wordlist[i].findnum = 0;
		}
	}
	return 0;
}
int CreateGlobalData(char **argv, int argc)
{
	int i;
	int ret;
	WordNetIndexor * cIndexor = new WordNetIndexor;

	ret = cIndexor->LoadIndexmmap("data/index/word_net","data/index/term_info.index","data/index/relate_info.index",global_data.TermIndex,global_data.TermInfo, &(global_data.file_mmap),&(global_data.total_index_info));
	
	if(ret < 0)
	{
		nx_log(NGX_LOG_ERR,"load index error, quit!");
		return -1;
	}
	nx_log(NGX_LOG_ALERT,"load index successful !");
	
	global_data.hot_term_buf = (sort_item_t*) malloc (1000 * sizeof(sort_item_t));
	if(global_data.hot_term_buf == NULL)
	{
		nx_log(NGX_LOG_ERR,"init error, quit!");
		return -1;
	}

	return 0;
}

int ReleaseGlobalData()
{
	return 0;
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
	//workData->title=NULL;
	//workData->url = NULL;
	//workData->time = 0;
	workData->content=NULL;
	return 0;
}

int ParseJsonInput(WorkData_t *workData, BYTE *pSrcData, unsigned int srcLen)
{
	int ret;
	JsonTree_t *ptree = workData->ptree;
	PairNode_t *ppair;
	ret = ParseJson((char*)pSrcData, srcLen, ptree);
	if(ret<=0)
		return -1;

	if(ptree->rootType!=V_PAIR_ARRAY)
		return -2;

	ForEachPairNode(ptree, 0, ppair)
	{
		/*
		if((strcmp(ppair->keyStr,"title")==0))
		{
			if(ppair->v_type != V_STR)
			{
				return -1;
			}
		//	workData->title = ppair->pStr;
		}
		else if((strcmp(ppair->keyStr,"url")==0))
		{
			if(ppair->v_type != V_STR)
			{
				return -1;
			}
		//	workData->url = ppair->pStr;
		}
		else if((strcmp(ppair->keyStr,"time")==0))
		{
			if(ppair->v_type != V_VALUE_STR)
			{
				return -1;
			}
		//	workData->time = atoi(ppair->pStr);
		}
		*/
		if((strcmp(ppair->keyStr,"qt")==0))
		{
			if(ppair->v_type != V_STR)
			{
				return -1;
			}
			workData->query_type = ppair->pStr;
		}
		else if((strcmp(ppair->keyStr,"content")==0))
		{
			if(ppair->v_type != V_STR)
			{
				return -1;
			}
			workData->content = ppair->pStr;
		}
	}

	return 0;
}
int GetHotTerm(WorkData_t *workData,BYTE *pResultData, unsigned int * resultLen)
{

	workData->ptext->curLen=0;

	vector<term_info_t>::iterator iter_term_info;

	nx_log(NGX_LOG_NOTICE,"get input begin: type [%d] require [%s] ",atoi(workData->query_type),workData->content);
	int i=0;
	for (iter_term_info = global_data.TermInfo.begin() ; iter_term_info != global_data.TermInfo.end(); iter_term_info ++)
	{
		if(i<1000)
		{
			global_data.hot_term_buf[i].sort_int = iter_term_info->df_now;
			global_data.hot_term_buf[i].appendix = i;
			if(i == 999)
			{
				build_min_heap(global_data.hot_term_buf,1000);	
			}
		}
		else
		{
			if (iter_term_info->df_now > global_data.hot_term_buf[0].sort_int)
			{
				global_data.hot_term_buf[0].sort_int = iter_term_info->df_now;
				global_data.hot_term_buf[0].appendix = i;
				heapify_r(global_data.hot_term_buf,1000,0);
			}
		}
		i++;
	}

	int term_index;
	//int term_len;
	char buf[2048];
	buf[0]='\0';
	for (i=999;i>=0 && strlen(buf) < 2000 ;i--)
	{
		term_index = global_data.hot_term_buf[i].appendix;
		//term_len = strlen(global_data.TermInfo[term_index].termstr);
		snprintf(buf + strlen(buf),100,"%s:%d ",global_data.TermInfo[term_index].termstr,global_data.TermInfo[term_index].df_now);
	}

	AddPairValue(workData->ptext,"hot_term",buf,V_STR);
	memcpy(pResultData, workData->ptext->text, workData->ptext->curLen);
	resultLen[0] =  workData->ptext->curLen;
	return 0;


}

int GetCoocTerm(WorkData_t *workData,BYTE *pResultData, unsigned int * resultLen)
{
	workData->ptext->curLen=0;
	char buf[1000];
	buf[0]='\0';

	u_int hash_value;
	u_int term_index;
	__gnu_cxx::hash_map<u_int,u_int>::iterator hmIndex;

	hash_value= hashlittle(workData->content, strlen(workData->content), 1);
	nx_log(NGX_LOG_NOTICE,"get input begin: require [%s] ",workData->content);

	hmIndex= global_data.TermIndex.find(hash_value);

	if (hmIndex != global_data.TermIndex.end())
	{
		term_index = hmIndex->second;
		//snprintf(buf,1000,"term:[%s] df [%d] relate_num [%d] ",global_data.TermInfo[term_index].termstr,global_data.TermInfo[term_index].df_now,global_data.TermInfo[term_index].relate_term_num);
		AddPairValue(workData->ptext,"term",global_data.TermInfo[term_index].termstr,V_STR);
		snprintf(buf,1000,"%d",global_data.TermInfo[term_index].df_now);
		AddPairValue(workData->ptext,"df",buf,V_STR);

		buf[0]='\0';
		
		nx_log(NGX_LOG_DEBUG,"get input: term info [%s] ",buf);

		vector<pair<string,u_int> > v_tmp;
		for (hmIndex=global_data.TermInfo[term_index].cooc_term.begin();(hmIndex !=global_data.TermInfo[term_index].cooc_term.end()) && (strlen(buf) < 900);hmIndex ++)
		{
			int weight;
			int idf;
			unsigned int offset;
			relate_term_info_t* tmp_relate_info;
			idf = int(log10(float(global_data.total_index_info.weibo_num) / float(global_data.TermInfo[global_data.TermIndex[hmIndex->first]].df_now + global_data.TermInfo[global_data.TermIndex[hmIndex->first]].df_history)));

			offset = (hmIndex->second) * sizeof(relate_term_info_t);
			if( readahead (global_data.file_mmap.fd, offset, sizeof(relate_term_info_t)) != 0)
			{
				nx_log(NGX_LOG_ALERT,"mmap error",workData->content,hash_value);
				return -1;
			}
			tmp_relate_info = (relate_term_info_t*) ( (char *)global_data.file_mmap.pmmap + offset);

			weight = tmp_relate_info->num_now * idf * idf;

			nx_log(NGX_LOG_NOTICE,"query [%s] cooc-term [%s]  cooc_df [%ud] weibo_num [%uL] idf [%d] weight [%d]",workData->content,global_data.TermInfo[global_data.TermIndex[hmIndex->first]].termstr,global_data.TermInfo[global_data.TermIndex[hmIndex->first]].df_now,global_data.total_index_info.weibo_num,idf,weight);
			v_tmp.push_back(pair<string,u_int> (global_data.TermInfo[global_data.TermIndex[hmIndex->first]].termstr, weight));
		}
		sort(v_tmp.begin(),v_tmp.end(),CmpTerm);

		vector<pair<string,u_int> >::iterator iter_tmp;
		for(iter_tmp=v_tmp.begin();iter_tmp!=v_tmp.end() && (strlen(buf) < 900);iter_tmp++)
		{
			snprintf(buf+strlen(buf),100,"%s:%u ",iter_tmp->first.c_str(),iter_tmp->second);
		}
	}
	else
	{
		nx_log(NGX_LOG_NOTICE,"get input: no such term [%s] hash_value [%ud]",workData->content,hash_value);
		
	}	

	AddPairValue(workData->ptext,"cooc",buf,V_STR);
	memcpy(pResultData, workData->ptext->text, workData->ptext->curLen);
	resultLen[0] =  workData->ptext->curLen;
	return 0;


}
int GenJsonResult(WorkData_t *workData,BYTE *pResultData, unsigned int * resultLen)
{

	switch (atoi(workData->query_type))
	{
		case 0: 
			GetHotTerm(workData,pResultData,resultLen);
			break;
		case 1:
			GetCoocTerm(workData,pResultData,resultLen);
			break;
	}


	return 0;
}

int DoWork(unsigned short command, DOCID_T docID,  BYTE *pSrcData, unsigned int srcLen, BYTE *pResultData, unsigned int * resultLen, void *working_data)
{
	int ret;
	char *text = (char *)pSrcData;
	WorkData_t *workData = (WorkData_t*)working_data;
	ret = ParseJsonInput( workData, pSrcData, srcLen);
	if (ret <0 || workData->content == NULL)
	{
		nx_log(NGX_LOG_ALERT,"get input json error");
		return 1;
	}
	/*
	ret = dm_search(g_dm_dict,workData->dm_pack,workData->content,strlen(workData->content),DM_OUT_FMM);
	if(ret <0)
	{
		nx_log(NGX_LOG_ALERT,"dm_search error");
		return 1;
	}	
	*/

	ret = GenJsonResult(workData,pResultData, resultLen);
	if(ret <0)
	{
		nx_log(NGX_LOG_ALERT,"GenJsonResult error");
		return 1;
	}	
	return 0;
}

