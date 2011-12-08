#ifndef CJPARSER_H
#define CJPARSER_H
#include <string>
#include <map>
#include <vector>
using namespace std;

class CJParser
{
	public:
		// ��ʼ��������
		static void InitParser(vector<string>& fields);

		// ����json
		static int ParseJson(string& strJson, map<string, string>& mapJson);

	private:
		static vector<string> m_vecFields;
};
#endif
