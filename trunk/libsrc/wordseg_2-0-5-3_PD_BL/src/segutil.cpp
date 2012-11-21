/*
 * Filename  : scwlib.cpp -> segutil.cpp
 * Descript  : here defines some utility functions used in dict building or segmetation.
 * Author  : Paul Yang, zhenhaoji@sohu.com
 * Time    : 2004-12-31
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h> 
#include <ul_log.h>
#include "property.h"
#include "scwdef.h"
#include "international.h"

#define MAX_HELPSTR_LEN 2048
// symbol for match all versions
#define ALL_MATCHED_VER "*-*-*-*"

char al_num_map[256] = 
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned char SCW_BASICWORD_ASCII[256]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x00-0x0F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x10-0x1F
	0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0, //0x20-0x2F
	1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0, //0x30-0x3F
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //0x40-0x4F
	1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1, //0x50-0x5F
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //0x60-0x6F
	1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0, //0x70-0x7F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x80-0x8F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x90-0x9F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xA0-0xAF
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xB0-0xBF
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xC0-0xCF
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xD0-0xDF
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xE0-0xEF
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xF0-0xFF

};
/*
int is_my_alnum(char * p)
{
	int ret = 0;
	if(*p > 0 && al_num_map[(int)*p])
	{
		ret = 1;
	}
	else if(IS_GB_1234(p) || IS_GB_ABCD(p))
	{
		ret = 2;
	}

	return ret;
}

int is_my_num(char * p)
{
	int ret = 0;
	if(*p > 0 && (al_num_map[(int)*p] & 2))
	{
		ret = 1;
	}
	else if(IS_GB_1234(p))
	{
		ret = 2;
	}

	return ret;
}

int is_my_alpha(char * p)
{
	int ret = 0;
	if(*p > 0 && (al_num_map[(int)*p] & 1))
	{
		ret = 1;
	}
	else if(IS_GB_ABCD((u_char*)p))
	{
		ret = 2;
	}
	return ret;
}
*/

scw_ver_map * create_version_map()
{
	const char * where = "create_version_map";
	scw_ver_map * pvm = NULL;
	if( (pvm = (scw_ver_map *)calloc(1,sizeof(scw_ver_map))) == NULL
			|| (pvm->m_sver_dict = db_creat(MAX_VERSION_NUM,-1)) == NULL
			|| (pvm->m_dver_dict = db_creat(MAX_VERSION_NUM,-1)) == NULL )
	{
		ul_writelog(UL_LOG_NOTICE,"error in %s!\n",where);
		return NULL;
	}

	if((pvm->m_strbuf = (char *)calloc(MAX_VERSION_NUM*10,sizeof(char))) == NULL)
	{
		db_del(pvm->m_sver_dict);
		db_del(pvm->m_dver_dict);
		free(pvm);
		pvm = NULL;

		ul_writelog(UL_LOG_NOTICE,"error in %s!\n",where);
		return NULL;
	}

	pvm->m_bufpos = 0;
	pvm->m_bufsize = MAX_VERSION_NUM*10;
	pvm->m_sver_count = 0;
	pvm->m_dver_count = 0;

	memset(pvm->m_ver_table,0,sizeof(pvm->m_ver_table));
	return pvm;
}

void destroy_version_map(scw_ver_map * pvm)
{
	if(pvm == NULL)
		return;
	db_del(pvm->m_sver_dict);
	db_del(pvm->m_dver_dict);
	free(pvm->m_strbuf);
	free(pvm);

	return;
}

int version_insert(scw_ver_map * pvm,char * ver_str,int len,int ver_type,int * pcode)
{
	const char * where = "version_insert";
	Sdict_snode snode;
	Sdict_build * psdict = NULL;
	u_int * pver_count;
	int ret = 0;
	char * newbuf = NULL;

	creat_sign_fs64(ver_str,len,&(snode.sign1),&(snode.sign2));

	if(ver_type == 0)
	{
		psdict = pvm->m_sver_dict;
		pver_count = &(pvm->m_sver_count);
	}
	else
	{
		psdict = pvm->m_dver_dict;
		pver_count = &(pvm->m_dver_count);
	}

	if((ret = db_op1(psdict,&snode,SEEK)) < 0)
	{
		ul_writelog(UL_LOG_NOTICE,"error in %s!\n",where);
		return -1;
	}
	else if(ret == 1)
	{
		*pcode = snode.code;
		return 0;
	}
	else
	{
		if((*pver_count)+1 > MAX_VERSION_NUM)
		{
			return -1;
		}
		while(pvm->m_bufsize < pvm->m_bufpos + len + 1)
		{
			if((newbuf = (char*)realloc(pvm->m_strbuf,2*pvm->m_bufsize)) == NULL)
			{
				return -1;
			}

			pvm->m_strbuf = newbuf;
			pvm->m_bufsize *= 2;
		}

		snode.code = *pver_count;
		snode.other = pvm->m_bufpos;

		if((ret=db_op1(psdict,&snode,ADD)) < 0)
		{
			ul_writelog(UL_LOG_NOTICE,"error in %s!\n",where);
			return -1;
		}

		memcpy(pvm->m_strbuf + pvm->m_bufpos,ver_str,len);
		pvm->m_strbuf[pvm->m_bufpos + len] = 0;
		pvm->m_bufpos += len+1;
		*pcode = *pver_count;
		(*pver_count)++;
	}

	return 1;
}

