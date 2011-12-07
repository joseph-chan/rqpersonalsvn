/*
* Copyright (c) 2005-2008
* All rights reserved.
* 
* �ļ����ƣ�POSTagger.h
* ժ    Ҫ�����Ա�ע�ӿ�
* ��    �ߣ��｡ 2006/01/05
* ��    ����1.0
* 
*/

#ifndef _IASK_NLP_POSTagger_H
#define _IASK_NLP_POSTagger_H

/************************************************************/
//		���Զ��岿��										//
/************************************************************/
#define POSTAG_ID_UNKNOW 0	//��֪��

#define POSTAG_ID_A  10	//	���ݴ�

#define POSTAG_ID_B	20	//	�����

#define POSTAG_ID_C	30	//	����
#define POSTAG_ID_C_N	31	//	�������
#define POSTAG_ID_C_Z	32	//	�־�����

#define POSTAG_ID_D	 40   //����
#define POSTAG_ID_D_B	41	//����	"��"
#define POSTAG_ID_D_M	42	//����	"û"

#define POSTAG_ID_E	  50	//̾��

#define POSTAG_ID_F	  60  //��λ��
#define POSTAG_ID_F_S	61	//��λ���� ������+��λ��
#define POSTAG_ID_F_N	62	//��λ���� ����+��λ�ʡ����ϡ�
#define POSTAG_ID_F_V	63	//��λ���� ����+��λ�ʡ�ȡǰ��
#define POSTAG_ID_F_Z	64	//��λ���� ����+��λ�ʡ�ȡǰ��

#define POSTAG_ID_H	70	   //ǰ�ӳɷ�
#define POSTAG_ID_H_M	71	//����ǰ׺ ������---��ʮ
#define POSTAG_ID_H_T	72	//ʱ���ǰ׺ ����Ԫ���������֡�
#define POSTAG_ID_H_NR 73	//����
#define POSTAG_ID_H_N 74	//����

#define POSTAG_ID_K	80	//��ӳɷ�
#define POSTAG_ID_K_M	81	//���ʺ�׺	������-- ʮ����
#define POSTAG_ID_K_T	82	//ʱ��ʺ�׺	��������ĩ����ʱ��
#define POSTAG_ID_K_N	83	//���ʺ�׺	���ǡ�
#define POSTAG_ID_K_S	84	//�����ʺ�׺	��Է�����
#define POSTAG_ID_K_Z	85	//״̬�ʺ�׺	��Ȼ��
#define POSTAG_ID_K_NT	86	//״̬�ʺ�׺	��Ȼ��
#define POSTAG_ID_K_NS	87	//״̬�ʺ�׺	��Ȼ��

#define POSTAG_ID_M	90	//����

#define POSTAG_ID_N	95	//����
#define POSTAG_ID_N_RZ 96	//����	��ë�󶫡�
#define POSTAG_ID_N_T	 97 //��������	���š�����ĸΪt�����ʴ���n��t����һ�𡣡���˾��
#define POSTAG_ID_N_TA	98
#define POSTAG_ID_N_TZ	99	//����������	"����"
#define POSTAG_ID_N_Z	100	//����ר��	��ר������ĸ�ĵ�1����ĸΪz�����ʴ���n��z����һ��
#define POSTAG_ID_NS	101	//������
#define POSTAG_ID_NS_Z	102	//����	������רָ�����й���
#define POSTAG_ID_N_M	103	//	n-m	���ʿ�ͷ������	����ѧ��
#define POSTAG_ID_N_RB 104	//n-rb	�������/���ʿ�ͷ������	��ѧУ������

#define POSTAG_ID_O	107	  //������
#define POSTAG_ID_P	108	  //���

#define POSTAG_ID_Q	110	  //����
#define POSTAG_ID_Q_V	111	//������	���ˡ����顱
#define POSTAG_ID_Q_T	112	//ʱ������	���ꡱ���¡����ڡ�
#define POSTAG_ID_Q_H	113	//�������� ��Ԫ������Ԫ����Ӣ���� 

