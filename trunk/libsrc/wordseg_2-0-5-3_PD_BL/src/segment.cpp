/* segment.h
 * Cont: Implementation of interface methods in isegment.h.
 * Author: David Dai
 * Created: 2007/08 
 */

#include "isegment.h"
#include "scwdef.h"
#include <ul_log.h>
#include "international.h"

// <typedef>
//------------------------------------------------------------------------------------------------------------------//
struct _handle_t
{
	scw_worddict_t* dict;
	scw_inner_t* agent;
};
// </typedef>

// <log>
//------------------------------------------------------------------------------------------------------------------//
// Macro Definition
#define LOG_ERROR(message)  ul_writelog(UL_LOG_FATAL, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
#define WARNING_LOG(message)  ul_writelog(UL_LOG_WARNING, "%s:%d:%s(): %s", __FILE__, __LINE__, __FUNCTION__, message)
// </log>

// <declare>
//------------------------------------------------------------------------------------------------------------------//
int _get_basic_token( scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, token_t tokens[], unsigned& pos, unsigned& start, unsigned max);
int _get_phrase_token( scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, token_t tokens[], unsigned& pos, unsigned& start, unsigned max);
int _get_pername_token( scw_worddict_t* pwdict, scw_inner_t* pir, scw_lemma_t* plm, token_t tokens[], unsigned& pos, unsigned& start, unsigned max );
// </declare>


// <interface-definition>
//------------------------------------------------------------------------------------------------------------------//
#define MAX_PATH_LENG 512
dict_ptr_t seg_dict_open( const char* dictpath )
{
	if( !dictpath )
		return 0;

	if( strlen(dictpath) >= MAX_PATH_LENG - 1 )
	{
		LOG_ERROR("error: length of path is too long");
		return -1;
	}

	dict_ptr_t ptr = 0;
	char path[MAX_PATH_LENG] = {0};
	strncpy(path,dictpath,strlen(dictpath));
	strncat(path,"/",1);
	path[strlen(dictpath)+1]=0;

	ptr = (dict_ptr_t)scw_load_worddict(path); // new!
	if( !ptr )
	{    
		LOG_ERROR("error: couldn't load the dictionary");
		return 0;
	}

	return ptr;
}

void seg_dict_close( dict_ptr_t pdict )
{
	if( pdict )
		scw_destroy_worddict( (scw_worddict_t*)pdict ); // delete!
}


handle_t seg_open( dict_ptr_t pdict, uint32_t size )
{
	if( !pdict )
		return 0;

	_handle_t* iHandle = new _handle_t(); // new!
	iHandle->dict = (scw_worddict_t*)pdict;
	iHandle->agent = scw_create_inner(size, SCW_OUT_ALL | SCW_OUT_PROP); // new!

	return (handle_t)iHandle;
}

/*
   ���������ͬһ��handle��β������ͻ�����core dump��
   �����������Ҫһ��handle��������ÿ�ε���seg_open������handle��
   �����ռ��������ڽ��̽���ǰͳһ�ͷš�
   ���ڵ���������ĵ��о����û����ܶ�ͬһ��handle��ιرգ�
   */
void seg_close( handle_t handle )
{  
	if( handle )
	{
		_handle_t* iHandle = (_handle_t*)handle;
		if( iHandle->agent )
		{
			scw_destroy_inner( iHandle->agent ); // delete!
			iHandle->agent = 0;
		}
		delete iHandle;  // delete!    
	}
}


int seg_segment( handle_t handle, const char* text, uint32_t length, token_t result[], uint32_t max )
{
	if( !text || !length || !handle )
		return 0;

	// segment the input text
	_handle_t* iHandle = (_handle_t*)handle;
	if( scw_seg(iHandle->dict,iHandle->agent,text,length) < 0 )
	{    
		WARNING_LOG("error: met exception when segmenting the input text");
		return 0;
	}

	// fill the segmented tokens
	unsigned offset = 0, pos = 0;
	scw_lemma_t* plm = NULL;  
	scw_inner_t* pir = iHandle->agent;
	for(unsigned i=0; i<pir->m_ppseg_cnt; i++)
	{    
		plm = pir->m_ppseg[i]; 
		if( !plm ) continue; // ���ﲻ����break����Ϊm_ppseg�е�Ԫ�ؿ����ǲ������ģ�

		unsigned len = plm->m_length; 
		if( len && (pos < max) )
		{        
			result[pos].offset = offset;
			result[pos].length = len;
			result[pos].type = 0;
			result[pos].weight = plm->m_weight;
			result[pos].prop.m_lprop = plm->m_property.m_lprop;
			result[pos].prop.m_hprop = plm->m_property.m_hprop;
			result[pos].index = (long)plm;

			memcpy(result[pos].buffer,text+offset,len);
			result[pos].buffer[len] = 0;

			pos++;
		}
		offset += len;
	}

	return pos;
}

