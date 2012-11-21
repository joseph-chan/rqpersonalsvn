/*
 * Filename : scwdict.cpp -> segdict.cpp
 * Descript : implemation file dictionary management.
 * Author   : Paul Yang, zhenhaoji@sohu.com
 * Time     : 2004-12-31
 */

#include <math.h>

#include <ul_log.h>

#include "property.h"
#include "scwdef.h"
#include "international.h"
// <log>
#define LOG_ERROR(message)  ul_writelog(UL_LOG_FATAL, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
#define WARNING_LOG(message)  ul_writelog(UL_LOG_WARNING, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
// </log>

u_int scw_seek_entry(scw_worddict_t * pwdict,u_int lde, u_int value);
scw_lemma_t* mm_seek_lemma(scw_worddict_t* pwdict, char* buff, int slen, int& curpos, int basiconly);

extern bool g_use_crf;

extern void set_prop_by_str(char* pstr,scw_property_t& property,int wdtype);
extern int extract_bracket(char* value, int size, char* begin, char** end); 
//extern int is_gbk_hz(u_char* pstr);
//extern int * scw_get_nameprob(scw_worddict_t* pwdict,scw_lemma_t * plm,int type);
extern u_int scw_get_word(const char* inbuf, int &pos, const int slen, const char* charset);
extern scw_lemma_t* get_oneword_lemma(scw_worddict_t* pwdict, int wordvalue);

extern int version_check(char * sgv,char * dtv,scw_ver_map* pvm);
extern int load_verinfo(char * verfile,scw_ver_map * pvm,char * dictver);
extern scw_ver_map * create_version_map();
extern void destroy_version_map(scw_ver_map * pvm);

// static functions declared here
static inline u_int scw_get_hashsize(u_int* sufentry);
static inline u_int scw_get_backentry(u_int* sufentry);
static inline u_int scw_get_sufentry(u_int* sufentry, u_int pos);
static inline void scw_set_hashsize(u_int* sufentry, u_int hashsize);
static inline void scw_set_backentry(u_int* sufentry, u_int backentrypos);
static inline void scw_set_sufentry(u_int* sufentry, u_int pos, u_int sepos);
static inline void scw_init_sufentry(u_int* sufentry, u_int hashsize, u_int backsepos);
//static inline void scw_init_utilinfo(scw_utilinfo_t * puti,int wdtype);

static int scw_resize_wordbuf(scw_worddict_t * pwdict,u_int newsize);
static int scw_resize_phinfo(scw_worddict_t * pwdict,u_int newsize);
static int scw_resize_seinfo(scw_worddict_t * pwdict,u_int newsize);
static int scw_resize_dentry(scw_worddict_t * pwdict,u_int newsize);
static int scw_resize_lemma(scw_worddict_t * pwdict,u_int newsize);
int scw_resize_probbuf(scw_worddict_t * pwdict,u_int newsize);
static int scw_resize_ambinfo(scw_worddict_t * pwdict,u_int newsize);
int scw_resize_utilinfo(scw_worddict_t * pwdict,u_int newsize);
static int scw_adjust_seinfo(scw_worddict_t *  pwdict);

static int scw_insert_dentry(scw_worddict_t * pwdict,u_int lastpos,u_int value,u_int &curpos);
static int scw_insert_ambsuffix(scw_worddict_t* pwdict,u_int flmpos,u_int slmpos);
static void scw_init_ambsuffix(u_int * ambinfo,u_int hash);
//static int scw_add_lmprob(scw_worddict_t * pwdict,scw_lemma_t * plm,int * prob,int pbsize,int type);
//int scw_seek_ambfrag(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos);
int update_lemma_with_man(scw_worddict_t* pwdict, scw_inner_t *pir, scw_inlemma_t * pilemma,char * line);
int worddict2inner(scw_worddict_t* pwdict, scw_lemma_t *pwdictplm, scw_lemma_t* pirplm, scw_inner_t *pir);
extern int scw_seg(scw_worddict_t * pwdict,scw_inner_t * pir, const char* inbuf,const int inlen, bool human_allow_blank);
extern int get_lmstr(scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, char* dest, u_int destlen);
extern void reset_inner(scw_inner_t* pir);
extern int poly_lemmas(scw_worddict_t* pwdict, scw_inner_t* pir, int bpos, int count,int bname = 0);


// scw_create_worddict
/*========================================================================================
 * function : create a new dict for word-seperate ,with the default size
 * return   : the pointer to a empty worddict if success
 *          : NULL if failed
 *=======================================================================================*/
scw_worddict_t * scw_create_worddict (void)
{
	const char* where="scw_create_worddict";
	scw_worddict_t *pwdict=NULL;
	char* pwordbuf=NULL;
	u_int *pphinfo=NULL;
	u_int *pseinfo=NULL;
	u_int *pambinfo=NULL;
	int *probbuf=NULL;
	scw_lemma_t * plemma=NULL;
	scw_dictentry_t * pde=NULL;
	scw_utilinfo_t * puti=NULL;

	// allocate the memory for pwdict
	if ((pwdict = (scw_worddict_t *)calloc (1,sizeof (scw_worddict_t)))== NULL)
	{
		goto failed;
	}
	if((pwordbuf=(char*)calloc(DEFAULT_WORDBUF_SIZE,sizeof(char)))==NULL)
	{
		goto failed;
	}
	if((pphinfo=(u_int*)calloc(DEFAULT_INFO_SIZE,sizeof(u_int))) ==NULL)
	{
		goto failed;
	}
	if((pseinfo=(u_int*)calloc(DEFAULT_INFO_SIZE,sizeof(u_int)))==NULL)
	{
		goto failed;
	}
	if((pambinfo=(u_int*)calloc(DEFAULT_INFO_SIZE,sizeof(u_int)))==NULL)
	{
		goto failed;
	}
	if((probbuf=(int*)calloc(DEFAULT_INFO_SIZE*2,sizeof(int)))==NULL)
	{
		goto failed;
	}
	if((plemma=(scw_lemma_t*)calloc(DEFAULT_LEMMA_COUNT,sizeof(scw_lemma_t)))==NULL)
	{
		goto failed;
	}
	if((pde=(scw_dictentry_t*)calloc(DEFAULT_DE_SIZE,sizeof(scw_dictentry_t)))==NULL)
	{
		goto failed;
	}
	if((puti = (scw_utilinfo_t*)calloc(DEFAULT_INFO_SIZE,sizeof(scw_utilinfo_t)))==NULL)
	{
		goto failed;
	}

	pwdict->m_wordbuf=pwordbuf;
	pwdict->m_wbsize=DEFAULT_WORDBUF_SIZE;
	pwdict->m_wbpos=0;

	pwdict->m_phinfo=pphinfo;
	pwdict->m_phisize=DEFAULT_INFO_SIZE;
	pwdict->m_phipos=0;

	pwdict->m_seinfo=pseinfo;
	pwdict->m_seisize=DEFAULT_INFO_SIZE;
	pwdict->m_seipos=0;

	pwdict->m_ambinfo=pambinfo;
	pwdict->m_ambsize=DEFAULT_INFO_SIZE;
	pwdict->m_ambpos=0;

	pwdict->m_probbuf=probbuf;
	pwdict->m_pbsize=DEFAULT_INFO_SIZE;
	pwdict->m_pbpos=0;

	pwdict->m_dictentry=pde;
	pwdict->m_desize=DEFAULT_DE_SIZE;
	pwdict->m_depos=0;

	pwdict->m_lemmalist=plemma;
	pwdict->m_lmsize=DEFAULT_LEMMA_COUNT;
	pwdict->m_lmpos=0;

	pwdict->m_utilinfo = puti;
	pwdict->m_utilisize = DEFAULT_INFO_SIZE;
	pwdict->m_utilipos = 0;

	//assert(pwdict->m_seisize > 3);
	if(pwdict->m_seisize <= 3)
	{
		ul_writelog(UL_LOG_FATAL, "error in %s seinfo error\n", where);
		return NULL;
	}
	pwdict->m_entrance=0;
	scw_init_sufentry(pwdict->m_seinfo+pwdict->m_entrance,1,DENTRY_FIRST);
	pwdict->m_seipos=3;

	pwdict->m_wdtype = 0;

	pwdict->pdict_rsn_rule = NULL;
	pwdict->pdict_lsn_rule = NULL;

	return pwdict;

failed:
	ul_writelog(UL_LOG_FATAL, "error in %s\n", where); 
	free(pwdict);
	free(pwordbuf);
	free(pphinfo);
	free(pseinfo);
	free(pambinfo);
	free(plemma);
	free(pde);
	free(probbuf);
	free(puti);
	return NULL;
}

/*
   load worddict man 
   @param pwdict: worddict
   @param dictpath:worddict.man dict path
   @return 0 success -1 fail
   */
int scw_load_worddict_man(scw_worddict_t *pwdict, const char* dictpath)
{
	const char *where = "scw_load_worddict_man";
	scw_inner_t *pir = NULL;
	pir = scw_create_inner(SCW_MAX_INNER_TERM,SCW_OUT_ALL | SCW_OUT_PROP);
	if(pir == NULL)
	{
		ul_writelog(UL_LOG_WARNING, "scw_create_inner failed %s\n",where);
		return -1;
	}
	reset_inner(pir);	
	int i = 0;	
	int len = 0;

	FILE *fp = NULL; 
	char fullpath[MAX_WORD_LEN] = "\0";
	char line[MAX_WORD_LEN] = "\0";
	char *version= NULL;
	int dictver_len = strlen(pwdict->version);
	snprintf(fullpath, sizeof(fullpath), "%s/%s", dictpath, SCW_MANNAME);
	if((fp=fopen(fullpath,"r"))==NULL)
	{
		ul_writelog(UL_LOG_WARNING, "can't open worddict.man file in %s\n", where);
		goto failed;
	}
	fgets(line, MAX_WORD_LEN -1, fp);
	if(line[0] == '\0')
	{
		ul_writelog(UL_LOG_WARNING, "no version in worddict.man file %s \n", where);
		goto failed;
	}
	len = strlen(line);
	for(i = len - 1; i >=0; i --)
	{
		if(line[i] == '\n' || line[i] == '\r')
		{
			line[i] = 0;
			len --;
		}
		else
		{
			break;
		}
	}
	if(len <= 0 || line[0] == '\0')
	{ 
		ul_writelog(UL_LOG_WARNING, "error when get man file's version in %s\n", where);
		goto failed;
	}

	version = strstr(line, "ver:");
	if(version == NULL)
	{
		ul_writelog(UL_LOG_WARNING, "can't get conf version %s\n", where);
		goto failed;
	}
	version += 4;
	pwdict->version[dictver_len++] = '-';
	memcpy(pwdict->version + dictver_len, version, line + strlen(line) - version );
	dictver_len += strlen(line);
	pwdict->version[dictver_len] = 0;


	//为动态词申请空间
	if((pwdict->m_dynbuf=(char*)calloc(DEFAULT_WORDBUF_SIZE,sizeof(char)))==NULL)
	{
		goto failed;
	}

	if((pwdict->m_dynphinfo=(u_int*)calloc(DEFAULT_INFO_SIZE,sizeof(u_int))) ==NULL)
	{
		goto failed;
	}
	pwdict->m_dynb_size=DEFAULT_WORDBUF_SIZE;
	pwdict->m_dynb_curpos=0;

	pwdict->m_dynph_size=DEFAULT_INFO_SIZE;
	pwdict->m_dynph_curpos=0;

	while(fgets(line, MAX_WORD_LEN -1 , fp ))
	{
		char token[MAX_WORD_LEN] = "\0";
		char info[MAX_WORD_LEN] = "\0";
		scw_inlemma_t inlemma;
		Sdict_build * pdynwdict = NULL;

		len = strlen(line);
		for(i = len - 1; i >=0; i --)
		{
			if(line[i] == '\n' || line[i] == '\r')
			{
				line[i] = 0;
				len --;
			}
			else
			{
				break;
			}
		}
		for(i = 0; i < len; i ++)
		{
			if(line[i] == ' ')
			{
				break;
			}
			else
			{
				token[i] = line[i];
			}
		}
		token[i] = 0;
		i ++;
		if(len - i <= 0)
		{
			continue;
		}
		memcpy(info, line+i, len - i);
		info[len-i] = 0;

		if(token[0] == '\0')
		{
			ul_writelog(UL_LOG_WARNING, "the format of line %s is wrong, can't be loaded\n", line);
			continue;
		}

		if(strncmp(token, "-N", 2) == 0)
		{
			if(update_lemma_with_line(pwdict, pdynwdict,&inlemma, info) < 0)
			{
				ul_writelog(UL_LOG_WARNING, "Add lemma of newword error.Line=[%s]\n",info);
				continue;
			}
			SET_MANWORD(inlemma.m_property);
			if(scw_add_lemma_man(pwdict,pdynwdict,inlemma) < 0)
			{
				ul_writelog(UL_LOG_WARNING, "Add lemma of newword error.Line=[%s]\n",info);
				continue;
			}
		}
		if(strncmp(token, "-M", 2) == 0)
		{
			if(update_lemma_with_man(pwdict,pir, &inlemma, info) < 0)
			{
				ul_writelog(UL_LOG_WARNING, "Add lemma of man error.Line=[%s]\n",info);
				continue;
			}
			SET_MANWORD(inlemma.m_property);
			if(scw_add_lemma_man(pwdict, pdynwdict, inlemma) <0)
			{
				ul_writelog(UL_LOG_WARNING, "Add lemma of man error.Line = [%s]\n", info);
				continue;
			}
		}
	}
	if(pir != NULL)
	{
		scw_destroy_inner(pir);
		pir = NULL;
	}

	if(fp)
	{
		fclose(fp);
	}

	return 0;

failed:
	if(pir != NULL)
	{
		scw_destroy_inner(pir);
		pir = NULL;
	}

	if(pwdict->m_dynbuf)
	{
		free(pwdict->m_dynbuf);
	}
	if(pwdict->m_dynphinfo)
	{
		free(pwdict->m_dynphinfo);
	}
	if(fp)
	{
		fclose(fp);
	}

	return -1;
}

// scw_load_worddict
/*========================================================================================
 * function : load worddict from the file
 * argu     : fullname
 * return   : the pointer to worddict if success
 *          : NULL if failed
 ========================================================================================*/