#define POSTAG_ID_R	120	  //����
#define POSTAG_ID_R_D	121	//�����Դ���	����ô��
#define POSTAG_ID_R_M	122	//�����Դ���	�����١�
#define POSTAG_ID_R_N	123	//�����Դ���	��ʲô����˭��
#define POSTAG_ID_R_S	124	//�������Դ���	���Ķ���
#define POSTAG_ID_R_T	125	//ʱ����Դ���	����ʱ��
#define POSTAG_ID_R_Z	126	//ν���Դ���	����ô����
#define POSTAG_ID_R_B	127	//������Դ���	��ĳ����ÿ��

#define POSTAG_ID_S		130 //������	ȡӢ��space�ĵ�1����ĸ����������
#define POSTAG_ID_S_Z		131 //������	ȡӢ��space�ĵ�1����ĸ����������
#define POSTAG_ID_T   132 //ʱ���	ȡӢ��time�ĵ�1����ĸ
#define POSTAG_ID_T_Z	133 //ʱ��רָ	���ƴ��������ܡ�

#define POSTAG_ID_U		140 //����
#define POSTAG_ID_U_N	141 //��������	���ġ�
#define POSTAG_ID_U_D	142	//״������	���ء�
#define POSTAG_ID_U_C	143	//��������	���á�
#define POSTAG_ID_U_Z	144	//ν�ʺ�����	���ˡ��š�����
#define POSTAG_ID_U_S	145	//��ʺ�����	���ȡ��ȵȡ�
#define POSTAG_ID_U_SO	146	//���� ������ 

#define POSTAG_ID_W		150 //������
#define POSTAG_ID_W_D	151 //�ٺ�	������
#define POSTAG_ID_W_SP	152 //���	������
#define POSTAG_ID_W_S	153 //�־�β��� ������������ 
#define POSTAG_ID_W_L	154 //�����ͱ����
#define POSTAG_ID_W_R	155 //�����ͱ���Ҳ� ��������]�������� 
#define POSTAG_ID_W_H	156 //��׺�ͷ���

#define POSTAG_ID_Y		160 //������	ȡ���֡������ĸ�����𡱡��ɡ�������

#define POSTAG_ID_V	170 //���ﶯ��	ȡӢ�ﶯ��verb�ĵ�һ����ĸ��
#define POSTAG_ID_V_O 171	//������ν��	ν���ṹ����ͷ��
#define POSTAG_ID_V_E 172		//�����ṹ����	��ȡ�������ŵ���
#define POSTAG_ID_V_SH	173	//���ʡ��ǡ�
#define POSTAG_ID_V_YO	174	//���ʡ��С�
#define POSTAG_ID_V_Q	175	//���򶯴� ��������ȥ���������� 
#define POSTAG_ID_V_A	176	//������ ��Ӧ�á����ܹ��� 

#define POSTAG_ID_Z		180 //״̬�� �����ﶯ��	v-o��sp֮��Ĳ����ﶯ��

#define POSTAG_ID_X   190 //������
#define POSTAG_ID_X_N 191  //��������	������
#define POSTAG_ID_X_V 192  //��������	������
#define POSTAG_ID_X_S 193  //����������	���С����ա�������
#define POSTAG_ID_X_T 194  //ʱ�������	���ơ����Ρ���Ԫ��
#define POSTAG_ID_X_Z 195  //״̬������	��ΰ��������
#define POSTAG_ID_X_B 196  //״̬������	��ΰ��������

#define POSTAG_ID_SP  200  //������ν��	��ν�ṹ�����ᡱ��ͷ�ۡ�
#define POSTAG_ID_MQ  201  //��������	��������
#define POSTAG_ID_RQ  202  //��������	�������

#define POSTAG_ID_AD  210  //���δ�	ֱ����״������ݴ�
#define POSTAG_ID_AN  211  //���δ�	�������ʹ��ܵ����ݴ�
#define POSTAG_ID_VD  212  //������	ֱ����״��Ķ���
#define POSTAG_ID_VN  213  //������	ָ�������ʹ��ܵĶ���

#define POSTAG_ID_SPACE  230  //�ո�

#include "WordSegment.h"

