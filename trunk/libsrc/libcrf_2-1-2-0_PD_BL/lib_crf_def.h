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
/* 词语属性记录 */
typedef struct CRFP_SCW_PROPERTY_T
{
  int m_hprop;
  int m_lprop;
}crf_scw_property_t;


/* 分词结果存储结构I，对应接口I keep */
typedef struct CRF_SCW_OUT_T
{  
  //0 basic word sep result
  char* wordsepbuf;            // 字符缓冲（需预先初始化），用于存储分词结果
  u_int wsbsize;               // 字符缓冲大小（初始化缓冲区时给定）
  u_int wsb_curpos;            // 当前可用缓冲区开始为位置
  u_int wordtotallen;          // 当前被切分文本长度（不含用于分隔切分片段的'\t'）
  u_int wsbtermcount;          // 切分片段数目
  u_int* wsbtermoffsets;       // 基本词计数，也就是当前切分片段是从第几个基本词开始的
  u_int* wsbtermpos;           // 当前切分片段的字节偏移以及长度，整数的低24bit为偏移，高8bit为长度
  crf_scw_property_t* wsbtermprop; // 各切分片段的属性信息

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

  //scw_inner_t * m_pir; // 分词结果中间存储结构

}crf_scw_out_t; 










//wordner 需求
struct neprop_t
{
	char iNeName[11]; //CRF对于term的专名属性Id，与属性字典相对应
	int iWeight;   //对应属性id的概率值，或者得分
};

//term本身的信息
/*struct crfprop_t
{
	int weight;      //用crf每个字的分数值的加权平均

};
*/

struct crf_term_inner_buffer
{};
struct crf_inner_buffer
{};


struct crf_term_out_t
{

//	char* crfbuf;                // 字符缓冲(需预先初始化)，用于存储分词结果
//	u_int crfbufsize;			 // 字符缓冲大小（初始化缓冲区时给定）
//	u_int crf_curpos;			 // 当前可用缓冲区开始为位置
	u_int wordtotallen;          // 当前被切分文本长度（不含用于分隔切分片段的'\t'）
	u_int crftermcount;          // 切分片段数目
	u_int* crftermoffsets;        // 基本词计数，也就是当前切分片段是从第几个基本词开始的
	u_int* crftermpos;			 // 当前切分片段的字节偏移以及长度，整数的低24bit为偏移，高8bit为长度
	crf_scw_property_t* crftermprop; // 各切分片段的属性信息
	
	//crfprop_t* crfprop;            // 每个term的分数值，用crf每个字的分数值的加权平均
	neprop_t* crfneprop;           // wordner的相关信息
	crf_term_inner_buffer* term_inner; // 备用接口
};

struct crf_out_t
{
	int nbest;	                 // 返回的nbest值
	crf_term_out_t term_buf[MAX_NBEST_NUM];
	crf_inner_buffer* inner; //备用接口
};




#endif  //__LIB_CRF_DEF_H_


