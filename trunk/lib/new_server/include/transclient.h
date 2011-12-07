/*
* Copyright (c) 2006, �������з�����
* All rights reserved.
* 
* �ļ����ƣ�
* ժ    Ҫ��
* ��    �ߣ��Ž�ǿ 2006/02/23
* ��    ����1.0
*/
#ifndef _TRANS_CLIENT_H_
#define _TRANS_CLIENT_H_ 1

#include "searchdef.h"


typedef struct ClientBuffer_t ClientBuffer_t;

/*
	���ܣ������ĵ��Ļ�������		
	����˵��:
		maxDoc: �趨һ�δ��������ĵ�����
	����:
		��NULL:����
		NULL:����ʧ��
*/
ClientBuffer_t* CreateClientBuffer(unsigned short maxDoc);


/*
	���ܣ��ͷ��ĵ��Ļ�������
	����˵����
		pClientBuffer���ĵ���������
	����:
*/
void ReleaseClientBuffer(ClientBuffer_t *pClientBuffer);


/*
	���ܣ���ʼ���ĵ����档ʹ��֮ǰ������ʹ��ǰ��������á�
	����˵����
		pClientBuffer:�ĵ���Ϣ��������
	����:
*/
void ResetClientBuffer(ClientBuffer_t *pClientBuffer);

/*
	���ܣ����һ�����ĵ���
	����˵����
		pClientBuffer���ĵ�������.
		docID:	�ĵ�docid, ����Ϊ0.
		pSrcData: doc�䳤����. ����ΪNULL
		dataLen:����
	����:
		0���ɹ�
		-1:��������ĵ��������ʧ�ܡ�
		-2:���������������ʧ��
		-3:�������Ϸ�
*/
int AddDocSrc(ClientBuffer_t * pClientBuffer, DOCID_T docID, BYTE *pSrcData, unsigned int dataLen);



/*
	���ܣ����͸��������������ȡ�����
	����˵����
		sock��socket�˿�
		pClientBuffer:�ĵ�������
		timeout:�趨�������ݵ����ʱʱ�䡣//��û��������.
	����:
		0:����
		-1:���ݴ���ʧ��
		(-10,-20):�����������⡣
		(-20,-30):�����������⡣
		
*/
int DoClientDocs(int sock, ClientBuffer_t * pClientBuffer,unsigned int timeout);


/*
	���ܣ���ȡ��i���ĵ����,�����������û��ͷ�
	����˵����
		pClientBuffer���ĵ�������
		i:��i������0��ʼ��
		docID: ָ��docid��ָ��	
		*pResultData: ָ��doc���������.	
		dataLen:ָ�����ݳ��ȵ�ָ��.�������Ը��ĵ�����ʧ�ܣ���¼�˴���ֵ
	����:
		0���ɹ�
		-1:û����
		-2:�������Ϸ�
*/
int GetDocResult(ClientBuffer_t *pClientBuffer, unsigned int  i,  DOCID_T *docID, BYTE **pResultData, int *dataLen);

/*
	����Ľӿ��Ǹ����ӿڣ�����ʹ�á�
*/

/*
	���ܣ���ȡ���������ĵ�������
	����˵����
		pClientBuffer���ĵ�������
	���أ�
		>=0:�ɹ�
		-1: ʧ��
*/
int GetDocNum(ClientBuffer_t *pClientBuffer);

/*
	���ܣ������ĵ�������Ϣ
	����˵����
		pClientBuffer���ĵ�������
		averageSrcLen:	Դ�ĵ�ƽ������		
		averageResultLen:  �ĵ����ƽ������
	���أ�
		0:�ɹ�
		-1: ʧ��	
	
*/
int SetDocTransInfo(ClientBuffer_t *pClientBuffer, unsigned int  averageSrcLen, unsigned int  averageResultLen);


/*
	���ܣ����ð�ͷ����
	����˵����
		pClientBuffer���ĵ�������
		command:	����
	���أ�
*/
void SetCommand(ClientBuffer_t *pClientBuffer, unsigned short command);



/*
	���ܣ���ȡdocID���ĵ����,�����������û��ͷ�
	����˵����
		pClientBuffer���ĵ�������
		docID: �ĵ����
		*pResultData: ָ��doc���������.	
		dataLen:ָ�����ݳ��ȵ�ָ��.�������Ը��ĵ�����ʧ�ܣ���¼�˴���ֵ
	����:
		0���ɹ�
		-1:û����
		-2:�������Ϸ�
*/
int GetDocResultByID(ClientBuffer_t *pClientBuffer, DOCID_T docID, BYTE **pResultData, unsigned int  *dataLen );



/*
	���ܣ���ȡ��i���ĵ�Դ����,�����������û��ͷ�
	����˵����
		pClientBuffer���ĵ�������
		i:��i������0��ʼ��
		docID: ָ��docid��ָ��	
		*pSrcData: ָ��docԴ������.	
		dataLen:ָ�����ݳ��ȵ�ָ��.
	����:
		0���ɹ�
		-1:û����
		-2:�������Ϸ�
*/
int GetDocSrc(ClientBuffer_t *pClientBuffer, unsigned int  i,   DOCID_T* docID,BYTE **pSrcData, unsigned int  *dataLen );

/*
	���ܣ���ȡdocid���ĵ�Դ����,�����������û��ͷ�
	����˵����
		pClientBuffer���ĵ�������		
		docID: �ĵ����
		*pSrcData: ָ��docԴ������.	
		dataLen:ָ�����ݳ��ȵ�ָ��.
	����:
		0���ɹ�
		-1:û����
		-2:�������Ϸ�
*/
int GetDocSrcByID(ClientBuffer_t *pClientBuffer, DOCID_T docID, BYTE **pSrcData, unsigned int *dataLen );


/*
	���ܣ���ȡ��ͷ����
	����˵����
		pClientBuffer���ĵ�������
	���أ�
*/
unsigned short GetCommand(ClientBuffer_t *pClientBuffer);

int PrintBufferInfo(ClientBuffer_t *pClientBuffer, char *infoStr, int strlen);

#endif