int get_vercode(scw_ver_map * pvm,const char* str,int ver_type)
{
	Sdict_build * psdict = NULL;
	Sdict_snode snode;
	int ret;

	if(ver_type == 0)
	{
		psdict = pvm->m_sver_dict;
	}
	else
	{
		psdict = pvm->m_dver_dict;
	}

	creat_sign_fs64((char*)str,strlen(str),&snode.sign1,&snode.sign2);
	ret=db_op1(psdict,&snode,SEEK);

	if(ret <= 0)
	{
		return -1;
	}
	else
	{
		return  snode.code;
	}
}

const char* get_version(scw_ver_map * pvm,int code,int ver_type)
{
	int bcount = 0;
	Sdict_bnode_block * pcblock = NULL;
	Sdict_bnode * pbnode = NULL;
	Sdict_build * psdict = NULL;
	u_int ver_count = 0;

	if(ver_type == 0)
	{
		psdict = pvm->m_sver_dict;
		ver_count = pvm->m_sver_count;
	}
	else
	{
		psdict = pvm->m_dver_dict;
		ver_count = pvm->m_dver_count;
	}

	if((code < 0) || (code >=(int)ver_count))
	{
		return NULL;
	}

	bcount = code / NODE_BLOCK_NUM;
	pcblock = psdict->nblock;

	for(int i=0;i<bcount;i++)
	{
		pcblock=pcblock->next;
	}

	pbnode = pcblock->nodes + code % NODE_BLOCK_NUM;

	return pvm->m_strbuf + pbnode->other;
}

int load_verinfo(char* verfile,scw_ver_map* pvm,char* dictver)
{
	const char * where = "get_version_map";
	char line[MAX_IL_LEN];
	FILE * fp = fopen(verfile,"r");
	char * p = NULL;
	int len = 0;
	int flag = 0;//0 seg 1 dict
	int pos = 0;
	int seg_ver_set[MAX_VERSION_NUM] = {0};
	int dict_ver_set[MAX_VERSION_NUM] = {0};
	int seg_pos = 0;
	int dict_pos = 0;
	line[0] = '\0';

	if(fp == NULL)
	{
		return 0;
	}

	while(fgets(line,sizeof(line),fp))
	{
		seg_pos = 0;
		dict_pos = 0;
		flag = 0;

		len = strlen(line);
		if(len <= 0)
			continue;
		while(line[len-1] == '\n'|| line[len-1] == '\r')
		{
			len--;
			if(len <= 0)
				break;
		}
		line[len] = 0;

		if(len == 0 || line[0] == '#')
			continue;

		p = strtok(line," ");

		if(strcmp(p,SCW_DICTVER_TAG) == 0)
		{
			p = strtok (NULL, " "); 
			if(strcmp(p,":") == 0)
			{
				p = strtok (NULL, " ");
				if(p)
				{
					strncpy(dictver,p,MAX_WORD_LEN);
					dictver[MAX_WORD_LEN-1] = 0;
					continue;
				}
			}
		}
		else if(strcmp(p,SCW_VERMAP_TAG) == 0)
		{
			while ((p = strtok(NULL," ")))
			{
				if(strcmp(p,":") == 0)
				{
					flag = 1;
					continue;
				}

				if(version_insert(pvm,p,strlen(p),flag,&pos) >= 0)
				{
					if(flag == 0)
						seg_ver_set[seg_pos++] = pos;
					else
						dict_ver_set[dict_pos++] = pos;
				}
				else
				{
					ul_writelog(UL_LOG_NOTICE,"error in %s!\n",where);
					return -1;
				}
			}

			for(int i=0;i<seg_pos;i++)
			{
				for(int j=0;j<dict_pos;j++)
				{
					pvm->m_ver_table[seg_ver_set[i]][dict_ver_set[j]] = 1;
				}
			}
		}
	}

	fclose(fp);

	return 1;
}