int seg_newword( handle_t handle,token_t newword[], uint32_t max )
{
	if( !handle )
		return -1;

	_handle_t* iHandle = (_handle_t*)handle;    
	scw_inner_t* pir = iHandle->agent;

	u_int tmcnt=0, len=0, bufsize=0;
	u_int *tmpos=NULL, *tmoff=NULL;
	scw_property_t* tmprop = NULL;

	char* src = NULL; int pos = 0;
	scw_newword_t* pnewword = pir->pnewword;
	src    = pnewword->newwordbuf;
	len    = pnewword->newwordb_curpos;
	bufsize  = pnewword->newwordbsize;
	tmcnt  = pnewword->newwordbtermcount;
	tmpos  = pnewword->newwordbtermpos;
	tmoff  = pnewword->newwordbtermoffsets;
	tmprop  = pnewword->newwordbtermprop;


	if( tmcnt > max)
		tmcnt = max;

	for( unsigned i=0; i<tmcnt; i++ )
	{
		pos = GET_TERM_POS(tmpos[i]);
		len = GET_TERM_LEN(tmpos[i]);

		strncpy(newword[i].buffer, src+pos, len);  
		newword[i].buffer[len] = '\0';
		newword[i].offset = tmoff[i*2]; // ---�����offset��2λ����һλװ���ǻ��ŵı�ţ��ڶ�λ�ǰ������ŵĸ���
		newword[i].length = len;
	//	if( type & SCW_OUT_PROP )
		{
			newword[i].prop.m_lprop = tmprop[i].m_lprop;
			newword[i].prop.m_hprop = tmprop[i].m_hprop;
		}
		newword[i].type = 0;
		newword[i].index = 0;
	}

	return tmcnt;
}
int seg_tokenize( handle_t handle,token_t token, uint32_t type, token_t subtokens[], uint32_t max )
{
	if( !handle )
		return 0;

	_handle_t* iHandle = (_handle_t*)handle;    
	scw_worddict_t* pwdict = iHandle->dict;
	scw_inner_t* pir = iHandle->agent;
	scw_lemma_t* plm = (scw_lemma_t*)token.index;  

	if( !plm )
		return 0;

	unsigned offset = 0, pos = 0;

	// ������
	if(type == TOK_BASIC)
	{
		if(_get_basic_token(pwdict, pir, plm, subtokens, pos, offset, max) < 0)
		{      
			return 0;
		}  
	}  
	// �Ӷ�������� ��
	else if( (type == TOK_SUBPHR) && IS_PHRASE(plm->m_property))
	{      
		if( IS_HUMAN_NAME(plm->m_property) ) // ������ﱾ�������������ټ�����Ƿ����Ӷ���
		{
			unsigned len = plm->m_length;
			if(pos < max)
			{
				subtokens[pos].offset = offset;
				subtokens[pos].length = len;
				subtokens[pos].weight = plm->m_weight;
				subtokens[pos].prop.m_lprop = plm->m_property.m_lprop;
				subtokens[pos].prop.m_hprop = plm->m_property.m_hprop;
				pos++;    
			}
			offset += len;
		}
		else if(_get_phrase_token(pwdict, pir, plm, subtokens, pos, offset, max) < 0)
		{      
			return 0;
		}      
	}
	// ����
	else if(type == TOK_PERNAME)
	{      
		if( IS_HUMAN_NAME(plm->m_property) ) // ������ﱾ���������������жϹ��ɴ˶���Ļ����ʻ����Ӷ����Ƿ�Ϊ����
		{
			unsigned len = plm->m_length;
			if(pos < max)
			{
				subtokens[pos].offset = offset;
				subtokens[pos].length = len;
				subtokens[pos].weight = plm->m_weight;
				subtokens[pos].prop.m_lprop = plm->m_property.m_lprop;
				subtokens[pos].prop.m_hprop = plm->m_property.m_hprop;
				pos++;    
			}
			offset += len;
		}
		else if( IS_PHRASE(plm->m_property) ) // �����ж����Ӵ��Ƿ�Ϊ����
		{
			if(_get_pername_token(pwdict, pir, plm, subtokens, pos, offset, max) < 0)
			{      
				return 0;
			}  
		}      
	}  

	char* term = token.buffer;
	for(unsigned i=0; i<pos; i++)
	{
		subtokens[i].type = 0;
		subtokens[i].index = 0; // ע�������ʹ�ô����ʱ����뽫���ʼ��Ϊ0��
		memcpy(subtokens[i].buffer,term+subtokens[i].offset,subtokens[i].length);
		subtokens[i].buffer[subtokens[i].length] = 0;
	}

	return pos;
}
// </interface-definition>


// <internal-function-definition>
//------------------------------------------------------------------------------------------------------------------//
/*!Get all basic tokens info recursively.
 * @param <tokens> - [out] store the info of tokens.
 * @param <pos> - [in][out] suffix of current token, start from 1.
 * @param <start> - [in][out] byte offset of current token current input text.
 * @return If fails, return -1.    
 */
int _get_basic_token( scw_worddict_t* pwdict, 
		scw_inner_t* pir, 
		scw_lemma_t* plm, 
		token_t tokens[], 
		unsigned& pos, unsigned& start, unsigned max )
{
	//assert(tokens && (pos >= 0) && (start >= 0));

	if(plm == NULL)
	{
		return start;
	}  

	int type = plm->m_type;

	switch(type)
	{
		case LEMMA_TYPE_SBASIC:  
		case LEMMA_TYPE_DBASIC:      
			if( pos < max )
			{
				tokens[pos].offset = start;
				tokens[pos].length = plm->m_length;
				tokens[pos].weight = plm->m_weight;
				tokens[pos].prop.m_lprop = plm->m_property.m_lprop;
				tokens[pos].prop.m_hprop = plm->m_property.m_hprop;
				pos++;
			}

			start += plm->m_length;
			break;

			// ����Ƕ����ݹ��ȡ
		case LEMMA_TYPE_SPHRASE:  
			{
				int cnt = pwdict->m_phinfo[plm->m_phinfo_bpos];
				for(int i=0; i<cnt; i++)
				{
					int pos2 = pwdict->m_phinfo[plm->m_phinfo_bpos+i+1];
					scw_lemma_t* plm2= &pwdict->m_lemmalist[pos2];

					_get_basic_token(pwdict,pir,plm2,tokens,pos,start,max);          
				}
			}
			break;

		case LEMMA_TYPE_DPHRASE:
			{
				int cnt = pir->m_dynphinfo[plm->m_phinfo_bpos];
				for(int i=0; i<cnt; i++)
				{
					scw_lemma_t* plm2 = (scw_lemma_t*)pir->m_dynphinfo[plm->m_phinfo_bpos+i+1];

					_get_basic_token(pwdict,pir,plm2,tokens,pos,start,max);
				}
			}
			break;

		default:
			return -1;
	}

	return start;
}

//[���ﲻ��Ҫ�ݹ�����ˣ���������������Ҽ��ɣ�]
/*!Get all phrase tokens info. 
 * @param <tokens> - out,  store the info of tokens.
 * @param <pos> - in & out, suffix of current token, start from 1.
 * @param <start> - in & out, byte offset of current token current input text.
 * @return If fails, return -1.    
 */
