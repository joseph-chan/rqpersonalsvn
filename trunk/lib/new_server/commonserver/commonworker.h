#ifndef _COMMON_WORKER_H_
#define _COMMON_WORKER_H_ 1

#include "searchdef.h"

//创建全局数据
int CreateGlobalData(char **argv, int argc);

//释放全局数据
int ReleaseGlobalData();

//创建工作线程需要的数据
void* CreateWorkingData();

//释放工作线程需要的数据
void ReleaseWorkingData(void *working_data);

//处理函数
int DoWork(unsigned short command, DOCID_T docID,  BYTE *pSrcData, unsigned int srcLen, BYTE *pResultData, unsigned int * resultLen, void *working_data);

#endif
