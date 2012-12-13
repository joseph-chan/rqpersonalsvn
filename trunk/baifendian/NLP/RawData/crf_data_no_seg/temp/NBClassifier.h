/**
 * @file NBClassifier.h
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-05-11
 */

#ifndef NBCLASSIFIER_H
#define NBCLASSIFIER_H

//system lib
#include <map>
#include <set>
#include <vector>
#include <math.h>
#include <fstream>
#include <iostream>
//#include <ext/hash_fun.h>
#include <ext/hash_map>

//local lib
#include "CJParser.h"
#include "CStringTool.h"
#include "CTypeTool.h"
#include "CConfigInfo.h"
#include "CResource.h"
#include "ac.h"
#include "KNNClassifier.h"
#include "string_utils.h"

//third party
//#include <log4cxx/logger.h>
//#include <log4cxx/propertyconfigurator.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>

#include <glog/logging.h>
#include "leveldb/db.h"
//#include <Token.h>
//#include "Tokenizar.h"
#include "lookup.h"
#include "BfdSegment.h"
#include "json/json.h"
#include "json/reader.h"

using namespace std;
//using namespace log4cxx;

#ifdef assert
   #undef assert
#endif

#define Mstr2(m) # m
#define Mstr(m) Mstr2(m)
#define assert WINASSERT
#define WINASSERT(n_) \
{ \
   if (!(n_)) \
   { \
      LOG(FATAL) << "assertion failed:" <<  Mstr(n_); \
   } \
}

#define MAX_TERM_LENGTH 80
#define TOTAL_FIELD_NUM 23

// format in text
/*
#define PDATA_FIELD_CID 0
#define PDATA_FIELD_IID 1
#define PDATA_FIELD_NAME 3
#define PDATA_FIELD_CATE 8
#define PDATA_FIELD_DESC 18
#define PDATA_FIELD_TEST_CATE 24
*/

/* format from protobuf 
*/
#define PDATA_FIELD_CID 19
#define PDATA_FIELD_IID 0
#define PDATA_FIELD_NAME 2
#define PDATA_FIELD_CATE 6
#define PDATA_FIELD_DESC 14
#define PDATA_FIELD_ATTR 15
#define PDATA_FIELD_TEST_CATE 24
/*
#define PDATA_FIELD_BID 0
#define PDATA_FIELD_ID 1
#define PDATA_FIELD_TITLE 2
#define PDATA_FIELD_CATE 3
#define PDATA_FIELD_DESC 4
*/
//#define js_data_merge(str) (str[item_base_field_name]+str[item_base_field_url]+str[item_base_field_pid]+str[item_base_field_price])
#define js_data_merge(str) (str[2]+str[3]+str[6]+str[7]+str[17])
#define crawl_data_merge(str) (str[4]+str[5]+str[8]+str[9]+str[10]+str[11]+str[12]+str[13]+str[14]+str[15]+str[16]+str[18]+str[19]+str[20]+str[21]+str[22] )
#define unknown_data_merge(str) (str[item_base_field_name]+str[item_base_field_pid]+str[item_base_field_description]+str[item_base_field_attr])
enum item_base_field
{
	item_base_field_iid =0,
	item_base_field_deleted ,
	item_base_field_name ,
	item_base_field_url ,
	item_base_field_large_img ,
	item_base_field_small_img ,
	item_base_field_pid ,
	item_base_field_price ,
	item_base_field_market_price ,
	item_base_field_member_price ,
	item_base_field_vip_price ,
	item_base_field_discount ,
	item_base_field_start_time ,
	item_base_field_end_time ,
	item_base_field_description ,
	item_base_field_attr ,
	item_base_field_creation_time ,
	item_base_field_brand ,
	item_base_field_group ,
	item_base_field_cid ,
	item_base_field_location ,
	item_base_field_district ,
	item_base_field_tag ,
	item_base_field_type
};
enum item_profile_field
{
	iid = 0 ,
	category ,
	tag ,
	brand ,
	price_level ,
	discount ,
	num_of_people ,
	city ,
	district ,
	appraisal ,
	appraisal_url ,
	creation_time ,
	cid 
};

class PQComparison
{
	bool reverse;
	public:
	PQComparison(const bool& revparam=false)
	{reverse=revparam;}
	bool operator() (pair<int,float>& lhs, pair<int,float> & rhs) const
	{
		if (reverse) return (lhs.second>rhs.second);
		else return (lhs.second<rhs.second);
	}
};

class TagInfo
{
	public:
		vector<string> TagKey;
		vector<int>  Tagid;
};

class TermInfo
{
	public:
		int idf;
		int df;
		int TotalClassItemDf;
		char termstr[MAX_TERM_LENGTH];
		map<int,int> ClassDf;
		map<int,int> ClassItemDf;
};

class ClassNode
{
	public:
		int level;
		int id;
		int father_id;
		int foreign_id;
		int term_num;
		int train_data_num;
		/** leaf num of this node. which <= son num. need this num to determine how many train data num does this node required.  
		 * -- obsoleted 201200823 
		 *  because:
		 *  at the beginning, I think every class of a same father in the same level should have same  train data num , otherwise the one have more train data num will have high priority. if the train data num exceeds the proper_train_data_num ,then the rest data will be deleted.
		 *  though ,this is not a good idea
		 *  now, we take a prior parameter, every class will multiply a parameter : max_train_data/self_train_data_num, means  father->son_max_train_data/train_data_num  */
		int leaf_num;
		//int proper_train_data_num;

