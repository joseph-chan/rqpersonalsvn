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
int Parse(char * pagefile, char *resultfile);

int main(int argc, char**argv)
{
	if(argc!=6)
	{
		printf("%s server port pagefile docsplit resultfile\n",argv[0]);
		return -1;
	}
	g_host = argv[1];
	g_port = atoi(argv[2]);
	
	g_docSplitStr = argv[4];
	TransStr(g_docSplitStr);	
	g_splitLen = strlen(g_docSplitStr);
	assert(g_splitLen>0);
	fprintf(stderr,"g_host:%s\tg_port:%d\n",g_host,g_port);
	return Parse(argv[3],argv[5]);
}

int Parse(char * pagefile, char *resultfile)
{	
	unsigned long docID=1;
	char docBuffer[65536];
	int i,ret;
	unsigned pos=0;
	fprintf(stderr,"g_host:%s\tg_port:%d\n",g_host,g_port);
	int sock = net_tcp_connect(g_host,g_port);
	assert(sock>0);
	
	FILE *fp = fopen(pagefile,"r");
	FILE *fpw = fopen(resultfile,"w");
	assert(fp!=NULL);
	assert(fpw!=NULL);
	ClientBuffer_t* pClientBuffer=CreateClientBuffer(ONE_GROUP_NUM); 
	assert(pClientBuffer!=NULL);
	i = 0;
	while(feof(fp)==0)
	{
		char *bufferPtr;
		char *docSplit;
		int needLen=65535-pos;
		int resultLen = fread(docBuffer+pos,1,needLen, fp);		
		assert(resultLen>0);
		docBuffer[pos+resultLen]=0;
		bufferPtr = docBuffer;
		docSplit = strstr(bufferPtr,g_docSplitStr);
		while(docSplit!=NULL)
		{
			int docLen = docSplit - bufferPtr;
			ret = AddDocSrc(pClientBuffer,docID++,bufferPtr,docLen);
			i++;
			if(i==ONE_GROUP_NUM)
			{
				do{
					ret = DoClientDocs(sock, pClientBuffer, 10);
					if(ret<0)
					{
						fprintf(stderr, "DoClientDocs failed, ret:%d, curdocID:%u\n",ret,docID);					
						sleep(1);
						sock = net_tcp_connect(g_host,g_port);
						if(sock<0)
						{
							fprintf(stderr, "server can not connected\n");
							return -1;
						}
					}
				}while(ret!=0);
				
				for(i=0; i<ONE_GROUP_NUM; i++)
				{
					unsigned long retdocID;
					char *keyStr;						
					int keyLen;
					ret = GetDocResult(pClientBuffer, i, &retdocID, &keyStr, &keyLen);
					if(ret<0)
					{
						fprintf(fpw,"get %d %d\n",i,ret);
					}
					else if(keyStr==NULL)
					{
						fprintf(stderr, "do doc failed, ret:%d, doc:%lu\n",keyLen,retdocID);						
					}
					else
					{
						keyStr[keyLen]=0;
						fprintf(fpw,"%lu %s\n",retdocID,keyStr);
					}
				}
				ResetClientBuffer(pClientBuffer);		
				i=0;
			}
			else 
			{
				
			}
			
			bufferPtr = docSplit+g_splitLen;
			docSplit = strstr(bufferPtr,g_docSplitStr);			
		}
		
		pos = resultLen+pos-(bufferPtr-docBuffer);
		memmove(docBuffer,bufferPtr,pos);		
	}
	
	if(pos>0)
	{
		int docLen = pos;
		ret = AddDocSrc(pClientBuffer,docID++,docBuffer,docLen);
		i++;
	}
	
	if(i>0)
	{
		int lasti = i;
		do{
			ret = DoClientDocs(sock, pClientBuffer, 10);
			if(ret<0)
			{
				fprintf(stderr, "DoClientDocs failed, ret:%d, curdocID:%u\n",ret,docID);
				sleep(1);
				sock = net_tcp_connect(g_host,g_port);
				if(sock<0)
				{
					fprintf(stderr, "server can not connected\n");
					return -1;
				}
			}
		}while(ret!=0);

		for(i=0; i<lasti; i++)
		{
			unsigned long retdocID;
			char *keyStr;						
			int keyLen;
			ret = GetDocResult(pClientBuffer, i, &retdocID, &keyStr, &keyLen);
			if(ret<0)
			{
				fprintf(fpw,"%lu %d\n",retdocID,ret);
			}
			else if(keyStr==NULL)
			{
				fprintf(stderr, "do doc failed, ret:%d, doc:%lu\n",keyLen,retdocID);						
			}
			else
			{
				keyStr[keyLen]=0;
				fprintf(fpw,"%lu %s\n",retdocID,keyStr);
			}
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