//�ṩ���û���Ľڵ���Ϣ
struct WordPOSNode
{
	_WordNodeU outnode;   //�ṩ���û���Ĵ���Ϣ���ɷִʽ���������ڴ��Ա�ע�����в����޸����е��κ���Ϣ; �����ڶ���ϲ������п��ܸı������������ǣ�

	unsigned char chType; //����ʵĴ�����Ϣ�Ƿ��ڴʵ���
	//1 �ô�ֻ��һ������
	//2 �ôʵĴ��������ڷִʵĽڵ�����
	//3 �ô��ж������
	//4 �ڴ��Դʵ��в����ڸôʣ��ǲ²������
	
	//5 �ô���Ӧ�ò�ϲ������ִʽ���õ�
	//6 �ô�ֻ�Ƿִʽ���ڵ�
	//7 ����Ӧ�ò�ڵ㣬Ҳ�Ƿִʽ���ڵ�
	unsigned short synonymtype;
	
	unsigned char postagid;//�ṩ���û���Ĵ���id
	char postag[6];      //�����5���ֽ�, �ṩ���û���Ĵ�����Ϣ 
};

typedef struct WordPOSNode _WordPOSNode, *_PWordPOSNode;


#define POSTAGGER_OPT_ALL			  0x7fff
#define POSTAGGER_OPT_USESEGINFO 0x0001      //�̳зִ�ϵͳ�е��������

#ifdef __cplusplus
// Interface for POSTagger:
class  IPOSTagger {
public:

	// ����ֵ  ��	0			�ɹ�
	//						<0		ʧ��
	virtual int InitPOSTagger(IWordSegment *pWordSegment) = 0;
	
	//�ӿں���1
	//�ı�->WordSegment->POSTagger
	//�밮�ʷִ�ϵͳ�ӿ�
	// ����ֵ  ��	0			�ɹ�
	//						<0		ʧ��
	virtual int POSTagger(unsigned short* szSrc, unsigned int nLen, _PWordNodeU pHeadNode, unsigned short nOption) = 0;
	
	//�ӿں���2
	//�ı� -> WordSegmentPOSTagger���ִ�����Ա�עһ�������
	// ����ֵ  ��	0			�ɹ�
	//						<0		ʧ��
	virtual int WordSegmentPOSTagger(unsigned short* szSrc, unsigned int nLen, unsigned short nOption) = 0;
	
	//�ӿں���3
	//�ÿո�������ı�->POSTagger
	//���봮Ϊchar *pSentence���ÿո�����ľ��� w1 w2 w3 ...,��'\0'Ϊ������
	//virtual int POSTag_SegmentedSent(unsigned short* szSrc, unsigned int nLen) = 0;

	//Traversing node list methods:
	virtual _PWordPOSNode GetNodeListHead() = 0;
	virtual _PWordPOSNode GetNextNode(_PWordPOSNode pCurNode) = 0;

	//�ɴ���id -> ����
	// ����ֵ  ��	0			�ɹ�
	//						<0		ʧ��
	virtual int TagId2POSTag(unsigned char posid,  char *postag) = 0;

	//�ɴ��� -> ����id
	// ����ֵ  ��	0			�ɹ�
	//						<0		ʧ��
	virtual int POSTag2Id(const char *postag, unsigned char *posid) = 0;
	
	//�ɵ��ʲ鵽�õ��ʵ������ʵ��Ǹ�����id
	//���Ҹ������ж��ٸ�����
	//����ô�û�д���,��ôposnum=0
	
	// ����ֵ  ��	0			�ɹ�
	//					<0		ʧ��
	virtual int GetWordPOSId(unsigned short *uniword, unsigned char *posid, int *posnum) = 0;
	
};


// Create POSTagger instance, return pointer to the interface of it(Call Release to destroy it):
extern "C"  IPOSTagger* CreatePOSTaggerInstance(void);
typedef IPOSTagger* (* CREATEPOSTAGGERINSTANCE)(void);

//Call it to release POSTagger instance
extern "C"  int ReleasePOSTagger(IPOSTagger *);
typedef int (*RELEASEPOSTAGGER)(IPOSTagger*);

#endif //end ifdef __cplusplus

#endif //end _IASK_NLP_POSTagger_H