void get_matched_version(scw_ver_map * pvm,int pos,int flag,char * buf)
{
	if( pos <0 || pos >= MAX_VERSION_NUM )
		return;

	char *p = NULL;
	int len = 0;
	int ret =0;
	u_int count = 0;
	buf[len] = 0;

	count = pvm->m_sver_count;
	if(count < pvm->m_dver_count)
	{
		count = pvm->m_dver_count;
	}

	for(int i = 0;i<(int)count;i++)
	{
		if(flag == 0)
			ret = pvm->m_ver_table[pos][i];
		else
			ret = pvm->m_ver_table[i][pos];  

		if(ret)
		{
			p = (char *)get_version(pvm,i,!flag);

			if(len + strlen(p) > MAX_HELPSTR_LEN)
				break;
			strcpy(buf+len,p);
			strcat(buf," * ");
			len += strlen(p)+3;
		}
	}
	if(len > 0)
		buf[len-1] = 0;
	else
		strcpy(buf,"NULL");
	return;
}

// If version matched, return 0, else return -1.
int version_check(char* sgv,char* dtv,scw_ver_map* pvm)
{
	int spos = 0;
	int dpos = 0;
	char shp[MAX_HELPSTR_LEN];
	char dhp[MAX_HELPSTR_LEN];
	char * p = NULL;
	char * q = NULL;
	char sgv_high[MAX_WORD_LEN];
	char dtv_high[MAX_WORD_LEN];

	if( strcmp(ALL_MATCHED_VER,sgv) == 0 || strcmp(ALL_MATCHED_VER,dtv) == 0)
	{
		return 0;
	}

	sgv_high[0] = 0;
	dtv_high[0] = 0;

	//check if the higher two level of versions matches
	p = strstr(sgv, "-");
	if(p!=NULL && p+1!=NULL && (q=strstr(p+1, "-")) != NULL)
	{
		strncpy(sgv_high, sgv, q-sgv);
		sgv_high[q-sgv] = 0;
	}
	p = strstr(dtv, "-");
	if(p!=NULL && p+1!=NULL && (q=strstr(p+1, "-")) != NULL)
	{
		strncpy(dtv_high, dtv, q-dtv);
		dtv_high[q-dtv] = 0;
	}
	if(sgv_high[0] != 0 && dtv_high[0] != 0)
	{
		spos = get_vercode(pvm,sgv_high,0);
		dpos = get_vercode(pvm,dtv_high,1);
		get_matched_version(pvm,spos,0,shp);
		get_matched_version(pvm,dpos,1,dhp);
		if(spos >= 0 && dpos >= 0)
		{
			if(pvm->m_ver_table[spos][dpos])
				return 0;
		}
	}

	spos = get_vercode(pvm,sgv,0);
	dpos = get_vercode(pvm,dtv,1);

	get_matched_version(pvm,spos,0,shp);
	get_matched_version(pvm,dpos,1,dhp);

	if(spos < 0 && dpos >= 0)
	{
		ul_writelog(UL_LOG_WARNING,"Version mismatch occur between wordseg and worddict!\n"
				"\t\t\t* wordseg_%s has no matched worddict\n"
				"\t\t\t* worddict_%s should match wordseg_%s\n",sgv,dtv,dhp);
		return -1;
	}
	else if(spos >= 0 && dpos < 0)
	{
		ul_writelog(UL_LOG_WARNING,"Version mismatch occur between wordseg and worddict!\n"
				"\t\t\t* wordseg_%s should match worddict_%s\n"
				"\t\t\t* worddict_%s has no matched wordseg\n",sgv,shp,dtv);
		return -1;
	}
	else if(spos < 0 && dpos < 0)
	{
		ul_writelog(UL_LOG_WARNING,"Version mismatch occur between wordseg and worddict!\n"
				"\t\t\t* wordseg_%s has no matched worddict\n"
				"\t\t\t* worddict_%s has no matched wordseg\n",sgv,dtv);
		return -1;
	}

	if(pvm->m_ver_table[spos][dpos])
	{
		return 0;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING,"Version mismatch occur between wordseg and worddict!\n"
				"\t\t\t* wordseg_%s should match worddict_%s\n"
				"\t\t\t* worddict_%s should match wordseg_%s\n",sgv,shp,dtv,dhp);  
		return -1;        
	}
}

