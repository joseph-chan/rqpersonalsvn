#ifndef _STR2NUM_H_
#define _STR2NUM_H_

#ifdef __cplusplus
extern "C" {
#endif

struct Modeldata
{
	int col;	//ģ�ʹ���
	int num;	//ģ��ά��
	int *matr;	//ģ������
	int *svd;	//ģ������ֵ
};

struct cell
{
	int dim;	//ά��
	int val;	//��ά�ϵĵ÷�
};

struct vecData
{
	char *vec;
	char *newvec;
	int cellnum;	//��������
	struct cell *dimInfo;	//������
};

/* * * * * * * * ********************************************************
 * ��ȡģ������ 
 * ���������
 * infile ģ���ļ���vec��ģ���и�Ԫ�ؾ�����int��
 * svdfile ģ���ļ���svd��ģ���и�Ԫ�ؾ�����int��
 * wordlist ģ���ļ���Ӧ�Ĵʱ����и���˳��Ϊ����ģ���ļ�ʱ˳��
 * model ����ģ�͵Ľṹ��
 * ����ֵ���ɹ� >0 ģ�ʹ���
 * 		   ʧ�� ����
 * * * * * * * * ********************************************************/
	int ReadMatrixI(char *infile,char *svdfile,char *wordlist,struct Modeldata &model);

/* * * * * * * * ********************************************************
 * �����ĵ�����
 * ���������
 * vecstr �����ַ��� ��ʽΪ����:ֵ �м���\t�ָ���ֵ��Ϊint��
 * neednum ���ؽ����
 * Imodel ģ���ļ��Ľṹ��
 * result �������Ľṹ��
 * ����ֵ���ɹ� 0
 * 		   ʧ�� ����
 * * * * * * * * ********************************************************/
	int dealDocVec(char *vecstr,int neednum,struct Modeldata &Imodel,struct vecData &result);

/* * * * * * * * ********************************************************
 * �ͷ���Դ
 * * * * * * * * ********************************************************/
	void freeRe(struct Modeldata &Imodel);
/* * * * * * * * ********************************************************
 * ��ѯ�ʵ�ά����Ϣ
 * word Ҫ��ѯ�Ĵ�
 * Imodel ģ���ļ��Ľṹ��
 * result �������Ľṹ��
 * ����ֵ��
 * 		ά��������Ϊ0ʱ��ʾ��ѯ�����ô�
 * * * * * * * * ********************************************************/
	int checkword(char *word,struct Modeldata &Imodel,struct vecData &result);

	int outdiminfo(int dimno,struct Modeldata &model);
#ifdef __cplusplus
}
#endif
#endif