int _get_phrase_token( scw_worddict_t* pwdict, 
		scw_inner_t* pir, 
		scw_lemma_t* plm, 
		token_t tokens[], 
		unsigned& pos, unsigned& start, unsigned max )
{
	//assert(tokens && (pos >= 0) && (start >= 0));

	if(plm == NULL)
	{
		return start;
	}

	int type = plm->m_type;

	switch(type)
	{  
		case LEMMA_TYPE_SBASIC:  
		case LEMMA_TYPE_DBASIC:    
			start += plm->m_length;
			break;

		case LEMMA_TYPE_SPHRASE:  
			{
				int offset = start;
				// ����
				int len = plm->m_length;
				if( len )
				{          
					if( pos < max )
					{
						tokens[pos].offset = start;
						tokens[pos].length = len;
						tokens[pos].weight = plm->m_weight;
						tokens[pos].prop.m_lprop = plm->m_property.m_lprop;
						tokens[pos].prop.m_hprop = plm->m_property.m_hprop;
						pos++;
					}        
				}    
				// �Ӷ���
				if( plm->m_subphinfo_bpos != COMMON_NULL )
				{        
					// Ϊ���ܼ�����Ӷ����ڵ�ǰ�����е�ƫ�ƣ���ҪԤȡ��ǰ��������л����ʳ��ȣ�
					token_t lens[MAX_LEMMA_LENGTH];
					unsigned bascnt = 1, startpos = 0;
					_get_basic_token(pwdict,pir,plm,lens,bascnt,startpos,max);

					int cnt = pwdict->m_phinfo[plm->m_subphinfo_bpos];        
					for(int i=0; i<cnt; i++)
					{          
						// ���ڼ���ʱ������Ҫ�Դ����ȴʲ�ֵ�Ե�ʣ����offset��Ӧ�Ĳ��Ƕ�������丸������ֽ�ƫ�ƣ����ǵڼ��������ʼ�����
						// ����취�ǣ�Ҫô�޸Ĵʵ����ݣ�Ҫô��֮ǰԤȡ���л����ʳ��ȣ�ǰ����Ȼ��ͨ�ã�������ʹ�õ��Ǻ��ߣ�

						// �����Ӷ����ֽ�ƫ��
						int newoff = offset;
						int basoff = pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+1]; 
						for(int j=1; j<(int)bascnt && j<=basoff; j++)
							newoff += lens[j].length;          

						//int newoff = offset + pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+1]; // �˴�����Ҫ�ڴʵ��д����Ӷ��������ֽ�ƫ�ƺ����ã�
						int pos2 = pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+2];
						scw_lemma_t* plm2 = &pwdict->m_lemmalist[pos2];

						len = plm2->m_length;
						if( len && (pos < max) )
						{
							tokens[pos].offset = newoff;
							tokens[pos].length = len;
							tokens[pos].weight = plm2->m_weight;
							tokens[pos].prop.m_lprop = plm2->m_property.m_lprop;
							tokens[pos].prop.m_hprop = plm2->m_property.m_hprop;
							pos++;
						}            
					}
				}

				start += plm->m_length;
			}
			break;

		case LEMMA_TYPE_DPHRASE:
			{
				int offset = start;
				// ����      
				int len = plm->m_length;
				if( len )
				{
					if( pos < max )
					{
						tokens[pos].offset = start;
						tokens[pos].length = len;
						tokens[pos].weight = plm->m_weight;
						tokens[pos].prop.m_lprop = plm->m_property.m_lprop;
						tokens[pos].prop.m_hprop = plm->m_property.m_hprop;
						pos++;
					}        
				}
				// �Ӷ���
				if( plm->m_subphinfo_bpos != COMMON_NULL )
				{        
					// Ԥȡ�����ʳ���
					token_t lens[MAX_LEMMA_LENGTH];
					unsigned bascnt = 1, startpos = 0;
					_get_basic_token(pwdict,pir,plm,lens,bascnt,startpos,max);

					int cnt = pir->m_dynphinfo[plm->m_subphinfo_bpos];
					for(int i=0; i<cnt; i++)
					{
						// �����Ӷ����ֽ�ƫ��
						int newoff = offset;
						int basoff = pir->m_dynphinfo[plm->m_subphinfo_bpos+i*2+1]; 
						for(int j=1; j<(int)bascnt && j<=basoff; j++)
							newoff += lens[j].length;  

						//int newoff = offset + pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+1];
						scw_lemma_t* plm2 = (scw_lemma_t*)pir->m_dynphinfo[plm->m_subphinfo_bpos+i*2+2];

						len = plm2->m_length;
						if( len && (pos < max) )
						{
							tokens[pos].offset = newoff;
							tokens[pos].length = len;
							tokens[pos].weight = plm2->m_weight;
							tokens[pos].prop.m_lprop = plm2->m_property.m_lprop;
							tokens[pos].prop.m_hprop = plm2->m_property.m_hprop;
							pos++;
						}      
					}
				}  

				start += plm->m_length;
			}
			break;

		default:
			return -1;
	}

	return start;
}