//  set_prop_by_str
/*========================================================================================
 * function : convert the string property to an integer. 
 * argu    : pstr, the string property
 * return  : the integer property.
 ========================================================================================*/
/*
void ch_set_prop_by_str(char*pstr,scw_property_t& property)
{
	unsigned i;

	for(i=0;i<strlen(pstr);i++)
	{
		switch(pstr[i])
		{
			case 'S' :
				SET_SPECIAL(property);
				break;
			case 'Z':
				SET_TEMP(property);
				break;
			case 'P' :
				SET_PHRASE(property);
				break;
			case 'O' :
				SET_ONEWORD(property);
				break;
			case '0' :
				SET_STOPWORD(property);
				break;
			case '1' :
				SET_SURNAME(property);
				break;
			case '2' :
				SET_QIANZHUI(property);
				break;
			case '3' :
				SET_HOUZHUI(property);
				break;
			case '4' :
				SET_QIANDAO(property);
				break;
			case '5' :
				SET_HOUDAO(property);
				break;
			case '6' :
				SET_CFOREIGN(property);
				break;
			case '7' :
				SET_NAMEMID(property);
				break;
			case '8' :
				SET_NAMEEND(property);
				break;
			case '9' :
				SET_NAME_STICK(property);
				break;
			case 'm' :
				SET_CNUMBER(property);
				break;
			case 't' :
				SET_TIME(property);
				break;
			case 'M' :
				SET_MARK(property);
				break;
			case 'H' :
				SET_HUMAN_NAME(property);
				break;
			case 'q':
				SET_QUANTITY(property);
				break;
			case 'Q':
				SET_QYPD(property);
				break;
			case 'B':
				SET_BOOKNAME(property);
				break;
			case 'b':
				SET_BOOKNAME_START(property);
				break;
			case 'e':
				SET_BOOKNAME_END(property);
				break;
			case 'A':
				SET_2WHPREFIX(property);
				break;
			case 'd':
				SET_DI(property);
				break;
			case 'C' : 
				SET_CH_NAME(property);
				break;    
			case 'R':
				SET_FMNAME(property);
				break;
			case 'u':
				SET_UNIVERSITY(property);
				break;
			case 'F':
				SET_SOFTNAME(property);
				break; 
			case 'D':
				SET_LOC_SMALL(property);
				break;
			case 'n':
				SET_BRAND(property);
				break;
			case 'J':
				SET_ORGNAME(property);
				break;
			case 'a':
				SET_ASCIIWORD(property);
				break;
			case 'E':
				SET_SPEC_ENSTR(property);
				break;
			case 'f':
				SET_LOC_FOLLOW(property);
				break;
			case 'l':
				SET_LOC(property);
				break;
			case 'x':
				SET_FNAMEBGN(property);
				break;
			case 'y':
				SET_FNAMEMID(property);
				break;
			case 'z':
				SET_FNAMEEND(property);
				break;
			case 'w':
				SET_2FNAMEEND(property);
				break;
			case 'N':
				SET_1W_SUFFIX(property);
				break;
			case 'U':
				SET_VIDEO(property);
				break;
			case 'V':
				SET_GAME(property);
				break;
			case 'W':
				SET_SONG(property);
				break;
			case 'c':
				SET_SCW_PRODUCT(property);
				break;
			case 'p':
				SET_PLACE_SUFFIX(property);
				break;
			case 'v':
				SET_STATIC_BOOK(property);
				break;
			default:
				break;
		}
	}

	return;
}
*/
void jp_set_prop_by_str(char*pstr,scw_property_t& property)
{
	unsigned i;

	for(i=0;i<strlen(pstr);i++)
	{
		switch(pstr[i])
		{
			case 'P' :
				SET_PHRASE(property);
				break;
			case 'U' :
				SET_BIG_PHRASE(property);
				break;
			case 'Z' :
				SET_TEMP(property);
				break;
			case 'O' :
				SET_ONEWORD(property);
				break;
			case 'M' :
				SET_MARK(property);
				break;
			case 'D':
				SET_CNUMBER(property);
				break;
			case 'R' :
				SET_HUMAN_NAME(property);
				break;
			case 'q':
				SET_QUANTITY(property);
				break;
			case 'p':
				SET_PLACE_SUFFIX(property);
				break;
			case 'N':
				SET_NOUN(property);
				break;
			case 'S':
				SET_SURNAME(property);
				break;
			case 'G':
				SET_GIVENNAME(property);
				break;
			case 'J':
				SET_ORGNAME(property);
				break;
			case 'L':
				SET_LOC(property);
				break;
			case 'm':
				SET_FMNAME(property);
				break;
			case 's':
				SET_SOFTNAME(property);
				break;
			case 'd':
				SET_DI(property);
				break;
			case '1':
				SET_PINGJM(property);
				break;
			case '2':
				SET_PIANJM(property);
				break;
			case 'F':
				SET_PAJMSTR(property);
				break;
			case '0':
				SET_BOOKNAME(property);
				break;
			case 'b':
				SET_BOOKNAME_START(property);
				break;
			case 'e':
				SET_BOOKNAME_END(property);
				break;
			case '7':
				SET_NAMEMID(property);
				break;
			case '8':
				SET_NAMEEND(property);
				break;
			case '9':
				SET_NAME_SUFFIX(property);
				break;
			case 'V':
				SET_VERB(property);
				break;
			case 'a':
				SET_ASCIIWORD(property);
				break;
			default :
				break;
		}
	}
	return ;
}

