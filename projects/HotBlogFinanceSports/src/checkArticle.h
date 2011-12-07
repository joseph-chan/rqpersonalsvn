#ifndef CHECKARTICLE_H
#define CHECKARTICLE_H

#include <iostream>
#include "lexicon.h"
#include <string>
using namespace std;
/***************************************************************
 * date:2008.10.06
 * author:guibin
 * description:对正文的有意义程度进行评价
 * version:1.1
 * input:	WORD_SEGMENT *该句群分词后的结构体
 * 			char *text 句群的内容，用于计算压缩率
 * 			int textLen 句子的长度
 * return:>=0 句群得分
 *        ==-1.0 程序异常，退出
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