// Get all person name tokens recursively.
// If fails, return -1.
int _get_pername_token( scw_worddict_t* pwdict, 
		scw_inner_t* pir, 
		scw_lemma_t* plm, 
		token_t tokens[], 
		unsigned& pos, unsigned& start, unsigned max )
{
	//assert(tokens && (pos >= 0) && (start >= 0));

	if(plm == NULL)
	{
		return start;
	}  

	int type = plm->m_type;

	switch(type)
	{
		case LEMMA_TYPE_SBASIC:  
			if( IS_HUMAN_NAME(plm->m_property) )
			{
				if( pos < max )
				{
					tokens[pos].offset = start;
					tokens[pos].length = plm->m_length;
					tokens[pos].weight = plm->m_weight;
					tokens[pos].prop.m_lprop = plm->m_property.m_lprop;
					tokens[pos].prop.m_hprop = plm->m_property.m_hprop;
					pos++;
				}    
			}

			start += plm->m_length;
			break;

		case LEMMA_TYPE_DBASIC:  
			if( IS_HUMAN_NAME(plm->m_property) )
			{
				if( pos < max )
				{
					tokens[pos].offset = start;
					tokens[pos].length = plm->m_length;
					tokens[pos].weight = plm->m_weight;
					tokens[pos].prop.m_lprop = plm->m_property.m_lprop;
					tokens[pos].prop.m_hprop = plm->m_property.m_hprop;
					pos++;
				}
			}

			start += plm->m_length;
			break;

			// ����Ƕ�����ж�������ʺ��Ӷ����Ƿ�������
		case LEMMA_TYPE_SPHRASE:  
			{      
				if( plm->m_subphinfo_bpos == COMMON_NULL )
				{
					start += plm->m_length;
					break;
				}

				// �ȼ�������(һ������²���Ҫ�������ʣ������ڼ���Ӷ���ʱ��Ҫ��ȡ��������Ϣ������ƫ�ƣ���������һ������˻����ʣ�)
				int lens[MAX_LEMMA_LENGTH] = {0}; 
				int newoff = start;
				int bascnt = (int)pwdict->m_phinfo[plm->m_phinfo_bpos];
				for(int i=0; i<bascnt; i++)
				{
					int pos2 = pwdict->m_phinfo[plm->m_phinfo_bpos+i+1];
					scw_lemma_t* plm2= &pwdict->m_lemmalist[pos2];

					int len = plm2->m_length;
					/*if( len && IS_HUMAN_NAME(plm2->m_property) ) // Ϊ�˱��ֺ��Ͻӿ�һ�£�������ʱ���ε�����Ҫ�ڽ����汾�зſ���
					  {          
					  if( pos < max )
					  {
					  tokens[pos].offset = newoff;
					  tokens[pos].length = len;
					  tokens[pos].weight = plm2->m_weight;
					  tokens[pos].prop.m_lprop = plm2->m_property.m_lprop;
					  tokens[pos].prop.m_hprop = plm2->m_property.m_hprop;
					  pos++;
					  }    
					  newoff += len;
					  }*/

					lens[i] = len;
				}

				// �ټ���Ӷ���
				int cnt = pwdict->m_phinfo[plm->m_subphinfo_bpos];
				for(int i=0; i<cnt; i++)
				{
					int pos2 = pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+2];
					scw_lemma_t* plm2= &pwdict->m_lemmalist[pos2];

					int len = plm2->m_length;
					if( len && IS_HUMAN_NAME(plm2->m_property) ) // ������ﱾ���������������жϹ��ɴ˶���Ļ������Ƿ�Ϊ����
					{          
						if( pos < max )
						{
							// �����Ӷ����ֽ�ƫ��
							newoff = start;
							int basoff = pwdict->m_phinfo[plm->m_subphinfo_bpos+i*2+1]; 
							for(int j=0; j<(int)bascnt && j<basoff; j++)
								newoff += lens[j];  

							tokens[pos].offset = newoff;
							tokens[pos].length = len;
							tokens[pos].weight = plm2->m_weight;
							tokens[pos].prop.m_lprop = plm2->m_property.m_lprop;
							tokens[pos].prop.m_hprop = plm2->m_property.m_hprop;
							pos++;
						}              
					}      
					/*else if( len ) // ����ݹ��жϴ��Ӷ����Ƿ������Ӷ����ǰϵͳ�¶���ֻ���������������������ʱ���ã���
					  {
					  _get_pername_token(pwdict,pir,plm2,tokens,pos,start,max);          
					  }*/
				}

				start += plm->m_length;
			}
			break;

		case LEMMA_TYPE_DPHRASE:
			{
				if( plm->m_subphinfo_bpos == COMMON_NULL )
				{
					start += plm->m_length;
					break;
				}

				// �ȼ�������
				int lens[MAX_LEMMA_LENGTH] = {0};
				int newoff = start;
				int bascnt = pir->m_dynphinfo[plm->m_phinfo_bpos];
				for(int i=0; i<bascnt; i++)
				{
					scw_lemma_t* plm2 = (scw_lemma_t*)pir->m_dynphinfo[plm->m_phinfo_bpos+i+1];

					int len = plm2->m_length;
					/*if( len && IS_HUMAN_NAME(plm2->m_property) ) // Ϊ�˱��ֺ��Ͻӿ�һ�£�������ʱ���ε�����Ҫ�ڽ����汾�зſ���
					  {          
					  if( pos < max )
					  {
					  tokens[pos].offset = newoff;
					  tokens[pos].length = len;
					  tokens[pos].weight = plm2->m_weight;
					  tokens[pos].prop.m_lprop = plm2->m_property.m_lprop;
					  tokens[pos].prop.m_hprop = plm2->m_property.m_hprop;
					  pos++;
					  }    
					  newoff += len;
					  }*/

					lens[i] = len;
				}

				// �ټ���Ӷ���
				int cnt = pir->m_dynphinfo[plm->m_subphinfo_bpos];
				for(int i=0; i<cnt; i++)
				{
					scw_lemma_t* plm2= (scw_lemma_t*)pir->m_dynphinfo[plm->m_subphinfo_bpos+i*2+2];

					int len = plm2->m_length;
					if( len && IS_HUMAN_NAME(plm2->m_property) )  
					{          
						if( pos < max )
						{            
							// �����Ӷ����ֽ�ƫ��
							newoff = start;
							int basoff = pir->m_dynphinfo[plm->m_subphinfo_bpos+i*2+1]; 
							for(int j=0; j<(int)bascnt && j<basoff; j++)
								newoff += lens[j];  

							tokens[pos].offset = newoff;
							tokens[pos].length = len;    
							tokens[pos].weight = plm2->m_weight;
							tokens[pos].prop.m_lprop = plm2->m_property.m_lprop;
							tokens[pos].prop.m_hprop = plm2->m_property.m_hprop;
							pos++;
						}                
					}      
					/*else if( len ) // ����ݹ��жϴ��Ӷ����Ƿ������Ӷ����ǰϵͳ�¶���ֻ���������������������ʱ���ã���
					  {
					  _get_pername_token(pwdict,pir,plm2,tokens,pos,start,max);          
					  }*/
				}

				start += plm->m_length;
			}
			break;

		default:
			return -1;
	}

	return start;
}


