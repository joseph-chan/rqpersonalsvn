#ifndef CJPARSER_H
#define CJPARSER_H
#include <string>
#include <map>
#include <vector>
using namespace std;

class CJParser
{
	public:
		// 初始化解析器
		static void InitParser(vector<string>& fields);

		// 解析json
		static int ParseJson(string& strJson, map<string, string>& mapJson);

	private:
		static vector<string> m_vecFields;
};
#endif
