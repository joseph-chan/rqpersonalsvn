#ifndef _STR2NUM_H_
#define _STR2NUM_H_

#ifdef __cplusplus
extern "C" {
#endif

struct Modeldata
{
	int col;	//模型词数
	int num;	//模型维数
	int *matr;	//模型数据
	int *svd;	//模型奇异值
};

struct cell
{
	int dim;	//维数
	int val;	//该维上的得分
};

struct vecData
{
	char *vec;
	char *newvec;
	int cellnum;	//请求结果数
	struct cell *dimInfo;	//保存结果
};

/* * * * * * * * ********************************************************
 * 读取模型数据 
 * 输入参数：
 * infile 模型文件，vec。模型中各元素均需是int型
 * svdfile 模型文件，svd。模型中各元素均需是int型
 * wordlist 模型文件对应的词表，表中各词顺序为生成模型文件时顺序
 * model 保存模型的结构体
 * 返回值：成功 >0 模型词数
 * 		   失败 其它
 * * * * * * * * ********************************************************/
	int ReadMatrixI(char *infile,char *svdfile,char *wordlist,struct Modeldata &model);

/* * * * * * * * ********************************************************
 * 处理文档向量
 * 输入参数：
 * vecstr 向量字符串 格式为：词:值 中间以\t分隔，值需为int型
 * neednum 返回结果数
 * Imodel 模型文件的结构体
 * result 保存结果的结构体
 * 返回值：成功 0
 * 		   失败 其它
 * * * * * * * * ********************************************************/
	int dealDocVec(char *vecstr,int neednum,struct Modeldata &Imodel,struct vecData &result);

/* * * * * * * * ********************************************************
 * 释放资源
 * * * * * * * * ********************************************************/
	void freeRe(struct Modeldata &Imodel);
/* * * * * * * * ********************************************************
 * 查询词的维度信息
 * word 要查询的词
 * Imodel 模型文件的结构体
 * result 保存结果的结构体
 * 返回值：
 * 		维度数量，为0时表示查询不到该词
 * * * * * * * * ********************************************************/
	int checkword(char *word,struct Modeldata &Imodel,struct vecData &result);

	int outdiminfo(int dimno,struct Modeldata &model);
#ifdef __cplusplus
}
#endif
#endif