// [Ϊ��ʹ�þɷִʽӿڿ��Ժ��µı�עϵͳ���ݣ������������������!]
/// If fails, return 0, or return the count of elements in result array.
int scw_get_token_1( scw_out_t* pout, int type, token_t result[], unsigned max )
{
	u_int tmcnt=0, len=0, bufsize=0;
	u_int *tmpos=NULL, *tmoff=NULL;
	scw_property_t* tmprop = NULL;

	char* src = NULL; int pos = 0;
	int flag = pout->m_pir->m_flag;

	if((flag & type) <= 1)
	{
		WARNING_LOG("error: type of output is illegal");
		return 0;
	}

	if(type & SCW_OUT_BASIC)
	{
		src    = pout->wordsepbuf;
		len    = pout->wsb_curpos;
		bufsize  = pout->wsbsize;
		tmcnt  = pout->wsbtermcount;
		tmpos  = pout->wsbtermpos;
		tmoff  = pout->wsbtermoffsets;
		tmprop  = pout->wsbtermprop;
	}
	else if(type & SCW_OUT_WPCOMP)
	{
		src    = pout->wpcompbuf;
		len    = pout->wpb_curpos;
		bufsize  = pout->wpbsize;
		tmcnt  = pout->wpbtermcount;
		tmpos  = pout->wpbtermpos;
		tmoff  = pout->wpbtermoffsets;
		tmprop  = pout->wpbtermprop;
	}
	else if(type & SCW_OUT_SUBPH)
	{
		src    = pout->subphrbuf;
		len    = pout->spb_curpos;
		bufsize  = pout->spbsize;
		tmcnt  = pout->spbtermcount;
		tmpos  = pout->spbtermpos;
		tmoff  = pout->spbtermoffsets;
		tmprop  = pout->spbtermprop;
	}
	else if(type & SCW_OUT_HUMANNAME)
	{
		src    = pout->namebuf;
		len    = pout->nameb_curpos;
		bufsize  = pout->namebsize;
		tmcnt  = pout->namebtermcount;
		tmpos  = pout->namebtermpos;
		tmoff  = pout->namebtermoffsets;
		tmprop  = pout->namebtermprop;
	}
	else if(type & SCW_OUT_BOOKNAME)
	{
		src    = pout->booknamebuf;
		len    = pout->bnb_curpos;
		bufsize  = pout->bnbsize;
		tmcnt  = pout->bnbtermcount;
		tmpos  = pout->bnbtermpos;
		tmoff  = pout->bnbtermoffsets;
		tmprop  = pout->bnbtermprop;
	}
	else if(type & SCW_OUT_NEWWORD)
	{
		scw_newword_t* pnewword = pout->pnewword;
		src    = pnewword->newwordbuf;
		len    = pnewword->newwordb_curpos;
		bufsize  = pnewword->newwordbsize;
		tmcnt  = pnewword->newwordbtermcount;
		tmpos  = pnewword->newwordbtermpos;
		tmoff  = pnewword->newwordbtermoffsets;
		tmprop  = pnewword->newwordbtermprop;
	}
	else
	{    
		WARNING_LOG("error: type of output is illegal");
		return 0;
	}

	if( tmcnt > max)
		tmcnt = max;

	for( unsigned i=0; i<tmcnt; i++ )
	{
		pos = GET_TERM_POS(tmpos[i]);
		len = GET_TERM_LEN(tmpos[i]);

		strncpy(result[i].buffer, src+pos, len);  
		result[i].buffer[len] = '\0';
		if(type & SCW_OUT_NEWWORD ) // �����������´�
			result[i].offset = tmoff[i*2]; // ---�����offset��2λ����һλװ���ǻ��ŵı�ţ��ڶ�λ�ǰ������ŵĸ���
		else
			result[i].offset = tmoff[i];
		result[i].length = len;
		if( type & SCW_OUT_PROP )
		{
			result[i].prop.m_lprop = tmprop[i].m_lprop;
			result[i].prop.m_hprop = tmprop[i].m_hprop;
		}
		result[i].type = 0;
		result[i].index = 0;
	}

	return tmcnt;  
}

/*!get the token list of basic term, mixed term and so on.
 * @param <pwdict> - dictionary pointer.
 * @param <pir> - inner data structure of segment result.
 * @param <type> - token type, e.g. basic term, phrase, sunphrase, ...
 * @param <result> - [out], store the segment result infomation.
 * @param <max> - maximum count of result tokens.
 * @return If fails, return 0, or return the count of elements in result array.  
 */
int scw_get_token_2(scw_worddict_t* pwdict, scw_inner_t* pir, int type, token_t result[], unsigned max)
{
	//assert( pwdict && pir && result );
	unsigned offset = 0, pos = 0;
	scw_lemma_t* plm = NULL;  

	// ������
	if(type & SCW_OUT_BASIC)
	{
		for(unsigned i=0; i<pir->m_ppseg_cnt; i++)
		{    
			plm = pir->m_ppseg[i]; 
			if(_get_basic_token(pwdict, pir, plm, result, pos, offset, max) < 0)
			{      
				return 0;
			}  
		}
	}
	// �����ʶ������
	else if(type & SCW_OUT_WPCOMP)
	{
		for(unsigned i=0; i<pir->m_ppseg_cnt; i++)
		{    
			plm = pir->m_ppseg[i]; 
			if( !plm ) continue; // ���ﲻ����break����Ϊm_ppseg�е�Ԫ�ؿ����ǲ������ģ�

			unsigned len = plm->m_length; 
			if( len && (pos < max) )
			{        
				result[pos].offset = offset;
				result[pos].length = len;
				result[pos].weight = plm->m_weight;
				result[pos].prop.m_lprop = plm->m_property.m_lprop;
				result[pos].prop.m_hprop = plm->m_property.m_hprop;
				pos++;
			}
			offset += len;
		}
	}
	// �Ӷ�������� ��
	else if(type & SCW_OUT_SUBPH)
	{
		for(unsigned i=0; i<pir->m_ppseg_cnt; i++)
		{    
			plm = pir->m_ppseg[i];
			if( !plm ) continue;

			if( IS_HUMAN_NAME(plm->m_property) ) // ������ﱾ�������������ټ�����Ƿ����Ӷ���
			{
				unsigned len = plm->m_length;
				if(pos < max)
				{
					result[pos].offset = offset;
					result[pos].length = len;
					result[pos].weight = plm->m_weight;
					result[pos].prop.m_lprop = plm->m_property.m_lprop;
					result[pos].prop.m_hprop = plm->m_property.m_hprop;
					pos++;    
				}
				offset += len;
			}
			else  if(_get_phrase_token(pwdict, pir, plm, result, pos, offset, max) < 0)
			{      
				return 0;
			}          
		}
	}
	// ����
	else if(type & SCW_OUT_HUMANNAME)
	{
		for(unsigned i=0; i<pir->m_ppseg_cnt; i++)
		{    
			plm = pir->m_ppseg[i];
			if( !plm ) continue;

			if( plm && IS_HUMAN_NAME(plm->m_property) ) // ������ﱾ���������������жϹ��ɴ˶���Ļ������Ƿ�Ϊ����
			{
				unsigned len = plm->m_length;
				if(pos < max)
				{
					result[pos].offset = offset;
					result[pos].length = len;
					result[pos].weight = plm->m_weight;
					result[pos].prop.m_lprop = plm->m_property.m_lprop;
					result[pos].prop.m_hprop = plm->m_property.m_hprop;
					pos++;    
				}
				offset += len;
			}
			else if( IS_PHRASE(plm->m_property) ) // �����ж����ӽṹ�Ƿ�Ϊ����
			{
				if(_get_pername_token(pwdict, pir, plm, result, pos, offset, max) < 0)
				{      
					return 0;
				}  
			}      
		}
	}
	// ����
	else if(type & SCW_OUT_BOOKNAME)
	{
		for(unsigned i=0; i<pir->m_ppseg_cnt; i++)
		{    
			plm = pir->m_ppseg[i];
			if( !plm ) continue;

			unsigned len = plm->m_length;
			if( IS_BOOKNAME(plm->m_property) )
			{
				if(pos < max)
				{
					result[pos].offset = offset;
					result[pos].length = len;
					result[pos].weight = plm->m_weight;
					result[pos].prop.m_lprop = plm->m_property.m_lprop;
					result[pos].prop.m_hprop = plm->m_property.m_hprop;
					pos++;    
				}      
			}
			offset += len;
		}
	}
	else
	{    
		WARNING_LOG("error: type of output is illegal");
		return 0;
	}

	return pos;
}


