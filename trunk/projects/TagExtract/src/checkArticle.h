#ifndef CHECKARTICLE_H
#define CHECKARTICLE_H

#include <iostream>
#include "lexicon.h"
#include <string>
using namespace std;
/***************************************************************
 * date:2008.10.06
 * author:guibin
 * description:�����ĵ�������̶Ƚ�������
 * version:1.1
 * input:	WORD_SEGMENT *�þ�Ⱥ�ִʺ�Ľṹ��
 * 			char *text ��Ⱥ�����ݣ����ڼ���ѹ����
 * 			int textLen ���ӵĳ���
 * return:>=0 ��Ⱥ�÷�
 *        ==-1.0 �����쳣���˳�
 * supply:no
 * ************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

double checkArticle(WORD_SEGMENT *wordseg,char *text,int textLen);

#ifdef __cplusplus
}
#endif

#endif
