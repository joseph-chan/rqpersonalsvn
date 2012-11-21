#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scwdef.h"

extern u_int scw_seek_lemma(scw_worddict_t * pwdict,const char* term,int len);
extern void write_prop_to_str(scw_property_t& property, char* buffer,int wdtype);
extern int get_static_lmstr(scw_worddict_t* pwdict,scw_lemma_t* plm, char* dest, u_int destlen);

int main(int argc,char** argv)
{
	scw_worddict_t * pwdict;
	char line[1024000];
	u_int pos = 0;
	scw_lemma_t * plm = NULL, *plm2 = NULL;
	int offset =0;
	int cnt = 0;
	char property[256];
	char segbuf[512];
	char subbuf[512];
	char lemma[256];
	char tmpbuf[512];
	int weight = 0;

	if(argc!=2)
	{
		fprintf(stderr, "usage: %s dictpath\n", argv[0]);
		exit(-1);
	}

	if((pwdict=scw_load_worddict(argv[1]))==NULL)
	{
		fprintf(stderr,"Load worddict failed.Filename=%s\n",argv[1]);
		return 1;
	}

	int linenum=0;
	while(fgets(line,sizeof(line),stdin))
	{
		if(++linenum%1000==0)
			fprintf(stderr, "%d\n", linenum);
		
		int len=strlen(line);
		while((line[len-1]=='\r') ||(line[len-1]=='\n'))
			line[--len]=0;
		printf("=================== Word Information =====================\n");
		if((pos = scw_seek_lemma(pwdict,line,len))==LEMMA_NULL)
		{
			printf("NULL\n\n");
			continue;
		}

		plm = &pwdict->m_lemmalist[pos];

		memset(property,0,sizeof(property));
		write_prop_to_str(plm->m_property,property,pwdict->m_wdtype);		
		weight = plm->m_weight;
			
		if(plm->m_type == LEMMA_TYPE_SBASIC)
		{
			if(strlen(property) == 0)
				strcpy(property,"-");
			printf("[%s] [0(%s)] [] %s %d\n\n",line,line,property,weight);
		}
		else if(plm->m_type == LEMMA_TYPE_SPHRASE)
		{
			cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
			strcpy(segbuf,"[");
	                for(int i=0; i<cnt; i++)
			{
				pos = pwdict->m_phinfo[plm->m_phinfo_bpos+i+1];
				plm2= &pwdict->m_lemmalist[pos];
				len = get_static_lmstr(pwdict,plm2, lemma, sizeof(lemma));		
				sprintf(tmpbuf,"%d(%s)",i,lemma);
				strcat(segbuf,tmpbuf);
			}
			strcat(segbuf,"]");

			offset = 0;
			strcpy(subbuf,"[");
			if(plm->m_subphinfo_bpos!=COMMON_NULL)
			{
				cnt = pwdict->m_phinfo[plm->m_subphinfo_bpos];
				for(int i=0; i<cnt; i++)
				{
					offset = pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+1];
					pos = pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+2];
					plm2 = &pwdict->m_lemmalist[pos];
					len = get_static_lmstr(pwdict,plm2, lemma,sizeof(lemma));
				
					sprintf(tmpbuf,"%d(%s)",offset,lemma);
					strcat(subbuf,tmpbuf);
				}
			}
			strcat(subbuf,"]");

			printf("[%s] %s %s %s %d\n\n",line,segbuf,subbuf,property,weight);
		}
		else
			printf("NULL\n\n");
	}

	scw_destroy_worddict(pwdict);
	return 0;
}