// ���Ž���ж��ﶯ̬��֣��õ���������������ȣ������Ƚ��ڻ��źͻ�����֮�䡣
int seg_split( Sdict_search* phrasedict, scw_out_t* pout, token_t subtokens[], int tcnt )
{
	int ret = 0;
	Sdict_snode snode;    
	int ds_ret = 0;

	if (pout == NULL  || phrasedict == NULL)
	{      
		WARNING_LOG("error: pout or phrasedict is null!");
		return ret;
	}

	//���Ž��
	for (u_int i = 0; i < pout->wpbtermcount; i++)
	{
		u_int pos = GET_TERM_POS( pout->wpbtermpos[i] );
		u_int len = GET_TERM_LEN( pout->wpbtermpos[i] );
		u_int off = pout->wpbtermoffsets[i];

		char term[256] = "\0";
		if(len >= 256)
		{
			WARNING_LOG("error: term length longer than 256!");
			return ret;
		}
		strncpy( term, pout->wpcompbuf+pos, len );
		term[len] = '\0';

		//book name combine
		//        char *p = pout->wpcompbuf+pos;
		//       if (strncmp( p, "��", 2 ) == 0 &&  strncmp( p+len-2, "��", 2 ) == 0)
		if(IS_BOOKNAME(pout->wpbtermprop[i]))
		{          
			u_int bsccnt = 0;        
			u_int boff = off;
			u_int rlen = len;

			while (rlen > 0)
			{
				bsccnt += 1;//basic����
				rlen -= GET_TERM_LEN( pout->wsbtermpos[boff] );
				boff += 1;
			}

			if(IS_BOOKNAME_START(pout->wsbtermprop[off]) && IS_BOOKNAME_END(pout->wsbtermprop[off + bsccnt - 1]))
			{
				if (bsccnt == 3)
				{
					if (ret + 3 > tcnt)
					{
						WARNING_LOG("error : term result number more than max term count!");
						return ret;
					}    

					boff = off;
					while (boff < off+3)
					{
						u_int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
						u_int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

						strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
						subtokens[ret].buffer[blen] = '\0';
						subtokens[ret].offset = boff;
						subtokens[ret].length = blen;
						subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
						subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
						subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
						subtokens[ret].index = 0;

						boff += 1;
						ret += 1;
					}
					continue;
				}

				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}  
				boff = off;
				u_int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
				u_int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

				strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
				subtokens[ret].buffer[blen] = '\0';
				subtokens[ret].offset = boff;
				subtokens[ret].length = blen;
				subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
				subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
				subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
				subtokens[ret].index = 0;

				ret += 1;

				//build complete seg from subphrase buffer        
				rlen = len - 4;
				boff = off + 1;
				while (rlen > 0)
				{
					u_int lentemp = len;
					int maxsubphridx = -1;
					unsigned int maxsubphrlen = 0;
					while(1)
					{
						for (int j = 0; j < (int)pout->spbtermcount; j++)
						{
							//u_int spos = GET_TERM_POS( pout->spbtermpos[j] );
							u_int slen = GET_TERM_LEN( pout->spbtermpos[j] );

							//pass not aligned subphrase
							if (pout->spbtermoffsets[j] != boff)
							{
								continue;
							}

							//pass itself,or longer than it 
							if (slen >= len || slen >= lentemp)
							{    
								continue;    
							}
							if (maxsubphrlen < slen)
							{
								maxsubphridx = j;
								maxsubphrlen = slen;
							}                
						}
						if (maxsubphridx >= 0)
						{
							u_int spos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
							u_int slen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );

							char sterm[256] = "\0";
							if(slen >= 256)
							{
								WARNING_LOG("error: term length longer than 256!");
								return ret;
							}
							strncpy( sterm, pout->subphrbuf+spos, slen );
							sterm[slen] = '\0';

							//pass subphrase can split 
							creat_sign_f64 (sterm, slen, &(snode.sign1), &(snode.sign2));
							ds_ret = ds_op1 (phrasedict, &snode, SEEK);
							if ( ds_ret == 1)
							{
								lentemp = slen;
								//   j = -1;
								maxsubphridx = -1;
								maxsubphrlen = 0;
								continue;
							}
							else if(ds_ret != 0)
							{
								WARNING_LOG("error: ds_op1 result error!");
								return ret;
							}
							else
							{
								break;
							}
						}
						break;
					}

					//copy max subphrase
					if (maxsubphridx >= 0)
					{					
						if(ret + 1 > tcnt)
						{
							WARNING_LOG("error : term result number more than max term count!");
							return ret;
						}
						u_int mpos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
						u_int mlen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );
						u_int moff = pout->spbtermoffsets[maxsubphridx];

						strncpy( subtokens[ret].buffer, pout->subphrbuf+mpos, mlen );
						subtokens[ret].buffer[mlen] = '\0';
						subtokens[ret].offset = moff;
						subtokens[ret].length = mlen;
						subtokens[ret].prop.m_hprop = pout->spbtermprop[maxsubphridx].m_hprop;
						subtokens[ret].prop.m_lprop = pout->spbtermprop[maxsubphridx].m_lprop;
						subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
						subtokens[ret].index = 0;

						ret += 1;									
						rlen -= mlen;
						int bcnt = 1;
						mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );
						while (mlen > 0)
						{
							bcnt += 1;
							moff += 1;
							mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );
						}

						boff += bcnt;
					}
					//copy basic
					else
					{		
						if(ret + 1 > tcnt)
						{
							WARNING_LOG("error : term result number more than max term count!");
							return ret;
						}
						u_int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
						u_int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

						strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
						subtokens[ret].buffer[blen] = '\0';
						subtokens[ret].offset = boff;
						subtokens[ret].length = blen;
						subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
						subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
						subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
						subtokens[ret].index = 0;

						ret += 1;				
						rlen -= blen;
						boff += 1;
					}
				}

				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
				blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

				strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
				subtokens[ret].buffer[blen] = '\0';
				subtokens[ret].offset = boff;
				subtokens[ret].length = blen;
				subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
				subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
				subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
				subtokens[ret].index = 0;

				ret += 1;
				continue;
			}
		}

		creat_sign_f64 (term, len, &(snode.sign1), &(snode.sign2));
		if (ds_op1 (phrasedict, &snode, SEEK) != 1)
		{
			if(ret + 1 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}

			strncpy( subtokens[ret].buffer, term, len );
			subtokens[ret].buffer[len] = '\0';
			subtokens[ret].offset = off;
			subtokens[ret].length = len;
			subtokens[ret].prop.m_hprop = pout->wpbtermprop[i].m_hprop;
			subtokens[ret].prop.m_lprop = pout->wpbtermprop[i].m_lprop;
			subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
			subtokens[ret].index = 0;

			ret += 1;	  	
			continue;
		}

		u_int rlen = len;
		u_int boff = off;
		while (rlen > 0)
		{
			u_int lentemp = len;//phrase itself length
			int maxsubphridx = -1;
			unsigned int maxsubphrlen = 0;
			while(1)
			{
				for (int j = 0; j < (int)pout->spbtermcount; j++)
				{
					//u_int spos = GET_TERM_POS( pout->spbtermpos[j] );
					u_int slen = GET_TERM_LEN( pout->spbtermpos[j] );

					//pass not aligned subphrase
					if (pout->spbtermoffsets[j] != boff)
					{
						continue;
					}

					//pass itself	
					if (slen >= len || slen >= lentemp)
					{
						continue;	
					}
					if (maxsubphrlen < slen)
					{
						maxsubphridx = j;
						maxsubphrlen = slen;
					}		
				}
				if (maxsubphridx >= 0)
				{
					u_int spos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
					u_int slen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );

					char sterm[256] = "\0";
					if(slen >= 256)
					{
						WARNING_LOG("error: term length longer than 256!");
						return ret;
					}
					strncpy( sterm, pout->subphrbuf+spos, slen );
					sterm[slen] = '\0';

					//pass subphrase can split 
					creat_sign_f64 (sterm, slen, &(snode.sign1), &(snode.sign2));
					ds_ret = ds_op1 (phrasedict, &snode, SEEK);
					if ( ds_ret == 1)
					{
						lentemp = slen;
						//j = -1;
						maxsubphridx = -1;
						maxsubphrlen = 0;
						continue;
					}
					else if(ds_ret != 0)
					{
						WARNING_LOG("error: ds_op1 result error!");
						return ret;
					}
					else
					{
						break;
					}
				}
				break;
			}

			//copy max subphrase
			if (maxsubphridx >= 0)
			{					              
				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				int mpos = GET_TERM_POS( pout->spbtermpos[maxsubphridx] );
				int mlen = GET_TERM_LEN( pout->spbtermpos[maxsubphridx] );
				int moff = pout->spbtermoffsets[maxsubphridx];

				strncpy( subtokens[ret].buffer, pout->subphrbuf+mpos, mlen );
				subtokens[ret].buffer[mlen] = '\0';
				subtokens[ret].offset = moff;
				subtokens[ret].length = mlen;
				subtokens[ret].prop.m_hprop = pout->spbtermprop[maxsubphridx].m_hprop;
				subtokens[ret].prop.m_lprop = pout->spbtermprop[maxsubphridx].m_lprop;
				subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
				subtokens[ret].index = 0;
				ret += 1;				
				rlen -= mlen;

				int sbcnt = 1;
				mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );//���basic����
				while (mlen > 0)
				{
					sbcnt += 1;
					moff += 1;
					mlen -= GET_TERM_LEN( pout->wsbtermpos[moff] );
				}

				boff += sbcnt;
			}
			//copy basic
			else
			{			
				if(ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				int bpos = GET_TERM_POS( pout->wsbtermpos[boff] );
				int blen = GET_TERM_LEN( pout->wsbtermpos[boff] );

				strncpy( subtokens[ret].buffer, pout->wordsepbuf+bpos, blen );
				subtokens[ret].buffer[blen] = '\0';
				subtokens[ret].offset = boff;
				subtokens[ret].length = blen;
				subtokens[ret].prop.m_hprop = pout->wsbtermprop[boff].m_hprop;
				subtokens[ret].prop.m_lprop = pout->wsbtermprop[boff].m_lprop;
				subtokens[ret].weight = 0;//poutû��weight��index�ֶ�,����ֵΪ0
				subtokens[ret].index = 0;

				ret += 1;				
				rlen -= blen;
				boff += 1;
			}
		}
	}

	return ret;
}

