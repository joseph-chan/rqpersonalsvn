#include "scwdef.h"

#define SYM_HIMIN  0xA1  /* GB symbol : minmum value of first byte */
#define SYM_HIMAX  0xF7  /* GB symbol : maxmum value of first byte */
#define SYM_LOMIN  0xA1  /* GB symbol : minmum value of second byte */
#define SYM_LOMAX  0xFE  /* GB symbol : maxmum value of second byte */

#define CHAR_HIMIN  0xB0  /* GB char   : min - 1st byte */
#define CHAR_HIMAX  0xF7  /* GB char   : max - 1st byte */
#define CHAR_LOMIN  0xA1  /* GB char   : min - 2nd byte */
#define CHAR_LOMAX  0xFE  /* GB char   : max - 2nd byte */

#define CHAR_HINUM      0x48  /* (0xF7-0xB0+1) */
#define CHAR_LONUM      0x5E  /* (0xFE-0xA1+1) */

#define MARK_HIMIN  0xA1  /* GB mark   : min - 1st byte */
#define MARK_HIMAX  0xAF  /* GB mark   : max - 1st byte */
#define MARK_LOMIN  0xA1  /* GB mark   : min - 2nd byte */
#define MARK_LOMAX  0xFE  /* GB mark   : max - 2nd byte */

#define SMARK_HIMIN      0xA3    /* simple GB mark   : min - 1st byte */
#define SMARK_HIMAX      0xA3    /* simple GB mark   : max - 1st byte */
#define SMARK_LOMIN      0xA1    /* simple GB mark   : min - 2nd byte */
#define SMARK_LOMAX      0xFE    /* simple GB mark   : max - 2nd byte */

#define DIGIT_HIMIN     0xa3    /* GB 0---9 */
#define DIGIT_HIMAX     0xa3
#define DIGIT_LOMIN     0xb0
#define DIGIT_LOMAX     0xb9

#define HI_HIMIN        0xa3    /* GB A---Z */   
#define HI_HIMAX        0xa3
#define HI_LOMIN        0xc1
#define HI_LOMAX        0xda

#define LOW_HIMIN       0xa3    /* GB a---z */ 
#define LOW_HIMAX       0xa3
#define LOW_LOMIN       0xe1
#define LOW_LOMAX       0xfa

#define GBK_HIMIN       0x81
#define GBK_HIMAX       0xfe
#define GBK_LOMIN       0x40
#define GBK_LOMAX       0xfe

/* define the range of gb */
#define IN_RANGE(ch, min, max) ( (((unsigned char)(ch))>=(min)) && (((unsigned char)(ch))<=(max)) )

#define IS_GB(cst)  ( IN_RANGE((cst)[0], SYM_HIMIN, SYM_HIMAX) && IN_RANGE((cst)[1], SYM_LOMIN, SYM_LOMAX) )

#define IS_GBK(cst)      ( IN_RANGE((cst)[0], GBK_HIMIN, GBK_HIMAX) && IN_RANGE((cst)[1], GBK_LOMIN, GBK_LOMAX) )

#define IS_GB_CODE(cst)  ( IN_RANGE((cst)[0], CHAR_HIMIN, CHAR_HIMAX) && IN_RANGE((cst)[1], CHAR_LOMIN, CHAR_LOMAX) )

#define IS_GB_MARK(cst)  ( IN_RANGE((cst)[0], MARK_HIMIN, MARK_HIMAX) && IN_RANGE((cst)[1], MARK_LOMIN, MARK_LOMAX) )

#define IS_GB_SMAEK(cst) ( IN_RANGE((cst)[0], SMARK_HIMIN, SMARK_HIMAX) && IN_RANGE((cst)[1], SMARK_LOMIN, SMARK_LOMAX) )

#define IS_GB_SPACE(cst) ( (((unsigned char)((cst)[0]))==SYM_HIMIN) && (((unsigned char)((cst)[1]))==SYM_LOMIN) )

#define IS_GB_NOBREAK(cst) ( (unsigned char)(cst)[0]==0xa3 && ((unsigned char)(cst)[1]==0xa6 ||(unsigned char)(cst)[1]==0xad ||(unsigned char)(cst)[1]==0xdf) )

#define IS_GB_1234(cst) (IN_RANGE((cst)[0],DIGIT_HIMIN,DIGIT_HIMAX) && IN_RANGE((cst)[1],DIGIT_LOMIN,DIGIT_LOMAX) )

