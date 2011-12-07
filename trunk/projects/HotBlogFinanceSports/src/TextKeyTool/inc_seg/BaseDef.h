/*
* Copyright (c) 2005, ������
* All rights reserved.
* 
* �ļ����ƣ�BaseDef.h
* ժ    Ҫ��Basic definition for IASK Word Segment API (Unicode)
* ��    �ߣ�Sun Jian 2005/10/16
* ��    �ߣ�Sun Jian 2006/6/25
* ��    ����1.0
* $Id: 
*/

#ifndef __BASEDEFU_H__
#define __BASEDEFU_H__

#define __STRU_INFO__ 1

typedef unsigned int				WORDID;			// ID of Word item
typedef unsigned int*				LPWORDID;       // Pointer to Word Id
#define INVALID_WORDID			 0xFFFFFFFF     // Invalid WORDID

#define MAX_INPUT_TEXT_LEN (1024*8)  //�ִ�ϵͳ �����ı�����󳤶ȣ�8K��
#define WORD_MAX_LEN_SHORT 15 //unicode���� ����������0

/////////////////////////////////////////////////////////////////////////
// Error codes
#define SUCCESS				    0  //�ɹ�
#define ERR_SUCCESS				0  //�ɹ�
#define ERR_NO_MEMORY			-1 //�ڴ治��
#define ERR_NO_FILE				-2 //�ļ�û���ҵ�
#define ERR_BAD_INPUT			-3 //�����������
#define ERR_BAD_OUTPUT			-4 //���������
#define ERR_LIB_NOT_READY		-5 //�ʵ�� û��׼����
#define ERR_NO_FIND				-6   //û���ҵ�
#define ERR_BAD_PARAM			-7   //��������
#define ERR_UNKNOWN				-99  //��������
// End of definition of error codes.
/////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
///////////Segmentation Option Tags
//#define SEG_OPT_    			      //0000 0000 0000 0001 ����
#define SEG_OPT_AMB 0x0002        //0000 0000 0000 0010 �����з�
#define SEG_OPT_SUFFIX 0x0004     //0000 0000 0000 0100 �����׺�ʡ�"������ ��ͼҵ ������"
#define SEG_OPT_PREFIX 0x0008     //0000 0000 0000 1000 ����ǰ׺�ʡ�"��Ͷ�� ������ ���� СŮ�� �ǹ����� ���᳤ С�к� ������ ������ ������"

#define SEG_OPT_QUANTIFIER 0x0010 //0000 0000 0001 0000 ���������ʡ����ʣ�����
#define SEG_OPT_ZUHEQIYI 0x0020   //0000 0000 0010 0000 ����������塡"���ܡ��н������"
#define SEG_OPT_OVERLAP 0x0040    //0000 0000 0100 0000 �����ص��ʡ�ABAB(��Ϣ��Ϣ) AA(����) AAB(������)
#define SEG_OPT_LIHE    0x0080    //0000 0000 1000 0000 ������ϴʡ�"��һ�ġ��򲻵�����õ�"

#define SEG_OPT_PER 0x0100     //0000 0001 0000 0000 ��̬ʶ������
#define SEG_OPT_LOC 0x0200     //0000 0010 0000 0000 ��̬ʶ�����
#define SEG_OPT_ORG 0x0400     //0000 0100 0000 0000 ��̬ʶ�������
#define SEG_OPT_QUERY 0x0800   //0000 1000 0000 0000 ��query�����з�

#define SEG_OPT_DICT  0x1000		//0001 0000 0000 0000 ��ʵ�
//#define SEG_OPT_    			//0010 0000 0000 0000 ����
//#define SEG_OPT_    			//0100 0000 0000 0000 ����
#define SEG_OPT_MM  0x8000  //1000 0000 0000 0000 ���λ�ǣ�����ô��������з�

#define SEG_OPT_ALL	0x07ff  //0000 0111 1111 1111
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//Word categories dynamically merged
//�������ֽ�����ʾ
//���ֽڱ�ʾ ��̬����
//���ֽڱ�ʾ ��������
//nCat���ǰ��������ֽ�ֱ�ӣ�����һ��

#define MASK_XINGTAI 0xFF00
#define MASK_YUYI    0x00FF

//�������Կ�ʼ
#define CAT_UNKNOWN				    0x0000	// Can't specify the category

#define CAT_HAN_PERSON_NAME   0x00B1	//�������������Ƶ�������������
#define CAT_JPN_NAME			          0x00B2	//�ձ�����
#define CAT_OUMEI_PERSON_NAME 0x00B3	//�����������Ҫ��ŷ�������ͷ�������������
#define CAT_SURNAME_TITLE  0x00B4	    //��+ͷ��
#define CAT_PN_OTHER 0x00B5	          //���� ���͵����� "�����"