void set_prop_by_str(char* pstr,scw_property_t& property,int wdtype)
{
	switch(wdtype)
	{
		case SCW_WD_CH:
			ch_set_prop_by_str(pstr,property);
			break;
		case SCW_WD_JP:
			jp_set_prop_by_str(pstr,property);
			break;
		default:
			break;
	}

	return;
}




//  write_prop_to_str
/*========================================================================================
 * function : convert integer property to string format. 
 * argu    : property, the interger property
 *       : buffer, buffer to hold the string property result.
 ========================================================================================*/
/*
void ch_write_prop_to_str(scw_property_t property, char* buffer)
{
	buffer[0] = 0;

	if(IS_NEWWORD(property))
		strcat(buffer, "-NW");
	if(IS_SURNAME(property))
		strcat(buffer, "-1");
	if(IS_TEMP(property))
		strcat(buffer,"-Z");
	if(IS_NAMEMID(property))
		strcat(buffer, "-7");
	if(IS_NAMEEND(property))
		strcat(buffer, "-8");
	if(IS_NAME_STICK(property))
		strcat(buffer, "-9");
	if(IS_QIANZHUI(property))
		strcat(buffer, "-2");
	if(IS_HOUZHUI(property))
		strcat(buffer, "-3");
	if(IS_QIANDAO(property))
		strcat(buffer, "-4");
	if(IS_HOUDAO(property))
		strcat(buffer, "-5");
	if(IS_CFOREIGN(property))
		strcat(buffer, "-6");
	if(IS_STOPWORD(property))
		strcat(buffer, "-0");
	if(IS_MARK(property))
		strcat(buffer, "-M");
	if(IS_PHRASE(property))
		strcat(buffer, "-P");
	if(IS_SPECIAL(property))
		strcat(buffer, "-S");
	if(IS_ONEWORD(property))
		strcat(buffer, "-O");
	if(IS_CNUMBER(property))
		strcat(buffer, "-m");
	if(IS_DI(property))
		strcat(buffer, "-d");
	if(IS_TIME(property))
		strcat(buffer, "-t");
	if(IS_HUMAN_NAME(property))
		strcat(buffer, "-H");
	if(IS_QUANTITY(property))
		strcat(buffer, "-q");
	if(IS_QYPD(property))
		strcat(buffer, "-Q");
	if(IS_BOOKNAME(property))
		strcat(buffer, "-B");
	if(IS_BOOKNAME_START(property))
		strcat(buffer, "-b");
	if(IS_BOOKNAME_END(property))
		strcat(buffer, "-e");
	if(IS_2WHPREFIX(property))
		strcat(buffer, "-A");
	if(IS_CH_NAME(property))
		strcat(buffer, "-C");
	if(IS_UNIVERSITY(property))
		strcat(buffer,"-u");
	if(IS_LOC_SMALL(property))
		strcat(buffer,"-D");
	if(IS_SOFTNAME(property))
		strcat(buffer,"-F");
	if(IS_FMNAME(property))
		strcat(buffer,"-R");
	if(IS_ORGNAME(property))
		strcat(buffer,"-J");
	if(IS_ASCIIWORD(property))
		strcat(buffer,"-a");
	// asc串是否为数词
	if(IS_ASCIINUM(property))
		strncat(buffer,"-g",sizeof("-g"));
	if(IS_SPEC_ENSTR(property))
		strcat(buffer,"-E");
	if(IS_LOC(property))
		strcat(buffer,"-l");
	if(IS_LOC_FOLLOW(property))
		strcat(buffer,"-f");
	if(IS_FNAMEBGN(property))
		strcat(buffer,"-x");
	if(IS_FNAMEMID(property))
		strcat(buffer,"-y");
	if(IS_FNAMEEND(property))
		strcat(buffer,"-z");
	if(IS_2FNAMEEND(property))
		strcat(buffer,"-w");
	if(IS_1W_SUFFIX(property))
		strcat(buffer, "-N"); 
	if(IS_VIDEO(property))
		strcat(buffer, "-U");
	if(IS_GAME(property))
		strcat(buffer, "-V");
	if(IS_SONG(property))
		strcat(buffer, "-W");
	if(IS_SCW_PRODUCT(property))
		strcat(buffer, "-c");
	if(IS_BRAND(property))
		strcat(buffer, "-n");
	if(IS_PLACE_SUFFIX(property))
		strcat(buffer, "-p");
	if(IS_STATIC_BOOK(property))
		strcat(buffer, "-v");
}
*/

