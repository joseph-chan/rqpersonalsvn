/*
* Copyright (c) 2006, �������з�����
* All rights reserved.
* 
* �ļ����ƣ�wordprop.h
* ��    ����
*/


#ifndef _WORD_PROP_H_
#define _WORD_PROP_H_

//Լ��������type���ԵĴʶ��ǽ��ôʡ�
#define WORDTYPE_INTERPUNCTION  0x01    //������
#define WORDTYPE_ENDSENTENCE    0x02    //���ӽ�����־
#define WORDTYPE_LINKWORD_ENGLISH    	0x04    //Ӣ������������Ӵ� 2004:03:05 2004/03/05  
#define WORDTYPE_LINKWORD_CHINESE    	0x08    //��������Ӵ�	���������� ����-���� ����.���� " ' �� �� �� ��  

#define WORDTYPE_PHRASE_MARK	0X10	//���� ��������
#define WORDTYPE_WORD_DROP	0X20	//���� �� �� ���� �� ��

#define WORDSTAT_NOPARSE        0x01            //����¼�ִʽ��
#define WORDSTAT_NOSEARCH       0x02            //��ֹ����

#define WORDSTAT_COMMONWORD		0X04	// �� �� �� ��
#define WORDSTAT_URLWORD		0X08	//www com net 

#define WORDSTAT_HASSTRU        0x40            //�ôʴ��ڽṹ
#define WORDSTAT_STRU   		0x80                  //�ṹ�ִʽ��

#define IS_SENTENCEMARK(type) ((type)&0x01)
#define IS_ENDSENTENCE(type) ((type)&0x02)
#define IS_LINKWORD_ENGLISH(type) ((type)&WORDTYPE_LINKWORD_ENGLISH)
#define IS_LINKWORD_CHINESE(type) ((type)&WORDTYPE_LINKWORD_CHINESE)

#define IS_NOPARSE(stat)  ((stat) & WORDSTAT_NOPARSE)						//���ô�

#define IS_COMMONWORD(stat) ((stat)&WORDSTAT_COMMONWORD)
#define IS_URLWORD(stat) ((stat)&WORDSTAT_URLWORD)

#define IS_STUCTWORD(stat) ((stat)&WORDSTAT_STRU) //�ṹ�ִʽ��
#define IS_HAS_STUCTWORD(stat) ((stat)&WORDSTAT_HASSTRU)//�ôʴ��ڽṹ

#define IS_SPACE(wordCat) ((wordCat) == 0x50)								//��
#define IS_ENG_WORD(wordCat) (((wordCat)==0x0003) || ((wordCat)==0x0011))	//Ӣ�Ĵʻ�����	


#endif 
