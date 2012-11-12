/**
 * 微博文本工具
 *  
 * 作者： 开江
 *
 * 日期：2011-07-15
 *
 * 版本：0.4
 *
 * 最新修改：
 *		增加方法：抹掉微博中的昵称
 *		增加方法：抹掉微博中的URL
 *		增加方法：抹掉微博中的##标记的话题
 *		增加方法：抹掉微博中的表情
 *		增加方法：抹掉微博中的说明文字，（以括号包围）
 */

#ifndef WEIBOTEXTTOOL_H
#define WEIBOTEXTTOOL_H
#include <map>
#include <string>
#include <set>
#include <vector>
#include <cctype>

using namespace std;

class CMBTextTool
{
	public:
		// 初始化工具
		//
		static void InitTool();

		// 提取微博文本中@用户昵称列表
		//
		// 参数：
		//		text -- 文本
		//		mapAt -- 提取的昵称及其次数映射表
		//
		// 返回：
		//		昵称个数
		//
		static int ExtractNickName(const char* text, map<string, int>& mapAt);

		// 提取微博文本中@用户昵称列表
		// 参数：
		//		text -- 微博文本
		//		mapAt -- 提取的昵称及其次数映射表
		//		bRemoveAt -- 是否抹掉提取的昵称，默认为否
		// 返回：
		//		总共的昵称个数
		//
		static int ExtractNickName(string& text, map<string, int>& mapAt, bool bRemoveAt = false);
		static int KillNickName(string& text);

		// 抹掉微博中的url
		// 参数：
		//		text -- 微博文本
		//		mapURL -- 被抹掉的url
		// 返回：
		//		被抹掉的url总个数
		//
		static int GetURL(string& text, map<string, int>& mapURL);
		static int KillURL(string& text);

		// 抹掉微博中的话题
		// 参数：
		//		text -- 微博原文
		//		vecTopics[OUT] -- 被抹掉的话题
		//
		//	返回：
		//		被抹掉的话题总数
		//
		static int KillTopic(string& text, vector<string>& vecTopics);

		// 抹掉微博中的表情
		// 参数：
		//		text -- 微博原文
		//
		// 返回：
		//		被抹掉的表情总数
		//
		static int KillEmotion(string& text);

		// 抹掉微博中的说明文字
		// 参数：
		//		text -- 微博原文
		//
		// 返回：
		//		无
		static void KillIntrInfo(string& text);

	private:
		// 判断是否为中文标点
		//
		// 参数：
		//		两个字节的中文字符
		//
		// 返回：
		//      true -- 是
		//      false -- 否
		//
		static bool IsPunction(const char* ch);


		// 给定起止符号，抹掉其中的文字
		// 参数：
		//		strText -- 输入文字
		//		strBegin -- 起始符号串
		//		strEnd -- 结束符号串
		//		vecElements[OUT] -- 被抹掉的串
		// 返回：
		//		被抹掉的串个数
		//
		static int KillElements(string& strText, string strBegin, string strEnd, vector<string>& vecELements);

	private:

		static set<string> m_setChPunct;		//汉语中的标点
};
#endif
