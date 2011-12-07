/*
 * Copyright (c) 2005, �������з�����
 * All rights reserved.
 *
 * �ļ����ƣ�selog.h
 * ժ    Ҫ��ͳһ��־����
 *         . ��־������/�쳣֮��
 *         . ��־��ʼ��ʱָ��·������־��ʶ
 *         . ��־����ͨ�õ�pid(��ѡ)��ʱ����Ϣ������ΪΪ�Զ�����Ϣ
 *         . ��־����ʱ֧���ļ��ֻ����ֻ���ʽ���԰�ʱ����С���У��ڳ�ʼ��ʱָ��
 *         . ���̰߳�ȫ
 *         . Ϊ����д��־Ӱ�칤���ٶȣ���־���Լӻ��壬���Դ�����־�����塣
 * ��    �ߣ����̱�, 2005.1.11
 * ��    ����1.0
 * $Id: selog.h,v 1.6 2005/04/08 02:38:26 lubing Exp $
 */
#ifndef _SE_LOG_H
#define _SE_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ��־����������� */
typedef void* LOG_HANDLE;

/* ��־���ѡ��壬�ڵ���seLogInitʱ���ݲ��������ֵ����ͨ��"��"һ�𴫵� */
enum
{
	SLO_OUTPUT_PID      = 0x00000001,  /* ���pid��־ */
	SLO_SWITCH_BY_HOUR  = 0x00000002,  /* ��־��Сʱ�����ֻ���� */
	SLO_SWITCH_BY_DAY   = 0x00000004,  /* ��־���ս����ֻ���� */
	SLO_SWITCH_BY_SIZE  = 0x00000008,  /* ��־�Դ�С�����ֻ������ͬʱ��ָ���ֻ���С */
	SLO_BUFFERED_OUTPUT = 0x00000010   /* ������־���л���������ͬʱ��ָ�������С */
};

/**
 * ��־��ʼ��
 * ����:
 *   phLogHandle: ���ڱ����ʼ�������־����������ڵ��ú�������ʱʹ��
 *   pszPath:     ��־���·��
 *   pszPrefix:   ��־�ļ���ǰ׺
 *   options:     ��־ѡ������壬��ѡ�����"��"��
 *                ���ѡ��SLO_SWITCH_BY_DAY���֣�SLO_SWITCH_BY_HOUR��������
 *   pszTimeFmt   ����ʱ��������ʽ��Ϊ����strftime��format�ַ�����ΪNULLʱʹ��"%Y-%m-%d %H:%M:%S"
 *   iSwitchSize: ���ֻ�����Ĵ�С(��λM)����ֵ��ѡ��SLO_SWITCH_BY_SIZEָ��ʱ�����ṩ�ұ������10M���޴�ѡ��ʱ������ʹ��ȱʡ�ֻ�ֵ
 *   iBufferSize: ������־�����С(��λK)����ֵ��ѡ��SLO_BUFFERED_OUTPUTָ��ʱ�����ṩ�ұ������10K���޴�ѡ��ʱ����
 * ����:
 *   0: ��ʼ���ɹ���phLogHandle������־���
 *   <0:��ʼ��ʧ�ܣ����ִ�����
 * [˵��] �ڲ�ָ����С�ֻ�������£�ȱʡ��СΪ1G���ֻ���С���>=2048M(2G)��Ҳ��ʹ��ȱʡ�ֻ���С
 */
int seLogInit(LOG_HANDLE* phLogHandle, const char *pszPath, const char *pszPrefix, int options, const char* pszTimeFmt, int iSwitchSize, int iBufferSize);

/**
 * �������������־����"Ex"Ϊ�̰߳�ȫ�ĺ���
 * ����:
 *   hLogHandle: ��������seLogInit����־���
 *   pszFormat:  �����־�ĸ�ʽ�������ο�printf
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seLog(LOG_HANDLE hLogHandle, const char *pszFormat, ...);
int seLogEx(LOG_HANDLE hLogHandle, const char *pszFormat, ...);

/**
 * ��������еĹ�����־[�̰߳�ȫ�ĺ���]
 * ����:
 *   hLogHandle: ��������seLogInit����־���
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seFlushLog(LOG_HANDLE hLogHandle);
int seFlushErrLog(LOG_HANDLE hLogHandle); /* ���Ե�ʱ���ã�����ή������ٶ� */

/**
 * ���������־����"Ex"Ϊ�̰߳�ȫ�ĺ���
 * seErrLog/seErrLogEx������ļ������к�
 * seErrLog2/seErrLog2Ex����ļ������к�
 * ����:
 *   hLogHandle: ��������seLogInit����־���
 *   pszFormat:  �����־�ĸ�ʽ�������ο�printf
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
#define seErrLog(   hLogHandle, pszFormat, ...)  seErrLogImpl  (NULL, 0, hLogHandle, pszFormat, ##__VA_ARGS__)
#define seErrLogEx( hLogHandle, pszFormat, ...)  seErrLogExImpl(NULL, 0, hLogHandle, pszFormat, ##__VA_ARGS__)
#define seErrLog2(  hLogHandle, pszFormat, ...)  seErrLogImpl  (__FILE__, __LINE__, hLogHandle, pszFormat, ##__VA_ARGS__)
#define seErrLog2Ex(hLogHandle, pszFormat, ...)  seErrLogExImpl(__FILE__, __LINE__, hLogHandle, pszFormat, ##__VA_ARGS__)

/**
 * ���������־����ʵ�֡���ʹ�����涨��ĺ���е���
 */
int seErrLogImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...);
int seErrLogExImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...);

/**
 * �ر���־���ͷŸ�����Դ
 * ����:
 *   phLogHandle: ��������seLogInit����־���
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seCloseLog(LOG_HANDLE* phLogHandle);

#ifdef __cplusplus
}
#endif

#endif
