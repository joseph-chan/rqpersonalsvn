/*
* Copyright (c) 2006, 新浪网研发中心
* All rights reserved.
* 
* 文件名称：
* 摘    要：
* 作    者：张金强 2006/02/23
* 版    本：1.0
*/
#ifndef _TRANS_CLIENT_H_
#define _TRANS_CLIENT_H_ 1

#include "searchdef.h"


typedef struct ClientBuffer_t ClientBuffer_t;

/*
	功能：创建文档的缓存区。		
	参数说明:
		maxDoc: 设定一次处理的最多文档个数
	返回:
		非NULL:正常
		NULL:申请失败
*/
ClientBuffer_t* CreateClientBuffer(unsigned short maxDoc);


/*
	功能：释放文档的缓存区。
	参数说明：
		pClientBuffer：文档缓存区。
	返回:
*/
void ReleaseClientBuffer(ClientBuffer_t *pClientBuffer);


/*
	功能：初始化文档缓存。使用之前和重新使用前，必须调用。
	参数说明：
		pClientBuffer:文档信息缓存区。
	返回:
*/
void ResetClientBuffer(ClientBuffer_t *pClientBuffer);

/*
	功能：添加一条新文档。
	参数说明：
		pClientBuffer：文档缓存区.
		docID:	文档docid, 可以为0.
		pSrcData: doc变长数据. 可以为NULL
		dataLen:长度
	返回:
		0：成功
		-1:到了最大文档数，添加失败。
		-2:缓存区不够，添加失败
		-3:参数不合法
*/
int AddDocSrc(ClientBuffer_t * pClientBuffer, DOCID_T docID, BYTE *pSrcData, unsigned int dataLen);



/*
	功能：发送给服务机器，并获取结果。
	参数说明：
		sock：socket端口
		pClientBuffer:文档缓存区
		timeout:设定接受数据的最大超时时间。//还没有起作用.
	返回:
		0:正常
		-1:数据处理失败
		(-10,-20):发送数据问题。
		(-20,-30):接收数据问题。
		
*/
int DoClientDocs(int sock, ClientBuffer_t * pClientBuffer,unsigned int timeout);


/*
	功能：获取第i条文档结果,数据区无须用户释放
	参数说明：
		pClientBuffer：文档缓存区
		i:第i条，从0开始。
		docID: 指向docid的指针	
		*pResultData: 指向doc结果数据区.	
		dataLen:指向数据长度的指针.如果服务对该文档处理失败，记录了错误值
	返回:
		0：成功
		-1:没数据
		-2:参数不合法
*/
int GetDocResult(ClientBuffer_t *pClientBuffer, unsigned int  i,  DOCID_T *docID, BYTE **pResultData, int *dataLen);

/*
	下面的接口是辅助接口，方便使用。
*/

/*
	功能：获取缓存区内文档个数。
	参数说明：
		pClientBuffer：文档缓存区
	返回：
		>=0:成功
		-1: 失败
*/
int GetDocNum(ClientBuffer_t *pClientBuffer);

/*
	功能：设置文档配置信息
	参数说明：
		pClientBuffer：文档缓存区
		averageSrcLen:	源文档平均长度		
		averageResultLen:  文档结果平均长度
	返回：
		0:成功
		-1: 失败	
	
*/
int SetDocTransInfo(ClientBuffer_t *pClientBuffer, unsigned int  averageSrcLen, unsigned int  averageResultLen);


/*
	功能：设置包头命令
	参数说明：
		pClientBuffer：文档缓存区
		command:	命令
	返回：
*/
void SetCommand(ClientBuffer_t *pClientBuffer, unsigned short command);



/*
	功能：获取docID的文档结果,数据区无须用户释放
	参数说明：
		pClientBuffer：文档缓存区
		docID: 文档编号
		*pResultData: 指向doc结果数据区.	
		dataLen:指向数据长度的指针.如果服务对该文档处理失败，记录了错误值
	返回:
		0：成功
		-1:没数据
		-2:参数不合法
*/
int GetDocResultByID(ClientBuffer_t *pClientBuffer, DOCID_T docID, BYTE **pResultData, unsigned int  *dataLen );



/*
	功能：获取第i条文档源数据,数据区无须用户释放
	参数说明：
		pClientBuffer：文档缓存区
		i:第i条，从0开始。
		docID: 指向docid的指针	
		*pSrcData: 指向doc源数据区.	
		dataLen:指向数据长度的指针.
	返回:
		0：成功
		-1:没数据
		-2:参数不合法
*/
int GetDocSrc(ClientBuffer_t *pClientBuffer, unsigned int  i,   DOCID_T* docID,BYTE **pSrcData, unsigned int  *dataLen );

/*
	功能：获取docid的文档源数据,数据区无须用户释放
	参数说明：
		pClientBuffer：文档缓存区		
		docID: 文档编号
		*pSrcData: 指向doc源数据区.	
		dataLen:指向数据长度的指针.
	返回:
		0：成功
		-1:没数据
		-2:参数不合法
*/
int GetDocSrcByID(ClientBuffer_t *pClientBuffer, DOCID_T docID, BYTE **pSrcData, unsigned int *dataLen );


/*
	功能：获取包头命令
	参数说明：
		pClientBuffer：文档缓存区
	返回：
*/
unsigned short GetCommand(ClientBuffer_t *pClientBuffer);

int PrintBufferInfo(ClientBuffer_t *pClientBuffer, char *infoStr, int strlen);

#endif