#define IS_GB_ABCD(cst) ( (IN_RANGE((cst)[0],HI_HIMIN,HI_HIMAX) && IN_RANGE((cst)[1],HI_LOMIN,HI_LOMAX) ) || (IN_RANGE((cst)[0],LOW_HIMIN,LOW_HIMAX) && IN_RANGE((cst)[1],LOW_LOMIN,LOW_LOMAX) ) ) 

#define IS_ALPHA_CH(cst) ( (cst[0] >= 'a' && cst[0] <= 'z') || ( cst[0] >= 'A' && cst[0] <= 'Z') )
#define IS_ALPHA_SPE(cst) ( cst[0] == '-' )
#define IS_ALPH(cst)  ( ((unsigned char)(cst)[0] & 0x80) != 0x80 )
#define IS_ALPHA_1234(cst) ( cst[0] >= '0' && cst[0] <= '9')
static const unsigned char l_symbol[96] = 
{   0, ' ', ',', '.',  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, '\'', '\'',
	'\"', '\"', '[', ']', '<', '>', '<', '>', '<', '>', '<', '>', '<', '>', '<', '>', 
	32, 33, 34, ':', 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, '\'', '\"', 70, '$', 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95
};


static const unsigned char l_low_map[256] = 
{	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
	'p','q','r','s','t','u','v','w','x','y','z', 91, 92, 93, 94, 95,
	96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255   
};


scw_crf_out *scw_crf_create(u_int tsize)
{
	if(tsize < 1)
	{
		return NULL;
	}
	scw_crf_out *crf_out = NULL;
	//1.为crf_out分配内存
	crf_out = (scw_crf_out*) malloc(sizeof(scw_crf_out));
	if (NULL == crf_out) 
	{
		ul_writelog(UL_LOG_FATAL, "[%s]: malloc for scw_crf_out error", __func__);		
		return NULL;
	}
	//2.为scw_crf_term_out分配内存
	for (int i=0; i<MAX_NBEST; i++) 
	{
		scw_crf_term* crf_term = &(crf_out->term_buf[i]);
		/*       	crf_term->crftermoffsets = (u_int *)malloc(sizeof(u_int) * (tsize + 1));
					if (NULL == crf_term->crftermoffsets) 
					{
					ul_writelog(UL_LOG_FATAL, "[%s]: malloc for scw_crf_term:%d crftermoffsets error", __func__, i);
					return NULL;	
					}
					*/
		//4.为crftermpos分配内存
		crf_term->crftermpos = (u_int *)malloc(sizeof(u_int) * (tsize + 1));
		if (NULL == crf_term->crftermpos) 
		{
			ul_writelog(UL_LOG_FATAL, "[%s]: malloc for scw_crf_term:%d crftermpos error",__func__, i);
			return NULL;
		}
		crf_term->wordtotallen = 0;
		crf_term->crftermcount = 0;
		for (u_int j=0; j < tsize; j++) 
		{
			//			crf_term->crftermoffsets[j] = 0;
			crf_term->crftermpos[j] = 0;
		}
	}
	return crf_out;
}

