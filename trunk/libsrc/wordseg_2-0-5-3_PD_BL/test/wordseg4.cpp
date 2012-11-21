#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scwdef.h"
#include "international.h"

static void dump_item(scw_item_t* item);
static void dump_item1(scw_item_t* item,int wdtype);
extern void write_prop_to_str(scw_property_t& property, char* buffer,int wdtype);

int main(int argc,char** argv)
{
	scw_worddict_t * pwdict;
	scw_inner_t *pir;
	scw_item_t *pitem;
	char line[1024000];
	int flag = 0;
	int tsize = 0;
	int ret = 0;

	if(argc!=4){
		fprintf(stderr, "usage: %s dictfilename outtype tsize\n", argv[0]);
		exit(-1);
	}

	if((pwdict=scw_load_worddict(argv[1]))==NULL){
		fprintf(stderr,"Load worddict failed.Filename=%s\n",argv[1]);
		return 1;
	}

	flag = atoi(argv[2]);
	tsize = atoi(argv[3]);
	if(tsize < 1)
	{
		fprintf(stderr,"tsize [%s] should > 1\n",argv[3]);
		return 1;
	}

	if((pir=scw_create_inner(tsize, SCW_OUT_ALL | SCW_OUT_PROP))==NULL){
		fprintf(stderr,"Init the output buffer error.\n");
		return -1;
	}

	if((pitem=scw_create_item(tsize))==NULL){
		fprintf(stderr, "Init pitem failed\n");
		return -1;
	}

	int linenum=0;
	while(fgets(line,sizeof(line),stdin)){
		if(++linenum%1000==0)
			fprintf(stderr, "%d\n", linenum);
		
		int len=strlen(line);
		while((line[len-1]=='\r') ||(line[len-1]=='\n'))
			line[--len]=0;
		
		if(scw_seg(pwdict,pir,line,len) < 0)
		{
			fprintf(stderr, "scw_seg return -1!\n");
		}
		
		ret = get_lgt_scw_seg(pwdict, pir);
		if(ret < 0)
		{
			fprintf(stderr, "get_lgt_scw_seg return -1\n");
			continue;
		}

		printf("============== Basic Word Result =============\n");
		if(scw_get_result(pitem, pwdict, pir, SCW_OUT_BASIC | SCW_OUT_PROP)<0){
			fprintf(stderr, "get basic seg result error!\n");
			continue;
		}
		if(flag == 0)
			dump_item(pitem);
		else if(flag == 1)
			dump_item1(pitem,pwdict->m_wdtype);

		printf("============== Word Phrase Result =============\n");
		if(scw_get_result(pitem, pwdict,pir, SCW_OUT_WPCOMP | SCW_OUT_PROP)<0){
			fprintf(stderr, "get word/phrase result error!\n");
			continue;
		}
		if(flag == 0)
			dump_item(pitem);
		else if(flag == 1)
			dump_item1(pitem,pwdict->m_wdtype);
		
		printf("============== Sub Phrase Result =============\n");
		if(scw_get_result(pitem, pwdict, pir, SCW_OUT_SUBPH | SCW_OUT_PROP)<0){
			fprintf(stderr,"get sub phrase result error!\n");
			continue;
		}
		if(flag == 0)
			dump_item(pitem);
		else if(flag == 1)
			dump_item1(pitem,pwdict->m_wdtype);
		
		printf("============== Human Name Result =============\n");
		if(scw_get_result(pitem, pwdict, pir, SCW_OUT_HUMANNAME | SCW_OUT_PROP)<0){
			fprintf(stderr, "get sub phrase result error!\n");
			continue;
		}
		if(flag == 0)
			dump_item(pitem);
		else if(flag == 1)
			dump_item1(pitem,pwdict->m_wdtype);
		
		printf("============== Book Name Result =============\n");
		if(scw_get_result(pitem, pwdict, pir, SCW_OUT_BOOKNAME | SCW_OUT_PROP)<0){
			fprintf(stderr,"get sub phrase result error!\n");
			continue;
		}
		if(flag == 0)
			dump_item(pitem);
		else if(flag == 1)
			dump_item1(pitem,pwdict->m_wdtype);

		printf("============== NEWWORD Result =============\n");
		if(scw_get_result(pitem, pwdict, pir, SCW_OUT_NEWWORD | SCW_OUT_PROP)<0){
			fprintf(stderr,"get newword result error!\n");
			continue;
		}
		if(flag == 0)
			dump_item(pitem);
		else if(flag == 1)
			dump_item1(pitem,pwdict->m_wdtype);
	}
	return 0;

}

void dump_item(scw_item_t* pitem)
{
	char word[256];
	int offset;

	for(int i=0;i<(int)pitem->m_tmcnt; i++){
		scw_gi_tmstr(pitem,i, word, sizeof(word));
		offset = scw_gi_tmoff(pitem,i);
		printf("[%d]%s ", offset, word);
	}
	printf("\n\n");
}

void dump_item1(scw_item_t* pitem,int wdtype)
{
	char word[256];
	char prop[256];
	int offset;
	
	for(int i=0;i<(int)pitem->m_tmcnt; i++){
		scw_gi_tmstr(pitem,i, word, sizeof(word));
		offset = scw_gi_tmoff(pitem,i);
		write_prop_to_str(pitem->m_tmprop[i], prop,wdtype);
		printf("[%d]%s(%s) ", offset, word,prop);
	}
	printf("\n\n");
}

