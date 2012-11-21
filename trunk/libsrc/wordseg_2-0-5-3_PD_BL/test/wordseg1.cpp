/*
 * 测试程序：使用最古老的分词接口，也是PS使用最广泛的接口。
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "scwdef.h"

static void scw_dump_out(scw_out_t* pout, char* line);
static void scw_dump_out1(scw_out_t* pout, char* line, int wdtype);

extern void write_prop_to_str(scw_property_t& property, char* buffer, int wdtype);

int main(int argc,char** argv)
{
	scw_worddict_t * pwdict = NULL;
	scw_out_t *pout = NULL;
	char line[1024000];
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
		if(flag == 0)
			scw_dump_out(pout,line);
		else
			scw_dump_out1(pout,line,pwdict->m_wdtype);
		line[0]=0;
	}

	// end timing
	double time = (double)( clock() - start ) / CLOCKS_PER_SEC;
	fprintf(stderr,"The elapsed time: %f seconds.\n", time);

	scw_destroy_out( pout);
	scw_destroy_worddict( pwdict);
	return 0;
}


void scw_dump_out(scw_out_t* pout, char* line)
{
	u_int i, pos, len;
	char word[256];

	// view basic word sep result

	if(pout->wsbtermcount > 0 )
	{
		printf("=================== Basic Word Sep Result =====================\n");
		for(i=0;i<pout->wsbtermcount; i++)
		{
			printf("[%d] ", pout->wsbtermoffsets[i]);
			pos = GET_TERM_POS(pout->wsbtermpos[i]);
			len = GET_TERM_LEN(pout->wsbtermpos[i]);
			strncpy(word, pout->wordsepbuf+pos, len);
			word[len]= 0;
			printf("%s  ", word);
		}
		printf("\n\n");
	}

	if(pout->wpbtermcount > 0 )
	{
		// view word phrase compond result
		printf("====================  Word Phrase Result  ======================\n");
		for( i = 0; i<pout->wpbtermcount; i++)
		{
			printf("[%d] ", pout->wpbtermoffsets[i]);
			pos = GET_TERM_POS(pout->wpbtermpos[i]);
			len = GET_TERM_LEN(pout->wpbtermpos[i]);
			strncpy(word, pout->wpcompbuf + pos, len);
			word[len] = 0;
			printf("%s  ", word);
		}
		printf("\n\n");
	}

	if( pout->spbtermcount > 0)
	{
		// view sub phrase result
		printf("====================  Sub Phrase Result   ======================\n");
		for(i=0; i<pout->spbtermcount; i++)
		{
			printf("[%d] ", pout->spbtermoffsets[i]);
			pos = GET_TERM_POS(pout->spbtermpos[i]);
			len = GET_TERM_LEN(pout->spbtermpos[i]);
			strncpy(word, pout->subphrbuf + pos, len);
			word[len] = 0;
			printf("%s  ", word);
		}
		printf("\n\n");
	}

	if(pout->pnewword->newwordbtermcount > 0)
	{
		// view newword result
		printf("====================  New Word  Result  ======================\n");
		scw_newword_t* pnewword = pout->pnewword;
		for( i = 0; i<pnewword->newwordbtermcount; i++)
		{
			printf("[%d] ", pnewword->newwordbtermoffsets[i*2]);//--这里是2*i,因为offset是不一样的
			pos = GET_TERM_POS(pnewword->newwordbtermpos[i]);
			len = GET_TERM_LEN(pnewword->newwordbtermpos[i]);
			strncpy(word, pnewword->newwordbuf + pos, len);
			word[len] = 0;
            if(IS_MULTI_TERM(pnewword->newwordbtermprop[i])) 
			printf("%s (m)", word);
            if(IS_NEWWORD(pnewword->newwordbtermprop[i])) 
                printf("%s  ", word);
		}
		printf("\n\n");
	}

	if(pout->namebtermcount > 0)
	{
		// view NER result
		printf("===================  Human Name Result   ======================\n");
		for(i=0; i<pout->namebtermcount; i++)
		{
			printf("[%d] ", pout->namebtermoffsets[i]);
			pos = GET_TERM_POS(pout->namebtermpos[i]);
			len = GET_TERM_LEN(pout->namebtermpos[i]);
			strncpy(word, pout->namebuf+pos, len);
			word[len] = 0;
			printf("%s  ", word);
		}
		printf("\n\n");
	}

	if( pout->bnbtermcount > 0)
	{
		// view the book name result
		printf("=======================  book names   =========================\n");
		for(i=0; i<pout->bnbtermcount; i++)
		{
			printf("[%d] ", pout->bnbtermoffsets[i]);
			pos = GET_TERM_POS(pout->bnbtermpos[i]);
			len = GET_TERM_LEN(pout->bnbtermpos[i]);
			strncpy(word, pout->booknamebuf+pos, len);
			word[len] = 0;
			printf("%s  ", word);
		}
		printf("\n\n");
	}
}

void scw_dump_out1(scw_out_t* pout, char* line, int wdtype)
{
	u_int i, pos, len;
	char word[256];
	char prop[256];

	// view basic word sep result
	printf("=================== Basic Word Sep Result =====================\n");
	for(i=0;i<pout->wsbtermcount; i++)
	{
		printf("[%d] ", pout->wsbtermoffsets[i]);
		pos = GET_TERM_POS(pout->wsbtermpos[i]);
		len = GET_TERM_LEN(pout->wsbtermpos[i]);
		strncpy(word, pout->wordsepbuf+pos, len);
		word[len]= 0;
		write_prop_to_str(pout->wsbtermprop[i], prop,wdtype);
		printf("%s(%s)  ", word,prop);
	}
	printf("\n\n");

	// view word phrase compond result
	printf("====================  Word Phrase Result  ======================\n");
	for( i = 0; i<pout->wpbtermcount; i++)
	{
		printf("[%d] ", pout->wpbtermoffsets[i]);
		pos = GET_TERM_POS(pout->wpbtermpos[i]);
		len = GET_TERM_LEN(pout->wpbtermpos[i]);
		strncpy(word, pout->wpcompbuf + pos, len);
		word[len] = 0;
		write_prop_to_str(pout->wpbtermprop[i], prop,wdtype);
		printf("%s(%s)  ", word,prop);
	}
	printf("\n\n");

	// view sub phrase result
	printf("====================  Sub Phrase Result   ======================\n");
	for(i=0; i<pout->spbtermcount; i++)
	{
		printf("[%d] ", pout->spbtermoffsets[i]);
		pos = GET_TERM_POS(pout->spbtermpos[i]);
		len = GET_TERM_LEN(pout->spbtermpos[i]);
		strncpy(word, pout->subphrbuf + pos, len);
		word[len] = 0;
		write_prop_to_str(pout->spbtermprop[i], prop,wdtype);
		printf("%s(%s)  ", word,prop);
	}
	printf("\n\n");

	// view NER result
	printf("===================  Human Name Result   ======================\n");
	for(i=0; i<pout->namebtermcount; i++)
	{
		printf("[%d] ", pout->namebtermoffsets[i]);
		pos = GET_TERM_POS(pout->namebtermpos[i]);
		len = GET_TERM_LEN(pout->namebtermpos[i]);
		strncpy(word, pout->namebuf+pos, len);
		word[len] = 0;
		write_prop_to_str(pout->namebtermprop[i], prop,wdtype);
		printf("%s(%s)  ", word,prop);
	}
	printf("\n\n");

	// view the book name result
	printf("=======================  book names   =========================\n");
	for(i=0; i<pout->bnbtermcount; i++)
	{
		printf("[%d] ", pout->bnbtermoffsets[i]);
		pos = GET_TERM_POS(pout->bnbtermpos[i]);
		len = GET_TERM_LEN(pout->bnbtermpos[i]);
		strncpy(word, pout->booknamebuf+pos, len);
		word[len] = 0;
		write_prop_to_str(pout->bnbtermprop[i], prop,wdtype);
		printf("%s(%s)  ", word,prop);
	}

	printf("\n\n");

	// view newword result
	printf("====================  New Word  Result  ======================\n");
	scw_newword_t* pnewword = pout->pnewword;
	for( i = 0; i<pnewword->newwordbtermcount; i++)
	{
		printf("[%d] ", pnewword->newwordbtermoffsets[i]);
		pos = GET_TERM_POS(pnewword->newwordbtermpos[i]);
		len = GET_TERM_LEN(pnewword->newwordbtermpos[i]);
		strncpy(word, pnewword->newwordbuf + pos, len);
		word[len] = 0;
		write_prop_to_str(pnewword->newwordbtermprop[i], prop,wdtype);
		printf("%s(%s)  ", word,prop);
	}
	printf("\n\n");
	return;
}