void scw_crf_des(scw_crf_out* pout)
{
	if(pout == NULL)
	{
		ul_writelog(UL_LOG_WARNING, "[%s]: pout is null", __func__);
		return;
	}
	//释放每个term_out的term_buf的内存
	for (int i=0; i<MAX_NBEST; i++) 
	{
		scw_crf_term* crf_term = &(pout->term_buf[i]);
		/*		if(crf_term->crftermoffsets != NULL)
				{
				free(crf_term->crftermoffsets);	
				crf_term->crftermoffsets = NULL;
				}
				*/

		if(crf_term->crftermpos != NULL)
		{
			free(crf_term->crftermpos);
			crf_term->crftermpos = NULL;
		}
	}
	if(pout != NULL)
	{
		free(pout);
		pout = NULL;
	}
}
static int crf_output(crf_out_t* crf_out, scw_crf_out* crf_res, int* str_hash_offset,const u_int max_count,const int start)
{	
	int term_pos = 0;
	int term_len = 0;
	int i = 0;
	for (int cur = 0; cur < crf_res->nbest; cur++) 
	{
		if( cur < (crf_out-> nbest) )
		{
			i = cur;
		}
		else
		{
			i = 0;
		}
		u_int* term_off	= &((crf_res->term_buf[cur]).crftermcount);
		crf_term_out_t *pout = &crf_out->term_buf[i];
		for (u_int j = 0; j < pout->crftermcount; j++) 
		{	
			term_pos = CRF_GET_TERM_POS(pout->crftermpos[j])+start;
			term_len = CRF_GET_TERM_LEN(pout->crftermpos[j]);
			int next_pos = term_pos + term_len;
			if(*term_off + 1 >=  max_count)
			{
				return -1;
			}
			(crf_res->term_buf[cur]).crftermpos[(*term_off)++] = ( ((str_hash_offset[next_pos] - str_hash_offset[term_pos] ) << 24) 
					|  (str_hash_offset[term_pos]) );
		}
	} 
	return 0;
}
static int crf_seg_word(SegCrfTag* tagger,scw_crf_out* crf_res,crf_out_t* crf_out, 
		char *row,const int row_len, int* str_hash_offset, const u_int max_count,const int nbest,const int vlevel,const char *inbuf,
		const int in_len,const int str_len)
{
	if(tagger == NULL || crf_res==NULL || crf_out==NULL || row==NULL || row_len<1 || str_hash_offset == NULL || max_count < 1 || inbuf==NULL)
	{

			ul_writelog(UL_LOG_FATAL, "[%s]: parameters error", __func__);
			return -1;
	}

	int n2 = 5;
	int n1 = 20;
	int pre_num = 2; //控制前面数字的个数
	int now_off = 0;
	int tail = str_len;
	bool first = true;
	int start = 0; 
	int end  = 0;
//	char temp[256];
//	int temp_len =0;
	int str_temp = 0;
//	for(int i = 0; i < row_len; i++)
//	{
//	    for(int j =0 ; row[j]; j++)
//		{
//		   temp[temp_len++] = row[j];
//		}
//	  
//	}
//	
//	temp[temp_len] = 0;
//	ul_writelog(UL_LOG_WARNING, "[%s]:MYMY %s\n", __func__, temp);


	for( int i = 0,j = 0; i < row_len; i++)
	{
		for( ; row[j]; j++) 
		{
			if((row[j] >='a' && row[j] <='z'))
			{
				first = false;
			}
			else if( row[j] >='0' && row[j] <= '9')
			{
				if( first )
				{
					++now_off;
				}
			}
			else
			{
				now_off = 0;
				goto DOWN;
			}
		}
		j++;
	}
	if( now_off >= pre_num || now_off == str_len )  //大于pre_num后，直接把前缀数字加入结果集
	{
		for( int i = 0; i < crf_res->nbest; i++)
		{
			u_int *term_off = &(crf_res->term_buf[i].crftermcount); 
			if(*term_off + 1 >=  max_count)
			{
				return -1;
			}
			crf_res->term_buf[i].crftermpos[(*term_off)++] = (str_hash_offset[now_off] - str_hash_offset[0] << 24)|str_hash_offset[0]; 	   
		} 
		if( now_off == str_len ) //纯数字，直接退出
		{
			return 0;
		}
	}
	else
	{
		now_off = 0;		 
	}
	for( int i = row_len*2 -2, j = str_len - 1; i >= 0; i-=2,--j)
	{
		if( row[i] >='0' && row[i] <= '9')
		{
			tail = j;
		}
		else 
		{
			break;
		}
	}

	start = str_hash_offset[now_off] - 1;
	end = str_hash_offset[tail];
	str_temp = tail - now_off;
	if(str_temp <= n2 || ( str_temp < n1 &&( (start >=0 && inbuf[start] ==' ') || (end < in_len && inbuf[end] == ' ') ) ))
	{
		for( int i = 0; i < crf_res->nbest; i++)
		{
			u_int *term_off = &(crf_res->term_buf[i].crftermcount); 
			if(*term_off + 1 >=  max_count)
			{
				return -1;
			}
			//	printf(">>>%d %d\n",str_len,str_hash_offset[])
			crf_res->term_buf[i].crftermpos[(*term_off)++] = ((str_hash_offset[tail] - str_hash_offset[now_off]) << 24)|str_hash_offset[now_off]; 	   
		}   
	}
	else
	{
DOWN:
		int ret = tagger->reset();
		if(ret < 0)
		{
			ul_writelog(UL_LOG_WARNING, "[%s]: reset error", __func__);
			return -1;		
		}
		//2.handle all characters
		int newlen = 0;
		newlen = row_len - now_off;
		newlen -= str_len;
		newlen += tail;

		ret = tagger->seg_add(row+now_off*2, newlen);
		if (ret < 0) 
		{
			ul_writelog(UL_LOG_FATAL, "[%s]: add word to tagger error", __func__);
			//fprintf(stderr,"tagger->my_add error");
			return -1;
		}
		//3.parse
		ret = tagger->seg_parse();
		if (ret < 0) 
		{
			ul_writelog(UL_LOG_FATAL, "[%s]: segment parse error", __func__);
			return -1;
		}
		//4.my_output
		ret = tagger->seg_output(crf_out);
		if (ret < 0) 
		{
			ul_writelog(UL_LOG_FATAL, "[%s]: segment output error", __func__);
			return -1;
		}
		if ( crf_output(crf_out,crf_res,str_hash_offset,max_count,now_off) == -1) 
		{
			ul_writelog(UL_LOG_FATAL, "[%s]: crf_output error", __func__);
			return -1;
		}
	}
	if( tail != str_len)
	{
		for( int i = 0; i < crf_res->nbest; i++)
		{
			u_int *term_off = &(crf_res->term_buf[i].crftermcount); 
			if(*term_off + 1 >=  max_count)
			{
				return -1;
			}
			crf_res->term_buf[i].crftermpos[(*term_off)++] = (str_hash_offset[str_len] - str_hash_offset[tail] << 24)|str_hash_offset[tail]; 	   
		} 
	}
	//ul_writelog(UL_LOG_WARNING, "[%s]:MYMY %s\n", __func__, temp);
	return 0;
}

