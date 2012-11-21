/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * $Id: lib_crf_def.h,v 1.6 2010/05/17 10:44:53 zhangchao01 Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file lib_crf_def.h
 * @author zhangwei01(zhangwei01@baidu.com)
 * @date 2010/03/03 18:00:32
 * @version $Revision: 1.6 $ 
 * @brief 
 */



#ifndef  __LIB_CRF_DEF_H_
#define  __LIB_CRF_DEF_H_


#include <stdio.h>
#include <assert.h>
#include <stdint.h>

// micros defined here
//--------------------------------------------------------------------------------//
#define u_int unsigned int
#define u_char unsigned char

#define CRF_GET_TERM_POS(property)  ((property) & 0x00FFFFFF)
#define CRF_GET_TERM_LEN(property)  (((unsigned)property) >> 24)

#define MAX_NBEST_NUM 10

/*
#ifdef CRF
#define CRF_MAX_QUERY_LEN 256
#endif

#ifdef WORDSEGCRF
#define CRF_MAX_QUERY_LEN 100
#endif
*/
#define CRF_MAX_QUERY_LEN 256

//#define CRF_MAX_QUERY_ROW_LEN  8


// data structure defined here (keep)
//--------------------------------------------------------------------------------//
/* �������Լ�¼ */
typedef struct CRFP_SCW_PROPERTY_T
{
  int m_hprop;
  int m_lprop;
}crf_scw_property_t;


/* �ִʽ���洢�ṹI����Ӧ�ӿ�I keep */
typedef struct CRF_SCW_OUT_T
{  
  //0 basic word sep result
  char* wordsepbuf;            // �ַ����壨��Ԥ�ȳ�ʼ���������ڴ洢�ִʽ��
  u_int wsbsize;               // �ַ������С����ʼ��������ʱ������
  u_int wsb_curpos;            // ��ǰ���û�������ʼΪλ��
  u_int wordtotallen;          // ��ǰ���з��ı����ȣ��������ڷָ��з�Ƭ�ε�'\t'��
  u_int wsbtermcount;          // �з�Ƭ����Ŀ
  u_int* wsbtermoffsets;       // �����ʼ�����Ҳ���ǵ�ǰ�з�Ƭ���Ǵӵڼ��������ʿ�ʼ��
  u_int* wsbtermpos;           // ��ǰ�з�Ƭ�ε��ֽ�ƫ���Լ����ȣ������ĵ�24bitΪƫ�ƣ���8bitΪ����
  crf_scw_property_t* wsbtermprop; // ���з�Ƭ�ε�������Ϣ

  //1 word phrase sep result
  char* wpcompbuf;
  u_int wpbsize;
  u_int wpb_curpos;
  u_int wpbtermcount;
  u_int* wpbtermoffsets;
  u_int* wpbtermpos;
  crf_scw_property_t* wpbtermprop;

  //2 sub-phrase result
  char*  subphrbuf;
  u_int  spbsize;
  u_int  spb_curpos;
  u_int  spbtermcount;
  u_int*  spbtermoffsets;
  u_int*  spbtermpos;
  crf_scw_property_t * spbtermprop;

  //3 human name result
  char*  namebuf;
  u_int  namebsize;
  u_int  nameb_curpos;
  u_int  namebtermcount;
  u_int* namebtermoffsets;
  u_int* namebtermpos;
  crf_scw_property_t * namebtermprop;
  
  //4 book name result
  char*  booknamebuf;
  u_int   bnbsize;
  u_int   bnb_curpos;
  u_int   bnbtermcount;
  u_int*  bnbtermoffsets;
  u_int*  bnbtermpos;
  crf_scw_property_t * bnbtermprop;
    
  //5 internal buffer for term merging
  int  phrase_merged;
  char*  mergebuf;
  u_int   mbsize;
  u_int   mb_curpos;
  u_int   mbtermcount;
  u_int*  mbtermoffsets;
  u_int*  mbtermpos;

  //scw_inner_t * m_pir; // �ִʽ���м�洢�ṹ

}crf_scw_out_t; 










//wordner ����
struct neprop_t
{
	char iNeName[11]; //CRF����term��ר������Id���������ֵ����Ӧ
	int iWeight;   //��Ӧ����id�ĸ���ֵ�����ߵ÷�
};

//term�������Ϣ
/*struct crfprop_t
{
	int weight;      //��crfÿ���ֵķ���ֵ�ļ�Ȩƽ��

};
*/

struct crf_term_inner_buffer
{};
struct crf_inner_buffer
{};


struct crf_term_out_t
{

//	char* crfbuf;                // �ַ�����(��Ԥ�ȳ�ʼ��)�����ڴ洢�ִʽ��
//	u_int crfbufsize;			 // �ַ������С����ʼ��������ʱ������
//	u_int crf_curpos;			 // ��ǰ���û�������ʼΪλ��
	u_int wordtotallen;          // ��ǰ���з��ı����ȣ��������ڷָ��з�Ƭ�ε�'\t'��
	u_int crftermcount;          // �з�Ƭ����Ŀ
	u_int* crftermoffsets;        // �����ʼ�����Ҳ���ǵ�ǰ�з�Ƭ���Ǵӵڼ��������ʿ�ʼ��
	u_int* crftermpos;			 // ��ǰ�з�Ƭ�ε��ֽ�ƫ���Լ����ȣ������ĵ�24bitΪƫ�ƣ���8bitΪ����
	crf_scw_property_t* crftermprop; // ���з�Ƭ�ε�������Ϣ
	
	//crfprop_t* crfprop;            // ÿ��term�ķ���ֵ����crfÿ���ֵķ���ֵ�ļ�Ȩƽ��
	neprop_t* crfneprop;           // wordner�������Ϣ
	crf_term_inner_buffer* term_inner; // ���ýӿ�
};

struct crf_out_t
{
	int nbest;	                 // ���ص�nbestֵ
	crf_term_out_t term_buf[MAX_NBEST_NUM];
	crf_inner_buffer* inner; //���ýӿ�
};




#endif  //__LIB_CRF_DEF_H_