		int son_max_train_data_num;
		//int son_min_train_data;
		vector<int> sons;

		map<int,bool> hash_tags;
		//the map of tag_key name and the ac machine of tags 
		map<string,Aho_Corasick*> TagMapAC;
		Aho_Corasick PerceptualTagAC;
};

class NBClassifier
{
	public:

		NBClassifier();
		~NBClassifier();

	public:

		/**
		 * @brief init config
		 *
		 * @Param pConfigFile
		 *
		 * @Returns   
		 */
		int NBClassifierInit(const char * pConfigFile);
		//int GetTopTerms();
		//int GetTermFrmOriginalLine(string strLine);
		//int SortTerms();

		int GetNBModel();
		int GetNBFrmOriginalLine(string strLine);
		int AddWord(const map<string, int> & all_terms,const vector<int> & class_tree);
		int GetChiSquare(int N,int N11,int N10,int N01,int N00);
		float GetChiSquare(float N,float N11,float N10,float N01,float N00);
			
		int DumpNBModel();
		int DumpClassFeatureTerm();
		int LoadNBModel();

		int NBPredict();
		int NBPredictLine(const string strLine,ofstream &fout_predict);
		int NBPredictLineRecurse(string strLine,ofstream &fout_predict);

		int ExtractTag();
		int ExtractTagFromLine(string strLine,ofstream & fout);
		//int GetTopNClass()
		//int RecordTerm(char * term,WORD_ITEM *seg_info );

	private:

		int LoadClassTree(const char* cClassTreeFile);
		int LoadTags(const char* cClassTagFile);
		int LoadCommonTags(const char  *pConfigFile);
		int LoadPerceptualTags(const char  *pConfigFile);
		void TestClassTree();
		void CalcTermIdf();
		int AddTagToClass(int class_id, string & tag_key, string & tag_name, int tag_id);
		int AddTagToAll(string & tag_key, string & tag_name, int tag_id);
		string GetClassNameTree(int class_id);
		string GetClassIdTree(int class_id);
		string GetClassFIdTree(int class_id);
		int GetTopNClass(map<string,int> & terms, ClassNode* BeginNode, float current_score, int topN);
		void SetMaxTrainDataNum(vector<int> & ids);

		int GetAncestorID(int class_id);
		bool GetDataFrmCache(const string line,int source_type,string &outputline);
		void SetDataCache(const string line,int data_source, const string profile);

		void Clear();
		inline  void js_data_update(const vector<string> x,vector<string> &y) 
		{y[item_base_field_name]=x[item_base_field_name];y[item_base_field_url]=x[item_base_field_url];y[item_base_field_pid]=x[item_base_field_pid];y[item_base_field_price]=x[item_base_field_price];y[item_base_field_brand]=x[item_base_field_brand];};
		inline  void crawl_data_update(vector<string> x,vector<string> &y) 
		{x[item_base_field_name]=y[item_base_field_name];x[item_base_field_url]=y[item_base_field_url];x[item_base_field_pid]=y[item_base_field_pid];x[item_base_field_price]=y[item_base_field_price];x[item_base_field_brand]=y[item_base_field_brand];y=x;};

/**************************数据成员***************************************/
	private:
		int m_field_need_to_predict;
		int m_need_content_in_model;
		int m_need_tag_in_model;
		int m_output_debug;
		int m_need_miniKNN;
		int m_need_test;
		int m_need_perceptual_tag;
		int m_data_source_type;

		ifstream m_test_file_in;
		ofstream m_test_out;

		CConfigInfo* m_pConfigInfo;

		KNNClassifier * miniKNN;
		int m_knn_match_count ;
		// m_train_data_num_per_first_class
		int m_train_data_num_per_class ;

		ClassNode * ClassRoot;

		//baifendian_tokenizar::Tokenizar* mTokenizar;
		BfdSegment * mpSegmentor;

		//the buffer to store predict output
		pair<int,float> PredictOut;

		//the map of Class id and Class name
		map<int,string> ClassNameMap;

		//tag name map,first int  is class_id << 32 + tag_id 
		map<int,string> TagNameMap;
		map<int,string> TagKeyMap;

		//the map of class id and the pointer of class node 
		map<int,ClassNode*> ClassNodeMap;

		//the map of class id and the ac machine of tags 
		//map<int,map<string,Aho_Corasick*> > ClassTagMap;

		__gnu_cxx::hash_map<uint32_t, uint32_t> TermHashMap;
		//first: bfd class id ,second: foreign id
		__gnu_cxx::hash_map<uint32_t, uint32_t> ClassIdMap;
		// first: foreign id , second: bfd class id
		__gnu_cxx::hash_map<uint32_t, uint32_t> ClassIdMapReverse;

		vector<TermInfo> mNBTerms;
		vector<string> testxxxx;
		map<int,map<int,string> > mPerceptualTag;
		int TotalClassNum;
		long long TotalTermNum;
		int TotalLearningItemNum;
		int TotalTermHashNum;
		int CorrectNumInTest[4];
		int TotalNumInTest;

		//first : tag_key, second: tag_id AC
		map<string,Aho_Corasick*> CommonTagMapAC;

		Json::Reader mJsonReader;
		Json::Value mJsonValue;

		leveldb::DB* mLeveldb; 
		leveldb::Options mLeveldbOptions;
		leveldb::Status mLeveldbStatus;
};
#endif
