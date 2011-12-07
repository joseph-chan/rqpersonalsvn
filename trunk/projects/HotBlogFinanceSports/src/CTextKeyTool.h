/**
 * 文本的key生成工具类，该key为32位长整型，用于代表该文本，
 * 一般，两篇不同的文本不太可能生成同样的key，因此，该key
 * 可用于对文本进行排重等。
 *
 * 作者：开江
 *
 * 日期：2011.07.13
 *
 */
 
#ifndef CTEXTKEYTOOL_H
#define CTEXTKEYTOOL_H

#include "paichong.h"
#include "CTokenizer.h"

class CTextKeyTool
{
	public:
		// 丹丽模式
		static CTextKeyTool* GetInstance();

		// 释放
		static void ReleaseInstance();

		// 初始化
		// 参数：
		//		cSegData -- 分词数据
		//		unFlag --产生key的前期处理标志位
		//				0x0001 -- 去掉@昵称
		//				0x0002 -- 去掉url
		//				0x0004 -- 去掉话题
		//				0x0008 -- 去掉表情
		//				16 -- 去掉以括号包围的说明文字
		//				ps : 可以组合
		// 返回：
		//		true -- 初始化成功
		//		false -- 失败
		//
		bool InitInstance(const char* cSegData, unsigned long unFlag = 0x0003);

		// 改变获取key的前期处理标志位
		//
		// 参数：
		//
		// 返回：
		//		unFlag --产生key的前期处理标志位
		//				0x0001 -- 去掉@昵称
		//				0x0002 -- 去掉url
		//				可以组合
		//
		unsigned long& GetFlag(){return m_unFlag;}

		// 给输入的文本生成一个key
		// 参数：
		//		cText -- 文本
		//		ullKey[OUT] -- 生成的key
		//		nMinTermCount -- 可以产生key的最少字数
		// 返回：
		//		true -- 成功
		//		false -- 错误
		//
		bool GetKey(const char* cText, unsigned long long & ullKey, int nMinTermCount = 15);

		// 给输入的文本生成多个模糊的key，对于某些模糊的位，既生成0又生成1，得到多个key
		// 参数：
		//		cText -- 文本
		//		vecKeys[OUT] -- 生成的多个key
		//		nMinTermCount -- 可以产生key的最少字数
		// 返回：
		//		true -- 成功
		//		false -- 失败
		bool GetKey(const char* cText, vector<unsigned long long>& vecKeys, int nMinTermCount = 15);

	private:
		// 构造函数
		CTextKeyTool();

		// 析构函数
		~CTextKeyTool();

		static CTextKeyTool* _instance;

		// 分词工具的实例
		CTokenizer* m_spTokenizer;

		unsigned long m_unFlag;
};

#endif
