/**
 * @file BfdClassTree.h
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-11-02
 */
#ifndef BFDCLASSTREE_H
#define BFDCLASSTREE_H

#include <map>
#include <set>
#include <vector>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>

#include <glog/logging.h>
#include "CConfigInfo.h"
#include "CStringTool.h"
#include "CTypeTool.h"

using std::string;
using std::vector;
using std::map;

class Aho_Corasick;


class ClassNode
{
	public:
		int level;
		int id;
		int father_id;
		int foreign_id;
		int term_num;
		int leaf_num;
		string class_name;
		vector<int> sons;
		map<int,bool> hash_tags;
		//the map of tag_key name and the ac machine of tags 
		map<string,Aho_Corasick*> TagMapAC;
};

class BfdClassTree
{
	public:
		void BfdClassTree_Init(const char *pConfigFile);
		void LoadClassTree(const char* class_tree);
		int LoadTags(const char* tag_file);

		int AddTagToClass(int class_id, string & tag_key, string & tag_name, int tag_id);
		string GetClassNameTree(int class_id);
		string GetClassIdTree(int class_id);
		string GetClassFIdTree(int class_id);
		int GetTopNClass(map<string,int> & terms, ClassNode* BeginNode, float current_score, int topN);

		int GetAncestorID(int class_id);
		inline ClassNode* GetClassNode(int class_id) {if (ClassNodeMap.find(class_id)!=ClassNodeMap.end()) return ClassNodeMap[class_id]; else return NULL;};
		inline map<int,string>& GetTagNameMap() {return TagNameMap;};
	private:

		ClassNode * ClassRoot;
		CConfigInfo* m_pConfigInfo;
		map<int,string> ClassNameMap;
		map<int,ClassNode*> ClassNodeMap;
		int TotalClassNum ;

		//tag name map,first int  is class_id << 32 + tag_id 
		map<int,string> TagNameMap;
		map<int,string> TagKeyMap;
};
#endif
