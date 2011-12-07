#include "transclient.h"
#include "searchdef.h"
#include "net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#define ONE_GROUP_NUM 1

char *g_host;
int g_port;
char *g_docSplitStr;
int g_splitLen;

int TransStr(char *str);
int Parse(char * pagefile, char *resultfile);//level表示程度

int main(int argc, char**argv)
{
	if(argc!=6)
	{
		printf("%s server port pagefile docsplit resultfile\n",argv[0]);
		return -1;
	}
	g_host = argv[1];
	g_port = atoi(argv[2]);//端口
	
	g_docSplitStr = argv[4];//分割符
	TransStr(g_docSplitStr);	
	g_splitLen = strlen(g_docSplitStr);
	assert(g_splitLen>0);
	return Parse(argv[3],argv[5]);//输入、输出
}

int Parse(char * pagefile, char *resultfile)
{	
	char docBuffer[65536];
	int i=0,ret=0;
	int sock = net_tcp_connect(g_host,g_port);
	assert(sock>0);
	
	FILE *fp = fopen(pagefile,"r");
	FILE *fpw = fopen(resultfile,"w");
	if(fp==NULL)
	{
		printf("%s can not be opened\n",pagefile);
		return -1;
	}
	assert(fpw!=NULL);
	ClientBuffer_t* pClientBuffer=CreateClientBuffer(1); 
	assert(pClientBuffer!=NULL);
	i = 0;
	
	char out[65536]="";
	while(fgets(docBuffer, 65535, fp))
	{
		int len = strlen(docBuffer);
		assert(len>0);
		docBuffer[len - 1] = 0;
		strcpy(out,docBuffer);
		
		time_t time_b = time(0);
		ret = AddDocSrc(pClientBuffer, 0,docBuffer,len);
		if(ret < 0){
			fprintf(stderr, "adddocsrc err:%d\n", ret);
			continue;
		}
		

		ret = DoClientDocs(sock, pClientBuffer, 1);
		if(ret < 0)
		{
			fprintf(stderr, "doclientdoc err:%d\n", ret);
			continue;
		}
		
		unsigned long retdocID;
		char *keyStr=NULL;
		int keyLen;
		ret = GetDocResult(pClientBuffer, 0, &retdocID,&keyStr, &keyLen);
		fprintf(fpw,"%s\t-->%s\n\n",out,keyStr);
		//fprintf(fpw,"%s\n",keyStr);
		ResetClientBuffer(pClientBuffer);
		time_t time_e = time(0);
		time_t timediff=time_e-time_b;
		if(timediff>3)
		{
			fprintf(stderr,"timeout\n");
		}
	}
	fclose(fp);
	fclose(fpw);
	return 0;
}

int TransStr(char *str)
{
	int pos=0;
	char *loopptr=str;

	while(loopptr[0])
	{
		if(loopptr[0]&0x80)
		{
			str[pos++]=loopptr[0];	
			str[pos++]=loopptr[1];	
			loopptr+=2;
		}
		else if(loopptr[0]=='\\')
		{
			int chrNum=2;
			switch(loopptr[1])
			{
				case 'n':
					str[pos++]='\n';
					break;
				case 'r':
					str[pos++]='\r';
					break;
				case 't':
					str[pos++]='\t';
					break;
				default:
					chrNum=1;
					break;
			}
			loopptr+= chrNum;			
		}
		else 
		{
			str[pos++]=loopptr[0];	
			loopptr++;
		}
	}
	str[pos]=0;

	return 0;
}