void jp_write_prop_to_str(scw_property_t& property, char* buffer)
{
	buffer[0] = 0;

	if(IS_JPNAME_BLANK(property))
		strcat(buffer, "-K");
	if(IS_BIG_PHRASE(property))
		strcat(buffer, "-U");
	if(IS_SURNAME(property))
		strcat(buffer, "-S");
	if(IS_ASCIIWORD(property))
		strcat(buffer,"-a");
	// asc串是否为数词
	if(IS_ASCIINUM(property))
		strncat(buffer,"-g",sizeof("-g"));
	if(IS_GIVENNAME(property))
		strcat(buffer, "-G");
	if(IS_NAMEMID(property))
		strcat(buffer,"-7");
	if(IS_NAMEEND(property))
		strcat(buffer,"-8");
	if(IS_LOC(property))
		strcat(buffer, "-L");
	if(IS_HUMAN_NAME(property))
		strcat(buffer, "-R");
	if(IS_ONEWORD(property))
		strcat(buffer, "-O");
	if(IS_QUANTITY(property))
		strcat(buffer, "-q");
	if(IS_CNUMBER(property))
		strcat(buffer, "-D");
	if(IS_MARK(property))
		strcat(buffer, "-M");
	if(IS_PHRASE(property))
		strcat(buffer, "-P");
	if(IS_ORGNAME(property))
		strcat(buffer, "-J");
	if(IS_FMNAME(property))
		strcat(buffer,"-m");
	if(IS_SOFTNAME(property))
		strcat(buffer,"-s");
	if(IS_PLACE_SUFFIX(property))
		strcat(buffer, "-p");
	if(IS_NOUN(property))
		strcat(buffer, "-N");
	if(IS_DI(property))
		strcat(buffer,"-d");
	if(IS_PINGJM(property))
		strcat(buffer,"-1");
	if(IS_PIANJM(property))
		strcat(buffer,"-2");
	if(IS_PAJMSTR(property))
		strcat(buffer,"-F");
	if(IS_BOOKNAME(property))
		strcat(buffer,"-0");
	if(IS_BOOKNAME_START(property))
		strcat(buffer,"-b");
	if(IS_BOOKNAME_END(property))
		strcat(buffer,"-e");
	if(IS_TEMP(property))
		strcat(buffer,"-Z");
	if(IS_VERB(property))
		strcat(buffer,"-V");
	if(IS_NAME_SUFFIX(property))
		strcat(buffer,"-9");
	return;
}

void write_prop_to_str(scw_property_t& property, char* buffer,int wdtype)
{
	switch(wdtype)
	{
		case SCW_WD_CH:
			ch_write_prop_to_str(property,buffer);
			break;
		case SCW_WD_JP:
			jp_write_prop_to_str(property,buffer);
			break;
		default:
			break;
	}
}




// extract_bracket
/*========================================================================================
 * function : extract the content between [ and ], the last ] must be following with ' '
 * argu    : value,size,  the string buffer and its size.
 *       : *begin, the beginning content
 *       : end, the pointer after "] "
 * return   : 1, success; other, failed.
 ========================================================================================*/
int extract_bracket(char * value,int size,char* begin,char** end)
{
	const char* where="extract_bracket";
	char *tmpend;
	int len;


	//assert(value && begin && end);
	//assert(size >0);

	//assert(*begin== '[');
	if(!value || !begin || !end || size<=0 || *begin!= '[')
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	if(*begin != '[')
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	tmpend=strstr(begin,"] ");
	if(tmpend== NULL)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	if(size < tmpend-begin)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -2;
	}
	len=tmpend-begin-1;
	memcpy(value,begin+1,len);
	value[len]='\0';
	*end=tmpend+2;

	return 1;
}


