/**
 * @file WeiboProcessor.h
 * @brief  
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

#ifndef WEIBOPROCESSOR_H
#define WEIBOPROCESSOR_H

#include "CConfigInfo.h"
#include "CResource.h"
#include "lexicon.h"
#include "fromuni.h"
#include "touni.h"
#include "ustring.h"
#include "checkArticle.h"
#include <map>
#include <set>
#include <vector>

extern "C"
{
	#include "nx_log.h"
}
extern ngx_log_t	*gNxlog;
using namespace std;

class WeiboProcessor
{
	public:

		WeiboProcessor();
		~WeiboProcessor();

	public:
		/**
		 * @brief 初始化实例 
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int Init(const char* cResource);

		/**
		 * @brief 处理分类文件
		 *
		 * @Param cWeiboFile
		 * @Param cProcessedWeiboFile
		 */
		int ProcessClassifiedData();

		/**
		 * @brief 处理分类文件的一行输入
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		int ParseLineClassified(string szLine);

		/**
		 * @brief 处理原始json数据
		 *
		 * @Param szInputFile
		 */
		int ProcessOriginalData();
		/**
		 * @brief 处理一行json原始数据
		 *
		 * @Param szLine
		 *
		 * @Returns   
		 */
		int ParseLineOriginal(string strLine);

		/**
		 * @brief 处理转发数据
		 *
		 * @Param 
		 */
		int ProcessForwardData();

		/**
		 * @brief 解析一行转发数据
		 *
		 * @Param strLine
		 *
		 * @Returns   
		 */
		int ParseLineForward(string strLine);

		/**
		 * @brief 处理评论数据 
		 *
		 * @Param szInputFile
		 */
		int ProcessCommentData();


		/**
		 * @brief 处理一行评论数据 
		 *
		 * @Param szInputFile
		 */
		int ParseLineComment(string strLine);


		/**
		 * @brief 计算所有微博权重
		 *
		 * @Returns   
		 */
		int CalcWeight();

		/**
		 * @brief 排序输出
		 *
		 * @Returns   
		 */
		int SortOutput();


	private:
		typedef struct BASEINFO
		{
			bool has_video;//是否有视频
			bool has_image;//是否有图片
			int nType;//分类
			int nGarbageScore;//垃圾评分
			int nYCScore;//数据源 客户端得分
			int nFilterScore;//filt字段得分
			int nForwardCount;//转发数
			int nValidCmtCount;//有效评论数
			int nContentLen;//微博长度
			int nAtNum;//微博里包含的@数量
			//int nClassifyScore;//分类得分
			//int nKeyWordsScore;//关键词得分
			//int nWords;//微博的词数
			//int nUidScore;//作者的分数
			//int nRubScore;//垃圾倾向评分
			unsigned long long nUid;//用户id
			float fTextScore;//微博文本评分
			string strKeywords;//关键字
			string strDebugContent;//微博内容，仅调试用
			string strDebugUrl;//url(如果有)
		}BaseInfo;

		typedef struct WEIBOINFO
		{
			BaseInfo base_info;
			int nWeight;//最终权重
		}WeiboInfo;

		/**
		 * @brief 加载资源
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int LoadResource(const char* cResource);

		/**
		 * @brief 加载切词库
		 *
		 * @Param cLexiconPath
		 *
		 * @Returns   
		 */
		int LoadLexicon(const char* cLexiconPath);

		void ReleaseWordSeg();


		/**
		 * @brief 微博内容判重
		 *
		 * @Param strContent
		 *
		 * @Returns   
		 */
		bool HaveDuplicates(string& strContent);

		void Clear();

/****************************以下为数据成员区**************************************/
	private:
		LEXICON * m_Lexicon; 
		WORD_SEGMENT *m_Wordseg ;
		unsigned long long m_WordSegFlags;

		// 需要处理的分类
		vector<int> m_types;

		// 微博的内容指纹
		set<unsigned long long> m_setContentKey;

		// 基本配置信息
		CConfigInfo* m_pConfigInfo;

		// 客户端配置信息
		CConfigInfo* m_pYCConfigInfo;

		//达人资源
		CResource* m_pDarenResource;

		//vip资源
		CResource* m_pVipResource;

		//type关系映射表(不是一个好方法)
		CResource* m_pTypeRelation;

		//IT类白名单
		CResource* m_pITWhiteList;

		//财经类白名单
		CResource* m_pFinanceWhiteList;

		//体育类白名单
		CResource* m_pSportsWhiteList;

		// 附加信息的个数
		//int m_nAttachInfoCount;

		int nMaxForwardCount;//转发数
		int nMaxValidCmtCount;//有效评论数

		// 关键词名称，bool值true表示关键词具有分值
		vector<pair<string, bool> > m_vecKeyWords;

		map<unsigned long long, WeiboInfo*> m_mapWeiboInfo;
};
#endif