scw_worddict_t* scw_load_worddict(const char* dictpath, int code , int language)
{
	const char* where="scw_load_worddict";
	char fullpath[MAX_WORD_LEN]={};
	char verfullpath[MAX_WORD_LEN]={};
	char dictver[MAX_WORD_LEN]={};
	char confName[MAX_WORD_LEN]={};
	int confVal = 0;
	int dictver_len = 0;
	scw_worddict_t * pwdict = NULL;
	scw_ver_map * pvm = NULL;
	FILE* fp=NULL;

	sprintf(verfullpath, "%s/%s",dictpath, SCW_VERINFONAME);

	init_function(code, language);
	if((pvm = create_version_map()) == NULL)
	{
		goto failed;
	}

	if( load_verinfo(verfullpath,pvm,dictver) <= 0 || 
			version_check(SEGVER,dictver,pvm) < 0 )
	{
		goto failed;
	}

	if((pwdict=(scw_worddict_t*)calloc(1,sizeof(scw_worddict_t)))==NULL)
	{
		goto failed;
	}

	//get worddict version
	dictver_len = strlen(dictver);
	if(dictver_len >= SCW_MAX_VERSION_LEN)
	{
		ul_writelog(UL_LOG_FATAL, "length of dictver is long than SCW_MAX_VERSION_LEN in %s", where);
		goto failed;
	}
	strncpy(pwdict->version, dictver, dictver_len);
	pwdict->version[dictver_len] = 0;

	sprintf(fullpath, "%s/%s", dictpath, SCW_DICTNAME);

	if((fp=fopen(fullpath,"rb"))==NULL)
	{
		goto failed;
	}

	// read wordbuf
	if(fread(&(pwdict->m_wbpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if((pwdict->m_wordbuf=(char*)malloc(pwdict->m_wbpos+4))==NULL)
	{
		goto failed;
	}
	pwdict->m_wbsize=pwdict->m_wbpos+4;

	if(fread(pwdict->m_wordbuf,sizeof(char),pwdict->m_wbpos,fp)!=pwdict->m_wbpos)
	{
		goto failed;
	}

	// read phinfo
	if(fread(&(pwdict->m_phipos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if((pwdict->m_phinfo=(u_int*)
				calloc(pwdict->m_phipos+1,sizeof(u_int)))==NULL)
	{
		goto failed;
	}
	pwdict->m_phisize=pwdict->m_phipos+1;

	if(fread(pwdict->m_phinfo,sizeof(u_int),pwdict->m_phipos,fp)!=pwdict->m_phipos)
	{
		goto failed;
	}

	// read seinfo
	if(fread(&(pwdict->m_seipos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if((pwdict->m_seinfo=(u_int*)
				calloc(pwdict->m_seipos+1,sizeof(u_int)))==NULL)
	{
		goto failed;
	}
	pwdict->m_seisize=pwdict->m_seipos+1;

	if(fread(pwdict->m_seinfo,sizeof(u_int),pwdict->m_seipos,fp)!=pwdict->m_seipos)
	{
		goto failed;
	}

	//read ambinfo
	if(fread(&(pwdict->m_ambpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if((pwdict->m_ambinfo=(u_int*)
				calloc(pwdict->m_ambpos+1,sizeof(u_int)))==NULL)
	{
		goto failed;
	}
	pwdict->m_ambsize=pwdict->m_ambpos+1;

	if(fread(pwdict->m_ambinfo,sizeof(u_int),pwdict->m_ambpos,fp)!=pwdict->m_ambpos)
	{
		goto failed;
	}

	// read dictentry
	if(fread(&(pwdict->m_depos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if((pwdict->m_dictentry=(scw_dictentry_t*)calloc(pwdict->m_depos+1,sizeof(scw_dictentry_t)))==NULL)
	{
		goto failed;
	}
	pwdict->m_desize=pwdict->m_depos+1;

	if(fread(pwdict->m_dictentry,sizeof(scw_dictentry_t),pwdict->m_depos,fp)
			!=pwdict->m_depos)
	{
		goto failed;
	}

	// read lemmalist
	if(fread(&(pwdict->m_lmpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	//malloc more for pwdict->m_lemmalist
	if((pwdict->m_lemmalist=(scw_lemma_t*)calloc(pwdict->m_lmpos + 1,sizeof(scw_lemma_t)))==NULL)
	{
		goto failed;
	}
	pwdict->m_lmsize=pwdict->m_lmpos +1;

	if(fread(pwdict->m_lemmalist,sizeof(scw_lemma_t),pwdict->m_lmpos,fp)
			!=pwdict->m_lmpos)
	{
		goto failed;
	}

	// read the entrance
	if(fread(&(pwdict->m_entrance),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	//read prop buf
	if(fread(&(pwdict->m_pbpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if((pwdict->m_probbuf=(int*)
				calloc(pwdict->m_pbpos+1,sizeof(int)))==NULL)
	{
		goto failed;
	}

	pwdict->m_pbsize=pwdict->m_pbpos+1;

	if(fread(pwdict->m_probbuf,sizeof(int),pwdict->m_pbpos,fp)
			!=pwdict->m_pbpos)
	{
		goto failed;
	}

	//read utilinfo
	if(fread(&(pwdict->m_utilipos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if((pwdict->m_utilinfo=(scw_utilinfo_t *)
				calloc(pwdict->m_utilipos+1,sizeof(scw_utilinfo_t))) == NULL)
	{
		goto failed;
	}

	pwdict->m_utilisize=pwdict->m_utilipos+1;

	if(fread(pwdict->m_utilinfo,sizeof(scw_utilinfo_t),pwdict->m_utilipos,fp)
			!= pwdict->m_utilipos)
	{
		goto failed;
	}

	//read wdtype
	if(fread(&(pwdict->m_wdtype),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}



	if(pwdict->m_wdtype & SCW_WD_CH)
	{
		sprintf(fullpath, "%s/", dictpath);
		if((pwdict->pdict_lsn_rule = ds_load(fullpath, SCW_LSN_RULEFILE)) == NULL) 
		{
			ul_writelog(UL_LOG_FATAL,"error: loading left rule dict failed\n");
			return NULL;
		}

	 	if((pwdict->pdict_rsn_rule = ds_load(fullpath, SCW_RSN_RULEFILE)) == NULL) 
		{
			ul_writelog(UL_LOG_FATAL,"error: loading left rule dict failed\n");
			return NULL;
		}
	}
	
	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}

	/*-------read configure file-------------*/
	sprintf(fullpath, "%s/%s", dictpath, SCW_CONFIGURE);

	if((fp=fopen(fullpath,"r"))==NULL)
	{
		ul_writelog(UL_LOG_WARNING, "load conf file failed: %s\n", fullpath);
	}
	else
	{
		confName[0]=0;
		confVal=0;
		while(fscanf(fp,"%s %d\n",confName, &confVal)!=EOF)
		{
			if(strcmp(confName, "crf") == 0 )
			{
				if(confVal == 0)
				{
					WARNING_LOG("Close CRF!");
					set_close_crf();
				}
			}
		}
		fclose(fp);
    		fp = NULL;
	}


	/*-----------initilization of crf model-----*/
	pwdict->crf_model=NULL;
	pwdict->newworddict=NULL;
	if(g_use_crf == true)
	{
		if( (pwdict->crf_model = new CrfModel() ) == NULL)
		{
			goto failed;
		}

		sprintf(fullpath, "%s/%s", dictpath, CRF_MODEL_NAME);
		if(crf_load_model(pwdict->crf_model,fullpath ) < 0)
		{
			ul_writelog(UL_LOG_FATAL, "Cannot load %s, set_close_crf()! Segmenting with wordseg139 now!\n", fullpath);       
			set_close_crf();
			//        goto failed;
		}
    	}
    

	if(g_use_crf == true)
	{
		sprintf(fullpath, "%s/%s", dictpath, SCW_NEWWORDDICT);
		if(fullpath[0]=='/')
			pwdict->newworddict = odb_load_search_mmap("", fullpath);
		else
			pwdict->newworddict = odb_load_search_mmap(".", fullpath);
		if(pwdict->newworddict ==NULL || pwdict->newworddict == (sodict_search_t*) ODB_LOAD_NOT_EXISTS )
		{
			ul_writelog(UL_LOG_FATAL, "Cannot load %s\n", fullpath);
			goto failed;
		}
	}

	pwdict->multitermdict=NULL;
	sprintf(fullpath, "%s/%s", dictpath, SCW_MULTIDICT);
	if((pwdict->multitermdict = dm_binarydict_load(fullpath)) ==NULL )
	{
		ul_writelog(UL_LOG_FATAL, "Cannot load %s\n", fullpath);
		goto failed;
	}
	if(scw_load_worddict_man(pwdict, dictpath) < 0)
	{
		ul_writelog(UL_LOG_WARNING, "load worddict.man file failed: %s\n", where);
	}

	destroy_version_map(pvm);
	return pwdict;

failed:
	if(pwdict != NULL)
	{
		scw_destroy_worddict(pwdict);
		pwdict = NULL;
	}
	destroy_version_map(pvm);

	if(fp != NULL)
	{
		fclose(fp);
        fp = NULL;
	}
	ul_writelog(UL_LOG_FATAL, "error in %s\n", where); 
	return NULL;
}
//


// scw_destroy_worddict
/*========================================================================================
 * function : free the the scw_worddict_t
 * argu     : pwdict : the pointer to  dict
 *=======================================================================================*/
void scw_destroy_worddict(scw_worddict_t * pwdict)
{
	char *where = "scw_destroy_worddict";  
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_FATAL, "error in %s pwdict NULL\n", where);
		return;
	}

	if(pwdict != NULL)
	{
		free(pwdict->m_wordbuf);
		free(pwdict->m_phinfo);
		if(pwdict->m_dynbuf)
		{
			free(pwdict->m_dynbuf);
		}
		if(pwdict->m_dynphinfo)
		{
			free(pwdict->m_dynphinfo);
		}
		free(pwdict->m_seinfo);
		free(pwdict->m_ambinfo);
		free(pwdict->m_dictentry);
		free(pwdict->m_lemmalist);
		free(pwdict->m_probbuf);
		free(pwdict->m_utilinfo);    

		if(pwdict->crf_model != NULL)
		{
			delete(pwdict->crf_model);
			pwdict->crf_model=NULL;
		}

		if(pwdict->newworddict !=NULL && pwdict->newworddict != (sodict_search_t*) ODB_LOAD_NOT_EXISTS )
		{   
			odb_destroy_search_mmap(pwdict->newworddict);
			pwdict->newworddict=NULL;
		}

		if(pwdict->multitermdict  != NULL)
		{      
			dm_dict_del(pwdict->multitermdict);
			pwdict->multitermdict=NULL;
		}

		if(pwdict->pdict_lsn_rule != NULL){
			ds_del(pwdict->pdict_lsn_rule);
		}
		if(pwdict->pdict_rsn_rule != NULL){
			ds_del(pwdict->pdict_rsn_rule);
		}

		free(pwdict);   
		pwdict=NULL;
	}  
}
// 


// scw_resize_wordbuf
/*========================================================================================
 * function : resize the word buf
 * argu     : pwdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 ========================================================================================*/
int scw_resize_wordbuf(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_wordbuf";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s pwdict NULL\n", where);
		return -1;
	}

	char * newwordbuf;

	if(newsize < pwdict->m_wbpos)
	{    
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_wbsize)
	{
		return 0;
	}
	if((newwordbuf=(char*)realloc(pwdict->m_wordbuf,newsize))!=NULL)
	{
		//success
		pwdict->m_wordbuf=newwordbuf;
		pwdict->m_wbsize=newsize;
		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}

// scw_resize_phinfo
/*========================================================================================
 * function : resize the phrase info list
 * argu     : pwdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 ========================================================================================*/
int scw_resize_phinfo(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_phinfo";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	u_int* newphinfo;

	if(newsize < pwdict->m_phipos)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_phisize)
	{
		return 0;
	}
	if( (newphinfo=(u_int*)realloc(pwdict->m_phinfo,newsize*sizeof(u_int))) != NULL )
	{
		//success
		pwdict->m_phinfo=newphinfo;
		pwdict->m_phisize=newsize;
		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}
// 

// scw_resize_seinfo
/*========================================================================================
 * function : resize the sufentry info list
 * argu     : pwdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 *=======================================================================================*/
int scw_resize_seinfo(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_seinfo";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	u_int* newseinfo;

	if(newsize < pwdict->m_seipos)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_seisize)
	{
		return 0;
	}
	if((newseinfo=(u_int*)realloc(pwdict->m_seinfo,newsize*sizeof(u_int)))!=NULL)
	{
		//success
		pwdict->m_seinfo=newseinfo;
		pwdict->m_seisize=newsize;
		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}
//     

int scw_resize_ambinfo(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_ambinfo";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	u_int* newambinfo;

	if(newsize < pwdict->m_ambsize)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_ambsize)
	{
		return 0;
	}
	if((newambinfo=(u_int*)realloc(pwdict->m_ambinfo,newsize*sizeof(u_int)))!=NULL)
	{
		//success
		pwdict->m_ambinfo=newambinfo;
		pwdict->m_ambsize=newsize;
		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}
//
int scw_resize_utilinfo(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_utilinfo";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	scw_utilinfo_t * newutilinfo;

	if(newsize < pwdict->m_utilisize)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_utilisize)
	{
		return 0;
	}
	if((newutilinfo=(scw_utilinfo_t*)realloc(
					pwdict->m_utilinfo,newsize*sizeof(scw_utilinfo_t)))!=NULL)
	{
		//success
		pwdict->m_utilinfo=newutilinfo;
		pwdict->m_utilisize=newsize;
		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}
// 

// scw_resize_propbuf
/*========================================================================================
 * function : resize the prop buf
 * argu     : pwdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 *=======================================================================================*/
int scw_resize_probbuf(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_probbuf";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	int* newprobbuf;

	if(newsize < pwdict->m_pbsize)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_pbsize)
	{
		return 0;
	}
	if((newprobbuf=(int*)realloc(pwdict->m_probbuf,newsize*sizeof(int)))!=NULL)
	{
		//success
		pwdict->m_probbuf=newprobbuf;
		pwdict->m_pbsize=newsize;

		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}

//     


// scw_resize_dentry
/*========================================================================================
 * function : resize the dictentry list
 * argu     : pwdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 =======================================================================================*/
int scw_resize_dentry(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_dentry";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	scw_dictentry_t * pde;

	if(newsize < pwdict->m_depos)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_desize)
	{
		return 0;
	}
	if((pde=(scw_dictentry_t*)realloc(pwdict->m_dictentry,newsize*sizeof(scw_dictentry_t)))!=NULL)
	{
		//success
		pwdict->m_dictentry=pde;
		pwdict->m_desize=newsize;
		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}
//


// scw_resize_lemma
/*========================================================================================
 * function : resize the lemma list
 * argu     : pwdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 ========================================================================================*/
int scw_resize_lemma(scw_worddict_t * pwdict,u_int newsize)
{
	const char* where="scw_resize_lemma";
	//assert(pwdict);
	if(!pwdict)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	scw_lemma_t * plemmalist;

	if(newsize < pwdict->m_lmpos)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		return -1;
	}
	if(newsize == pwdict->m_lmsize)
	{
		return 0;
	}


	if((plemmalist=(scw_lemma_t*)realloc(pwdict->m_lemmalist,newsize*sizeof(scw_lemma_t))) != NULL)
	{
		//success
		pwdict->m_lemmalist=plemmalist;
		pwdict->m_lmsize=newsize;
		return 1;
	}
	else
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
		// failed
		return -2;
	}
}
// 


/*!
 * Adjust the sufentry info list. Because when add item, the hash size of the suffix entry list will be changed, 
 * so a new suffix entry buffer will be used, and the last one will drop to unused but cannot be used again. 
 * We must adjust the suffix to convert the unused hole to useful.
 * @param <pwdict> - the worddict pointer.
 * @return 1 if succeeded; -1 if the worddict is inconsistent.
 */
int scw_adjust_seinfo(scw_worddict_t*  pwdict)
{  
	u_int nextentry=0;
	u_int newpos=0;
	u_int curde;
	u_int hsize;

	while(nextentry < pwdict->m_seipos)
	{
		curde = scw_get_backentry(pwdict->m_seinfo + nextentry);
		hsize = scw_get_hashsize(pwdict->m_seinfo + nextentry);

		if((curde != DENTRY_FIRST) && (curde >= pwdict->m_depos))
		{
			// inconsistent
			WARNING_LOG("error: illegal value"); 
			return -1;
		}

		if(((curde==DENTRY_FIRST) && (pwdict->m_entrance!=nextentry)) || 
				((curde!=DENTRY_FIRST) && (pwdict->m_dictentry[curde].m_suffix_pos!=nextentry) ) )
		{
			// not used, move the next 
			nextentry += hsize + 2;
		}
		else
		{
			// used 
			if(nextentry != newpos)
			{
				// need move 
				memmove(pwdict->m_seinfo+newpos,pwdict->m_seinfo+nextentry,(hsize+2)*sizeof(u_int));
				if(curde != DENTRY_FIRST)
				{
					pwdict->m_dictentry[curde].m_suffix_pos=newpos;
				}
				else
				{
					pwdict->m_entrance=newpos;
				}
			}
			// meve the two point to next;
			nextentry += hsize+2;
			newpos += hsize+2;
		}
	}

	if(pwdict->m_seipos != nextentry)
	{
		WARNING_LOG("error: illegal value"); 
		return -1;
	}
	pwdict->m_seipos=newpos;

	return 1;
}
//


// scw_save_worddict
/*========================================================================================
 * function : save the worddict to disk
 * argu     : pwdict, the worddict
 *          : path, the dict path
 *          : name, the main name of the dict
 * return   : 1 if success,
 *          : <0 if failed
 *          : -1 if open file error
 *          : -2 write failed.
 *          : -3 worddict is inconsistent
 ========================================================================================*/
int scw_save_worddict( scw_worddict_t * pwdict,const char* fullname)
{  
	FILE* fp;

	if(scw_adjust_seinfo(pwdict)<0)
	{
		WARNING_LOG("error: scw_adjust_seinfo() failed"); 
		return -3;
	}

	if( (fp=fopen(fullname,"wb")) == NULL )
	{
		WARNING_LOG("error: opening file failed"); 
		return -1;
	}

	if(fwrite(&(pwdict->m_wbpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}
	if(fwrite(pwdict->m_wordbuf,sizeof(char),pwdict->m_wbpos,fp) != pwdict->m_wbpos)
	{
		goto failed;
	}
	if(fwrite(&(pwdict->m_phipos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}
	if(fwrite(pwdict->m_phinfo,sizeof(u_int),pwdict->m_phipos,fp) != pwdict->m_phipos)
	{
		goto failed;
	}

	if(fwrite(&(pwdict->m_seipos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}
	if(fwrite(pwdict->m_seinfo,sizeof(u_int),pwdict->m_seipos,fp)
			!=pwdict->m_seipos)
	{
		goto failed;
	}

	if(fwrite(&(pwdict->m_ambpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}
	if(fwrite(pwdict->m_ambinfo,sizeof(u_int),pwdict->m_ambpos,fp)
			!=pwdict->m_ambpos)
	{
		goto failed;
	}

	if(fwrite(&(pwdict->m_depos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}
	if(fwrite(pwdict->m_dictentry,sizeof(scw_dictentry_t),pwdict->m_depos,fp)
			!=pwdict->m_depos)
	{
		goto failed;
	}

	if(fwrite(&(pwdict->m_lmpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}
	if(fwrite(pwdict->m_lemmalist,sizeof(scw_lemma_t),pwdict->m_lmpos,fp)
			!=pwdict->m_lmpos)
	{
		goto failed;
	}

	if(fwrite(&(pwdict->m_entrance),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if(fwrite(&(pwdict->m_pbpos),sizeof(u_int),1,fp)!=1)
	{
		goto failed;
	}

	if(fwrite(pwdict->m_probbuf,sizeof(int),pwdict->m_pbpos,fp)
			!=pwdict->m_pbpos)
	{
		goto failed;
	}

	if(fwrite(&(pwdict->m_utilipos),sizeof(u_int),1,fp) != 1)
	{
		goto failed;
	}

	if(fwrite(pwdict->m_utilinfo,sizeof(scw_utilinfo_t),pwdict->m_utilipos,fp)
			!= pwdict->m_utilipos)
	{
		goto failed;
	}
	if(fwrite(&(pwdict->m_wdtype),sizeof(u_int),1,fp) != 1)
	{
		goto failed;
	}

	fclose(fp);
	return 1;

failed:
	if(fp != NULL)
	{
		fclose(fp);
	}

	WARNING_LOG("error: IO failed"); 
	return -2;
}

/*!
 * Insert a new dentry to the worddict.
 * @param <pwdict> - the worddict.
 * @param <lastpos> - the prefix dictentry id.
 * @param <value> - the new value.
 * @param <curpos> - the output param, position of the current dictentry.
 * @return 1 if success; 0 if existed; <0 if failed.
 */
int scw_insert_dentry(scw_worddict_t* pwdict, u_int lastpos, u_int value, u_int& curpos)
{
	u_int tmpdepos;
	u_int curdepos;
	u_int sufpos;
	u_int hsize;
	u_int hpos;

	if( !pwdict )
	{
		WARNING_LOG("error: NULL pointer");
		return -1;
	}

	if(lastpos != DENTRY_FIRST && lastpos >= pwdict->m_depos)
	{
		WARNING_LOG("error: illegal value");
		return -1;
	}

	if(lastpos != DENTRY_FIRST)
	{
		sufpos = pwdict->m_dictentry[lastpos].m_suffix_pos;
	}
	else
	{
		sufpos = pwdict->m_entrance;
	}

	// initialize a new node
	if(sufpos == SUFENTRY_NULL)
	{
		// the last dict entry has no suffix entry
		if(pwdict->m_seipos + 3 > pwdict->m_seisize)
		{
			// the <m_seinfo> list is not enough, adjust it
			if(scw_adjust_seinfo(pwdict) < 0)
			{
				WARNING_LOG("error: scw_adjust_seinfo() failed"); 
				return -1;
			}
		}

		while(pwdict->m_seipos + 3 > pwdict->m_seisize)
		{
			// the <m_seinfo> list have no enough space again,resize it
			if(scw_resize_seinfo(pwdict,pwdict->m_seisize + MAX_EXTEND_ENTRY_SIZE) != 1)
			{
				WARNING_LOG("error: scw_resize_seinfo() failed"); 
				return -1;
			}
		}

		assert(pwdict->m_seipos +  3 <= pwdict->m_seisize); // assert!
		assert(lastpos != DENTRY_FIRST); // assert!

		pwdict->m_dictentry[lastpos].m_suffix_pos = pwdict->m_seipos;
		scw_init_sufentry(pwdict->m_seinfo + pwdict->m_seipos,1,lastpos); // init the hashsize as 1
		pwdict->m_seipos += 3;
		sufpos=pwdict->m_dictentry[lastpos].m_suffix_pos;
	}

	assert(sufpos != SUFENTRY_NULL); // assert!

	hsize = scw_get_hashsize(pwdict->m_seinfo + sufpos);

	assert(hsize > 0); // assert!

	hpos = value % hsize;

	tmpdepos = scw_get_sufentry(pwdict->m_seinfo+sufpos,hpos);
	if((tmpdepos != DENTRY_NULL) && (pwdict->m_dictentry[tmpdepos].m_value == value)) // the entry has exist
	{
		curpos = tmpdepos;
		return 0;
	}
	else
	{
		// no such dictentry in worddict, then create it 
		if(pwdict->m_depos == pwdict->m_desize)
		{
			if(scw_resize_dentry(pwdict,pwdict->m_desize+MAX_EXTEND_ENTRY_SIZE) != 1 )
			{
				WARNING_LOG("error: scw_resize_dentry() failed"); 
				return -1;
			}
		}

		assert(pwdict->m_depos < pwdict->m_desize);

		curdepos = pwdict->m_depos;
		pwdict->m_depos++;
		pwdict->m_dictentry[curdepos].m_value = value;
		pwdict->m_dictentry[curdepos].m_lemma_pos = LEMMA_NULL;
		pwdict->m_dictentry[curdepos].m_suffix_pos = SUFENTRY_NULL;

		// insert into the suffix list of the lastpos dictentry
		if(tmpdepos == DENTRY_NULL)  // the position in hash table is not used.
		{

			scw_set_sufentry(pwdict->m_seinfo+sufpos,hpos,curdepos);
			curpos = curdepos;
			return 1;
		}
		else  // the position is used.
		{
			// try to find a new hash size for the current subnodes.
			int newhash;
			for( newhash=hsize+1 ; ; newhash++ )
			{
				int conflict = 0;        
				if(pwdict->m_seipos + newhash + 2 > pwdict->m_seisize)
				{
					// the seinfo list is not enough, adjust it
					if(scw_adjust_seinfo(pwdict) < 0)
					{
						WARNING_LOG("error: scw_adjust_seinfo() failed"); 
						return -1;
					}
				}

				while(pwdict->m_seipos + newhash + 2 > pwdict->m_seisize)
				{
					// the seinfo list have no enough space again,resize it
					if(scw_resize_seinfo(pwdict,pwdict->m_seisize+MAX_EXTEND_ENTRY_SIZE)!=1 )
					{  
						WARNING_LOG("error: scw_adjust_seinfo() failed"); 
						return -1;
					}
				}

				assert(pwdict->m_seipos +  newhash + 2  <= pwdict->m_seisize); // assert!

				if(lastpos != DENTRY_FIRST)
				{
					sufpos = pwdict->m_dictentry[lastpos].m_suffix_pos;
				}
				else
				{
					sufpos = pwdict->m_entrance;
				}

				scw_init_sufentry(pwdict->m_seinfo+pwdict->m_seipos,newhash,lastpos);

				for( u_int i=0; i<hsize; i++ )
				{
					u_int others;
					if((others=scw_get_sufentry(pwdict->m_seinfo+sufpos,i)) != DENTRY_NULL)
					{
						u_int tmpvalue;
						u_int tmphpos;

						tmpvalue = pwdict->m_dictentry[others].m_value;
						tmphpos = tmpvalue % newhash;
						if(scw_get_sufentry(pwdict->m_seinfo+pwdict->m_seipos,tmphpos) == DENTRY_NULL)
						{
							scw_set_sufentry(pwdict->m_seinfo+pwdict->m_seipos,tmphpos,others);
						}
						else
						{
							conflict = 1;
							break;
						}
					}
				}

				if(conflict == 0 )
				{
					u_int tmpvalue;
					u_int tmphpos;

					tmpvalue = pwdict->m_dictentry[curdepos].m_value;
					tmphpos = tmpvalue % newhash;
					if(scw_get_sufentry(pwdict->m_seinfo+pwdict->m_seipos,tmphpos) == DENTRY_NULL)
					{
						scw_set_sufentry(pwdict->m_seinfo+pwdict->m_seipos,tmphpos,curdepos);
					}
					else
					{
						conflict = 1;
					}
				}

				if(conflict == 0)
				{
					// have find the minimal hash size
					if(lastpos != DENTRY_FIRST)
					{
						pwdict->m_dictentry[lastpos].m_suffix_pos = pwdict->m_seipos;
					}
					else
					{
						pwdict->m_entrance = pwdict->m_seipos;
					}

					pwdict->m_seipos += newhash + 2;
					curpos=curdepos;

					return 1;
				}
			}
		}
	}
}

/*
 * Find the lemma from the worddict.
 * @param <pwdict> - the worddict.
 * @param <term> - the lemma term.
 * @return LEMMA_NULL if not in; the position in the lemmalist if success
 */
u_int scw_seek_dynword(Sdict_build * pdict,const char * term,int len)
{
	if(pdict == NULL){
		return LEMMA_NULL;
	}
	u_int lmpos = 0;         
	Sdict_snode snode;
	creat_sign_f64((char *)term,len,&snode.sign1,&snode.sign2);
	if(db_op1(pdict,&snode,SEEK) <= 0){
		return LEMMA_NULL;
	}
	lmpos = (unsigned)snode.code;
	return lmpos;
}

int scw_insert_dynword(Sdict_build * pdict,const char * term,int len,u_int lmpos)
{
	if(pdict == NULL){
		return -1;  
	}
	Sdict_snode snode;
	int ret = 0;
	creat_sign_f64((char *)term,len,&snode.sign1,&snode.sign2);
	snode.code = (int)lmpos;
	ret =db_op1(pdict,&snode,ADD);

	if(ret < 0 ){
		return -1;
	}
	return 0;
}

u_int scw_seek_lemma(scw_worddict_t * pwdict,const char* term,int len)
{
	int curpos=0;
	u_int sufpos=0;
	u_int value=0;
	u_int hsize=0;
	u_int hpos=0;
	u_int nde=0;

	if(len <= 0){
		return LEMMA_NULL;
	}

	sufpos=pwdict->m_entrance;
	while(curpos < len)
	{
		if(sufpos ==SUFENTRY_NULL)
		{
			return LEMMA_NULL;
		} 

		value = scw_get_word(term,curpos,len,NULL);
		hsize = scw_get_hashsize(pwdict->m_seinfo+sufpos);
		hpos = value % hsize;
		nde = scw_get_sufentry(pwdict->m_seinfo+sufpos,hpos);

		if((nde == DENTRY_NULL) || (pwdict->m_dictentry[nde].m_value != value))
		{
			return LEMMA_NULL;
		}

		sufpos = pwdict->m_dictentry[nde].m_suffix_pos;
	}

	assert(pwdict->m_dictentry[nde].m_value == value); // assert!

	return pwdict->m_dictentry[nde].m_lemma_pos;

}
//     

// scw_search
/*========================================================================================
 * functon : seek the lemma from the worddict
 * argu    : pwdict, the worddict,
 *         : term, the lemma term
 * return  : 1 exist in the dict;
 *         : 0 nout exist
 ========================================================================================*/
int scw_search(scw_worddict_t* pwdict,const char* term,int len)
{
	u_int lmpos = 0;
	lmpos = scw_seek_lemma(pwdict,term,len);
	if(lmpos == LEMMA_NULL)
		return 0;
	else
		return 1;
}

// scw_add_lemma_man
/*!
 * Add a dict lemma into worddict.
 * @param <pwdict> - the worddict.
 * @param <pinlemma> - the input lemma struct.
 * @return 1 if success, 0 if exist; < 0 if failed.
 */
int scw_add_lemma_man(scw_worddict_t* pwdict,Sdict_build* pdynwdict,scw_inlemma_t& inlm)
{  
	int curpos = 0;
	int len = strlen(inlm.m_lmstr);

	u_int value = 0;
	u_int last_depos = DENTRY_FIRST;
	u_int cur_depos = COMMON_NULL;
	u_int lmpos = COMMON_NULL;
	u_int pre_lmpos = LEMMA_NULL;
	u_int& phipos = pwdict->m_phipos;

	// check whether exists ,需要将现在的切分片度替换原来的切分片段 
	if( (lmpos = scw_seek_lemma(pwdict, inlm.m_lmstr,len)) == LEMMA_NULL && 
			(lmpos = scw_seek_dynword(pdynwdict, inlm.m_lmstr,len)) == LEMMA_NULL )
	{
		// insert dict lemma node, update trie tree
		if( !IS_TEMP(inlm.m_property) )
		{
			while(curpos < len)
			{
				value = scw_get_word(inlm.m_lmstr,curpos,len,NULL);
				if(scw_insert_dentry(pwdict,last_depos,value,cur_depos) < 0)
				{
					WARNING_LOG("error: scw_insert_dentry() failed"); 
					return -1;
				}
				if(last_depos != DENTRY_FIRST && pwdict->m_dictentry[last_depos].m_lemma_pos != LEMMA_NULL)
				{
					pre_lmpos = pwdict->m_dictentry[last_depos].m_lemma_pos;  
				}
				last_depos=cur_depos;
			}
			assert(pwdict->m_dictentry[cur_depos].m_value == value); // assert!
		}
		if(pwdict->m_lmpos == pwdict->m_lmsize)
		{
			// If lemma list is full, resize it.
			if(scw_resize_lemma(pwdict,pwdict->m_lmsize + LEMMA_ADD_SIZE) != 1)
			{
				WARNING_LOG("error: scw_resize_lemma() failed"); 
				return -1;
			}
		}

		assert(pwdict->m_lmpos < pwdict->m_lmsize); // assert!
		// update the m_lemmalist
		lmpos = pwdict->m_lmpos;
		pwdict->m_lmpos++;
		if(!IS_TEMP(inlm.m_property))
			pwdict->m_dictentry[cur_depos].m_lemma_pos=lmpos;

		pwdict->m_lemmalist[lmpos].m_type = inlm.m_type;
		pwdict->m_lemmalist[lmpos].m_weight = inlm.m_weight;
		pwdict->m_lemmalist[lmpos].m_length = len;
		memcpy(&pwdict->m_lemmalist[lmpos].m_property,&inlm.m_property,sizeof(scw_property_t));
		pwdict->m_lemmalist[lmpos].m_prelm_pos = pre_lmpos;
		pwdict->m_lemmalist[lmpos].m_utilinfo_pos = COMMON_NULL;

		if( IS_TEMP(inlm.m_property) )
			scw_insert_dynword(pdynwdict,inlm.m_lmstr,len,lmpos);
	}
	else
	{
	//	fprintf(stderr,"has find the word in the dict--scw_add_lemma_man\n");	
		pwdict->m_lemmalist[lmpos].m_type = inlm.m_type;
		pwdict->m_lemmalist[lmpos].m_weight = inlm.m_weight;
		memcpy(&pwdict->m_lemmalist[lmpos].m_property,&inlm.m_property,sizeof(scw_property_t));
	}
	//fprintf(stderr,"type is %d %x\n",pwdict->m_lemmalist[lmpos].m_type,pwdict->m_lemmalist[lmpos].m_property);
	switch( inlm.m_type )
	{
		case LEMMA_TYPE_SBASIC:      
			while(pwdict->m_wbpos + len + 1 > pwdict->m_wbsize)
			{
				// wordbuf is not enough
				if(scw_resize_wordbuf(pwdict,pwdict->m_wbsize * 2 ) != 1)
				{
					WARNING_LOG("error: scw_resize_wordbuf() failed"); 
					return -1;
				}
			}
			assert(pwdict->m_wbpos + len + 1 <= pwdict->m_wbsize); // assert!
			// update m_wordbuf
			memcpy(pwdict->m_wordbuf+pwdict->m_wbpos,inlm.m_lmstr,len);
			pwdict->m_wordbuf[pwdict->m_wbpos+len] = 0;
			pwdict->m_lemmalist[lmpos].m_word_bpos = pwdict->m_wbpos;
			pwdict->m_wbpos += len + 1;
			pwdict->m_lemmalist[lmpos].m_subphinfo_bpos = COMMON_NULL;

			return 1;
			//break;

		case LEMMA_TYPE_SPHRASE:
		case LEMMA_TYPE_QYPD:
			// build word list and subphrase list
			while( pwdict->m_phipos + inlm.m_bwcount + 1 + 2 * inlm.m_sphcount + 1 > pwdict->m_phisize )
			{
				if(scw_resize_phinfo(pwdict,pwdict->m_phisize * 2 )!= 1)
				{
					WARNING_LOG("error: scw_resize_phinfo() failed");
					return -1;
				}
			}

			assert(pwdict->m_phipos + inlm.m_bwcount + 1 + 2 * inlm.m_sphcount + 1 <=  pwdict->m_phisize ); // assert!

			// build basci word list (phrase info) list
			pwdict->m_lemmalist[lmpos].m_phinfo_bpos = phipos;

			assert(inlm.m_bwcount > 1); // assert!
			// update m_phinfo
			pwdict->m_phinfo[pwdict->m_phipos++] = inlm.m_bwcount;
			for( int i=0; i<inlm.m_bwcount; i++ )
			{
				u_int off=0;
				u_int pos=0;
				off  = inlm.m_wordlist[i*2];
				pos  = inlm.m_wordlist[i*2+1];
				pwdict->m_phinfo[pwdict->m_phipos++] = pos;
			}

			// build subphrase info list
			if(inlm.m_sphcount > 0)
			{
				u_int off = 0;
				u_int pos = 0;
				pwdict->m_lemmalist[lmpos].m_subphinfo_bpos = pwdict->m_phipos;
				pwdict->m_phinfo[pwdict->m_phipos++] = inlm.m_sphcount;
				for(int i=0;i<inlm.m_sphcount;i++)
				{
					off  = inlm.m_subphlist[2*i];
					pos  = inlm.m_subphlist[2*i+1];
					pwdict->m_phinfo[pwdict->m_phipos++]  = off;
					pwdict->m_phinfo[pwdict->m_phipos++]  = pos;
				}
			}
			else
			{
				pwdict->m_lemmalist[lmpos].m_subphinfo_bpos=COMMON_NULL;
			}
			return 1;
		//	break;

			//增加一个人工控制片段的add lemma, 该人工控制片段为phrase,只有basic没有subphrase;
		case LEMMA_TYPE_MAN:
			while( pwdict->m_phipos + inlm.m_mcount + 1 > pwdict->m_phisize )
			{
				if(scw_resize_phinfo(pwdict,pwdict->m_phisize * 2 )!= 1)
				{
					WARNING_LOG("error: scw_resize_phinfo() failed");
					return -1;
				}
			}

			pwdict->m_lemmalist[lmpos].m_phinfo_bpos = pwdict->m_phipos;
			pwdict->m_lemmalist[lmpos].m_subphinfo_bpos=COMMON_NULL;

			if(inlm.m_mcount <= 1)
			{
				WARNING_LOG("inlm.m_mcount must be more than 1\n");
				return -1;
			}
			// update m_phinfo
			pwdict->m_phinfo[pwdict->m_phipos++] = inlm.m_mcount;
			for( u_int i=0; i<inlm.m_mcount; i++ )
			{
				u_int pos  = inlm.m_manuallist[i];
				pwdict->m_phinfo[pwdict->m_phipos++] = pos;
			}
			return 1;
		//	break;

		default:
			WARNING_LOG("error: unknown dict lemma type"); 
			return -1;
	}
}
// scw_add_lemma
/*!
 * Add a dict lemma into worddict.
 * @param <pwdict> - the worddict.
 * @param <pinlemma> - the input lemma struct.
 * @return 1 if success, 0 if exist; < 0 if failed.
 */
int scw_add_lemma(scw_worddict_t* pwdict,Sdict_build* pdynwdict,scw_inlemma_t& inlm)
{  
	int curpos = 0;
	int len = strlen(inlm.m_lmstr);

	u_int value = 0;
	u_int last_depos = DENTRY_FIRST;
	u_int cur_depos = COMMON_NULL;
	u_int lmpos = COMMON_NULL;
	u_int pre_lmpos = LEMMA_NULL;
	u_int& phipos = pwdict->m_phipos;

	// check whether exists  
	if( (lmpos = scw_seek_lemma(pwdict, inlm.m_lmstr,len)) != LEMMA_NULL || 
			(lmpos = scw_seek_dynword(pdynwdict, inlm.m_lmstr,len)) != LEMMA_NULL )
	{
		if( !IS_TEMP(pwdict->m_lemmalist[lmpos].m_property) && IS_TEMP(inlm.m_property) )
			return 0;
		// 如果辞典中已有，覆盖其属性
		pwdict->m_lemmalist[lmpos].m_length  = strlen(inlm.m_lmstr);
		pwdict->m_lemmalist[lmpos].m_weight = inlm.m_weight;
		memcpy(&pwdict->m_lemmalist[lmpos].m_property, &inlm.m_property, sizeof(scw_property_t));
		return 0;
	}

	// insert dict lemma node, update trie tree
	if( !IS_TEMP(inlm.m_property) )
	{
		while(curpos < len)
		{
			value = scw_get_word(inlm.m_lmstr,curpos,len,NULL);
			if(scw_insert_dentry(pwdict,last_depos,value,cur_depos) < 0)
			{
				WARNING_LOG("error: scw_insert_dentry() failed"); 
				return -1;
			}

			if(last_depos != DENTRY_FIRST && pwdict->m_dictentry[last_depos].m_lemma_pos != LEMMA_NULL)
			{
				pre_lmpos = pwdict->m_dictentry[last_depos].m_lemma_pos;  
			}

			last_depos=cur_depos;
		}

		assert(pwdict->m_dictentry[cur_depos].m_value == value); // assert!
	}

	if(pwdict->m_lmpos == pwdict->m_lmsize)
	{
		// If lemma list is full, resize it.
		if(scw_resize_lemma(pwdict,pwdict->m_lmsize + LEMMA_ADD_SIZE) != 1)
		{
			WARNING_LOG("error: scw_resize_lemma() failed"); 
			return -1;
		}
	}

	assert(pwdict->m_lmpos < pwdict->m_lmsize); // assert!

	// update the m_lemmalist
	lmpos = pwdict->m_lmpos;
	pwdict->m_lmpos++;
	if(!IS_TEMP(inlm.m_property))
		pwdict->m_dictentry[cur_depos].m_lemma_pos=lmpos;

	pwdict->m_lemmalist[lmpos].m_type = inlm.m_type;
	pwdict->m_lemmalist[lmpos].m_weight = inlm.m_weight;
	pwdict->m_lemmalist[lmpos].m_length = len;
	memcpy(&pwdict->m_lemmalist[lmpos].m_property,&inlm.m_property,sizeof(scw_property_t));
	pwdict->m_lemmalist[lmpos].m_prelm_pos = pre_lmpos;
	pwdict->m_lemmalist[lmpos].m_utilinfo_pos = COMMON_NULL;

	if( IS_TEMP(inlm.m_property) )
		scw_insert_dynword(pdynwdict,inlm.m_lmstr,len,lmpos);

	switch( inlm.m_type )
	{
		case LEMMA_TYPE_SBASIC:      
			while(pwdict->m_wbpos + len + 1 > pwdict->m_wbsize)
			{
				// wordbuf is not enough
				if(scw_resize_wordbuf(pwdict,pwdict->m_wbsize * 2 ) != 1)
				{
					WARNING_LOG("error: scw_resize_wordbuf() failed"); 
					return -1;
				}
			}
			assert(pwdict->m_wbpos + len + 1 <= pwdict->m_wbsize); // assert!
			// update m_wordbuf
			memcpy(pwdict->m_wordbuf+pwdict->m_wbpos,inlm.m_lmstr,len);
			pwdict->m_wordbuf[pwdict->m_wbpos+len] = 0;
			pwdict->m_lemmalist[lmpos].m_word_bpos = pwdict->m_wbpos;
			pwdict->m_wbpos += len + 1;
			pwdict->m_lemmalist[lmpos].m_subphinfo_bpos = COMMON_NULL;

			return 1;
			//break;

		case LEMMA_TYPE_SPHRASE:
		case LEMMA_TYPE_QYPD:
			// build word list and subphrase list
			while( pwdict->m_phipos + inlm.m_bwcount + 1 + 2 * inlm.m_sphcount + 1 > pwdict->m_phisize )
			{
				if(scw_resize_phinfo(pwdict,pwdict->m_phisize * 2 )!= 1)
				{
					WARNING_LOG("error: scw_resize_phinfo() failed");
					return -1;
				}
			}

			assert(pwdict->m_phipos + inlm.m_bwcount + 1 + 2 * inlm.m_sphcount + 1 <=  pwdict->m_phisize ); // assert!

			// build basci word list (phrase info) list
			pwdict->m_lemmalist[lmpos].m_phinfo_bpos = phipos;

			assert(inlm.m_bwcount > 1); // assert!
			// update m_phinfo
			pwdict->m_phinfo[pwdict->m_phipos++] = inlm.m_bwcount;
			for( int i=0; i<inlm.m_bwcount; i++ )
			{
				u_int off=0;
				u_int pos=0;
				off  = inlm.m_wordlist[i*2];
				pos  = inlm.m_wordlist[i*2+1];
				pwdict->m_phinfo[pwdict->m_phipos++] = pos;
			}

			// build subphrase info list
			if(inlm.m_sphcount > 0)
			{
				u_int off = 0;
				u_int pos = 0;
				pwdict->m_lemmalist[lmpos].m_subphinfo_bpos = pwdict->m_phipos;
				pwdict->m_phinfo[pwdict->m_phipos++] = inlm.m_sphcount;
				for(int i=0;i<inlm.m_sphcount;i++)
				{
					off  = inlm.m_subphlist[2*i];
					pos  = inlm.m_subphlist[2*i+1];
					pwdict->m_phinfo[pwdict->m_phipos++]  = off;
					pwdict->m_phinfo[pwdict->m_phipos++]  = pos;
				}
			}
			else
			{
				pwdict->m_lemmalist[lmpos].m_subphinfo_bpos=COMMON_NULL;
			}
			return 1;
			//	break;

			//增加一个人工控制片段的add lemma, 该人工控制片段为phrase,只有basic没有subphrase;
		case LEMMA_TYPE_MAN:
			while( pwdict->m_phipos + inlm.m_mcount + 1 > pwdict->m_phisize )
			{
				if(scw_resize_phinfo(pwdict,pwdict->m_phisize * 2 )!= 1)
				{
					WARNING_LOG("error: scw_resize_phinfo() failed");
					return -1;
				}
			}

			pwdict->m_lemmalist[lmpos].m_phinfo_bpos = pwdict->m_phipos;
			pwdict->m_lemmalist[lmpos].m_subphinfo_bpos=COMMON_NULL;

			if(inlm.m_mcount <= 1)
			{
				WARNING_LOG("inlm.m_mcount must be more than 1\n");
				return -1;
			}
			// update m_phinfo
			pwdict->m_phinfo[pwdict->m_phipos++] = inlm.m_mcount;
			for( u_int i=0; i<inlm.m_mcount; i++ )
			{
				u_int pos  = inlm.m_manuallist[i];
				pwdict->m_phinfo[pwdict->m_phipos++] = pos;
			}
			return 1;
			//	break;

		default:
			WARNING_LOG("error: unknown dict lemma type"); 
			return -1;
	}

}
// 

// scw_seek_wordentry
/*========================================================================================
 * function : build all the suffix entry of each lemma, this function will
 *          : be called recursively
 * argu     : pwdict, the worddict
 *          : words, the value contain the prefix of the entry
 *          : 
 * return   : 1 if success
 *          : <0 if failed
 ========================================================================================*/
u_int scw_seek_wordentry(scw_worddict_t * pwdict,u_int* words,int count)
{
	u_int value=0;
	u_int suf=0;
	u_int hsize=0;
	u_int hpos=0;
	u_int depos=COMMON_NULL;

	suf=pwdict->m_entrance;
	for(int i=0;i<count;i++)
	{
		if(suf == DENTRY_NULL)
		{
			return DENTRY_NULL;
		}

		value=words[i];
		hsize=scw_get_hashsize(pwdict->m_seinfo+suf);
		hpos=value % hsize;
		depos=scw_get_sufentry(pwdict->m_seinfo+suf,hpos);

		if((depos == DENTRY_NULL) || ( pwdict->m_dictentry[depos].m_value != value))
		{
			return DENTRY_NULL;
		}
		suf=pwdict->m_dictentry[depos].m_suffix_pos;
	}
	return depos;
}
// 


// scw_build_suffix
/*========================================================================================
 * function  : build the suffix entry of each lemma, suffix entry will 
 *       : be used by backtrack
 * argu    : last_depos, last dectentry pos
 *       : words, buffer to hold the searching word.
 *       : lastpos, last lemma position in word
 *       : curpos, current lemma position in word
 ========================================================================================*/ 
/*void scw_build_suffix (scw_worddict_t * pwdict, u_int last_depos, u_int *words, int lastpos, int curpos)
  {
  u_int sufpos;
  u_int hsize;
  u_int depos;
  u_int lmpos;
  int newlast;

  assert(last_depos!=DENTRY_NULL);
  if(last_depos == DENTRY_FIRST)
  {
  sufpos=pwdict->m_entrance;
  }
  else
  {
  if((sufpos=pwdict->m_dictentry[last_depos].m_suffix_pos)==DENTRY_NULL)
  {
  return;
  }
  }

  hsize=scw_get_hashsize(pwdict->m_seinfo+sufpos);
  for(int i=0;i<(int)hsize;i++)
  {
  if((depos=scw_get_sufentry(pwdict->m_seinfo+sufpos,i))!=DENTRY_NULL)
  {
  words[curpos]=pwdict->m_dictentry[depos].m_value;
  lmpos=pwdict->m_dictentry[depos].m_lemma_pos;
  if(lmpos != LEMMA_NULL)
  {
  newlast=curpos+1;
  }
  else
  {
  newlast=lastpos;
  }

  if( (lmpos != LEMMA_NULL) && (lastpos != 0) )
  {
  pwdict->m_lemmalist[lmpos].m_suffix_depos =
  scw_seek_wordentry(pwdict,words+lastpos,curpos-lastpos+1);
  }
  scw_build_suffix(pwdict,depos,words,newlast,curpos+1);
  }
  }
  return;
  }*/

// scw_seek_entry
/*
 * Get the dict entry by value and prefix dict entry
 * argu     : pwdict, the worddict
 *          : lde, the last dict entry
 *          : vale, the value
 * return   : DENTRY_NULL if not in
 *          : the dict entry position if IN
 */
u_int scw_seek_entry(scw_worddict_t* pwdict, u_int lde, u_int value)
{
	u_int sufpos;
	u_int nde;
	u_int hsize;
	u_int hpos;

	if(lde == DENTRY_FIRST)
	{
		sufpos=pwdict->m_entrance;
	}
	else
	{
		sufpos=pwdict->m_dictentry[lde].m_suffix_pos;
	}
	if(sufpos == SUFENTRY_NULL)
	{
		return DENTRY_NULL;
	}

	hsize = scw_get_hashsize(pwdict->m_seinfo+sufpos);
	hpos = value % hsize;

	if( ((nde=scw_get_sufentry(pwdict->m_seinfo+sufpos,hpos))==DENTRY_NULL) || 
			(pwdict->m_dictentry[nde].m_value != value) )
	{
		return DENTRY_NULL;
	}
	else
	{
		return nde;
	}
}
//

// convert_format
/*========================================================================================
 * function : convert word seperate result with format [0(中华)1(人民)2(共和)3(国)]
 *       : to the format like this "0 中华\t1 人民\t2 共和\t3 国\t"
 * argu    : src, the input str
 *       : dst,size, the output buffer and its size.
 * return   : >=0, success; <0, failed.
 ========================================================================================*/
int convert_format(scw_worddict_t* pwdict,Sdict_build * pdynwdict,char* src,u_int* dst,int dcnt, int is_bword)
{
	const char* where="convert_format";
	int offset;
	char* nextword;
	char* left;
	char* right;
	int remlen;
	int count=0;
	u_int lmpos;

	dst[0]=0;
	nextword=src;
	remlen=strlen(src);
	while(*nextword!='\0')
	{
		// get the offset
		offset=strtol(nextword,&left,10);
		if( (left==nextword) || (left==NULL) || ( *left!='(')||
				(offset > (int)MAX_LEMMA_LENGTH))
		{
			ul_writelog(UL_LOG_NOTICE,"error in %s\n", where);
			return -1;
		}
		dst[2*count]=offset;

		// get the sublemma position
		right=strchr(left+1,')');
		if((right== NULL)||(remlen -2 < right-left)
				||(right-left >(int) MAX_LEMMA_LENGTH)||(right-left==1))
		{  
			ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
			return -1;
		}
		char ch = *right;
		*right = 0;

		if((lmpos = scw_seek_lemma(pwdict, left+1, strlen(left+1))) == LEMMA_NULL
				&& (lmpos = scw_seek_dynword(pdynwdict, left+1, strlen(left+1))) == LEMMA_NULL)
		{
			ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
			return -1;
		}

		if(is_bword && pwdict->m_lemmalist[lmpos].m_type!= LEMMA_TYPE_SBASIC
				&& !IS_TEMP(pwdict->m_lemmalist[lmpos].m_property))
		{
			ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
			return -1;
		}

		dst[2*count+1]=lmpos;

		*right = ch;
		remlen=strlen(right+1);
		nextword=right+1;
		count++;
		if(count>=dcnt)
		{
			return -1;
		}
	}
	return count;
}

// worddict2inner
/*========================================================================================
 * function : 将人工控制片段中的每个切分的动态term从inner中拷贝到worddict中
 * argu    : src, the input str
 *       : dst,size, the output buffer and its size.
 * return   : >=0, success; <0, failed.
 ========================================================================================*/
int worddict2inner(scw_worddict_t* pwdict, scw_lemma_t *pwdictplm, scw_lemma_t * pirplm, scw_inner_t *pir)
{
	const char* where = "worddict2inner";
	int i = 0;
	u_int lmpos = 0;//在lemmalist中的偏移量

	if(pwdict == NULL || pwdictplm == NULL || pirplm == NULL || pir == NULL)
	{
		ul_writelog(UL_LOG_WARNING,"error in %s\n", where);
		return -1;
	}

	memcpy(pirplm, pwdictplm, sizeof(scw_lemma_t));
	if(pwdictplm->m_type == LEMMA_TYPE_DBASIC)
	{
		pirplm->m_word_bpos = pir->m_dynb_curpos;
		if(pir->m_dynb_curpos+ pwdictplm->m_length+1 >= pir->m_dynb_size)
		{
			ul_writelog(UL_LOG_WARNING, "error: m_dynb_curpos out of range in function %s\n", where);
			return -1;
		}
		memcpy(pir->m_dynbuf+pir->m_dynb_curpos, pwdict->m_dynbuf+pwdictplm->m_word_bpos, pwdictplm->m_length);
		pir->m_dynbuf[pir->m_dynb_curpos + pwdictplm->m_length] = 0;
		pir->m_dynb_curpos += pwdictplm->m_length + 1;
	}
	else if(pwdictplm->m_type == LEMMA_TYPE_DPHRASE)
	{
		int cntofbasic = pwdict->m_dynphinfo[pwdictplm->m_phinfo_bpos];
		pirplm->m_phinfo_bpos  = pir->m_dynph_curpos;

		if(pir->m_dynph_curpos+ cntofbasic + 1 >=pir->m_dynph_size)
		{
			ul_writelog(UL_LOG_WARNING,"error: m_dynph_curpos out of range in %s\n", where);
			return -1;
		}
		pir->m_dynphinfo[pirplm->m_phinfo_bpos] = cntofbasic;
		pir->m_dynph_curpos++;

		for(i = 0; i < cntofbasic; i ++)
		{
			lmpos = pwdict->m_dynphinfo[pwdictplm->m_phinfo_bpos + i+1];
			if(lmpos == LEMMA_NULL)
			{
				ul_writelog(UL_LOG_WARNING, "the lemma is null, please check the inner2worddict :%s\n",where);
				return -1;
			}
			scw_lemma_t* plmofbasic = &(pwdict->m_lemmalist[lmpos]);

			if(plmofbasic->m_type == LEMMA_TYPE_DBASIC)
			{
				u_int tmp = pir->m_dynb_curpos;
				if(pir->m_dynb_curpos+ plmofbasic->m_length+1 >= pir->m_dynb_size)
				{
					ul_writelog(UL_LOG_WARNING, "error: m_dynb_curpos out of range in function %s\n", where);
					return -1;
				}
				memcpy(pir->m_dynbuf+pir->m_dynb_curpos, pwdict->m_dynbuf+plmofbasic->m_word_bpos, plmofbasic->m_length);
				pir->m_dynbuf[pir->m_dynb_curpos + plmofbasic->m_length] = 0;
				pir->m_dynb_curpos += plmofbasic->m_length+1;

				if(pir->m_dynlm_cnt>=pir->m_dynlm_size)
				{
					ul_writelog(UL_LOG_WARNING, " m_dynlm_cnt out of range in %s\n", where);
					return -1;
				}

				memcpy(&(pir->m_dynlm[pir->m_dynlm_cnt]), plmofbasic, sizeof(scw_lemma_t));
				pir->m_dynlm[pir->m_dynlm_cnt].m_word_bpos = tmp;
				pir->m_dynphinfo[pir->m_dynph_curpos++] =(uintptr_t)&(pir->m_dynlm[pir->m_dynlm_cnt]);
				pir->m_dynlm_cnt++;
			}	
			else
			{
				pir->m_dynphinfo[pir->m_dynph_curpos++] = (uintptr_t)plmofbasic;
			}
		}

		//从pwdict中拷贝对应的subphrase信息到pir中,然后再生成动态的phrase
		int cntofsubphrase = 0;
		if(pwdictplm->m_subphinfo_bpos != COMMON_NULL) 
		{
			cntofsubphrase = pwdict->m_dynphinfo[pwdictplm->m_subphinfo_bpos];
			pir->m_dynph_curpos_tmp = 0;
			pir->m_dynphinfo_tmp[0] = cntofsubphrase;
			pir->m_dynph_curpos_tmp ++;
		}
		for(int i = 0; i < cntofsubphrase; i ++)
		{
			//(1)将其中的动态subphrase从pwdict中拷贝到pir中
			pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = pwdict->m_dynphinfo[pwdictplm->m_subphinfo_bpos + 2*i + 1];
			lmpos = pwdict->m_dynphinfo[pwdictplm->m_subphinfo_bpos + 2*i + 2];
			if(lmpos == LEMMA_NULL)
			{
				ul_writelog(UL_LOG_WARNING, "the lemma is null, please check the inner2worddict : %s\n",where);
				return -1;
			}
			scw_lemma_t *tmpplm = &(pwdict->m_lemmalist[lmpos]);

			if(tmpplm->m_type == LEMMA_TYPE_DPHRASE)
			{
				cntofbasic = pwdict->m_dynphinfo[tmpplm->m_phinfo_bpos];
				if(pir->m_dynph_curpos+ cntofbasic + 1 >=pir->m_dynph_size)
				{
					ul_writelog(UL_LOG_WARNING,"error: m_dynph_curpos out of range in %s\n", where);
					return -1;
				}
				int tmpoftmpplm  = pir->m_dynph_curpos;
				pir->m_dynphinfo[tmpoftmpplm] = cntofbasic;
				pir->m_dynph_curpos++;

				for(int m = 0; m < cntofbasic; m ++)
				{
					lmpos = pwdict->m_dynphinfo[tmpplm->m_phinfo_bpos + m+1];
					if(lmpos == LEMMA_NULL)
					{
						ul_writelog(UL_LOG_WARNING, "the lemma is null, please check the inner2worddict: %s\n",where);
						return -1;
					}
					scw_lemma_t *plm3 = &(pwdict->m_lemmalist[lmpos]);
					if(plm3->m_type == LEMMA_TYPE_DBASIC)
					{		
						u_int tmp = pir->m_dynb_curpos;
						if(pir->m_dynb_curpos+ plm3->m_length+1 >= pir->m_dynb_size)
						{
							ul_writelog(UL_LOG_WARNING, "error: m_dynb_curpos out of range in function %s\n", where);
							return -1;
						}
						memcpy(pir->m_dynbuf+pir->m_dynb_curpos, pwdict->m_dynbuf+plm3->m_word_bpos, plm3->m_length);
						pir->m_dynbuf[pir->m_dynb_curpos + plm3->m_length] = 0;
						pir->m_dynb_curpos += plm3->m_length + 1;

						if(pir->m_dynlm_cnt>=pir->m_dynlm_size)
						{
							ul_writelog(UL_LOG_WARNING, " m_dynlm_cnt out of range in %s\n", where);
							return -1;
						}

						memcpy(&(pir->m_dynlm[pir->m_dynlm_cnt]), plm3, sizeof(scw_lemma_t));
						pir->m_dynlm[pir->m_dynlm_cnt].m_word_bpos = tmp;
						pir->m_dynphinfo[pir->m_dynph_curpos++] = (uintptr_t)&(pir->m_dynlm[pir->m_dynlm_cnt]);
						pir->m_dynlm_cnt ++;
					}
					else
					{
						pir->m_dynphinfo[pir->m_dynph_curpos++] = (uintptr_t)plm3;
					}
				}

				if(pir->m_dynlm_cnt>=pir->m_dynlm_size)
				{
					ul_writelog(UL_LOG_WARNING, " m_dynlm_cnt out of range in %s\n", where);
					return -1;
				}

				memcpy(&(pir->m_dynlm[pir->m_dynlm_cnt]), tmpplm, sizeof(scw_lemma_t));
				pir->m_dynlm[pir->m_dynlm_cnt].m_phinfo_bpos=tmpoftmpplm;
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = (uintptr_t)&(pir->m_dynlm[pir->m_dynlm_cnt]);
				pir->m_dynlm_cnt ++;

			}
			else if(tmpplm->m_type == LEMMA_TYPE_SPHRASE)
			{
				pir->m_dynphinfo_tmp[pir->m_dynph_curpos_tmp ++] = (uintptr_t)tmpplm;
			}
			else
			{
				ul_writelog(UL_LOG_WARNING, "it's not a normal type lemma please check inner2worddict: %s\n", where);
				return -1;
			}
		}


		if(pir->m_dynph_curpos+ 2*cntofsubphrase + 2>=pir->m_dynph_size)
		{
			ul_writelog(UL_LOG_WARNING,"error: m_dynph_curpos out of range in %s\n", where);
			return -1;
		}

		if(cntofsubphrase > 0)
		{
			pirplm->m_subphinfo_bpos =	pir->m_dynph_curpos;
			memcpy(pir->m_dynphinfo + pir->m_dynph_curpos, pir->m_dynphinfo_tmp, sizeof(scw_lemma_t*)* pir->m_dynph_curpos_tmp);
			pir->m_dynph_curpos += pir->m_dynph_curpos_tmp;
		}
		else
		{
			pirplm->m_subphinfo_bpos = COMMON_NULL;
		}
	}
	return 0;
}

// innerbasic2worddict
/*========================================================================================
 * function : 将人工控制片段中的每个切分的动态term从inner中拷贝到worddict中
 * argu    : pwdict:worddict
 *            : newplm:plm in worddict
 : innerplm : plm in pir
 : pir
 * return   : >=0, success; <0, failed.
 ========================================================================================*/
int inner2worddict(scw_worddict_t* pwdict, scw_lemma_t *newplm, scw_lemma_t *innerplm, scw_inner_t *pir)
{
	const char* where="inner2worddict";
	int i = 0; 
	int cnt = 0;
	u_int lmpos = 0;

	if(pwdict == NULL || newplm == NULL || innerplm == NULL || pir == NULL)
	{
		ul_writelog(UL_LOG_WARNING,"some parameters are null in function %s\n", where);
		return -1;
	}

	//将相关信息拷贝过来，需要更新的是其中的m_word_bpos和m_phinfo_bpos以及m_subphinfo_bpos
	memcpy(newplm, innerplm, sizeof(scw_lemma_t));

	//动态basic只需要拷贝pir->m_dynbuf中的东东到pwdict->m_dynbuf中
	if(innerplm->m_type == LEMMA_TYPE_DBASIC)
	{
		if(pwdict->m_dynb_curpos + innerplm->m_length + 1 >= pwdict->m_dynb_size)
		{
			char *newwordbuf = NULL;
			int newsize = pwdict->m_dynb_size * 2;
			if((newwordbuf=(char*)realloc(pwdict->m_dynbuf,newsize + innerplm->m_length + 1)) == NULL)
			{
				ul_writelog(UL_LOG_WARNING,"realloc pwdict->m_dynbuf failed in %s \n", where);
				return -1;
			}
			pwdict->m_dynbuf=newwordbuf;
			pwdict->m_dynb_size=newsize + innerplm->m_length + 1;
		}

		memcpy(pwdict->m_dynbuf+pwdict->m_dynb_curpos,pir->m_dynbuf+innerplm->m_word_bpos,innerplm->m_length);//将basic存储在pwdict中的动态basic空间中
		pwdict->m_dynbuf[pwdict->m_dynb_curpos+innerplm->m_length] = 0;				

		newplm->m_word_bpos = pwdict->m_dynb_curpos;
		pwdict->m_dynb_curpos += innerplm->m_length + 1;
	}
	else if(innerplm->m_type == LEMMA_TYPE_DPHRASE)//动态phrase
	{
		//如果是动态phrase，存储对应的basic关系和subphrase关系，但是subphrase不允许再有自己的subphrase
		cnt = pir->m_dynphinfo[innerplm->m_phinfo_bpos];
		if(pwdict->m_dynph_curpos + cnt + 1 >= pwdict->m_dynph_size)
		{
			u_int *newphinfo = NULL;
			int newsize = pwdict->m_dynph_size * 2;
			if( (newphinfo=(u_int*)realloc(pwdict->m_dynphinfo,(newsize + cnt + 1)*sizeof(u_int))) == NULL )
			{
				ul_writelog(UL_LOG_WARNING,"realloc pwdict->m_dynphinfo failed in %s \n", where);
				return -1;
			}
			pwdict->m_dynphinfo=newphinfo;
			pwdict->m_dynph_size=newsize + cnt + 1;
		}
		newplm->m_phinfo_bpos  = pwdict->m_dynph_curpos;
		pwdict->m_dynph_curpos ++;
		pwdict->m_dynphinfo[newplm->m_phinfo_bpos] = cnt;
		for(i = 0; i < cnt; i ++)
		{
			scw_lemma_t *plm = (scw_lemma_t*)pir->m_dynphinfo[innerplm->m_phinfo_bpos+i+1];
			if(plm->m_type==LEMMA_TYPE_SBASIC)
			{
				char word[256] = "\0";
				int len = 0;
				len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
				if(len < 0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
					return -1;
				}
				lmpos = scw_seek_lemma(pwdict,word,len);
				if(lmpos == LEMMA_NULL)
				{
					if(pwdict->m_lmpos +1 >= pwdict->m_lmsize)
					{
						ul_writelog(UL_LOG_WARNING, "pwdict->m_lmpos is more than pwdict->m_lemmalist %s\n",where);
						return -1;
					}
					lmpos = pwdict->m_lmpos;
					pwdict->m_lmpos++;
					memcpy(&(pwdict->m_lemmalist[lmpos]), plm, sizeof(scw_lemma_t));
				}
				pwdict->m_dynphinfo[pwdict->m_dynph_curpos++] = lmpos;
			}
			else if(plm->m_type == LEMMA_TYPE_DBASIC)
			{
				if(pwdict->m_dynb_curpos + plm->m_length + 1 >= pwdict->m_dynb_size)
				{
					char *newwordbuf = NULL;
					int newsize = pwdict->m_dynb_size * 2;
					if((newwordbuf=(char*)realloc(pwdict->m_dynbuf,newsize + plm->m_length + 1))==NULL)
					{
						ul_writelog(UL_LOG_WARNING,"realloc pwdict->m_dynbuf failed in %s \n", where);
						return -1;
					}
					pwdict->m_dynbuf=newwordbuf;
					pwdict->m_dynb_size=newsize + plm->m_length + 1;
				}
				memcpy(pwdict->m_dynbuf+pwdict->m_dynb_curpos, pir->m_dynbuf+plm->m_word_bpos, plm->m_length);
				pwdict->m_dynbuf[pwdict->m_dynb_curpos+plm->m_length] = 0;
				if(pwdict->m_lmpos +1 >= pwdict->m_lmsize)
				{
					ul_writelog(UL_LOG_WARNING, "pwdict->m_lmpos is more than pwdict->m_lemmalist %s\n",where);
					return -1;
				}
				lmpos = pwdict->m_lmpos;
				pwdict->m_lmpos++;
				memcpy(&(pwdict->m_lemmalist[lmpos]), plm, sizeof(scw_lemma_t));
				pwdict->m_lemmalist[lmpos].m_word_bpos = pwdict->m_dynb_curpos;
				pwdict->m_dynb_curpos += plm->m_length + 1;
				pwdict->m_dynphinfo[pwdict->m_dynph_curpos++]=lmpos;
				//	pwdict->m_dynphinfo[pwdict->m_dynph_curpos++]=(uintptr_t)&(pwdict->m_lemmalist[lmpos]);
			}
		}

		//其次存储对应的subphrase关系
		if(innerplm->m_subphinfo_bpos == COMMON_NULL){
			return 0;
		}
		cnt =  pir->m_dynphinfo[innerplm->m_subphinfo_bpos];
		for(i = 0; i < cnt; i ++)
		{
			scw_lemma_t *plm = (scw_lemma_t*)pir->m_dynphinfo[innerplm->m_subphinfo_bpos+2*i+2];
			if(plm->m_type == LEMMA_TYPE_DPHRASE)
			{
				u_int tmp = pwdict->m_dynph_curpos; //该subphrase的basic开始位置
				pwdict->m_dynph_curpos ++;
				int cntofplm = pir->m_dynphinfo[plm->m_phinfo_bpos];

				if(pwdict->m_dynph_curpos + cntofplm + 1 >= pwdict->m_dynph_size)
				{
					u_int *newphinfo = NULL;
					int newsize = pwdict->m_dynph_size * 2;
					if( (newphinfo=(u_int*)realloc(pwdict->m_dynphinfo,(newsize + cntofplm + 1)*sizeof(u_int))) == NULL )
					{
						ul_writelog(UL_LOG_WARNING,"realloc pwdict->m_dynphinfo failed in %s \n", where);
						return -1;
					}
					pwdict->m_dynphinfo=newphinfo;
					pwdict->m_dynph_size=newsize + cntofplm + 1;
				}
				for(int m = 0; m <cntofplm; m ++)
				{	
					scw_lemma_t *plm3=(scw_lemma_t *)pir->m_dynphinfo[plm->m_phinfo_bpos + m + 1];
					if(plm3->m_type==LEMMA_TYPE_SBASIC)
					{
						char word[256] = "\0";
						int len = 0;
						len = get_lmstr(pwdict, pir, plm3, word, sizeof(word));
						if(len < 0)
						{
							ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
							return -1;
						}
						lmpos = scw_seek_lemma(pwdict,word,len);
						if(lmpos == LEMMA_NULL)
						{
							if(pwdict->m_lmpos +1 >= pwdict->m_lmsize)
							{
								ul_writelog(UL_LOG_WARNING, "pwdict->m_lmpos is more than pwdict->m_lemmalist %s\n",where);
								return -1;
							}
							lmpos = pwdict->m_lmpos;
							pwdict->m_lmpos++;
							memcpy(&(pwdict->m_lemmalist[lmpos]), plm3, sizeof(scw_lemma_t));
						}
						pwdict->m_dynphinfo[pwdict->m_dynph_curpos++] = lmpos;
						//pwdict->m_dynphinfo[pwdict->m_dynph_curpos++]=(uintptr_t)plm3;
					}
					else if(plm3->m_type == LEMMA_TYPE_DBASIC)
					{
						if(pwdict->m_dynb_curpos + plm3->m_length + 1 >= pwdict->m_dynb_size)
						{
							char *newwordbuf = NULL;
							int newsize = pwdict->m_dynb_size * 2;
							if((newwordbuf=(char*)realloc(pwdict->m_dynbuf,newsize + plm3->m_length + 1 ))==NULL)
							{
								ul_writelog(UL_LOG_WARNING,"realloc pwdict->m_dynbuf failed in %s\n", where);
								return -1;
							}
							pwdict->m_dynbuf=newwordbuf;
							pwdict->m_dynb_size=newsize + plm3->m_length + 1 ;
						}
						//先将动态basic存储在pwdict->m_dynbuf中
						memcpy(pwdict->m_dynbuf+pwdict->m_dynb_curpos, pir->m_dynbuf+plm3->m_word_bpos, plm3->m_length);
						pwdict->m_dynbuf[pwdict->m_dynb_curpos+ plm3->m_length] = 0;
						if(pwdict->m_lmpos +1 >= pwdict->m_lmsize)
						{
							ul_writelog(UL_LOG_WARNING, "pwdict->m_lmpos is more than pwdict->m_lemmalist %s\n",where);
							return -1;
						}
						lmpos = pwdict->m_lmpos;
						pwdict->m_lmpos++;
						memcpy(&(pwdict->m_lemmalist[lmpos]), plm3, sizeof(scw_lemma_t));
						pwdict->m_lemmalist[lmpos].m_word_bpos = pwdict->m_dynb_curpos;
						pwdict->m_dynb_curpos +=  plm3->m_length + 1;
						pwdict->m_dynphinfo[pwdict->m_dynph_curpos++]=lmpos;
						//pwdict->m_dynphinfo[pwdict->m_dynph_curpos++]=(uintptr_t)&(pwdict->m_lemmalist[lmpos]);
					}			
				}

				if(pwdict->m_lmpos +1 >= pwdict->m_lmsize)
				{
					ul_writelog(UL_LOG_WARNING, "pwdict->m_lmpos is more than pwdict->m_lemmalist %s\n",where);
					return -1;
				}
				lmpos = pwdict->m_lmpos;
				pwdict->m_lmpos++;
				memcpy(&(pwdict->m_lemmalist[lmpos]), plm, sizeof(scw_lemma_t));
				pwdict->m_lemmalist[lmpos].m_phinfo_bpos = tmp;
				pwdict->m_dynphinfo[tmp] = cntofplm;			
				pwdict->m_lemmalist[lmpos].m_subphinfo_bpos = COMMON_NULL;//因为规定subphrase不能有自己的subphrase，因此将其设为空
				pir->m_dynphinfo[innerplm->m_subphinfo_bpos+2*i+2] = lmpos;
				//pir->m_dynphinfo[innerplm->m_subphinfo_bpos+2*i+2] = (uintptr_t)&(pwdict->m_lemmalist[lmpos]);
			}
			else if(plm->m_type == LEMMA_TYPE_SPHRASE)
			{
				char word[256] = "\0";
				int len = 0;
				len = get_lmstr(pwdict, pir, plm, word, sizeof(word));
				if(len < 0)
				{
					ul_writelog(UL_LOG_WARNING, "error in %s\n",where);
					return -1;
				}
				lmpos = scw_seek_lemma(pwdict,word,len);
				if(lmpos == LEMMA_NULL)
				{
					if(pwdict->m_lmpos +1 >= pwdict->m_lmsize)
					{
						ul_writelog(UL_LOG_WARNING, "pwdict->m_lmpos is more than pwdict->m_lemmalist %s\n",where);
						return -1;
					}
					lmpos = pwdict->m_lmpos;
					pwdict->m_lmpos++;
					memcpy(&(pwdict->m_lemmalist[lmpos]), plm, sizeof(scw_lemma_t));
				}
				pir->m_dynphinfo[innerplm->m_subphinfo_bpos+2*i+2] = lmpos;
			}
			else
			{
				ul_writelog(UL_LOG_WARNING, "no such phrase type in %s", where);
				return -1;
			}
		}

		newplm->m_subphinfo_bpos  = pwdict->m_dynph_curpos;
		pwdict->m_dynph_curpos ++; 
		pwdict->m_dynphinfo[newplm->m_subphinfo_bpos] = cnt;

		if(pwdict->m_dynph_curpos + 2*cnt + 2 >= pwdict->m_dynph_size)
		{
			u_int *newphinfo = NULL;
			int newsize = pwdict->m_dynph_size * 2;
			if( (newphinfo=(u_int*)realloc(pwdict->m_dynphinfo,(newsize + 2*cnt + 2 )*sizeof(u_int))) == NULL )
			{
				ul_writelog(UL_LOG_WARNING,"realloc pwdict->m_dynphinfo failed in %s \n", where);
				return -1;
			}
			pwdict->m_dynphinfo=newphinfo;
			pwdict->m_dynph_size=newsize + 2*cnt + 2 ;
		}
		for(i = 0; i < cnt; i ++)
		{
			int offset = pir->m_dynphinfo[innerplm->m_subphinfo_bpos+2*i+1];
			lmpos = pir->m_dynphinfo[innerplm->m_subphinfo_bpos+2*i+2];
			pwdict->m_dynphinfo[pwdict->m_dynph_curpos ++] = offset;//每个suphrase的offset
			pwdict->m_dynphinfo[pwdict->m_dynph_curpos ++] = lmpos;
		}			
	}
	return 0;
}

// convert_format_man
/*========================================================================================
 * function : 将人工控制片段中的每个切分term的lemmalist偏移量保存在pinlemma->m_manuallist中
 * argu    : src, the input str
 *       : dst,size, the output buffer and its size.
 * return   : >=0, success; <0, failed.
 ========================================================================================*/
int convert_format_man(scw_worddict_t* pwdict,scw_inner_t *pir, char* src,u_int* dst,int dcnt)
{
	const char* where="convert_format";
	int offset = 0;
	char* nextword = NULL;
	char* left = NULL;
	char* right = NULL;
	int remlen = 0;
	int count=0;
	u_int lmpos = 0;

	if(pwdict == NULL || pir == NULL || src == NULL || src[0] == 0 || dst == NULL)
	{
		ul_writelog(UL_LOG_WARNING,"error in parameters of function %s\n", where);
		return -1;
	}

	dst[0]=0;
	nextword=src;
	remlen=strlen(src);
	while(*nextword!='\0')
	{
		// get the offset
		offset=strtol(nextword,&left,10);
		if( (left==nextword) || (left==NULL) || ( *left!='(')||
				(offset > (int)MAX_LEMMA_LENGTH))
		{
			ul_writelog(UL_LOG_NOTICE,"error in get man fragment of '(' %s\n", where);
			return -1;
		}

		right=strchr(left+1,')');
		if((right== NULL)||(remlen -2 < right-left)
				||(right-left >(int) MAX_LEMMA_LENGTH)||(right-left==1))
		{  
			ul_writelog(UL_LOG_NOTICE, "error in get man fragment of ')' %s\n", where);
			return -1;
		}
		char ch;
		ch= *right;
		*right = 0;

		if((lmpos = scw_seek_lemma(pwdict, left+1, strlen(left+1))) != LEMMA_NULL)
		{
			if(pwdict->m_lemmalist[lmpos].m_type == LEMMA_TYPE_MAN)
				return -1;
			dst[count]=lmpos;
		}
		else
		{
			//是一个词典中没有的词，存储为动态词
			//由于pir中存储的是lemma的指针，因此不能在使用pir的时候对其进行resize
			//因此提前resize，然后再使用pir
			if(pwdict->m_lmpos + MAX_POT_LEMMA_SIZE >= pwdict->m_lmsize)
			{
				scw_lemma_t * plemmalist = NULL;
				int newsize = pwdict->m_lmsize + LEMMA_ADD_SIZE;
				if((plemmalist=(scw_lemma_t*)realloc(pwdict->m_lemmalist,newsize*sizeof(scw_lemma_t))) == NULL)
				{
					ul_writelog(UL_LOG_WARNING, "realloc pwdict->m_lemmalist failed in %s",where);
					return -1;
				}

				pwdict->m_lemmalist=plemmalist;
				pwdict->m_lmsize=newsize;
			}
			lmpos = pwdict->m_lmpos;
			pwdict->m_lmpos++;
			scw_lemma_t *newplm = &(pwdict->m_lemmalist[lmpos]);

			reset_inner(pir);
			if(scw_seg(pwdict, pir, left+1, strlen(left+1)) < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in scw_seg of %s\n", left+1);
				return -1;
			}

			int real_ppseg_cnt = 0;
			for(u_int i = 0; i < pir->m_ppseg_cnt; i ++)
			{
				if(pir->m_ppseg[i] == NULL){
					continue;
				}
				real_ppseg_cnt ++;
			}
			if(real_ppseg_cnt > 1)
			{
				if(poly_lemmas(pwdict, pir, 0, pir->m_ppseg_cnt) < 0)
				{
					ul_writelog(UL_LOG_WARNING, "error in poly_lemmas of %s\n", where);
					return -1;
				}
			}


			//因为该切分片段是一个动态词，因此需要将该信息从pir中拷贝到worddict中
			if(inner2worddict(pwdict, newplm, pir->m_ppseg[0],pir) < 0)
			{
				ul_writelog(UL_LOG_WARNING, "error in copy from inner to worddict %s\n", where);
				return -1;
			}
			dst[count]=lmpos;
		}


		*right = ch;
		remlen=strlen(right+1);
		nextword=right+1;
		count++;
		if(count>=dcnt){
			return -1;
		}
	}

	return count;
}

// is_3human_need_reseg
/*========================================================================================
 * function : judge if 3-long chinese human name should be reseg.
 * argu    : pwdict
 *         : pilemma
 *         : src
 * return   : 1, success; 0, can not process, <0, failed.
 ========================================================================================*/
/*
int is_3human_need_reseg(scw_worddict_t* pwdict,scw_inlemma_t* pilemma,char* src)
{
	const char* where="is_3human_need_reseg";
	int offset;
	char* nextword;
	char* left;
	char* right;
	int remlen=0,slen=0;
	u_int lmpos;
	scw_lemma_t *plm=NULL;

	nextword=src;
	remlen=strlen(src);
	if(nextword==NULL||*nextword==0)
		return 0;

   offset=strtol(nextword,&left,10);
   if( (left==nextword) || (left==NULL) || ( *left!='(') || (offset > (int)MAX_LEMMA_LENGTH) )
   {
   ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
   return -1;
   }

// get the sublemma position
right=strchr(left+1,')');
if((right== NULL)||(remlen -2 < right-left) ||(right-left >(int) MAX_LEMMA_LENGTH)||(right-left==1))
{  
ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
return -1;
}

char ch = *right;
 *right = 0;

 lmpos = scw_seek_lemma(pwdict, left+1, strlen(left+1));
 if(lmpos == LEMMA_NULL)
 {
 ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
 return -1;
 }

 *right=ch;
 plm = &pwdict->m_lemmalist[lmpos];
 slen=plm->m_length;

 if(IS_SURNAME(plm->m_property) || IS_ONEWORD(plm->m_property) || (remlen == 8 && slen == 4))
 return 0;// not need reseg  
 else 
 return 1;//need reseg
 }
 */
// 

// reseg_3human_phrase
/*!
 * Resegment the human name with 3 hanzis.
 * @param <pwdict> - dict pointer
 * @param <pilemma> - the named lemma pointer
 * @return 1, succeeded; 0, can not process; <0, failed.
 */
/*
int reseg_3human_phrase(scw_worddict_t* pwdict,scw_inlemma_t *pilemma)
{
	//const char* where="reseg_3human_phrase";
char * phr=NULL;
int len=0,slen=2,len22=2;
int off=0;
u_int lmpos=0,lmpos1=0;
scw_lemma_t *plm=NULL;
int i=0;

phr=pilemma->m_lmstr;
len=strlen(phr);

if(len!=6 && len!=8)
return 0;

if(len==8)
slen=4;

while(i<len) //check if include ascii
{
if(*(phr+i)<0)
i+=2;
else if(*(phr+i)>0)
return 0;
else 
i++;
}

i=0;

pilemma->m_wordlist[0]=0;
pilemma->m_subphlist[0]=0;
pilemma->m_bwcount=0;
pilemma->m_sphcount=0;

lmpos=scw_seek_lemma(pwdict,phr+slen,len-slen); //check the given name
if(lmpos==LEMMA_NULL)
{
pilemma->m_bwcount=3;

lmpos=scw_seek_lemma(pwdict,phr,slen);
if(lmpos==LEMMA_NULL)
return -1;

pilemma->m_wordlist[0]=0;
pilemma->m_wordlist[1]=lmpos;
off=slen;

for(i=0;i<pilemma->m_bwcount-1;i++)
{
lmpos=scw_seek_lemma(pwdict,phr+slen+2*i,2);
if(lmpos==LEMMA_NULL)   
return -1;

pilemma->m_wordlist[2*i+2]=off;
pilemma->m_wordlist[2*i+3]=lmpos;
off+=2;
}

lmpos=scw_seek_lemma(pwdict,phr,len-len22);
if(lmpos==LEMMA_NULL || !IS_PHRASE(pwdict->m_lemmalist[lmpos].m_property))
return 1;// have not sub phrase
pilemma->m_sphcount=1;
pilemma->m_subphlist[0]=0;
pilemma->m_subphlist[1]=lmpos;
}
else
{
plm=&pwdict->m_lemmalist[lmpos];
if(IS_PHRASE(plm->m_property))
{
	pilemma->m_bwcount=3;

	lmpos1=scw_seek_lemma(pwdict,phr,slen);
	if(lmpos1==LEMMA_NULL)   
		return -1;
	pilemma->m_wordlist[0]=0;
	pilemma->m_wordlist[1]=lmpos1;  
	off=slen;    

	for(i=0;i<pilemma->m_bwcount-1;i++)
	{
		lmpos1=scw_seek_lemma(pwdict,phr+slen+2*i,2);
		if(lmpos1==LEMMA_NULL)   
			return -1;
		pilemma->m_wordlist[2*i+2]=off;
		pilemma->m_wordlist[2*i+3]=lmpos1;
		off+=2;
	}

	pilemma->m_sphcount=1;
	pilemma->m_subphlist[0]=1;
	pilemma->m_subphlist[1]=lmpos;
}
else
{
	pilemma->m_bwcount=2;

	pilemma->m_wordlist[2]=slen;
	pilemma->m_wordlist[3]=lmpos;

	lmpos=scw_seek_lemma(pwdict,phr,slen);
	if(lmpos==LEMMA_NULL)   
		return -1;
	pilemma->m_wordlist[0]=0;
	pilemma->m_wordlist[1]=lmpos;
}
}

return 1;
}
*/
/// Get the static lemma in dictionary. 
int get_static_lmstr(scw_worddict_t* pwdict,scw_lemma_t* plm, char* dest, u_int destlen)
{
	int cnt = 0;
	int i = 0;
	int pos = 0;
	int type =0;
	int dpos=0;
	scw_lemma_t* plm2=NULL;
	dest[0]=0;

	if( plm == NULL ){
		return -1;
	}

	type = plm->m_type;
	switch(type)
	{
		case LEMMA_TYPE_SBASIC:
			memcpy(dest, pwdict->m_wordbuf+plm->m_word_bpos, plm->m_length+1);
			dpos += plm->m_length;
			break;
		case LEMMA_TYPE_SPHRASE:
			cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
			for(i=0;i<cnt;i++)
			{
				pos = pwdict->m_phinfo[plm->m_phinfo_bpos+i+1];
				plm2= &pwdict->m_lemmalist[pos];          
				memcpy(dest+dpos, pwdict->m_wordbuf+plm2->m_word_bpos, plm2->m_length);
				dpos+=plm2->m_length;
			}
			dest[dpos]=0;
		default :
			break;
	}

	return dpos;
}
// update_multichar_lmprob
/*!
 * Set certain (2 or more char ) words lemma prob using 1 word lemma prob.
 * @param <pilemma> - the inlemma struct.
 * @param <pwdict> - word dict pointer.
 * @return 1, succeeded; <0, failed; 0, not 2w lemma.
 */
/*
   int update_multichr_lmprob(scw_worddict_t* pwdict,scw_lemma_t * plm,int type)
   {
   const char* where="update_multichr_lmprob";
   int len=0,len1 = 0;
   u_int value = 0;
   scw_lemma_t * psblm=NULL;
   int *ppb = NULL;
   char lemma[MAX_LEMMA_LENGTH];
   int prob[NAME_PROP_COUNT];
   int cnt = 0;
   int const_smooth_val = (int)(NORMALIZE_INTEGER*log(LEMMA_SMOOTH_PROB));
   int const_default_val = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB));

   memset(prob,0,NAME_PROP_COUNT*sizeof(int));

   if(type == DATA_CNAME )
   {
   cnt = 0;
   if(IS_NAMEEND(plm->m_property)
   || IS_SURNAME(plm->m_property))
   {
   len = get_static_lmstr(pwdict,plm, lemma, sizeof(lemma));

   while((value = scw_get_word(lemma,(int&)len1,len,NULL)) != 0)
   {
   psblm = get_oneword_lemma(pwdict,value);

   if(psblm == NULL)
   {
   ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
   return -1;
   }

   ppb = scw_get_nameprob(pwdict,psblm,type);

   if(IS_SURNAME(plm->m_property))
   {
   prob[NAME_BGN] += const_default_val;
   }
   else
   {
   prob[NAME_BGN] += const_smooth_val;
   }

   if(IS_NAMEEND(plm->m_property))
   {
   if(ppb)
   {
   if(len == len1)
   {
   prob[NAME_END] += ppb[NAME_END];
   }
   else
   {
   prob[NAME_END] += ppb[NAME_MID];
   }
   }
   else
   {
   prob[NAME_END] += const_default_val;
   }
   }
   else
   {
   prob[NAME_END] += const_smooth_val;
   }

   if(ppb)
   {
   prob[NAME_OTH] += ppb[NAME_OTH];
}
else
{
	prob[NAME_OTH] += const_default_val;
}

prob[NAME_MID] += const_smooth_val;

cnt++;
}

if(cnt != 2)
{
	ul_writelog(UL_LOG_NOTICE, "unvalid multichar lemma %s!\n", lemma);
	return -1;
}

if(IS_NAMEEND(plm->m_property))
{
	prob[NAME_END] += (int)(NORMALIZE_INTEGER*3*log((double)cnt));
}

scw_add_lmprob(pwdict,plm,prob,NAME_PROP_COUNT,type);
}
}
else if(type == DATA_FNAME)
{
	cnt = 0;
	if( IS_FNAMEEND(plm->m_property)
			|| IS_FNAMEBGN(plm->m_property)
			|| IS_FNAMEMID(plm->m_property) )
	{
		len = get_static_lmstr(pwdict,plm, lemma, sizeof(lemma));

		while((value = scw_get_word(lemma,(int&)len1,len,NULL)) != 0)
		{
			psblm = get_oneword_lemma(pwdict,value);

			if(psblm == NULL)
			{
				ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
				return -1;
			}

			ppb = scw_get_nameprob(pwdict,psblm,type);

			if(IS_FNAMEBGN(plm->m_property))
			{
				if(ppb)
				{
					if(cnt == 0)
					{
						prob[NAME_BGN] += ppb[NAME_BGN];  
					}
					else
					{
						prob[NAME_BGN] += ppb[NAME_MID];
					}
				}
				else
				{  
					prob[NAME_BGN] += const_default_val;  
				}
			}
			else
			{
				prob[NAME_BGN] += const_smooth_val;
			}

			if(IS_FNAMEMID(plm->m_property))
			{
				if(ppb)
				{
					prob[NAME_MID] += ppb[NAME_MID];
				}
				else
				{
					prob[NAME_MID] += const_default_val; 
				}
			}
			else
			{
				prob[NAME_MID] += const_smooth_val;
			}

			if(IS_FNAMEEND(plm->m_property))
			{
				if(ppb)
				{
					if(len1 == len)
					{
						prob[NAME_END] += ppb[NAME_END];
					}
					else
					{
						prob[NAME_END] += ppb[NAME_MID];
					}
				}
				else
				{
					prob[NAME_END] += const_default_val;
				}
			}
			else
			{
				prob[NAME_END] += const_smooth_val;
			}

			if(ppb)
			{
				if(cnt == 0)
				{
					prob[NAME_OTH] += ppb[NAME_BGN];
				}
				else if(len1 == len)
				{
					prob[NAME_OTH] += ppb[NAME_END];
				}
				else
				{
					prob[NAME_OTH] += ppb[NAME_MID];
				}
			}
			else
			{
				prob[NAME_OTH] += const_default_val;
			}

			cnt++;    
		}

		//if(IS_FNAMEBGN(plm->m_property)){
		prob[NAME_BGN] -= (int)(NORMALIZE_INTEGER*log((double)cnt));
	}                 

	if(IS_FNAMEMID(plm->m_property)){
		prob[NAME_MID] += (int)(NORMALIZE_INTEGER*log((double)cnt));
	}                 

	if(IS_FNAMEEND(plm->m_property)){
		prob[NAME_END] -= (int)(NORMALIZE_INTEGER*log((double)cnt));
	}

	prob[NAME_OTH] -= (int)(NORMALIZE_INTEGER*log((double)cnt));*/

		/*			scw_add_lmprob(pwdict,plm,prob,NAME_PROP_COUNT,type);  
					}    
					}
					else if(type == DATA_JNAME)
					{
					cnt = 0;
					if( IS_NAMEEND(plm->m_property)
					|| IS_SURNAME(plm->m_property)
					|| IS_NAMEMID(plm->m_property)
					|| IS_GIVENNAME(plm->m_property) )
					{
					len = get_static_lmstr(pwdict,plm, lemma, sizeof(lemma));

					while((value = scw_get_word(lemma,(int&)len1,len,NULL)) != 0)
					{
					psblm = get_oneword_lemma(pwdict,value);

					if(psblm == NULL)
					{
					ul_writelog(UL_LOG_NOTICE, "error in %s\n", where);
					return -1;
					}

					ppb = scw_get_nameprob(pwdict,psblm,type);

					if(IS_SURNAME(plm->m_property))
					{
					if(ppb)
					{
					if(cnt == 0)
					{
					prob[NAME_BGN] += ppb[NAME_BGN];
					}
					else
					{
					prob[NAME_BGN] += ppb[NAME_MID];
					}
					}
					else
					{
					prob[NAME_BGN] += const_default_val;
					}
					}
					else
					{
					prob[NAME_BGN] += const_smooth_val;
					}  

					if(IS_NAMEEND(plm->m_property) || IS_GIVENNAME(plm->m_property)){
					if(ppb)
					{
					if(len == len1)
					{
					prob[NAME_END] += ppb[NAME_END];
					}
					else
					{
					prob[NAME_END] += ppb[NAME_MID];
					}
					}
					else
					{
					prob[NAME_END] += const_default_val;
					}
					}
					else
					{
					prob[NAME_END] += const_smooth_val;
					}

					if(IS_NAMEMID(plm->m_property))
	{
		if(ppb)
		{
			prob[NAME_MID] +=  ppb[NAME_MID];
		}
		else
		{
			prob[NAME_MID] +=  const_default_val;
		}
	}
					else
					{
						prob[NAME_MID] += const_smooth_val;
					}

	if(ppb)
	{
		prob[NAME_OTH] += ppb[NAME_OTH];
	}
	else
	{
		prob[NAME_OTH] += const_default_val;
	}

	cnt++;
	}  

	scw_add_lmprob(pwdict,plm,prob,NAME_PROP_COUNT,type);
	}
	}

return 0;   
}
*/
/*
 * Initialize a inlemma  with the information from the line.
 * @param <pilemma> - the inlemma struct.
 * @param <line> - string buffer which hold the lemma info.
 * @return 1, succeeded; <0, failed.
 */
int update_lemma_with_line(scw_worddict_t* pwdict, Sdict_build * pdynwdict,scw_inlemma_t * pilemma,char * line)
{
	const char* where="update_lemma_with_line";
	char * pnext=NULL;
	char * pbegin=NULL;
	char str_prop[1024]={};
	char bwordstr[MAX_IL_LEN]={};
	char subphstr[MAX_IL_LEN]={};
	int ret=0;
	int ret1=0;

	assert(pilemma && line);
	if(strlen(line)>MAX_IL_LEN)
	{
		return -1;
	}
	pbegin=line;

	if(*pbegin != '[')
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	// get word
	if(extract_bracket(pilemma->m_lmstr,MAX_LEMMA_LENGTH,pbegin,&pnext)!=1)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	pbegin=pnext;

	//已经在worddict中存在的词条暂时不处理
	// if( scw_seek_lemma(pwdict, pilemma->m_lmstr,strlen(pilemma->m_lmstr)) != LEMMA_NULL)
	// {
	//	ul_writelog(UL_LOG_WARNING,"the line is %s already exists in worddict", pilemma->m_lmstr);
	//  return -1;
	// }

	// get word seperate result
	if(extract_bracket(bwordstr,sizeof(bwordstr),pbegin,&pnext)!=1)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	pbegin=pnext;

	// get phrase seperate result
	if(extract_bracket(subphstr,sizeof(subphstr),pbegin,&pnext)!=1)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	pbegin=pnext;

	// get property and weight
	if(sscanf(pbegin,"%s %d",str_prop,&(pilemma->m_weight))!= 2)
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}

	INIT_PROPERTY(pilemma->m_property);
	set_prop_by_str(str_prop,pilemma->m_property,pwdict->m_wdtype);

	if(pilemma->m_weight== 0)
	{
		pilemma->m_weight++;  // 0频率的话，加1平滑！
	}

	pilemma->m_bwcount = 1;
	pilemma->m_sphcount = 0;
	pilemma->m_type = LEMMA_TYPE_SBASIC;

	if(IS_PHRASE(pilemma->m_property))
	{  
		if((pwdict->m_wdtype & SCW_WD_CH) && IS_CH_NAME(pilemma->m_property))
		{
			ret = is_3human_need_reseg(pwdict,pilemma,bwordstr);

			if(ret < 0)
			{
				ul_writelog(UL_LOG_WARNING, "add %s failed!\n", line);
				return -1;
			}
			else if(ret>0)
			{
				ret=reseg_3human_phrase(pwdict,pilemma);
				if(ret<0)
				{ 
					ul_writelog(UL_LOG_WARNING, "add %s failed!\n", line);
					return -1;
				}
				else if(ret>0)
					ret1=1;
			}
		}

		if(ret1 == 0)
		{
			pilemma->m_bwcount=convert_format(pwdict,pdynwdict, bwordstr,
					pilemma->m_wordlist, sizeof(pilemma->m_wordlist), 1);
			pilemma->m_sphcount=convert_format(pwdict,pdynwdict, subphstr, 
					pilemma->m_subphlist, sizeof(pilemma->m_subphlist),0);
			if(pilemma->m_bwcount <= 1 || pilemma->m_sphcount<0)
			{
				ul_writelog(UL_LOG_WARNING, "add %s failed!\n", line);
				return -1;
			}
		}

		pilemma->m_type=LEMMA_TYPE_SPHRASE;
	}

	return 1;
}

/*
 * Initialize a inlemma  with the information from the line of man control.
 * @param <pilemma> - the inlemma struct.
 * @param <line> - format:[2009年初中试题] [0(2009年)1(初中)2(试题)].
 * @return 1, succeeded; <0, failed.
 */
int update_lemma_with_man(scw_worddict_t* pwdict, scw_inner_t *pir, scw_inlemma_t * pilemma,char * line )
{
	const char* where="update_lemma_with_man";
	char * pnext = NULL;
	char * pbegin = NULL;
	char   manwordstr[MAX_IL_LEN] = "\0";

	if(pwdict == NULL || pir == NULL || line == NULL || line[0] == '\0' || pilemma == NULL)
	{
		ul_writelog(UL_LOG_WARNING, "pwdict  or pir or line is null in function %s\n", where);
		return -1;
	}
	if(strlen(line)>MAX_IL_LEN)
	{
		ul_writelog(UL_LOG_WARNING, "length of line %s is more than 2408\n", line);
		return -1;
	}
	pbegin=line;

	if(*pbegin != '[')
	{
		ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
		return -1;
	}
	// get word
	if(extract_bracket(pilemma->m_lmstr,MAX_LEMMA_LENGTH,pbegin,&pnext)!=1)
	{
		ul_writelog(UL_LOG_WARNING, "error when extract_bracket in %s\n", where);
		return -1;
	}
	pbegin=pnext;

	//已经在worddict中存在的词条暂时不处理
	/*if( scw_seek_lemma(pwdict, pilemma->m_lmstr,strlen(pilemma->m_lmstr)) != LEMMA_NULL)
	{
		ul_writelog(UL_LOG_WARNING,"the line is %s already exists in worddict", pilemma->m_lmstr);
		return -1;
	}*/

	// 获得切分片段
	if(extract_bracket(manwordstr,sizeof(manwordstr),pbegin,&pnext)!=1)
	{
		ul_writelog(UL_LOG_WARNING, "error in extract_bracket of %s\n", where);
		return -1;
	}
	pbegin=pnext;

	INIT_PROPERTY(pilemma->m_property);
	pilemma->m_type = LEMMA_TYPE_MAN;
	pilemma->m_bwcount = COMMON_NULL;
	pilemma->m_sphcount = COMMON_NULL;
	pilemma->m_weight = 1;

	int count = convert_format_man(pwdict,pir, manwordstr,pilemma->m_manuallist,sizeof(pilemma->m_manuallist));
	if(count <= 1)
	{
		ul_writelog(UL_LOG_WARNING, "add %s failed! number of segment should be more than 1\n", line);
		return -1;
	}
	else 
		pilemma->m_mcount = count;

	return 0;
}

// inline functions defined as follows.
u_int scw_get_wdtype(scw_worddict_t * pwdict)
{
	return pwdict->m_wdtype;
}

void scw_set_wdtype(scw_worddict_t * pwdict,u_int type)
{
	pwdict->m_wdtype = type;
	return;
}

u_int scw_get_hashsize(u_int * sufentry)
{
	assert(sufentry);
	return sufentry[0];
}

u_int scw_get_backentry(u_int * sufentry)
{
	assert(sufentry);
	return sufentry[1];
}

void scw_set_hashsize(u_int * sufentry,u_int hashsize)
{
	assert(sufentry);
	sufentry[0]=hashsize;
	return ;
}

void scw_set_backentry(u_int * sufentry,u_int backentrypos)
{
	assert(sufentry);
	sufentry[1]=backentrypos;
	return;
}

u_int scw_get_sufentry(u_int* sufentry,u_int pos)
{
	assert(pos < sufentry[0]);
	return sufentry[pos+2];
}

void scw_set_sufentry(u_int* sufentry,u_int pos,u_int sepos)
{
	assert(pos < sufentry[0]);
	sufentry[2+pos]=sepos;
}

void scw_init_sufentry(u_int* sufentry, u_int hashsize, u_int backsepos)
{
	assert(sufentry);
	scw_set_hashsize(sufentry,hashsize);
	scw_set_backentry(sufentry,backsepos);
	for( int i=0; i<(int)hashsize; i++ )
	{
		sufentry[2+i] = DENTRY_NULL;
	}  
}

/*!
 * Find the next lemma by maximam matching algorithm.
 * @param <buff> - the input buffer. 
 * @param <len> - the length of <buffer>.
 * @param <curpos> - current position in buff.
 * @param <basiconly> - if only need basic word.
 * @return NULL, can not find a lemma, else lemma position.
 */
scw_lemma_t* mm_seek_lemma(scw_worddict_t* pwdict, char* buff, int slen, int& curpos, int basiconly)
{
	u_int value=0;
	u_int sufpos=0;
	u_int hsize=0;
	u_int hpos=0;
	u_int nde=0;
	u_int lmpos=COMMON_NULL;
	int newpos= curpos;

	if(curpos >= slen){
		return NULL;
	}

	// find the first entry
	sufpos=pwdict->m_entrance;
	value=scw_get_word(buff,curpos,slen,NULL);
	hsize=scw_get_hashsize(pwdict->m_seinfo+sufpos);
	hpos=value % hsize;
	nde=scw_get_sufentry(pwdict->m_seinfo+sufpos,hpos);
	if((nde==DENTRY_NULL)||(pwdict->m_dictentry[nde].m_value != value)){
		return NULL;
	}

	if(pwdict->m_dictentry[nde].m_lemma_pos != COMMON_NULL)
	{
		lmpos = pwdict->m_dictentry[nde].m_lemma_pos;
		newpos = curpos;
	}
	sufpos=pwdict->m_dictentry[nde].m_suffix_pos;

	while(curpos<slen)
	{
		if(sufpos ==SUFENTRY_NULL){
			break;
		}
		value=scw_get_word(buff,curpos,slen,NULL);
		hsize=scw_get_hashsize(pwdict->m_seinfo+sufpos);
		hpos=value % hsize;
		nde=scw_get_sufentry(pwdict->m_seinfo+sufpos,hpos);
		if((nde==DENTRY_NULL)||(pwdict->m_dictentry[nde].m_value != value)){
			break;
		}

		if(pwdict->m_dictentry[nde].m_lemma_pos!=COMMON_NULL)
		{
			u_int old_lmpos = lmpos;
			int back_newpos = newpos;
			lmpos = pwdict->m_dictentry[nde].m_lemma_pos;
			newpos = curpos;
			if(basiconly==1)
			{
				if(pwdict->m_lemmalist[lmpos].m_type==LEMMA_TYPE_SPHRASE)
				{
					lmpos = old_lmpos;
					newpos = back_newpos;
					break;
				}
			}
		}

		sufpos=pwdict->m_dictentry[nde].m_suffix_pos;
	}

	curpos = newpos;

	if(lmpos==COMMON_NULL){
		return NULL;
	}
	else{
		return &(pwdict->m_lemmalist[lmpos]);
	}
}
//
/*
   void scw_init_utilinfo(scw_utilinfo_t * puti,int wdtype)
   {
   puti->m_ambsuffix_pos = COMMON_NULL;
   puti->m_fnameprob_pos = COMMON_NULL;

   if(wdtype & SCW_WD_JP)
   puti->m_jnameprob_pos = COMMON_NULL;
   else if(wdtype & SCW_WD_CH)
   puti->m_cnameprob_pos = COMMON_NULL;
   return;
   }
   */
// the fragfile must be sorted! 
int scw_build_ambfrag(const char* fragfile,scw_worddict_t* pwdict)
{
	char * where = "scw_build_ambfrag";
	FILE * fp = fopen(fragfile,"r");

	if(fp == NULL){
		return 0;
	}

	char flemma[MAX_WORD_LEN]={};
	char slemma[MAX_WORD_LEN]={};
	char pre_flemma[MAX_WORD_LEN]={};
	int flen = 0;
	int slen = 0; 
	u_int amb_flmpos = LEMMA_NULL;
	u_int slmpos = LEMMA_NULL;

	pre_flemma[0] = 0;
	while(fscanf(fp,"%s\t%s\n",flemma,slemma)!=EOF)
	{
		flen = strlen(flemma);
		slen = strlen(slemma);

		if(strcmp(flemma,pre_flemma) != 0 || amb_flmpos == LEMMA_NULL)
		{
			if((amb_flmpos = scw_seek_lemma(pwdict,flemma,flen)) == LEMMA_NULL)
			{
				ul_writelog(UL_LOG_NOTICE, "add amb frag %s %s failed!\n", flemma,slemma);
				continue;
			}
		}

		if((slmpos = scw_seek_lemma(pwdict,slemma,slen)) == LEMMA_NULL)
		{
			ul_writelog(UL_LOG_NOTICE, "add amb frag %s %s failed!\n", flemma,slemma);
			continue;
		}

		if(scw_insert_ambsuffix(pwdict,amb_flmpos,slmpos) <0)
		{
			ul_writelog(UL_LOG_NOTICE, "error in %s!\n",where);
			continue;
		}

		memcpy(pre_flemma,flemma,flen);
		pre_flemma[flen] = 0;
	}

	fclose(fp);

	return 0;
}
// 
int scw_insert_ambsuffix(scw_worddict_t* pwdict,u_int flmpos,u_int slmpos)
{
	char * where = "scw_insert_ambsuffix";
	scw_lemma_t * fplm=NULL;
	u_int hash = 1;
	u_int newhash = 0;
	u_int * pinfo = NULL;
	u_int tmppos = 0;
	u_int sufpos = 0;
	scw_utilinfo_t * puti = 0;
	u_int tmpset[MAX_AMB_NUM];
	u_int amb_num = 0;
	u_int value = 0;
	int conflict = 0;

	fplm = &(pwdict->m_lemmalist[flmpos]);

	if(fplm->m_utilinfo_pos == COMMON_NULL)
	{
		if(pwdict->m_utilipos + 1 >= pwdict->m_utilisize)
		{
			if(scw_resize_utilinfo(pwdict,pwdict->m_utilisize*2)!=1)
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
				return -1;
			}
		}

		puti = pwdict->m_utilinfo + pwdict->m_utilipos;
		scw_init_utilinfo(puti,pwdict->m_wdtype);
		fplm->m_utilinfo_pos = pwdict->m_utilipos;
		pwdict->m_utilipos++;
	}
	else
	{
		puti = pwdict->m_utilinfo + fplm->m_utilinfo_pos;
	}

	if((sufpos=puti->m_ambsuffix_pos) == COMMON_NULL)
	{
		if(pwdict->m_ambpos + hash + 1 >= pwdict->m_ambsize)
		{
			if(scw_resize_ambinfo(pwdict,pwdict->m_ambsize*2)!=1 )
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
				return -1;
			}
		}
		pwdict->m_ambinfo[pwdict->m_ambpos] = hash;
		pwdict->m_ambinfo[pwdict->m_ambpos + 1] = slmpos;
		puti->m_ambsuffix_pos = pwdict->m_ambpos;
		pwdict->m_ambpos += 2;
		return 0;
	}

	pinfo = pwdict->m_ambinfo + sufpos;
	hash = pinfo[0];
	value = slmpos + 1;
	tmppos = value % hash;
	if(pinfo[tmppos+1] == COMMON_NULL)
	{
		pinfo[tmppos+1] = slmpos;
		return 0;
	}

	newhash = hash+1;

	for(u_int i=0;i<hash;i++)
	{
		if(pinfo[i+1] != COMMON_NULL)
		{
			if(amb_num+1 > MAX_AMB_NUM)
				return 0;
			tmpset[amb_num] = pinfo[i+1];
			amb_num++;    
		}
	}

	assert(pwdict->m_ambpos - hash -1 == sufpos);

	while(1)
	{
		conflict = 0;

		if(pwdict->m_ambpos + newhash - hash >= pwdict->m_ambsize)
		{
			if(scw_resize_ambinfo(pwdict,pwdict->m_ambsize*2)!=1 )
			{
				ul_writelog(UL_LOG_WARNING, "error in %s\n", where); 
				return -1;
			}
		}

		pinfo = pwdict->m_ambinfo + sufpos;
		scw_init_ambsuffix(pinfo,newhash);

		for(u_int i=0;i<amb_num;i++)
		{
			value = tmpset[i] + 1;
			tmppos = value % newhash;
			if(pinfo[tmppos+1] == COMMON_NULL)
			{
				pinfo[tmppos+1] = tmpset[i];
			}
			else
			{
				conflict = 1;
				break;
			}      
		}

		if(conflict == 0)
		{
			value = slmpos + 1;
			tmppos = value % newhash;
			if(pinfo[tmppos+1] == COMMON_NULL)
			{
				pinfo[tmppos+1] = slmpos;
				pwdict->m_ambpos = sufpos + newhash + 1;
				return 0;
			}
			else
			{
				conflict = 1;
			}
		}  

		newhash++;
	}

	return 0;
}

/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
void scw_init_ambsuffix(u_int * ambinfo,u_int hash)
{
	assert(ambinfo);
	ambinfo[0] = hash;
	for(u_int i=0;i<hash;i++)
	{
		ambinfo[i+1] = COMMON_NULL;
	}

	return;
}

/*
   int scw_seek_ambfrag(scw_worddict_t* pwdict,scw_lemma_t * pflm,scw_lemma_t * pslm,u_int slmpos)
   {
   u_int tmppos  = 0;
   u_int utilpos = 0;
   u_int * pinfo = NULL;

   if((utilpos = pflm->m_utilinfo_pos) != COMMON_NULL
   && pwdict->m_utilinfo[utilpos].m_ambsuffix_pos != COMMON_NULL)
   {
   pinfo = pwdict->m_ambinfo + pwdict->m_utilinfo[utilpos].m_ambsuffix_pos;

   tmppos = (slmpos+1) % pinfo[0];

   if(pinfo[tmppos+1] == slmpos)
   return 1;
   }

   if((pwdict->m_wdtype & SCW_WD_CH))
   {
   if(IS_LOC(pflm->m_property) && IS_LOC_FOLLOW(pslm->m_property))
   return 1;
   }

   return 0;
   }
   */

/*
   int scw_add_lmprob(scw_worddict_t * pwdict,scw_lemma_t * plm,int * prob,int pbsize,int type)
   {
   const char * where = "scw_add_lmprob";
   int * plm_prob = NULL;
   scw_utilinfo_t * puti = NULL;
   u_int * ppbpos =NULL;

   if(plm == NULL || prob == NULL || pwdict == NULL)
   return 0;

   if(plm->m_utilinfo_pos == COMMON_NULL)
   {
   if(pwdict->m_utilipos + 1 >= pwdict->m_utilisize)
   {
   if(scw_resize_utilinfo(pwdict,pwdict->m_utilisize*2)!=1)
   {
   ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
   return -1;
   }
   }
   puti = pwdict->m_utilinfo + pwdict->m_utilipos;
   scw_init_utilinfo(puti,pwdict->m_wdtype);
   plm->m_utilinfo_pos = pwdict->m_utilipos;
   pwdict->m_utilipos++;
   }
   else
   {
   puti = pwdict->m_utilinfo + plm->m_utilinfo_pos;
   }

   if(type == DATA_CNAME)
   {
   ppbpos = &puti->m_cnameprob_pos;  
   }
   else if(type == DATA_FNAME)
   {
   ppbpos = &puti->m_fnameprob_pos;
   }
   else if(type == DATA_JNAME)
   {
   ppbpos = &puti->m_jnameprob_pos;
   }

   if(*ppbpos != COMMON_NULL)//lemma prob exist
   {
   plm_prob = pwdict->m_probbuf + (*ppbpos);
   memcpy(plm_prob,prob,pbsize*sizeof(int));
   return 0;
   }

   if(pwdict->m_pbpos + pbsize >= pwdict->m_pbsize)
   {
   if(scw_resize_probbuf(pwdict,pwdict->m_pbsize*2) != 1)
   {
   ul_writelog(UL_LOG_WARNING, "error in %s\n", where);
   return -1;
   }
   }

   plm_prob = pwdict->m_probbuf + pwdict->m_pbpos;
   memcpy(plm_prob,prob,pbsize*sizeof(int));
 *ppbpos = pwdict->m_pbpos;
 pwdict->m_pbpos += pbsize;

 return 0;
 }
 */
//
/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
int scw_add_extra_property(char * propfile,scw_worddict_t* pwdict)
{
	FILE * fp = fopen(propfile,"r");
	if(fp == NULL)
		return 0;

	char line[MAX_IL_LEN];
	char lemma[MAX_WORD_LEN];
	char property[MAX_WORD_LEN];
	u_int lmpos = LEMMA_NULL;
	int len = 0;
	scw_lemma_t * plm = NULL;

	while(fgets(line,MAX_IL_LEN,fp))
	{
		len = strlen(line);
		while(line[len-1] == '\n'
				|| line[len-1] == '\r')
			len--;
		line[len] = 0;
		if(sscanf(line,"%s%s",lemma,property) == 2)
		{
			if((lmpos = scw_seek_lemma(pwdict,lemma,strlen(lemma))) == LEMMA_NULL)
			{
				ul_writelog(UL_LOG_NOTICE, "add lemma %s property  %s failed!\n", lemma,property);
				continue;
			}
			plm = &(pwdict->m_lemmalist[lmpos]);
			set_prop_by_str(property,plm->m_property,pwdict->m_wdtype);
		}
	}

	fclose(fp);
	return 0;
}
/*
   int scw_build_nameinfo(const char * namefile,int type,scw_worddict_t * pwdict)
   {
   FILE * fp = fopen(namefile,"r");
   if(fp == NULL)   
   return 0;

   char line[MAX_IL_LEN];
   char lemma[MAX_LEMMA_LENGTH];
   double prob[NAME_PROP_COUNT];
   int lnprob[NAME_PROP_COUNT];
   int len = 0;
   int i = 0,multibgn = 0;
   u_int lmpos = LEMMA_NULL;
   scw_lemma_t * plm = NULL;

   for(i=0;i<(int)pwdict->m_lmpos;i++)
   {
   plm = &(pwdict->m_lemmalist[i]);
   if(type == DATA_CNAME)
   {
   if(IS_ONEWORD(plm->m_property))
   {
   if(!IS_HZ(plm->m_property))
   continue;  

   for(int j=0;j<(int)NAME_PROP_COUNT;j++){
   lnprob[j] = (int)(NORMALIZE_INTEGER*log(LEMMA_SMOOTH_PROB));
   }

   lnprob[NAME_MID]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB));
   lnprob[NAME_END]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB));

   scw_add_lmprob(pwdict,plm,lnprob,NAME_PROP_COUNT,type);
   }
   else
   {
   multibgn = i;
   break;
   }
   }
   else if(type == DATA_FNAME)
   {
   if(IS_ONEWORD(plm->m_property))
   {
   if(!IS_HZ(plm->m_property))
   continue;
   }
   else
   {
   multibgn = i;
   break;
   }
   }
   else if(type == DATA_JNAME)
   {
   if(IS_SURNAME(plm->m_property) || IS_GIVENNAME(plm->m_property) 
   || IS_NAMEEND(plm->m_property) || IS_NAMEMID(plm->m_property))
   {
   for(int j=0;j<(int)NAME_PROP_COUNT;j++)
   {
   lnprob[j] = (int)(NORMALIZE_INTEGER*log(LEMMA_SMOOTH_PROB_JP));
   }
   lnprob[NAME_MID]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB_JP));
   lnprob[NAME_END]  = (int)(NORMALIZE_INTEGER*log(LEMMA_DEFAULT_PROB_JP));

   scw_add_lmprob(pwdict,plm,lnprob,NAME_PROP_COUNT,type);
   }
   }
   }

while(fgets(line,MAX_IL_LEN,fp))
{
	len = strlen(line);
	while(line[len-1] == '\n'
			|| line[len-1] =='\r')
		len--;
	line[len] = 0;

	if(sscanf(line,"%s%lf%lf%lf%lf",lemma,&prob[NAME_BGN],&prob[NAME_MID],
				&prob[NAME_END],&prob[NAME_OTH]) != 5)
	{
		ul_writelog(UL_LOG_NOTICE,"add nameinfo line %s failed!\n",line);
		continue;
	}

	if((lmpos = scw_seek_lemma(pwdict,lemma,strlen(lemma))) == LEMMA_NULL)
		continue;

	plm = &(pwdict->m_lemmalist[lmpos]);

	for(int j=0;j<(int)NAME_PROP_COUNT;j++){
		lnprob[j] = (int)(NORMALIZE_INTEGER*log(prob[j]+LEMMA_SMOOTH_PROB));
	}

	scw_add_lmprob(pwdict,plm,lnprob,NAME_PROP_COUNT,type);
}

if(type == DATA_CNAME || type == DATA_FNAME)
{
	for(i = multibgn;i<(int)pwdict->m_lmpos;i++)
	{
		plm = &(pwdict->m_lemmalist[i]);
		update_multichr_lmprob(pwdict,plm,type);
	}
}

fclose(fp);

return 0;
}
*/

//
/*!
 * The comments of this funtion will be updated later.
 * @comments will be updated later
 * @thank you!
 */
void scw_show_version()
{
	printf("SEGVER: %s\n", SEGVER);
	return ;
}

/*
 * get wordseg version
 * success : length of version str;
 * fail : -1;
 */
int scw_get_wordseg_version(char *str, int len)
{
	const char* where="scw_get_wordseg_version";
	int seg_len = strlen(SEGVER);
	if(seg_len >= len)
	{
		ul_writelog(UL_LOG_FATAL, "error in %s, wordseg version is long than string length\n", where);
		return -1;
	}

	if(	str == NULL || seg_len <= 0)
	{
		ul_writelog(UL_LOG_FATAL, "error in %s, str is null or seg_len is smaller than 0\n", where);
		return -1;
	}

	strncpy(str, SEGVER, seg_len);
	str[seg_len] = 0;
	return seg_len;
}

/* get the dict version
 * success:return the version in high 3 byte in u_int
 * fatal: return 0
 */
u_int scw_get_version()
{
	const char* where="scw_get_version";

	char version_str[256];
	version_str[0] = '\0';
	char delims[] = "-";
	char* p_version_num = NULL;

	u_int version_return = 0;
	int i = 3;

	sprintf(version_str,"%s", SEGVER);
	p_version_num = strtok(version_str, delims);
	while(p_version_num != NULL)
	{
		int version_temp1 = 0;
		u_int version_temp2 = 0;
		version_temp1 = atoi(p_version_num);
		version_temp2 = (unsigned int)version_temp1;
		version_return |= version_temp2 << (i * 8);
		i --;
		p_version_num = strtok(NULL, delims);
	}

	if(i != 0)
	{
		ul_writelog(UL_LOG_FATAL, "error in %s , the seg version not in right format\n", where);
		return 0;
	}
	else
	{
		return version_return;
	}
}