// </internal-function-definition>

//
// ���Ž���ж��ﶯ̬��֣��õ���������������ȣ������Ƚ��ڻ��źͻ�����֮�䡣
//�½ӿڵĵ��ú���
int seg_split_tokenize( Sdict_search* phrasedict, handle_t handle, token_t token, token_t subtokens[], int tcnt)
{
	int ret = 0;
	Sdict_snode snode;
	token_t subphrase[256];//�����Ļ�,�û�����ôָ����?
	token_t basic[256];
	int ds_ret = 0;

	if (token.length <= 2)
	{
		memcpy( subtokens, &token, sizeof( token_t ) );
		ret = 1;
		return ret;
	}

	//book name combine
	//    char *p = token.buffer;
	//   if (strncmp( p, "��", 2 ) == 0 && strncmp( p+token.length-2, "��", 2 ) == 0)
	int subcnt = seg_tokenize(handle, token, TOK_SUBPHR,subphrase,256);
	int bsccnt = seg_tokenize(handle, token, TOK_BASIC, basic, 256);

	if(IS_BOOKNAME(token.prop) && IS_BOOKNAME_START(basic[0].prop) && IS_BOOKNAME_END(basic[bsccnt - 1].prop))
	{
		//     int subcnt = seg_tokenize(handle, token, TOK_SUBPHR,subphrase,256);
		//     int bsccnt = seg_tokenize(handle, token, TOK_BASIC, basic, 256);

		if (bsccnt == 3)
		{
			if (ret + 3 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}    
			memcpy( subtokens+ret, basic, sizeof( token_t ) );
			ret += 1;
			memcpy( subtokens+ret, basic+1, sizeof( token_t ) );
			ret += 1;
			memcpy( subtokens+ret, basic+2, sizeof( token_t ) );
			ret += 1;

			return ret;
		}

		if (ret + 1 > tcnt)
		{
			WARNING_LOG("error : term result number more than max term count!");
			return ret;
		}
		memcpy( subtokens+ret, basic, sizeof( token_t ) );
		ret += 1;

		//build complete seg from subphrase buffer        
		unsigned int len = token.length-4;
		unsigned int off = 2;
		while (len > 0)
		{
			u_int splitlen = token.length;//�����phrase�ĳ���
			u_int tmplen = token.length;
			int maxsubphridx = -1;
			unsigned int maxsubphrlen = 0;
			while(1)
			{
				for (int i = 0; i < subcnt; i++)
				{
					//pass not aligned subphrase
					if (subphrase[i].offset != off)
						continue;

					//pass itself    
					if (subphrase[i].length >= tmplen || subphrase[i].length >= splitlen)
						continue;    

					if (maxsubphrlen < subphrase[i].length)
					{
						maxsubphridx = i;
						maxsubphrlen = subphrase[i].length;
					}
				}
				if(maxsubphridx >= 0)
				{
					//pass subphrase can split 
					creat_sign_f64 (subphrase[maxsubphridx].buffer, subphrase[maxsubphridx].length, &(snode.sign1), &(snode.sign2));
					ds_ret = ds_op1 (phrasedict, &snode, SEEK);
					if (ds_ret == 1)
					{
						splitlen = maxsubphrlen;
						maxsubphridx = -1;
						maxsubphrlen = 0;
						continue;
					}
					else if(ds_ret != 0)
					{
						WARNING_LOG("error: ds_op1 result error!");
						return ret;
					}
					else
					{
						break;
					}	
				}
				break;
			}
			//copy max subphrase
			if (maxsubphridx >= 0)
			{
				if (ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}
				memcpy( subtokens+ret, subphrase+maxsubphridx, sizeof( token_t ) );
				ret += 1;                               
				len -= subphrase[maxsubphridx].length;
				off += subphrase[maxsubphridx].length;
			}
			//copy basic
			else
			{
				if (ret + 1 > tcnt)
				{
					WARNING_LOG("error : term result number more than max term count!");
					return ret;
				}

				int boff = 0;
				while (boff < bsccnt && basic[boff].offset != off)
				{
					boff += 1;
				}       

				if((boff >= bsccnt) || (basic[boff].offset != off))
				{
					WARNING_LOG("basic offset exceeds basic count");
					return ret;
				}
				//assert (boff < bsccnt );
				//assert (basic[boff].offset == off);                

				memcpy( subtokens+ret, basic+boff, sizeof( token_t ) );

				ret += 1;                
				len -= basic[boff].length;
				off += basic[boff].length;
			}    
		}
		if (ret + 1 > tcnt)
		{
			WARNING_LOG("error : term result number more than max term count!");
			return ret;
		}
		memcpy( subtokens+ret, basic+bsccnt-1, sizeof( token_t ) );
		ret += 1;

		return ret;
	}

	creat_sign_f64 (token.buffer, token.length, &(snode.sign1), &(snode.sign2));
	if (ds_op1 (phrasedict, &snode, SEEK) != 1)
	{
		memcpy( subtokens, &token, sizeof( token_t ) );
		ret = 1;
		return ret;
	}

	//  int subcnt = seg_tokenize(handle, token,TOK_SUBPHR,subphrase,32);
	//  int bsccnt = seg_tokenize(handle, token, TOK_BASIC, basic, 32);

	if (bsccnt == 1)//unexpect : phrasedict error
	{
		memcpy( subtokens, &token, sizeof( token_t ) );
		ret = 1;
		return ret;
	}

	//split phrase
	unsigned int len = token.length;
	unsigned int off = 0;
	while (len > 0)
	{
		u_int splitlen = token.length;
		u_int tmplen = token.length;
		int maxsubphridx = -1;
		unsigned int maxsubphrlen = 0;
		while(1)
		{
			for (int i = 0; i < subcnt; i++)
			{
				//pass not aligned subphrase
				if (subphrase[i].offset != off)
					continue;

				//pass itself    
				if (subphrase[i].length >= tmplen || subphrase[i].length >= splitlen)
					continue;     

				if (maxsubphrlen < subphrase[i].length)
				{
					maxsubphridx = i;
					maxsubphrlen = subphrase[i].length;
				}
			}
			if(maxsubphridx >= 0)
			{
				//pass subphrase can split 
				creat_sign_f64 (subphrase[maxsubphridx].buffer, subphrase[maxsubphridx].length, &(snode.sign1), &(snode.sign2));
				ds_ret = ds_op1 (phrasedict, &snode, SEEK);
				if ( ds_ret == 1)    
				{
					splitlen = maxsubphrlen;
					maxsubphridx = -1;
					maxsubphrlen = 0;
					continue;
				} 
				else if(ds_ret != 0)
				{
					WARNING_LOG("error: ds_op1 result error!");
					return ret;
				}
				else
				{
					break;
				}	
			}
			break;
		}

		//copy max subphrase
		if (maxsubphridx >= 0)
		{         
			if (ret + 1 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}
			memcpy( subtokens+ret, subphrase+maxsubphridx, sizeof( token_t ) );
			ret += 1;                
			len -= subphrase[maxsubphridx].length;
			off += subphrase[maxsubphridx].length;
		}
		//copy basic
		else
		{
			if (ret + 1 > tcnt)
			{
				WARNING_LOG("error : term result number more than max term count!");
				return ret;
			}
			int boff = 0;
			while (boff < bsccnt && basic[boff].offset != off)
			{
				boff += 1;
			}       
			if((boff >= bsccnt) || (basic[boff].offset != off))
			{
				WARNING_LOG("basic offset exceeds basic count");
				return ret;
			}
			//assert (boff < bsccnt );
			//assert (basic[boff].offset == off);                

			memcpy( subtokens+ret, basic+boff, sizeof( token_t ) );
			ret += 1;

			len -= basic[boff].length;
			off += basic[boff].length;
		}    
	}

	return ret;
}



