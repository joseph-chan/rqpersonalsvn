/**
 * @file BfdClassTree.cpp
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-11-02
 */

#include "bfd_classtree.h"

void BfdClassTree::BfdClassTree_Init(const char * pConfigFile)
{
	m_pConfigInfo = new CConfigInfo(pConfigFile);
	ClassRoot = new ClassNode;
	/**init class root */
	ClassRoot->id=0;
	ClassRoot->level=0;
	ClassRoot->father_id=-1;
	ClassRoot->foreign_id=0;
	ClassRoot->term_num=0;
	ClassRoot->train_data_num=0;
	ClassRoot->son_max_train_data_num=0;

	// 装载资源
	string strYCConfig = m_pConfigInfo->GetValue("bfd_class_tree");
	 LoadClassTree(strYCConfig.c_str());

	strYCConfig = m_pConfigInfo->GetValue("bfd_tags");
//	ret = LoadTags(strYCConfig.c_str());
}

void BfdClassTree::LoadClassTree(const char * class_tree)
{
	ifstream fin(class_tree);
	assert (fin.is_open());

	string line;
	vector<string> vstring;
	int line_num=0;
	int class_id,level,father_id,foreign_id;
	while(!fin.eof())
	{
		getline(fin,line);
		if(line.size() == 0 || line[0] == '#')
		{
			continue;
		}
		line_num++;
		vstring = CStringTool::SpliteByChar(line,'\t');

		if(vstring.size() != 5)
		{
			LOG(WARNING)<<"line format error [" << line.c_str() << "]" << "size"  << vstring.size();
			continue;
		}
		class_id = CTypeTool<int>::StrTo(vstring[0]);
		level = CTypeTool<int>::StrTo(vstring[1]);
		father_id = CTypeTool<int>::StrTo(vstring[2]);
		foreign_id = CTypeTool<int>::StrTo(vstring[4]);

		if(ClassNodeMap.find(class_id) != ClassNodeMap.end())
		{
			LOG(WARNING)<<"class id duplicate! [" <<line << "]";
			continue;
		}
		//if(father_id >0 && ClassNodeMap.find(father_id) == ClassNodeMap.end() )
		if( ClassNodeMap.find(father_id) == ClassNodeMap.end() )
		{
			LOG(WARNING)<<"father id not found ! " << class_id << " "<<father_id;
			continue;
		}

		ClassNameMap[class_id]=vstring[3];
		ClassNode *NodeBuf = new ClassNode;

		NodeBuf->id=class_id;
		NodeBuf->level=level;
		NodeBuf->father_id=father_id;
		NodeBuf->foreign_id=foreign_id;
		NodeBuf->term_num =0;
		NodeBuf->train_data_num =0;
		NodeBuf->leaf_num =0;
		NodeBuf->son_max_train_data_num =0;

		ClassNodeMap[class_id]=NodeBuf;

		//push back a son to father
		ClassNodeMap[father_id]->sons.push_back(class_id);

		++ TotalClassNum ;
	}
	fin.close();
	
	//get leaf num
	for(map<int,ClassNode*>::iterator  it = ClassNodeMap.begin();it != ClassNodeMap.end();++it)
	{
		if (it->second->sons.empty() == true)
		{
			//  leaf node
			int father=it->first;

			while (father > 0)
			{
				ClassNodeMap[father]->leaf_num ++;
				father = ClassNodeMap[father]->father_id;
			}
		}

	}

}

/*
int BfdClassTree::LoadTags(const char * tag_file)
{
	ifstream fin(tag_file);
	assert(fin.is_open());

	string line;
	vector<string> vstring;
	vector<string> vtags;
	int line_num=0;
	int class_id,level,tag_id;
	string key_str,tag_str;


	while(!fin.eof())
	{
		getline(fin,line);
		if(line.size() == 0)
		{
			continue;
		}
		line_num++;

		boost::algorithm::split(vstring,line,boost::is_any_of("\t"));

		if(vstring.size() < 5)
		{
			LOG(WARNING)<<"line format error [" << line.c_str() << "]";
			continue;
		}
		class_id = CTypeTool<int>::StrTo(vstring[0]);
		level = CTypeTool<int>::StrTo(vstring[1]);
		tag_id = CTypeTool<int>::StrTo(vstring[2]);
		key_str = vstring[3];
		tag_str = vstring[4];
		//boost::algorithm::split(vtags,vstring[2],boost::is_any_of(":"));
		if(ClassNodeMap.find(class_id) == ClassNodeMap.end())
		{
			LOG(WARNING)<<"line format error or id error [" << line.c_str() << "]";
			continue;
		}

		int tmp_fatherid;

			AddTagToClass(class_id,key_str,tag_str,tag_id);
			TagKeyMap[tag_id] = key_str;
			TagNameMap[tag_id] = tag_str;

			tmp_fatherid = (ClassNodeMap[class_id])->father_id;

			while (tmp_fatherid > 0)
			{
				AddTagToClass(tmp_fatherid,key_str,tag_str,tag_id);
				tmp_fatherid= (ClassNodeMap[tmp_fatherid])->father_id;
			}
			
		//}
	}

	for(map<int,ClassNode*>::iterator it = ClassNodeMap.begin(); it != ClassNodeMap.end(); ++it)
	{
		for(map<string, Aho_Corasick*>::iterator it2 = it->second->TagMapAC.begin(); it2 != it->second->TagMapAC.end(); ++it2)
		{
			it2->second->build_automata();
		}
	}
	return 0;

}
*/
