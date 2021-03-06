/**
 * @file CResource.h
 * @brief  解析资源文件
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-22
 */

#ifndef CRESOURCE_H
#define CRESOURCE_H
#include <map>
#include <string>
#include <vector>

using namespace std;

class CResource
{
	public:
		// 构造函数
		CResource();
		CResource(const char* cConfigFile,int nBlockNum, char cSplitChar = '\t');
		~CResource();

		// 根据key获取value
		int GetValue(string strKey,vector<string> &vOutput);

		const map<string, vector<string> > & GetAllValue();

		// 修改value
		//void SetValue(string strKey, string strValue);

		// 保存配置
		//void SaveConfig();

		inline int GetKeyCount() const {return m_mapResource.size();}
	private:
		// 加载配置文件
		//
		void LoadResource(const char* cResourceFile,int nBlockNum=0,char cSplitChar = '\t');

	private:
		string m_strResourceFile;
		map<string, vector<string> > m_mapResource;
};
#endif
