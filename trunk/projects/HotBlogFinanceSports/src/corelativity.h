#ifndef CORELATIVITY_H
#define CORELATIVITY_H
#include "checkArticle.h"
#include "get_keyword2.h"
#include <vector>
#include "lexicon.h"
struct Core_Information
{
	double topic_score;//���ĵ÷�
	double reply_score;//�ظ�������̶ȵ÷�
	double corelativity;//���ĺͻظ���ض�,������
	unsigned short topic_value;//��������
	//topic_value=0;����Ϊһ��״̬
	//topic_value=1;����ֵ���Ƽ�
	//topic_value=2;������Ӧ���˵�
	unsigned short reply_value;
	//reply_value=0;һ������
	//reply_value=1;�Ƽ�,��ʾ�ڵ�һҳ
	//reply_value=2;����,Ӧ���˵�
};
typedef struct hdict_t Hash;
int get_corelativity(WORD_SEGMENT *wordseg_t,WORD_SEGMENT *wordseg_r,char *topic,char * reply,     
Core_Information &ans);
int Init_CoreInfor(Core_Information &ans);
int mvFileInHash_Value(char *filename);
void hash_map_clear();//��ش�hash_map�����
double test_hash(char *keyword);
#endif