// convert_to_blanksep
/*========================================================================================
 * function : convert the dict format to blank format
 * argu    : psrc, input buffer
 *       : pdst, output bufer and its size
 * return   : 1, success; <0, failed.
 ========================================================================================*/
int convert_to_blanksep(char* psrc,char* pdst,int size)
{
	const char* where="convert_to_blanksep";
	int offset;
	char value[MAX_LEMMA_LENGTH];
	char* nextword;
	char* left;
	char* right;
	int remlen;
	int len;

	pdst[0]=0;
	nextword=psrc;
	remlen=size;
	while(*nextword!='\0')
	{
		offset=strtol(nextword,&left,10);
		if( (left==nextword) || (left==NULL) || ( *left!='(')||
				(offset >(int) MAX_LEMMA_LENGTH))
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		right=strchr(left+1,')');
		if((right== NULL)||(remlen-1 < right-left)
				||(right-left >(int) MAX_LEMMA_LENGTH)||(right-left==1))
		{
			ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
			return -1;
		}
		len=right-left-1;
		memcpy(value,left+1,len);
		value[len]=' ';
		value[len+1]=0;
		strcat(pdst,value);
		remlen=strlen(right+1);
		nextword=right+1;
	}
	return 1;
}

/*
int is_gbk_hz(unsigned char* pstr)
{
	char *where = "is_gbk_hz";
	//assert(pstr);
	if(!pstr)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	if(pstr[0]==0 || pstr[1]==0)
		return 0;

	if(pstr[1] == 0x7f)
		return 0;

	// if it is a gbk/2 word
	if ((pstr[0]>=0xB0) && (pstr[0]<=0xF7) &&
			(pstr[1]>=0xA1) && (pstr[1]<=0xFE))
	{
		return 1;
	}
	// if it is a gbk/3 word
	else if ((pstr[0]>=0x81) && (pstr[0]<=0xa0) &&
			(pstr[1]>=0x40) && (pstr[1]<=0xfe))
	{
		return 1;
	}
	// if it is a gbk/4 word
	else if ((pstr[0]>=0xaa) && (pstr[0]<=0xfe) &&
			(pstr[1]>=0x40) && (pstr[1]<=0xa0))
	{
		return 1;
	}
	else{
		return 0;
	}
}

// is_my_ascii
/========================================================================================
 * function : if the current char satisfy my definition. 
 * argu     : p, the pointer of the current char in string
 ========================================================================================*/
/*
int is_my_ascii(char * pstr)
{
	int ret=0;
	u_char ch = 0;
	u_char * p = (u_char*)pstr;

	if(*p > 0x80)
	{
		if(IS_GB_1234(p) || IS_GB_ABCD(p))
		{
			ret = 2;
		}
		else if(p[0] == 0xa3 && (p[1] == 0xab || p[1] == 0xad || p[1] == 0xa5 || p[1] == 0xa6))
		{
			ret = 2;
		}
		else if(p[0] == 0xa3 && (p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ch = *(p+2);
			if(ch!='\0' && ch!=' ' && ch!='\n' && ch!='\r' && (ch < 0x80 || ch == 0xa3))
			{
				ret = 2;
			}  
		}
	}
	else if(al_num_map[*p]!=0)
		ret = true;
	else if(*p=='+'||*p=='-'||*p=='&'||*p=='%')
		ret = true;
	else if(*p=='.'||*p=='@'||*p=='_')
	{
		ch = *(p+1);
		if(ch!='\0' && ch!=' ' && ch!='\n' && ch!='\r'&& (ch < 0x80 || ch == 0xa3))
			ret = 1;
	}

	return ret;
}
*/

// is_mid_mark
/*========================================================================================
 * function : if the current char is a middle mark. the term of 'middle mark' is a 
 *       : definition used in proc_ascii.
 * argu     : ch, the char
 ========================================================================================*/
