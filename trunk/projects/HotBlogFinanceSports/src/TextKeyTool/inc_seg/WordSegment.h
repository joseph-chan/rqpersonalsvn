/*
* Copyright (c) 2005, ������
* All rights reserved.
* 
* �ļ����ƣ�WordSegment.h
* ժ    Ҫ��IASK Word Segment API (Unicode)
* ��    �ߣ�Sun Jian 2005/10/16
* ��    ����1.0
* $Id: 
*/

#ifndef __IWORDSEGMENT_H__
#define __IWORDSEGMENT_H__

#include "BaseDef.h"

#ifdef __cplusplus

// Initialize segmentation(szPath: data lib path, nBufSize: buffer size(default 16KB)):
// Note : length of szPath can not be more than 256.
// Return value: (failed -1, successful 0).
extern "C"  int CoInitWordSegment(char* szPath, unsigned int nBufSize = MAX_INPUT_TEXT_LEN);
typedef int (*COINITWORDSEGMENT)(char*, unsigned int);

extern "C" int EndWordSegment(void);
typedef int (*ENDWORDSEGMENT)(void);

// Call back function for word-lib interface
// Used for list word items
typedef int ( *_FP_ZxListWordItem)(_PWordItemU);

// Word Lib interface:
class  IWordLib {
public:

	//********************************************************************			
	//	��������:	Load word lib
	//	����˵��:	
	//	�� �� ֵ:	SUCCESS:success; <0: fail
	//	��    ��:	
	//	��֪����:	��
	//	�� �� ��:	��	
	//********************************************************************
	virtual int Load() = 0;				// Load word lib

	//********************************************************************
	//	��������:	GetWordID
	//	����˵��:	
	//	�� �� ֵ:	0<, <INVALID_WORDID:success; INVALID_WORDID: ������
	//	��    ��:	unsigned short *szWord �Ľ�β�� 0 ����
	//	��֪����:	��
	//	�� �� ��:	��	
	//********************************************************************
	virtual int GetWordID(unsigned short *szWord, LPWORDID pWordId) = 0;             // Get word id.
	virtual int GetWordID(unsigned short *szWord, int unilen, LPWORDID pWordId) = 0;             // Get word id
	virtual int GetWordID(const char *sWord, LPWORDID pWordId) = 0;             // Get word id.

	//********************************************************************
	//	��������:	GetWordItemFromID
	//	����˵��:	
	//	�� �� ֵ:	SUCCESS:success; <0: fail
	//	��    ��:	unsigned short *szWord �Ľ�β�� 0 ����
	//	��֪����:	��
	//	�� �� ��:	��	
	//********************************************************************
	virtual int GetWordItemFromID(unsigned short *szWord, LPWORDID pWordId) = 0;	 // Get word item from WORDID

#ifdef __STRU_INFO__
	/**
	 * Set Word Structure Information.
	 * PARAM:
	 *	pWordId: pointer to WORDID
	 pStruInfo: the structure information of word, formatted as follows:
	 *		e.g. for word: "�����ʵ��ѧ", its struct typed as {����{�ʵ�/��ѧ}}, and the tree-format is:
	 *							{����{�ʵ�/��ѧ}}(6)
	 *								/		\
	 *							����(2)  {�ʵ�/��ѧ}(4)
	 *										 /  \
	 *									�ʵ�(2) ��ѧ(2)
	 *			 So, the structural information <pStruInfo> can be formatted as Preorder list of the tree:
	 *			 0x86 0x02 0x84 0x02 0x02 [0x00(NULL terminator)]
	 *			 (
	 *			  In structure information, each (byte) for a tree-node. if the highest bit of the node(byte) is set, 
	 *			  the node is non-leaf node, and length of the node is ((byte) & 0x7f); otherwise, the node is 
	 *			  leaf node, and length of the node is the exact value of the (byte).
	 *			 )
	 * RETURN:
	 *	SUCCESS: return SUCCESS;
	 *	FAILED:	 return value < 0;
	 */
	virtual int SetWordStruInfo(unsigned short *szWord, unsigned char *pStruInfo) = 0;
	virtual int GetWordStruInfo(unsigned short *szWord, unsigned char *pStruInfo) = 0;	
#endif //__STRU_INFO__
	
	/*
	*	SUCCESS: return SUCCESS;
	*	FAILED:	 return value < 0;
	*/
	virtual int ListAllWordItem(_FP_ZxListWordItem fpZxListWordItem) = 0;           // List all word items in the lib.	
	
	// Get the state of lib. (Return value: Unloaded 0, Loaded 1, Changed 2)
	virtual int GetLibState() = 0;													// Get the state of lib. (Return value: Unloaded 0, Loaded 1, Changed 2)