#define CAT_BOOK_NAME  0x00B6	         //�������л�������

#define CAT_DIMING		    0x00C1			    //����
#define CAT_DIMING_SUFFIX   0x00C2	//������׺ ��: "ʡ"

//#define CAT_CORP_NAME			0x00D1		//������
#define CAT_TRADE_MARK       0x00D2			//�̱�
#define CAT_CORP_SUFFIX       0x00D3			//��������׺ ��:����˾��
#define CAT_CORP_NAME_ABCD			0x00D4		     //������ ����/����/������/��˾
#define CAT_CORP_NAME_ABD			0x00D5		     //������ ����/����/��˾
#define CAT_CORP_NAME_ACD			0x00D6		     //������ ����/�ʵ�/��ѧ
#define CAT_CORP_NAME_AD			    0x00D7		     //������  ����/��ѧ
#define CAT_CORP_NAME_AACD			0x00D8		     //������
#define CAT_CORP_NAME_BCD			0x00D9		     //������  ����/������/��˾
#define CAT_CORP_NAME_BD			    0x00DA		     //������  ����/��˾
#define CAT_CORP_NAME_OTHER		0x00DB		     //������  ������ʽ�Ļ�����

#define CAT_DATE          0x00E1    //����
#define CAT_TIME		    0x00E2      //ʱ��

#define CAT_TELEPHONE_NUM 0x00F1 //�绰����
#define CAT_EMAIL 0x00F2 //e-mail��ַ
#define CAT_URL 0x00F3 //url
#define CAT_IP 0x00F4 //ip��ַ
#define CAT_MAYBE_VERSION 0x00F5 //may be version
//�������Խ���

//��̬���Կ�ʼ
#define CAT_ENG_WORD	    0x0100		//Ӣ�Ĵ� ȫ����Ӣ��
#define CAT_DIGITAL		        0x0200      //���ִ� ȫ��������

#define CAT_SPACES				0x1000			//Spaces

#define CAT_SUF_PHRASE			0x2000	  //Suffix phrase
#define CAT_PREFIX_PHRASE		0x2100	  //Prefix phrase

#define __GPWS_QUANTIFIER__ 0
#ifdef __GPWS_QUANTIFIER__
#define CAT_QUANTIFIER 			0x3000	        //������
#endif

#define CAT_OVERLAP_AA  	  0x4000	          //�ص���ʽAA
#define CAT_OVERLAP_ABAB  	0x4100	          //�ص���ʽABAB
#define CAT_OVERLAP_AABB  	0x4200	          //�ص���ʽAABB
#define CAT_OVERLAP_AAB  	  0x4300	      //�ص���ʽAAB
#define CAT_OVERLAP_ABB  	  0x4400	      //�ص���ʽABB

#define __GPWS_VERB_ADJ_VARIATION__ 1
#ifdef __GPWS_VERB_ADJ_VARIATION__		//
//��
#define CAT_VAR_BIN2			0x5000 			//V��N V��N V��N

//��
#define CAT_VAR_DAN3			0x5100			//VһV[N]

//��
#define CAT_VAR_BU1				0x5200			//V��C V��C
#endif // __GPWS_VERB_ADJ_VARIATION__
//��̬���Խ���

// End of categories dynamically merged.
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Word node for segment list
typedef unsigned long long ATTRIBUTE;
#define TERM_MAX_DOMAIN_NUM 2 //ÿһ��term����м�������
#define TERM_MAX_SEMANTIC_NUM 2 //ÿһ��term����м�����������
struct tagWordNodeU
{
   //Data
   int            nStart;		//Position in source string
   WORDID         Id;		        //WORDID of cut-out word item
		  			//�������δ��¼�ʣ���ô���Ǵʵ��е�wordid
			          	//�����δ��¼�ʣ���ôId��Ϊ-1
   unsigned short nCat;		  //Attribute of word item, such as personal name, corporation name, and so on
   
   unsigned char nLen;	    //Length of this word
   unsigned char idf;      //0-255. 2007.12.10
   unsigned char domainnum; //��term�м�������
   unsigned char semanticnum; //��term�м�����������
   
   unsigned short domainid[TERM_MAX_DOMAIN_NUM]; //��������
   unsigned short semanticid[TERM_MAX_SEMANTIC_NUM]; //��������
   
   //Pointer
   unsigned char right;		// Point to right node
};
typedef struct tagWordNodeU   _WordNodeU, *_PWordNodeU;

// Word item
struct tagWordItemU
{
	WORDID      Id;				// Id of word item
	unsigned short sWord[WORD_MAX_LEN_SHORT+1];	// Word item
};
typedef struct tagWordItemU         _WordItemU, *_PWordItemU;

/////////////////////////////////////////////////////////////////////

#endif //__BASEDEFU_H__
