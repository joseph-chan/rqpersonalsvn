/*
 * 测试程序：使用最古老的分词接口，也是PS使用最广泛的接口。
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "scwdef.h"

static void scw_dump_out(scw_out_t* pout, char* line, u_int inlen);

char line[102400];
char line2[102400];
char line3[102400];
int main(int argc,char** argv)
{
	scw_worddict_t * pwdict = NULL;
	scw_out_t *pout = NULL;

	u_int scw_out_flag;
	int flag = 0;

	if( argc < 2 )
	{
		fprintf(stderr, "usage: %s  dictpath  [outtype]\n", argv[0]);
		exit(-1);
	}

	if((pwdict=scw_load_worddict(argv[1]))==NULL)
	{
		fprintf(stderr,"Load worddict failed.Filename=%s\n",argv[1]);
		return -1;
	}

	if( argc > 2 )
		flag = atoi(argv[2]);

	scw_out_flag = SCW_OUT_ALL | SCW_OUT_PROP;
	if((pout=scw_create_out(10000, scw_out_flag))==NULL)
	{
		fprintf(stderr,"Init the output buffer error.\n");
		return -1;
	}

	// start timing
	clock_t start = clock();

	line[0]=0;
	while(fgets(line,sizeof(line),stdin))
	{
		int len=strlen(line);
		while((line[len-1]=='\r') ||(line[len-1]=='\n'))
			line[--len]=0;
		scw_segment_words(pwdict,pout,line,len);

		scw_dump_out(pout,line, len);
		line[0]=0;
	}

	// end timing
	double time = (double)( clock() - start ) / CLOCKS_PER_SEC;
	fprintf(stderr,"The elapsed time: %f seconds.\n", time);

	scw_destroy_out( pout);
	scw_destroy_worddict( pwdict);
	return 0;
}


void scw_dump_out(scw_out_t* pout, char* line, u_int inlen)
{
	u_int i, pos, len;
	line2[0]=0;
	line3[0]=0;
	u_int alllen = 0;


	// view basic word sep result

	for(i=0;i<pout->wsbtermcount; i++)
	{
		pos= pout->wsbtermoffsets[i];
		len = GET_TERM_LEN(pout->wsbtermpos[i]);

		strncat(line2,line+pos, len);


		pos = GET_TERM_POS(pout->wsbtermpos[i]);
		strncat(line3, pout->wordsepbuf+pos, len);

		alllen += len;
	}
	line2[alllen] = 0;
	line3[alllen] = 0;

	if(alllen != inlen)
	{
		printf("%s\n%s\n%s\n-----------\n", line, line2, line3);
	}
	for(u_int i = 0 ; i < inlen; i++)
	{
		if(line[i] != line2[i] || line[i] != line3[i] )
		{
			printf("%s\n%s\n%s\n-----------\n", line, line2, line3);
			return;
		}
	}

}