	/*
	*	SUCCESS: return SUCCESS
	*	FAILED:	 return value < 0	     
	*/
	virtual int GetLibInfo(unsigned int *pWordItemNum, unsigned int *pPageNum) = 0;

public:
	int m_iWordLibId;
};
extern "C" IWordLib* CreateWordLibInstance(int iWordLibId);   //Create a word lib instance
typedef IWordLib* (* CREATEWORDLIBINSTANCE)(int);

//Call it to release word lib instance
extern "C" int ReleaseWordLib(IWordLib*);
typedef int (*RELEASEWORDLIB)(IWordLib*);

// Call back function for Segmentation:
#ifdef __SEGMENT_CALLBACK__
class IWordSegment;
typedef void (* WORDSEGMENT_CALLBACK)(IWordSegment *pWordSegment, unsigned short* szSrc);
#endif //__SEGMENT_CALLBACK__

// Interface for segmentation
class  IWordSegment {
public:
	
	/*
	* Prepare segment, call it before segmentation.ֻ����һ�ξͿ�����.
	*	SUCCESS: return SUCCESS;
	*	FAILED:	 return value < 0; 
	*/
	virtual int PrepareSegment(IWordLib** ppWordLibs, int nLibNum) = 0;
	
	
	/*
	* Main word-segment function
	*	SUCCESS: return SUCCESS;
	*	FAILED:	 return value < 0;
	*/
#ifdef __SEGMENT_CALLBACK__
	virtual int WordSegment(unsigned short* szSrc, unsigned int nLen, unsigned short nOption = SEG_OPT_ALL, WORDSEGMENT_CALLBACK pCallBack = NULL) = 0;
#else
	virtual int WordSegment(unsigned short* szSrc, unsigned int nLen, unsigned short nOption = SEG_OPT_ALL) = 0;
#endif //__SEGMENT_CALLBACK__
	
	//Traversing node list methods:
	virtual _PWordNodeU GetNodeListHead() = 0;					//Get first node.
	virtual _PWordNodeU GetNextNode(_PWordNodeU pCurNode) = 0;	//Get next node.
	virtual _PWordNodeU GetPreNode(_PWordNodeU pCurNode) = 0;	//Get previous node.

#ifdef __STRU_INFO__
	//********************************************************************
	//	��������:	Get word structural information
	//	����˵��:	�������ṩpStruInfo�Ŀռ�
	//	�� �� ֵ:	
	//  SUCCESS: return SUCCESS;
	//  FAILED:	 return value < 0;
	//********************************************************************
	virtual int GetWordStruInfo(_PWordNodeU pWordNode, unsigned char* pStruInfo) = 0;	
	
	//********************************************************************
	//	��������:	Get word structural information
	//	����˵��:	�������ṩpStruInfo�Ŀռ�
	//	�� �� ֵ:	
	//  SUCCESS: return SUCCESS;
	//  FAILED:	 return value < 0;
	//  ˵��������unicode�ַ������õ����ڴʵ��еĽṹ��Ϣ
	//********************************************************************
	virtual int GetWordStruInfo_Dict(unsigned short *szWord, unsigned char* pStruInfo) = 0;
	
	//virtual int GetWordStruInfo2(_PWordNodeU pWordNode, unsigned char* pStruInfo) = 0;
#endif // __STRU_INFO__

#ifdef __GRAM_INFO__
	//********************************************************************
	//	��������:	Get word grammar information
	//	����˵��:	�������ṩpGramInfo�Ŀռ�
	//	�� �� ֵ:	
	//  SUCCESS: return SUCCESS;
	//  FAILED:	 return value < 0;	     
	//	��    ��:	
	//	��֪����:	��
	//	�� �� ��:	��
	//********************************************************************
	virtual int GetWordGramInfo(_PWordNodeU pWordNode, _PGramInfo pGramInfo) = 0;
#endif //__GRAM_INFO__
	
	//�õ�������ʵı�����ʽ (�߸����� -> ����)
	//����У��򷵻�true�����ѽ������szWord��
	virtual bool GetAnotherMorphForm(unsigned short *pInput, _PWordNodeU pWordNode, unsigned short *szWord) = 0;		
};

// Create word segment instance, return pointer to the interface of it(Call Release to destroy it):
extern "C"  IWordSegment* CreateWordSegmentInstance(void);
typedef IWordSegment* (* CREATEWORDSEGMENTINSTANCE)(void);
	
//Call it to release word-segment instance
extern "C" int ReleaseSegment(IWordSegment*);
typedef int (*RELEASESEGMENT)(IWordSegment*);

#endif //#ifdef __cplusplus

#endif //__IWORDSEGMENT_H__