//int crf_segment_words_processed(SegCrfTag* crf_tag, crf_out_t* crf_out, 
//		scw_crf_out* crf_res, const u_int max_count, const char* inbuf,const int inlen, const int nbest,const int vlevel)
int crf_segment_words_processed(scw_inner_t *pir, const char* inbuf,const int inlen, const int nbest,const int vlevel)
{
	int ret = 0;

	if ( NULL == pir->crf_tag || NULL == pir->crf_out || NULL == inbuf || inlen <0 || nbest <0 || nbest > 10 
			|| vlevel < 0 || vlevel > 2) 
	{
		ul_writelog(UL_LOG_FATAL, "[%s]: some of argument is error", __func__);
		return -1;
	}
	SegCrfTag* crf_tag = pir->crf_tag;
	crf_out_t* crf_out = pir->crf_out;
	scw_crf_out* crf_res = pir->crf_res;
	u_int max_count = pir->m_maxterm;
	
	crf_res -> nbest = nbest;
	crf_tag->set_nbest(nbest);
	crf_tag->set_vlevel(vlevel);
	char row[CRF_MAX_QUERY_LEN * 2]={};
	int str_hash_offset[CRF_MAX_QUERY_LEN * 2]={};
	int row_len = 0; 
	int m = 0;   //扫描指针
	int str_len = 0; //当前准备用crf切的字节数
	int offset = 0;
	u_int manword_cnt = 0; //记录worddict.man中的词的当前处理个数

	for( int i = 0; i < crf_res->nbest; i++)
	{
		(crf_res->term_buf[i]).crftermcount = 0;
	}
	while( m < inlen )
	{
		if( str_len + 4 >= CRF_MAX_WORD_LEN )
		{
			ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, str_hash_offset,max_count ,nbest, vlevel,inbuf,inlen,str_len);
			if( ret < 0 )
			{
				ul_writelog(UL_LOG_FATAL, "[%s]: crf_seg_word is error", __func__);
				return -1;
			}
			row_len = 0;
			offset = 0;
			str_len = 0;
		}
		if(manword_cnt<pir->m_mandictword_cnt)
		{
			int man_pos = pir->m_mandictword_prop[manword_cnt] & 0x00FFFFFF;
			if(man_pos == m)
			{
				if( row_len != 0 )
				{
					ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, 
							str_hash_offset,max_count ,nbest, vlevel,inbuf,inlen,str_len);
					if( ret < 0 )
					{
						ul_writelog(UL_LOG_FATAL, "[%s]: crf_seg_word is error", __func__);
						return -1;
					}
					row_len = 0;
					offset = 0;
					str_len = 0;
				}

				int man_len = pir->m_mandictword_prop[manword_cnt] >>24;
				for( int i = 0; i < crf_res->nbest; i++)
				{											
					u_int *term_off = &(crf_res->term_buf[i].crftermcount);
					if(*term_off + 1 >=  max_count)
					{
						return -1;
					}
					crf_res->term_buf[i].crftermpos[(*term_off)++] = (man_len << 24)|m;
				}
				m += man_len;
				manword_cnt ++;
				continue;
			}
		}
		int ch_len =  ul_next_gb18030( inbuf+m );
		if( ch_len == 1 || ch_len == -1 || ch_len == 0 ) 
		{
			if( IS_ALPHA_1234((inbuf+m)) || IS_ALPHA_CH((inbuf+m)) )    
			{
				row[offset++] = (char) l_low_map[(unsigned char)inbuf[m]];
				row[offset++] = '\0';
				str_hash_offset[str_len] = m;
				++m;
				++str_len;
				++row_len;  
				str_hash_offset[str_len] = m;
			}
			else 
			{
				if( row_len != 0 )
				{
					ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, str_hash_offset,max_count ,nbest, vlevel,inbuf,inlen,str_len);
					//   ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, str_hash_offset,max_count ,nbest, vlevel);
					if( ret < 0 )
					{
						ul_writelog(UL_LOG_FATAL, "[%s]: crf_seg_word is error", __func__);
						return -1;
					}
					row_len = 0;
					offset = 0;
					str_len = 0;
				}
				for( int i = 0; i < crf_res->nbest; i++)
				{
					u_int *term_off = &(crf_res->term_buf[i].crftermcount); 
					if(*term_off + 1 >=  max_count)
					{
						return -1;
					}
					crf_res->term_buf[i].crftermpos[(*term_off)++] = (1 << 24)|m; 	   
				}   
				++m;
			}
		}
		else if( ch_len == 2 )
		{
			if( IS_GB_1234((inbuf+m)) || IS_GB_ABCD((inbuf+m)) )
			{
				//全角转半角,大写转小写
				row[offset++]= l_low_map[((unsigned char)inbuf[m+1]) - 0x80];
				row[offset++] = '\0';
				str_hash_offset[str_len] = m;
				m+=2;
				str_len += 1;
				++row_len;
				str_hash_offset[str_len] = m; 
			}
			else if( IN_RANGE(inbuf[m],0xa1,0xa3) )
			{
				if( row_len != 0 )
				{
					ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, str_hash_offset,max_count ,nbest, vlevel,inbuf,inlen,str_len);
					//   ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, str_hash_offset,max_count ,nbest, vlevel);
					if( ret < 0 )
					{
						ul_writelog(UL_LOG_FATAL, "[%s]: crf_seg_word is error", __func__);
						return -1;
					}
					row_len = 0;
					offset = 0;
					str_len = 0;
				}
				for( int i = 0; i < crf_res->nbest; i++)
				{
					u_int *term_off = &(crf_res->term_buf[i].crftermcount); 
					if(*term_off + 1 >=  max_count)
					{
						return -1;
					}
					crf_res->term_buf[i].crftermpos[(*term_off)++] = (2 << 24)|m;	       
				}
				m+=2;
			}
			else
			{
				row[offset++] = inbuf[m];
				row[offset++] = inbuf[m+1];
				row[offset++] = '\0';
				str_hash_offset[str_len] = m;
				m+=2;
				str_len += 2;
				++row_len;
				str_hash_offset[str_len] = m;
			}
		}
		else if( ch_len == 4 )
		{
			row[offset++] = inbuf[m];
			row[offset++] = inbuf[m+1];
			row[offset++] = inbuf[m+2];
			row[offset++] = inbuf[m+3];
			row[offset++] = '\0';
			str_hash_offset[str_len] = m;
			m+=4;
			str_len += 4;
			++row_len;
			str_hash_offset[str_len] = m;    
		}
	}
	if( row_len != 0 )
	{
		ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, str_hash_offset,max_count ,nbest, vlevel,inbuf,inlen,str_len);
		// ret = crf_seg_word(crf_tag, crf_res, crf_out, row, row_len, str_hash_offset,max_count ,nbest, vlevel);
		if( ret < 0 )
		{
			ul_writelog(UL_LOG_FATAL, "[%s]: crf_seg_word is error", __func__);
			return -1;
		}
		row_len = 0;
		offset = 0;
		str_len = 0;
	}
	return 0;
}