/*int is_mid_mark(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80){
		if(*p == '@' || *p == '.'
				|| *p == '-' || *p == '+' 
				|| *p == '&' || *p == '_'
				|| *p == '%')
		{
			ret = 1;
		}
	}
	else{
		if(p[0] == 0xa3 &&
				(p[1] == 0xab || p[1] == 0xad || p[1] == 0xa5 || p[1] == 0xa6
				 || p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ret = 2;
		}
	}

	return ret;
}

int is_split_headmark(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80)
	{
		if( *p == '@' || *p == '.'
				|| *p == '-' || *p == '+' 
				|| *p == '%' || *p == '_')
		{
			ret = 1;
		}
	}
	else{
		if(p[0] == 0xa3 &&
				(p[1] == 0xab || p[1] == 0xad || p[1] == 0xa5
				 || p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ret = 2;
		}
	}

	return ret;
}
int is_split_tailmark(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80)
	{
		if(*p == '@' || *p == '.'
				|| *p == '-' || *p == '_' 
				|| *p == '&')
		{
			ret = 1;
		}
	}
	else{
		if(p[0] == 0xa3 &&
				(p[1] == 0xad || p[1] == 0xa6
				 || p[1] == 0xae || p[1] == 0xc0 || p[1] == 0xdf))
		{
			ret = 2;
		}
	}

	return ret;
}

int is_radix_point(char * pstr)
{
	int ret = 0;
	u_char * p = (u_char*)pstr;

	if(*p < 0x80 && *p == '.')
	{
		ret = 1;
	}
	else if(p[0] == 0xa3 && p[1] == 0xae)
	{
		ret = 2;
	}

	return ret;
}

int is_fourbit_point(const char * word)
{
	u_char * p = (u_char*)word;
	if((p[0] == 0x81 && p[1] ==0x39 && p[2] ==0xA7 && p[3] ==0x39))
		return 1;
	return 0;
}
*/
int is_gb_pajm(char * word)
{
	u_char * p = (u_char*)word;
	if((p[0] == 0xa5 && p[1] >= 0xa1 && p[1] <= 0xf6)
			||(p[0] == 0xa9 && p[1] == 0x60))
		return 1;
	return 0;
}

int is_gb_pijm(char * word)
{
	u_char * p = (u_char*)word;
	if((p[0] == 0xa4 && p[1] >= 0xa1 && p[1] <= 0xf3))
		return 1;
	return 0;
}
/*
inline u_int get_value_gb(const char * buf, int& pos, const int len)
{
	if(pos >= len)
		return 0;

	unsigned char *tmpbuf=(unsigned char*)(buf + pos);
	u_int value = tmpbuf[0];
	int off = 0;  
	off = 1;

	if((pos+1 < len) && (tmpbuf[0] >= 0x81 && tmpbuf[0] <= 0xfe))
	{
		// hanzi with 2 char
		if(tmpbuf[1] >= 0x40 && tmpbuf[1] <= 0xfe)
		{
			off = 2;
			value = tmpbuf[0] *256 + tmpbuf[1];
		}       
		// hanzi with 4 char
		else if((tmpbuf[1] >= 0x30 && tmpbuf[1] <= 0x39))
		{
			if( pos + 3 < len 
					&&(tmpbuf[2] >= 0x81 && tmpbuf[2] <= 0xfe)
					&&(tmpbuf[3] >= 0x30 && tmpbuf[3] <= 0x39))
			{
				value = tmpbuf[0]*16777216 + tmpbuf[1]*65536 + tmpbuf[2]*256 + tmpbuf[3];
				off = 4;
			}
		}
	}

	pos += off;
	return value;
}

int is_oneword_gb(char * word)
{
	u_char *tmpbuf=(u_char *)word;
	int len = strlen(word);

	if(len == 1)
		return 1;
	if(len == 2 && (tmpbuf[0] >= 0x81 && tmpbuf[0] <= 0xfe)
			&& (tmpbuf[1] >= 0x40 && tmpbuf[1] <= 0xfe))
		return 2;
	if(len == 4 && (tmpbuf[0] >= 0x81 && tmpbuf[0] <= 0xfe)
			&& (tmpbuf[1] >= 0x30 && tmpbuf[1] <= 0x39)
			&& (tmpbuf[2] >= 0x81 && tmpbuf[2] <= 0xfe)
			&& (tmpbuf[3] >= 0x30 && tmpbuf[3] <= 0x39))
		return 4;

	return 0;
}
*/
// scw_get_word
/*========================================================================================
 * function : get a word from the buffer
 *          : inbuf, the original buffer
 *          : pos, when input, it is the begin pos,when output,it will be
 *          :     the next position
 *          : slen, the buffer size
 *          : charset, the encoding system of the inbuf,But we only support
 *          :     GBK now.
 * return   : the value in unsigned  int
 ========================================================================================*/
u_int scw_get_word (const char *inbuf, int &pos, const int slen, const char *charset)
{
	u_int result = 0;

	if(pos >= slen)
		return 0;

	result = get_value_gb(inbuf,pos,slen);
	return result;
}



