#ifndef _COMMON_WORKER_H_
#define _COMMON_WORKER_H_ 1

#include "searchdef.h"

//����ȫ������
int CreateGlobalData(char **argv, int argc);

//�ͷ�ȫ������
int ReleaseGlobalData();

//���������߳���Ҫ������
void* CreateWorkingData();

//�ͷŹ����߳���Ҫ������
void ReleaseWorkingData(void *working_data);

//������
int DoWork(unsigned short command, DOCID_T docID,  BYTE *pSrcData, unsigned int srcLen, BYTE *pResultData, unsigned int * resultLen, void *working_data);

#endif
