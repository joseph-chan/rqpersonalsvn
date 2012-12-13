/**
 * @file NBClassifier.cpp
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 1.0.0
 * @date 2012-04-17
 */

#include "NBClassifier.h"
using namespace std;


bool CmpPairSecond(pair<int,int> term1, pair<int,int> term2)
{   
    return term1.second > term2.second;
}  
bool CmpPair(pair<string,float> term1, pair<string,float> term2)
{   
    return term1.second > term2.second;
}  

/**
 * @brief Constructor
 */
NBClassifier::NBClassifier()
{
	m_pConfigInfo= NULL;
}

/**
 * @brief Destructor
 */
NBClassifier::~NBClassifier()
{
	Clear();
}

/**
 * @brief extract tags from classified data
 *
 * @Returns   
 */
int NBClassifier::ExtractTag()
{
	int ret=0;
	const char * szData;
	const char * szOutput;

	szData=m_pConfigInfo->GetValue("extract_tag_data").c_str();
	szOutput=m_pConfigInfo->GetValue("extract_tag_output").c_str();

	ifstream fin(szData);
	ofstream fout(szOutput);
	if(!fin.is_open() ||  !fout.is_open())
	{
		LOG(ERROR)<<"can not open training data file";
		return -1;
	}

	string line;
	int line_num=0;
	while(!fin.eof())
	{
		++line_num;
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = ExtractTagFromLine(line,fout);
		if(ret < 0 )
		{
			LOG(ERROR)<<"original data line parse fail";
		}
		LOG(INFO)<<"original data line [" <<line_num << "] parsed over ";
	}

	fin.close();
	return 0;

}

int NBClassifier::ExtractTagFromLine(string strLine, ofstream & fout)
{
	vector<string> strContent = CStringTool::SpliteByChar(strLine,'\t'); 
	if(strContent.size() < 5 )
	{
			LOG(INFO)<<"parse line error, field < 5 :" << strLine;
			return -1;
	}
	// the 4th column is item type
	int foreign_id = CTypeTool<int>::StrTo(strContent[3]); 
	string target_str;

	for(int i =4; i< strContent.size(); i++)
	{
		target_str += strContent[i];
	}

	__gnu_cxx::hash_map<uint32_t, uint32_t>::iterator itHM;
	itHM= ClassIdMapReverse.find(foreign_id);


	fout<< strContent[0] << "\t" << strContent[1] << "\t" << strContent[2] << "\t" << strContent[3] << "\t" ;

	if (itHM == ClassIdMapReverse.end() || ClassNodeMap.find(ClassIdMapReverse[foreign_id]) == ClassNodeMap.end())
	{
		fout << endl;
		LOG(ERROR)<<"can not find this class id" << strContent[0] << " " << strContent[1] << " " << strContent[3];
		return 1;
	}

	std::set<int> tmp_st;
	for(map<string,Aho_Corasick*>::iterator it = ClassNodeMap[itHM->second]->TagMapAC.begin(); it != ClassNodeMap[itHM->second]->TagMapAC.end();++it)
	{
		tmp_st = it->second->get_match(target_str.c_str());
		if(tmp_st.size()> 0)
		{
			fout << it->first;
			for(set<int>::iterator it2 = tmp_st.begin(); it2 != tmp_st.end(); ++it2)
			{
				fout << ":"<< TagNameMap[*it2];
			}
			fout << ";";
		}
	}
	fout << endl;
	
	return 0;
}

int NBClassifier::GetNBModel()
{
	int ret=0;
	const char * szTrainingData;

	szTrainingData=m_pConfigInfo->GetValue("training_data").c_str();
	ifstream fin(szTrainingData);
	if(!fin.is_open() )
	{
		LOG(ERROR)<<"can not open training data file";
		return -1;
	}

	m_need_content_in_model = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("if_need_content_in_model"));
	m_need_content_in_model = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("if_need_tag_in_model"));


	string line;
	int line_num=0;
	while(!fin.eof())
	{
		++line_num;
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = GetNBFrmOriginalLine(line);
		if(ret < 0 )
		{
			LOG(WARNING)<<"original data line parse fail";
		}
		LOG(INFO)<<"original data line [" <<line_num << "] parsed over ";
	}

	CalcTermIdf();
	if (mNBTerms.size() != TotalTermHashNum)
	{
		LOG(ERROR)<<"term hash number may error : " << mNBTerms.size() << " " << TotalTermHashNum;

	}

	vector<int> class_ids (ClassRoot->sons);
	SetMaxTrainDataNum(class_ids);

	fin.close();
	return 0;

}
void NBClassifier::SetMaxTrainDataNum(vector<int> & ids)
{
	int father_id=0;
	int max_train_data_num=0;
	father_id = ClassNodeMap[ids[0]]->father_id;

	for(vector<int>::iterator it= ids.begin(); it != ids.end(); ++it )
	{
		if ( ClassNodeMap[*it]->father_id != father_id)
		{
			LOG(FATAL) << "father id not equal " << father_id ;
			exit(-1);
		}
		if (ClassNodeMap[*it]->train_data_num > max_train_data_num)
		{
			max_train_data_num= ClassNodeMap[*it]->train_data_num;
			DLOG(INFO)<< "class id :" << *it << " have max_train_data_num  " << max_train_data_num;
		}
	}
	if (max_train_data_num<=0)
	{
		LOG(WARNING) << "max_train_data_num may error " << max_train_data_num ;
	}
	ClassNodeMap[father_id]->son_max_train_data_num = max_train_data_num;
	DLOG(INFO)<< "class id :" << father_id << " max_train_data_num  " << max_train_data_num;

	/** recurse*/
	for(vector<int>::iterator it= ids.begin(); it != ids.end(); ++it )
	{
		if (ClassNodeMap[*it]->sons.empty() ==true )
		{
			continue;
		}
		else
		{
			vector<int> class_ids;
			class_ids = ClassNodeMap[*it]->sons;
			SetMaxTrainDataNum(class_ids);
		}
	}
	return ;
}

void NBClassifier::CalcTermIdf()
{
	for (vector<TermInfo>::iterator it = mNBTerms.begin(); it!= mNBTerms.end();++it)
	{
		(*it).idf = int( 1.0+ log2(double(TotalTermNum)/double((*it).df) ));
	}
}

/*
int NBClassifier::GetTopTerms()
{
	int ret=0;
	const char * szInputFile;
	const char * szOutputFile;
	// 初始化山寨json解析器
	vector<string> json_fields;
	json_fields.push_back("flag");
	json_fields.push_back("id");
	json_fields.push_back("uid");
	json_fields.push_back("filter");
	json_fields.push_back("content");
	json_fields.push_back("yc_source");
	CJParser::InitParser(json_fields);

	szInputFile=m_pConfigInfo->GetValue("original_data").c_str();
	ifstream fin(szInputFile);
	if(!fin.is_open() )
	{
		LOG(ERR,"can not open input file");
		return -1;
	}
	string line;
	int line_num=0;
	while(!fin.eof())
	{
		++line_num;
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = GetTermFrmOriginalLine(line);
		if(ret < 0 )
		{
			LOG(ERR,"original data line parse fail");
		}
		LOG(INFO)<<"original data line [%d] parsed over",line_num);
	}
	fin.close();
	return 0;

}
*/
/**
 * @brief 初始化实例
 *
 * @Param cResource
 *
 * @Returns   
 */
int NBClassifier::NBClassifierInit(const char * pConfigFile)
{
	int ret =0 ;

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

	m_field_need_to_predict = 0;
	m_output_debug =0;
	m_need_miniKNN =0;
	m_need_test=0;
	m_need_perceptual_tag =1;

	memset(CorrectNumInTest,0,sizeof(CorrectNumInTest));

	TotalNumInTest=0;

	miniKNN = new KNNClassifier;

	m_need_content_in_model = 0;
	m_need_tag_in_model = 0;

	m_knn_match_count=0;
	m_train_data_num_per_class =0;

	ClassNodeMap[0]=ClassRoot;
	ClassNameMap[0]="Root";



	TotalClassNum=0;
	TotalTermNum=0;
	TotalTermHashNum=0;
	TotalLearningItemNum=0;
	m_need_test = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("need_test"));
	if (m_need_test == 1)
	{
		string test_file;
		test_file=m_pConfigInfo->GetValue("test_input").c_str();
		m_test_file_in.open(test_file.c_str(),ifstream::in);

		test_file=m_pConfigInfo->GetValue("test_output").c_str();
		m_test_out.open(test_file.c_str());
		if (m_test_file_in.is_open() == false || m_test_out.is_open()== false)
		{
			LOG(ERROR)<<"init test file error";
			return -1;
		}

	}

	m_need_perceptual_tag = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("need_perceptual_tag"));


	 m_train_data_num_per_class = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("train_data_num_per_class"));

	 if (m_train_data_num_per_class <= 0)
	 {
		m_train_data_num_per_class = 10000;
	 }
	 m_output_debug = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("output_debug"));
	 if (m_output_debug !=0 && m_output_debug !=1 )
	 {
		m_output_debug=0;
	 }
	 

	// 装载资源
	string strYCConfig = m_pConfigInfo->GetValue("bfd_class_tree");
	if(strYCConfig.size() == 0)
	{
		LOG(ERROR)<<"load resource error";
		return -1;
	}
	ret = LoadClassTree(strYCConfig.c_str());
	if (ret <0)
	{
		LOG(ERROR)<<"load class tree error";
		return -1;
	}

	strYCConfig = m_pConfigInfo->GetValue("bfd_tags");
	if(strYCConfig.size() == 0)
	{
		LOG(ERROR)<<"load resource error";
		return -1;
	}
	ret = LoadTags(strYCConfig.c_str());
	if (ret <0)
	{
		LOG(ERROR)<<"load tags error";
		return -1;
	}
	
	//20120719 add a new common tag file, search common tag in any type
	strYCConfig = m_pConfigInfo->GetValue("common_tags");
	ret = LoadCommonTags(strYCConfig.c_str());
	if (ret <0)
	{
		LOG(ERROR)<<"load common tags error";
		return -1;
	}

	//20120725 add a new perceptual_tags file, perceptual_tags is just a show
	strYCConfig = m_pConfigInfo->GetValue("perceptual_tags");
	ret = LoadPerceptualTags(strYCConfig.c_str());
	if (ret <0)
	{
		LOG(ERROR)<<"load perceptual tags error";
		return -1;
	}


	// Tokenizar
	// deleted , use new word segment lib
	/*
	mTokenizar = new baifendian_tokenizar::Tokenizar(m_pConfigInfo->GetValue("ICTCLAS_conf").c_str());

	if(mTokenizar == NULL)
	{
		LOG(ERROR)<<"init error");
		return -1;
	}
	*/
	mpSegmentor = new BfdSegment;
	mpSegmentor->BfdSegment_Init(m_pConfigInfo);
	
	m_need_miniKNN = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("need_miniKNN"));

	if (m_need_miniKNN == 1)
	{
		ret= miniKNN->KNNClassifierInit(pConfigFile,mpSegmentor);
		if (ret <0)
		{
			LOG(ERROR)<<"init miniKNN error";
			return -1;
		}
	}

	/** init level db*/
	
	mLeveldbOptions.create_if_missing = true;
	strYCConfig=m_pConfigInfo->GetValue("leveldb_path");
	mLeveldbStatus = leveldb::DB::Open(mLeveldbOptions, strYCConfig, &mLeveldb);
	assert(mLeveldbStatus.ok());

	/** data source type:
	 * 1: js data
	 * 2: crawl data
	 * 3: not official data*/
	m_data_source_type = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("data_source_type"));
	if (m_data_source_type>3 || m_data_source_type <1)
	{
		m_data_source_type=3;
	}
	//TestClassTree();
	return 0;

}

int NBClassifier::LoadPerceptualTags(const char  *pConfigFile)
{
	ifstream fin(pConfigFile);
	if( !fin.is_open())
	{
		LOG(ERROR)<<"load class tree error";
		return -1;
	}
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

		if(vstring.size() < 4)
		{
			LOG(WARNING) << "line format error [" << line.c_str() << "]";
			continue;
		}
		class_id = CTypeTool<int>::StrTo(vstring[0]);
		tag_id = CTypeTool<int>::StrTo(vstring[1]);
		key_str = vstring[2];
		tag_str = vstring[3];

		LOG(INFO)<<"key_str " << key_str << "tag_str" << tag_str ;
		
		if (ClassNodeMap.find(class_id) == ClassNodeMap.end())
		{
			LOG(WARNING)<<"classid error : no such class id[" << line << "]";
			continue;
		}
		
		// add tag to class AC
		ClassNodeMap[class_id]->PerceptualTagAC.insert(tag_str.c_str(),tag_id);

		mPerceptualTag[class_id][tag_id] = key_str;
	}

	for(map<int,ClassNode*>::iterator it2 = ClassNodeMap.begin(); it2 != ClassNodeMap.end(); ++it2)
	{
		it2->second->PerceptualTagAC.build_automata();
	}
	return 0;

}


int NBClassifier::LoadCommonTags(const char  *pConfigFile)
{
	ifstream fin(pConfigFile);
	if( !fin.is_open())
	{
		LOG(ERROR)<<"load class tree error";
		return -1;
	}
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

		DLOG(WARNING)<<"key_str " << key_str << "tag_str" << tag_str ;
		AddTagToAll(key_str,tag_str,tag_id);
		TagKeyMap[tag_id] = key_str;
		TagNameMap[tag_id] = tag_str;
	}

	for(map<string, Aho_Corasick*>::iterator it2 = CommonTagMapAC.begin(); it2 != CommonTagMapAC.end(); ++it2)
	{
		it2->second->build_automata();
	}
	return 0;

}

int NBClassifier::LoadTags(const char  *pConfigFile)
{
	ifstream fin(pConfigFile);
	if( !fin.is_open())
	{
		LOG(ERROR)<<"load class tree error";
		return -1;
	}
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
		//for(vector<string>::iterator it = vtags.begin(); it != vtags.end(); ++it)
		//{
		//	if(it == vtags.begin())
		//	{
		//		key_str = *it;
		//		continue;
		//	}

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

/*
int NBClassifier::AddTagToClass(int class_id, string & tag_key, string & tag_name, int tag_id)
{
	int tag_name_hash , pair_hash;
	string buf;

	// cat key and name as a string, check if we have this tag
	buf = tag_key + tag_name;
	pair_hash = hashlittle((buf).c_str(),(buf).size(),1);

	if(ClassNodeMap[class_id]->hash_tags.find(pair_hash) != ClassNodeMap[class_id]->hash_tags.end())
	{
		DLOG(WARNING)<<"already have this tag " << tag_key << " " << tag_name);
		return 1;
	}

	if(ClassNodeMap[class_id]->TagMapAC.find(tag_key) == ClassNodeMap[class_id]->TagMapAC.end())
	{
		ClassNodeMap[class_id]->TagMapAC[tag_key] = new Aho_Corasick;

	}

	tag_name_hash = hashlittle((tag_name).c_str(),(tag_name).size(),1);
	TagNameMap[tag_name_hash] = tag_name;


	DLOG(WARNING)<<"tag_key " << tag_key << " class_id " << class_id);
	ClassNodeMap[class_id]->TagMapAC[tag_key]->insert((tag_name).c_str(),tag_name_hash);
	return 0;
}
*/

int NBClassifier::AddTagToClass(int class_id, string & tag_key, string & tag_name, int tag_id)
{

	int64_t tag_full_id;

	if(ClassNodeMap[class_id]->hash_tags.find(tag_id) != ClassNodeMap[class_id]->hash_tags.end())
	{
		DLOG(WARNING)<<"already have this tag " << tag_key << " " << tag_name;
		return 1;
	}

	if(ClassNodeMap[class_id]->TagMapAC.find(tag_key) == ClassNodeMap[class_id]->TagMapAC.end())
	{
		ClassNodeMap[class_id]->TagMapAC[tag_key] = new Aho_Corasick;

	}

	//tag_full_id = (int64_t(class_id) << 32) + tag_id;
	//TagNameMap[tag_full_id] = tag_name;


	DLOG(WARNING)<<"tag_key " << tag_key << " class_id " << class_id;
	ClassNodeMap[class_id]->TagMapAC[tag_key]->insert((tag_name).c_str(),tag_id);
	return 0;
}

int NBClassifier::AddTagToAll(string & tag_key, string & tag_name, int tag_id)
{

	int64_t tag_full_id;

	if(CommonTagMapAC.find(tag_key) == CommonTagMapAC.end())
	{
		CommonTagMapAC[tag_key] = new Aho_Corasick;
	}

	//tag_full_id = (int64_t) tag_id;
	//TagNameMap[tag_full_id] = tag_name;


	DLOG(WARNING)<<"tag_key " << tag_key << " tag_name" << tag_name <<" is common tag "  ;
	CommonTagMapAC[tag_key]->insert((tag_name).c_str(),tag_id);
	return 0;
}



int NBClassifier::GetAncestorID(int class_id)
{
	if (class_id <=0)
	{
		return 0;
	}
	int ancestor_id ;
	while( class_id> 0)
	{
		ancestor_id = class_id;
		class_id = ClassNodeMap[class_id]->father_id;
	}
	return ancestor_id;
}

int NBClassifier::LoadClassTree(const char  *pConfigFile)
{
	ifstream fin(pConfigFile);
	if( !fin.is_open())
	{
		LOG(ERROR)<<"load class tree error";
		return -1;
	}
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
		ClassIdMap[class_id]=foreign_id;
		ClassIdMapReverse[foreign_id]=class_id;
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

	//get proper_train_data_num
	//deleted . it's not right to give every leaf the same train data num.  if there are three class in a level, one have 3 leaf, the other two just have 1 leaf, then the first class will have more train data, and have more probability .
	/*
	for(map<int,ClassNode*>::iterator  it = ClassNodeMap.begin();it != ClassNodeMap.end();++it)
	{
		int train_data_num_per_leaf=0;
		int ancestor_id =0;

		//every leaf have same train data
		ancestor_id = GetAncestorID(it->first);
		if (ClassNodeMap[ancestor_id]->leaf_num ==0)
		{
			LOG(ERROR)<<"ancestor leaf num may error" );
			train_data_num_per_leaf =1;
		}
		else
		{
			train_data_num_per_leaf = (1 + ClassNodeMap[ancestor_id]->proper_train_data_num / ClassNodeMap[ancestor_id]->leaf_num );
		}
		it->second->proper_train_data_num =  it->second->leaf_num * train_data_num_per_leaf;

		DLOG(WARNING)<<"class id [" << it->first << "] leaf num [" << it->second->leaf_num << "] proper_train_data_num [" << it->second->proper_train_data_num << "] per leaf [" << train_data_num_per_leaf << "]" );

	}
	*/

	/** new policy 20120730: every class in the same level have same train data num --obsolete 20120823
	 *  because:
	 *  at the beginning, I think every class of a same father in the same level should have same  train data num , otherwise the one have more train data num will have high priority. if the train data num exceeds the proper_train_data_num ,then the rest data will be deleted.
	 *  though ,this is not a good idea
	 *  now, we take a prior parameter, every class will multiply a parameter : max_train_data/self_train_data_num, means  father->son_max_train_data/train_data_num  */
	//begin
	//vector<int> class_ids (ClassRoot->sons);
	//SetPropTrainDataNum(class_ids);
	//end

	return 0;

}

/*
void NBClassifier::SetPropTrainDataNum(vector<int> & ids)
{
	
	for(vector<int>::iterator it= ids.begin(); it != ids.end(); ++it )
	{
		if (ClassNodeMap[*it]->level == 1)
		{
			ClassNodeMap[*it]->proper_train_data_num = m_train_data_num_per_class;
		}
		else
		{
			int father_id;
			int train_data_num;
			father_id = ClassNodeMap[*it]->father_id;
			train_data_num = 1+  (ClassNodeMap[father_id]->proper_train_data_num / ClassNodeMap[father_id]->sons.size() );
			ClassNodeMap[*it]->proper_train_data_num = train_data_num;
			DLOG(INFO)<< "class id :" << *it << " train data num " << train_data_num;
		}

		if (ClassNodeMap[*it]->sons.empty() ==true )
		{
			continue;
		}
		else
		{
			vector<int> class_ids;
			class_ids = ClassNodeMap[*it]->sons;
			SetPropTrainDataNum(class_ids);
		}

	}
	return ;
}
*/

void NBClassifier::TestClassTree()
{
	map<int,ClassNode*>::iterator it;
	vector<int>::const_iterator it2;
	for(it = ClassNodeMap.begin();it != ClassNodeMap.end(); ++it)
	{
		cout << it->second->id << "\t" << it->second->level << "\t" << it->second->father_id << "\t" << ClassNameMap[it->second->id] << endl ;
		for(it2=it->second->sons.begin(); it2!= it->second->sons.end();++it2)
		{
			cout << *it2<<":"<<ClassNameMap[*it2]<< "\t";
		}
		cout << endl;
	}

	/*
	string seg_out="";
	mTokenizar->wordBreaker("百分点信息科技有限公司", seg_out);
	cout << seg_out << endl;
	LOG(INFO)<< "segment out :" << seg_out);
	mTokenizar->wordBreaker("you know what", seg_out);
	cout << seg_out << endl;
	LOG(INFO)<< "segment out :" << seg_out);
	*/

}

/**
 * @brief 处理原始json数据
 *
 * @Returns   >=0 正常
 *            <0  错误
 */
/*
int NBClassifier::ProcessOriginalData()
{
	const char * szInputFile;
	const char * szOutputFile;
	// 初始化山寨json解析器
	vector<string> json_fields;
	json_fields.push_back("flag");
	json_fields.push_back("id");
	json_fields.push_back("uid");
	json_fields.push_back("filter");
	json_fields.push_back("content");
	json_fields.push_back("yc_source");
	CJParser::InitParser(json_fields);

	szInputFile=m_pConfigInfo->GetValue("original_data").c_str();
	szOutputFile=m_pConfigInfo->GetValue("output_seg_data").c_str();
	ifstream fin(szInputFile);
	ofstream fout(szOutputFile);
	if(!fin.is_open() || !fout.is_open())
	{
		LOG(ERROR)<<"can not open input file");
		return -1;
	}
	string line;
	int ret;
	int line_num=0;
	while(!fin.eof())
	{
		++line_num;
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = ParseLineOriginal(line,fout);
		if(ret < 0 )
		{
			LOG(ERROR)<<"original data line parse fail");
		}
		LOG(INFO)<<"original data line [%d] parsed over",line_num);
	}
	fin.close();
	return 0;

}
*/

/*
int NBClassifier::RecordTerm(char * term,WORD_ITEM *seg_info )
{
	if (term == NULL)
	{
		return -1;
	}
	u_int hash_value=0;
	int term_index=0;
	int is_good_term=0;
	term_info_t *local_term_info = new term_info_t;
	__gnu_cxx::hash_map<u_int,u_int>::iterator iterhm;


	if (IsGoodTerm(seg_info->postagid) == true)
	{
		is_good_term = 1;
		LOG(INFO)<<"term [%s] is bad ,postagid [%d] ",term,seg_info->postagid);
	}

	hash_value=__gnu_cxx::__stl_hash_string(term);
	iterhm = mTermIndex.find(hash_value);

	// have never record this term,brand new
	if (iterhm == mTermIndex.end())
	{
		//record global info of this term
		snprintf(local_term_info->termstr,MAX_TERM_LENGTH,"%s",term);
		local_term_info->df_now =is_good_term;
		local_term_info->relate_term_num = 0;

		mTermIndex[hash_value] = m_total_info.term_num;

		// term count ++
		mTermInfo.push_back(*local_term_info);
		m_total_info.term_num ++;
		if (mTermInfo.size() != m_total_info.term_num)
		{
			LOG(ALERT,"mTermInfo size may be error size [%d] m_term_num [%d]",mTermInfo.size(), m_total_info.term_num);
			return -1;
		}
		DLOG(INFO)<<"new term added: term [%s] ",term);
	}
	else
	{
		term_index=iterhm->second;
		mTermInfo[term_index].df_now += is_good_term ;
		DLOG(INFO)<<"term update: term [%s] df_now [%d] df_his [%d] relate_term_num [%d]",mTermInfo[term_index].termstr,mTermInfo[term_index].df_now,mTermInfo[term_index].df_history,mTermInfo[term_index].relate_term_num);
	}
	delete local_term_info;
	return 0;

}
int NBClassifier::SortTerms()
{
	vector<pair<string,u_int> > terms;
	const char * szOuputFile;
	map<string,u_int>::iterator iterTerms;
	int weight;
	for(iterTerms=mTopTerms.begin();iterTerms != mTopTerms.end();iterTerms ++)
	{
		weight = iterTerms->second * (iterTerms->first).size();
		terms.push_back(pair<string,u_int>(iterTerms->first,weight));
	}
	sort(terms.begin(),terms.end(),CmpTerm);

	szOuputFile=m_pConfigInfo->GetValue("top_terms").c_str();
	ofstream fout(szOuputFile);

	vector<pair<string,u_int> >::iterator vIterTerms;
	for(vIterTerms= terms.begin();vIterTerms!=terms.end();vIterTerms++)
	{
		fout<< vIterTerms->first << "\t" << vIterTerms->second <<endl;
	}

}
*/
int NBClassifier::NBPredict()
{
	int ret=0;
	const char * szInputPredict;
	const char * szOutputPredict;
	int predict_type=1;

	// get predict type. 1 for greedy ,2 for recurse.
	// greedy has faster speed but lower accuracy
	// recurse has lower speed but higher accuracy
	predict_type= CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("predict_type"));

	// field needed to classify. 1 means use the first field,  2: use 1st and 2nd field . other : use all.
	m_field_need_to_predict= CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("field_need_to_class"));
	if (m_field_need_to_predict < 0 || m_field_need_to_predict > 3)
	{
		m_field_need_to_predict = 0;
	}


	if (m_need_test == 1)
	{
		szInputPredict=m_pConfigInfo->GetValue("test_input").c_str();
	}
	else
	{
		szInputPredict=m_pConfigInfo->GetValue("predict_file").c_str();
	}

	szOutputPredict=m_pConfigInfo->GetValue("predict_output").c_str();
	ifstream fin_predict(szInputPredict);
	ofstream fout_predict(szOutputPredict);
	if(!fin_predict.is_open() || !fout_predict.is_open())
	{
		LOG(ERROR)<<"can not open input file";
		return -1;
	}
	string line;
	int line_num=0;
	while(!fin_predict.eof())
	{
		++line_num;
		getline(fin_predict, line);
		if(line.size() == 0)
		  continue;

		if (predict_type ==1)
		{//judge every line..... not good
			ret = NBPredictLine(line,fout_predict);
		}
		else if (predict_type == 2)
		{
			ret = NBPredictLineRecurse(line,fout_predict);
		}
		else
		{
			ret = NBPredictLine(line,fout_predict);
		}
		if(ret < 0 )
		{
			LOG(ERROR)<<"original data line parse fail";
		}
		//LOG(INFO)<<"original data line "<< line_num << "predict over");
	}

	fin_predict.close();
	fout_predict.close();

	if (m_need_test == 1)
	{
		m_test_out << "TotalNumInTest:" << TotalNumInTest <<endl;
		m_test_out << "CorrectNumInTest:" << CorrectNumInTest[0] << " " << CorrectNumInTest[1] << " " << CorrectNumInTest[2] << " " << CorrectNumInTest[3] << endl;

		m_test_out << "ratio:" << float(CorrectNumInTest[0]) / float(TotalNumInTest) << " "<<float(CorrectNumInTest[1]) /float(TotalNumInTest) << " "<<float(CorrectNumInTest[2]) / float(TotalNumInTest) << " "<<float(CorrectNumInTest[3]) / float(TotalNumInTest) <<  endl;
	}
	m_test_file_in.close();
	m_test_out.close();

	return 0;
}

int NBClassifier::AddWord(const map<string, int> & all_terms,const vector<int> & class_tree)
{
	__gnu_cxx::hash_map<uint32_t, uint32_t>::iterator itMap;
	map<string,int>::const_iterator itTerm;
	uint32_t hash_value;
	for(itTerm = all_terms.begin(); itTerm != all_terms.end(); ++itTerm)
	{
		string term;
		term = itTerm->first;
		//added 20120718 , delete term too short 
		/*
		if (term.size()<=3)
		{
			DLOG(INFO)<<"term too short ,delete %s" << term );
			continue;
		}
		*/


		DLOG(WARNING)<<"adding term " << term ;
		int weight = itTerm->second;
		TotalTermNum += weight;
		hash_value = hashlittle(term.c_str(),term.size(),1);
		itMap = TermHashMap.find(hash_value);
		// have not record this term
		if(itMap == TermHashMap.end())
		{
			TermHashMap[hash_value] = TotalTermHashNum;
			TotalTermHashNum++;


			TermInfo local_term_info;
			local_term_info.df=weight;
			local_term_info.idf=0;
			local_term_info.TotalClassItemDf=1;
			snprintf(local_term_info.termstr,MAX_TERM_LENGTH,"%s",term.c_str());
			for(vector<int>::const_iterator it=class_tree.begin();it != class_tree.end() ; ++it)
			{
				(local_term_info.ClassDf)[*it] = weight;
				(local_term_info.ClassItemDf)[*it] = 1;
				DLOG(WARNING)<<"new term " << term << " in class " << *it;
				ClassNodeMap[*it]->term_num ++;
			}
			mNBTerms.push_back(local_term_info);
		}
		else
		{
			mNBTerms[itMap->second].df+=weight;
			mNBTerms[itMap->second].TotalClassItemDf += 1;
			for(vector<int>::const_iterator it=class_tree.begin();it != class_tree.end() ; ++it)
			{
				map<int,int>::iterator iter_tmp;
				ClassNodeMap[*it]->term_num ++;

				iter_tmp = mNBTerms[itMap->second].ClassDf.find(*it);
				if (iter_tmp == mNBTerms[itMap->second].ClassDf.end())
				{
					mNBTerms[itMap->second].ClassDf[*it]=weight;
					mNBTerms[itMap->second].ClassItemDf[*it]=1;
					DLOG(WARNING)<<"old term " << term << " in new class " << *it << " df " << mNBTerms[itMap->second].df << " weight " << mNBTerms[itMap->second].ClassDf[*it];
				}
				else
				{
					mNBTerms[itMap->second].ClassDf[*it] += weight;
					mNBTerms[itMap->second].ClassItemDf[*it] += 1;
					DLOG(WARNING)<<"old term " << term << " in old class " << *it << " df " << mNBTerms[itMap->second].df << " weight " << mNBTerms[itMap->second].ClassDf[*it];
				}
			}
		}
	}
	// count repeatedly
	return 0;
}



int NBClassifier::GetNBFrmOriginalLine(string strLine)
{
	int source_type=0; //json
	int ret;
	source_type = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("NBsource_file_type"));
	// record all words, term hash and weight
	map<string,int> all_words;
	vector<string> strContent = CStringTool::SpliteByChar(strLine,'\t'); 
	if(strContent.size()!=5 )
	{
			LOG(WARNING)<<"parse line error";
			return -1;
	}
	//1. the first column is item type
	vector<string> strType = CStringTool::SpliteByChar(strContent[0],':'); 
	vector<int> class_tree;
	int class_level=0;

	DLOG(WARNING)<<"data begin: " << strContent[0] << "\t" << strContent[1] << "\t" << strContent[2] << "\t" << strContent[4];
	// source_type =0, 
	//catalog is given by string  
	if(source_type == 0)
	{
		// root  of class tree
		vector<int> class_ids (ClassRoot->sons);
		// go through the class level one after another
		for(class_level=0; class_level < strType.size(); class_level++)
		{
			int id_found=0;
			// in a level, traverse every id of their father
			for(u_int i =0 ; i< class_ids.size(); i++)
			{
				if(strType[class_level] == ClassNameMap[class_ids[i]])
				{
					//found a id of this train data
					// the ids must be in a tree , otherwise the sons will not add this train data
					DLOG(WARNING)<<"found class id " << class_ids[i] << "name " << ClassNameMap[class_ids[i]] << "in level " << class_level << ". type string: " << strContent[0];
					id_found = class_ids[i];

					// add a check : the train data num should not exceeds proper_train_data_num, otherwise, do not need this train data
					/** obsoletedd 20120823*/
					/*
					if (ClassNodeMap[id_found]->train_data_num > ClassNodeMap[id_found]->proper_train_data_num )
					{
						DLOG(INFO)<<"class_id [" << id_found << "] train_data_num [" << ClassNodeMap[id_found]->train_data_num <<"] exceeds proper_num [" << ClassNodeMap[id_found]->proper_train_data_num << "]" ;
						return 1;
					}
					*/

					class_tree.push_back(id_found);
				}
			}
			//did not match any class, error 
			if(id_found == 0)
			{
				LOG(WARNING)<<"did not match any class:error";
				return -1;
			}
			class_ids = ClassNodeMap[id_found]->sons;
		}
	}
	else
	{
		LOG(INFO)<<"source_type 1 have not been implemented :error";
		return -1;
	}

	// 2. name , weight * 2
	string strName = strContent[1];

	// a violent method 
	// always add the class name to the item's name , which means ,give the class name term the highest occurence 

	strName = strName + " " + GetClassNameTree(class_tree.back());

	// end  violent method
	vector<string> seg_out;
	vector<pair<string,int> > new_word;
	map<string,int> all_terms;
	int word_count;

	word_count = mpSegmentor->bfd_segment_utf8(strName, seg_out,new_word);
	if(word_count <=0)
	{
		LOG(ERROR)<<"segment return no words ,input : " << strName;
	}
	for(int i=0; i < word_count ; i++)
	{
		if (seg_out[i].size()<=3 || is_digit(seg_out[i]))
		{
			DLOG(INFO) << "term too short,delete [" << seg_out[i] << "]";
			continue;
		}
		DLOG(INFO) << "will add term to index [" << seg_out[i] << "]";

		int weight =0;
		int ret=0;
		weight=2;
		
		if(all_terms.find(seg_out[i]) == all_terms.end())
		{
			all_terms[seg_out[i]] =weight;
		}
		else
		{
			all_terms[seg_out[i]] += weight;
		}
	}

	// 3. tags, weight *2
	if (m_need_tag_in_model == 1)
	{
		string strTag = strContent[2];
		word_count = mpSegmentor->bfd_segment_utf8(strTag, seg_out,new_word);
		if(word_count <=0)
		{
			LOG(ERROR)<<"segment return no words ,input : " << strTag;
		}
		for(int i=0; i < word_count ; i++)
		{
			if (seg_out[i].size()<=3 || is_digit(seg_out[i]))
			{
				DLOG(INFO) << "term too short,delete [" << seg_out[i] << "]";
				continue;
			}
			DLOG(INFO) << "will add term to index " << seg_out[i];
			int weight =0;
			int ret=0;
			weight=2;
			if(all_terms.find(seg_out[i]) == all_terms.end())
			{
				all_terms[seg_out[i]] = weight;

			}
			else
			{
				all_terms[seg_out[i]] += weight;

			}
		}
	}
	else
	{
		DLOG(WARNING)<<"do not add content to NB model according to config ";
	}

	// 4. content , weight *1
	if (m_need_content_in_model == 1)
	{

		string strDesc = strContent[3];
		word_count = mpSegmentor->bfd_segment_utf8(strDesc, seg_out,new_word);
		if(word_count <=0)
		{
			LOG(ERROR)<<"segment return no words ,input : " << strDesc;
		}
		for(int i=0; i < word_count ; i++)
		{
			if (seg_out[i].size()<=3 || is_digit(seg_out[i]))
			{
				DLOG(INFO) << "term too short,delete [" << seg_out[i] << "]";
				continue;
			}
			DLOG(INFO) << "will add term to index [" << seg_out[i] << "]";
			int weight =0;
			int ret=0;
			weight=1;
			if(all_terms.find(seg_out[i]) == all_terms.end())
			{
				all_terms[seg_out[i]] = weight;

			}
			else
			{
				all_terms[seg_out[i]] += weight;

			}
		}
	}
	else
	{
		DLOG(WARNING)<<"do not add content to NB model according to config ";
	}

	// 5. add all_words to class, once a learning item
	if(all_terms.size() <=0)
	{
		LOG(ERROR)<<"no term in item " << strLine 	;
		return -1;
	}
	ret = AddWord(all_terms,class_tree);
	if(ret <0)
	{
		LOG(ERROR)<<"add term error" << strLine ;
		return -1;
	}
	++TotalLearningItemNum;
	
	for(vector<int>::iterator it = class_tree.begin();it != class_tree.end();it++)
	{
		ClassNodeMap[*it]->train_data_num +=1;	
	}
	
	return 0;

}

int NBClassifier::GetChiSquare(int N,int N11,int N10,int N01,int N00)
{
	if (((N11+ N01) * (N11 + N10) * (N00 + N10) * (N00+ N01)) ==0)
	{
		return 0;
	}
	else
	{
		return int (double(N)* pow(double(N11 * N00 - N10 * N01),2.0) / double(double(N11+ N01) * double(N11 + N10) * double(N00 + N10) * double(N00+ N01)));
	}
}

float NBClassifier::GetChiSquare(float N,float N11,float N10,float N01,float N00)
{
	if (((N11+ N01) * (N11 + N10) * (N00 + N10) * (N00+ N01)) ==0)
	{
		return 0.0;
	}
	else
	{
		return  (N* powf((N11 * N00 - N10 * N01),2.0) / ((N11+ N01) * (N11 + N10) * (N00 + N10) * (N00+ N01)));
	}
}
/*
int NBClassifier::NBPredictLineAll(string strLine,ofstream &fout_predict)
{
	map<string,int> local_term_count;
	vector<string> seg_out;
	vector<string> new_word;

	char term[100];
	int word_count;
	word_count = mpSegmentor->bfd_segment_utf8(strLine.c_str(), seg_out,new_word);
	if(word_count <=0)
	{
		LOG(INFO)<<"segment return no words, input : " << strLine);
		return -1;
	}
	for(int i=0; i < word_count ; i++)
	{
		int weight =0;
		int ret=0;
		if (local_term_count.find(seg_out[i]) == local_term_count.end())
		{
			local_term_count[seg_out[i]]=1;
		}
		else
		{
			local_term_count[seg_out[i]] +=1;
		}
	}

	int tf;
	int N11,N10,N01,N00,N;
	int ChiSquare;
	int idf;
	int score;
	vector<pair<int,int> > vClassIdScore;

	for(map<int,ClassNode*>::iterator it_class = ClassNodeMap.begin(); it_class != ClassNodeMap.end(); ++it_class)
	{
		score=0;
		for(map<string,int>::iterator it_term = local_term_count.begin(); it_term != local_term_count.end(); ++ it_term)
		{
			tf= mNBTerms[it_term->first].ClassDf[it_class->first] * it_term->second ;
			idf=mNBTerms[it_term->first].idf;
			N11 = mNBTerms[it_term->first].ClassItemDf[it_class->first];
			N10 = mNBTerms[it_term->first].TotalClassItemDf - N11;
			N01 = (it_class->second)->train_data_num - N11;
			N = TotalLearningItemNum;
			N00 = N- N01 - N10 - N11;
			ChiSquare = GetChiSquare(N,N11,N10,N01,N00);

			score +=  int(1.0 + log10(double(tf))) * idf * int(1.0 + log(double(ChiSquare)));
			DLOG(WARNING)<<"classid [" <<it_class->first << "] term [" << it_term->first << "] tf [" << tf << " " << it_term->second << "] idf [" << idf << "] N N11 N10 N01 N00 ChiS ["  << N << " " << N11 << " " << N10 << " " << N01 << " " << N00 << " " << ChiSquare << "] score [" << score << "]" << " real tf ["<<log10(double(tf)) << "] real chisquare ["<< log(double(ChiSquare)) << "]");
		}
		vClassIdScore.push_back(pair<int,int>(it_class->first,score));

	}
	sort(vClassIdScore.begin(),vClassIdScore.end(),CmpPairSecond);

	
	vector<pair<int,int> >::iterator it = vClassIdScore.begin();
	fout_predict << it->first << "\t" << ClassNameMap[it->first] << "\t" <<it->second << "\t" << strLine << endl;
	
	return 0;

}
*/
string NBClassifier::GetClassNameTree(int class_id)
{
	string ClassTree;
	int tmp_id;
	int count;
	ClassTree="";
	if (class_id <= 0)
	{
		return ClassTree;
	}
	tmp_id = class_id;
	count =0;

	while (tmp_id >0 && count <10)
	{
		if(count ==0)
		{
			ClassTree = ClassNameMap[tmp_id] ;
		}
		else
		{
			ClassTree = ClassNameMap[tmp_id] +":" + ClassTree;
		}
		tmp_id = ClassNodeMap[tmp_id]->father_id;
		++count ;
	}
	return ClassTree;
}



string NBClassifier::GetClassIdTree(int class_id)
{
	string ClassTree;
	int tmp_id;
	int count;
	ClassTree="";
	char buf[128];
	if (class_id <= 0)
	{
		return ClassTree;
	}
	tmp_id = class_id;
	count =0;

	while (tmp_id >0 && count <10)
	{
		if(count ==0)
		{
			snprintf(buf,128,"%d",tmp_id);
			ClassTree = buf ;
		}
		else
		{
			snprintf(buf,128,"%d:",tmp_id);
			ClassTree = buf + ClassTree;
		}
		tmp_id = ClassNodeMap[tmp_id]->father_id;
		++count ;
	}
	return ClassTree;
}

string NBClassifier::GetClassFIdTree(int class_id)
{
	string ClassTree;
	int tmp_id;
	int count;
	ClassTree="";
	char buf[128];
	if (class_id <= 0)
	{
		return ClassTree;
	}
	tmp_id = class_id;
	count =0;

	while (tmp_id >0 && count <10)
	{
		if(count ==0)
		{
			snprintf(buf,128,"%d",ClassNodeMap[tmp_id]->foreign_id);
			ClassTree = buf ;
		}
		else
		{
			snprintf(buf,128,"%d:",ClassNodeMap[tmp_id]->foreign_id);
			ClassTree = buf + ClassTree;
		}
		tmp_id = ClassNodeMap[tmp_id]->father_id;
		++count ;
	}
	return ClassTree;
}

int NBClassifier::GetTopNClass(map<string,int> & terms, ClassNode* BeginNode, float current_score, int topN)
{
	int tf;
	float N11,N10,N01,N00,N;
	float ChiSquare;
	int idf;
	float score=0.0;

	vector<int> tmpV;
	pair<int,float> ClassScore;
	priority_queue< pair<int,float>, vector<pair<int,float> >, PQComparison > ClassScores(PQComparison(false));

	tmpV = BeginNode->sons;
	if(tmpV.empty() == true)
	{
		LOG(ERROR)<<"node have no sons ,may be error";
		return 1;
	}
	for(vector<int>::iterator it_class = tmpV.begin(); it_class != tmpV.end(); ++it_class)
	{
		//accumulate score from father
		score = current_score;
		for(map<string,int>::iterator it_term = terms.begin(); it_term != terms.end(); ++ it_term)
		{
			uint32_t hash_value;
			__gnu_cxx::hash_map<uint32_t, uint32_t>::iterator itMap;
			float term_score;
			hash_value = hashlittle(it_term->first.c_str(),it_term->first.size(),1);
			itMap = TermHashMap.find(hash_value);
			if (itMap == TermHashMap.end())
			{
				DLOG(INFO)<<"have not seen this term " << it_term->first << "hash " << hash_value;
				continue;
			}
			DLOG(INFO)<<"get term " << it_term->first << "hash " << hash_value << "index " << itMap->second;

			tf= mNBTerms[itMap->second].ClassDf[*it_class] * it_term->second ;
			idf=mNBTerms[itMap->second].idf;
			N11 = float(mNBTerms[itMap->second].ClassItemDf[*it_class]);
			N10 = float(mNBTerms[itMap->second].TotalClassItemDf) - N11;
			N01 = float((ClassNodeMap[*it_class])->train_data_num) - N11;
			N = float(TotalLearningItemNum);
			N00 = N- N01 - N10 - N11;
			ChiSquare = GetChiSquare(N,N11,N10,N01,N00);
			term_score =  (float(1.0) + log2f(float(1+tf))) * float(idf) * float(1.0 + log2f(float(1.0) +ChiSquare));
			score += term_score;

			DLOG(INFO)<<"classid [" <<*it_class << "] term [" << mNBTerms[itMap->second].termstr << "] tf [" << tf << " " << it_term->second << "] idf [" << idf << "] N N11 N10 N01 N00 ChiS ["  << N << " " << N11 << " " << N10 << " " << N01 << " " << N00 << " " << ChiSquare << "] score [" << score << "]" << "term_score [" << term_score << "] real tf ["<<log2f(1.0+ (tf)) << "] real chisquare ["<< log2f(1.0+(ChiSquare)) << "]";
		}
		ClassScore.first = *it_class;
		ClassScore.second = score;

		ClassScores.push(ClassScore);
		DLOG(INFO)<<" get a class [" << ClassScore.first << "] score [" << ClassScore.second << "] scores.size [" << ClassScores.size() << "]" ;
	}
	DLOG(INFO)<< "get scores end";

	for(int i=0;i<topN && (!ClassScores.empty());i++)
	{
		ClassScore = ClassScores.top();
		ClassScores.pop();
		DLOG(INFO)<<"get [" << i+1 <<"]st max class [" << ClassScore.first << "] score [" << ClassScore.second << "] scores.size [" << ClassScores.size()  << "]"  ;
		if (ClassNodeMap[ClassScore.first]->sons.empty() == true)
		{
			//recurse stops, get a result
			if(ClassScore.second > PredictOut.second)
			{
				DLOG(INFO)<<"change predict out from [" << PredictOut.first << "] to [" << ClassScore.first << "]";
				PredictOut = ClassScore;
			}
		}
		else
		{
			// has sons, get sons score recursively
			GetTopNClass(terms,ClassNodeMap[ClassScore.first],ClassScore.second,topN);

		}
	}

	return 0;

}

int NBClassifier::NBPredictLineRecurse(string strLine,ofstream &fout_predict)
{
	map<string,int> local_term_count;
	vector<string> seg_out;
	vector<string> VStrField;
	vector<pair<string,int> > new_word;
	string strClassNeeded;


	float score=0,max_score=0;
	int max_id=0;
	vector<pair<int,int> > vClassIdScore;

	PredictOut.first =0;
	PredictOut.second = 0;

	ClassNode * tmpNode = ClassRoot;
	vector<int> tmpV;

	VStrField = boost::algorithm::split(VStrField,strLine,boost::is_any_of("\t"));
	if(VStrField.size() < 5)
	{
		DLOG(INFO)<<"data format error, class fail " << strLine;
		goto CLASS_FAIL;
	}
	if(m_field_need_to_predict == 1)
	{
		strClassNeeded = VStrField[PDATA_FIELD_NAME];
	}
	else if(m_field_need_to_predict == 2)
	{
		strClassNeeded = VStrField[PDATA_FIELD_CATE] + VStrField[PDATA_FIELD_NAME];
	}
	else
	{
		strClassNeeded = VStrField[PDATA_FIELD_CATE] +VStrField[PDATA_FIELD_NAME] + VStrField[PDATA_FIELD_DESC];
	}
	char term[100];
	int word_count;
	word_count = mpSegmentor->bfd_segment_utf8(strClassNeeded, seg_out,new_word);
	if(word_count <=0)
	{
		LOG(INFO)<<"segment return no words, input : " << strClassNeeded;
		return -1;
	}
	for(int i=0; i < word_count ; i++)
	{
		if (seg_out[i].size()<=3 || is_digit(seg_out[i]))
		{
			DLOG(INFO) << "term too short,delete [" << seg_out[i] << "]";
			continue;
		}
		int weight =0;
		int ret=0;
		if (local_term_count.find(seg_out[i]) == local_term_count.end())
		{
			local_term_count[seg_out[i]]=1;
		}
		else
		{
			local_term_count[seg_out[i]] +=1;
		}
	}

	GetTopNClass(local_term_count, ClassRoot , 0, 3);



	if(PredictOut.first ==0)
	{
CLASS_FAIL:
		LOG(ERROR)<<"classify fail:  " << strLine;

		/*
		fout_predict <<  "99\t其他\t0.0\t" ;

		fout_predict << "\t"<< strLine << endl;
		*/
		fout_predict <<  strLine[0] << "\t" << strLine[1] <<"其他\t99\t0.0\t" ;
		fout_predict <<  endl;

		return 1;
	}
	max_id = PredictOut.first;
	max_score = PredictOut.second;

	// find tags
	std::set<int> tmp_st;
	//fout_predict << max_id << "\t" << GetClassNameTree(max_id) << "\t" << max_score << "\t" ;
	fout_predict << strLine[0] << "\t" <<strLine[1] << "\t" << ClassNameMap[max_id] << "\t"<< max_id << "\t"  << max_score << "\t" ;
	for(map<string,Aho_Corasick*>::iterator it = ClassNodeMap[max_id]->TagMapAC.begin(); it != ClassNodeMap[max_id]->TagMapAC.end();++it)
	{
		tmp_st = it->second->get_match(strLine.c_str());
		if(tmp_st.size()> 0)
		{
			fout_predict << it->first;
			for(set<int>::iterator it2 = tmp_st.begin(); it2 != tmp_st.end(); ++it2)
			{
				fout_predict << ":"<< TagNameMap[*it2];
			}
			fout_predict << ";";
		}
	}

	//fout_predict << "\t"<< strLine << endl;
	fout_predict << endl;

	return 0;
}

int NBClassifier::NBPredictLine(const string strLine,ofstream &fout_predict)
{
	map<string,int> local_term_count;
	vector<string> seg_out;
	vector<string> VStrField;
	vector<pair<string,int> > new_word;
	string strClassNeeded;
	string strTagNeeded;
	string output_profile;

	int tf;
	float N11,N10,N01,N00,N;
	float ChiSquare;
	int idf;
	float score=0,max_score=0;
	int max_id=0;
	vector<pair<int,int> > vClassIdScore;

	int foreign_id=0,bfd_id=0;

	ClassNode * tmpNode = ClassRoot;
	vector<int> tmpV;

	/** get data from cache(leveldb)*/
	if (GetDataFrmCache(strLine,m_data_source_type,output_profile)==true)
	{
		fout_predict << strLine << "\t" << output_profile << endl;
		DLOG(INFO) << "get data from cache! orig: [" << strLine << "] profile : [" << output_profile << "] data_source_type:[" <<m_data_source_type << "]" ;
		return 0;
	}

	VStrField = boost::algorithm::split(VStrField,strLine,boost::is_any_of("\t"));


	if(VStrField.size() < PDATA_FIELD_DESC)
	{
		LOG(WARNING)<<"data format error, class fail " << strLine;
		cout << strLine << endl;
		goto CLASS_FAIL;
	}
	if(m_field_need_to_predict == 1)
	{
		strClassNeeded = VStrField[PDATA_FIELD_NAME];
	}
	else if(m_field_need_to_predict == 2)
	{
		strClassNeeded = VStrField[PDATA_FIELD_CATE] + VStrField[PDATA_FIELD_NAME];
	}
	else if(m_field_need_to_predict == 3)
	{
		strClassNeeded = VStrField[PDATA_FIELD_CATE] + VStrField[PDATA_FIELD_NAME];
		try 
		{
			if(mJsonReader.parse(VStrField[PDATA_FIELD_ATTR],mJsonValue))
			{   
				if(mJsonValue.type() ==Json::objectValue && !mJsonValue["category"].isNull())
				{
					strClassNeeded += mJsonValue["category"].asString();
					DLOG(INFO) << "add tag category " << mJsonValue["category"].asString();
				}
			}
		}
		catch (...)
		{
			LOG(INFO) << "parse json error " << VStrField[PDATA_FIELD_ATTR];
		}
	}
	else
	{
		strClassNeeded = VStrField[PDATA_FIELD_CATE] +VStrField[PDATA_FIELD_NAME] + VStrField[PDATA_FIELD_DESC];
	}
	strTagNeeded = VStrField[PDATA_FIELD_CATE] +VStrField[PDATA_FIELD_NAME] + VStrField[PDATA_FIELD_DESC];
	char term[100];
	int word_count;
	word_count = mpSegmentor->bfd_segment_utf8(strClassNeeded, seg_out,new_word);
	if(word_count <=0)
	{
		LOG(INFO)<<"segment return no words, input : " << strClassNeeded;
		return -1;
	}
	for(int i=0; i < word_count ; i++)
	{
		if (seg_out[i].size()<=3 || is_digit(seg_out[i]))
		{
			DLOG(INFO) << "term too short,delete [" << seg_out[i]<< "]";
			continue;
		}
		DLOG(INFO) << "use term to predict [" << seg_out[i] << "]";
		int weight =0;
		int ret=0;
		if (local_term_count.find(seg_out[i]) == local_term_count.end())
		{
			local_term_count[seg_out[i]]=1;
		}
		else
		{
			local_term_count[seg_out[i]] +=1;
		}
	}


	//get class through mini KNN
	//we have a foreign class to bfd class mapping ,which is processed maually.
	//although the mapping can not give a through and detailed classification,  it is more precise because it is based on manually work,  not only class mapping, but also the classification by the editor of foreign website.
	if (m_need_miniKNN == 1)
	{
		foreign_id = miniKNN->PredictLine(VStrField[PDATA_FIELD_CID],VStrField[PDATA_FIELD_CATE]);

		DLOG(INFO)<<"try to get id using KNN  " << VStrField[PDATA_FIELD_CID] << " " << VStrField[PDATA_FIELD_CATE];
		if (foreign_id > 0 && ClassIdMapReverse.find(foreign_id) != ClassIdMapReverse.end())
		{
			bfd_id = ClassIdMapReverse[foreign_id] ;
			tmpNode = ClassNodeMap[bfd_id];
			max_id = bfd_id;

			m_knn_match_count ++;
			LOG(INFO)<<"get a bfd_id " << bfd_id << " using KNN ,match_count " << m_knn_match_count;
		}
	}

	// end of mini KNN

	while (!(tmpNode->sons.empty()))
	{
		tmpV = tmpNode->sons;

		score=0;
		max_score=0;
		max_id =0;
		for(vector<int>::iterator it_class = tmpV.begin(); it_class != tmpV.end(); ++it_class)
		{
			float prior;
			int father_id;
			father_id = ClassNodeMap[*it_class]->father_id;
			prior =( 1.0+ float(1.0+ClassNodeMap[father_id]->son_max_train_data_num)) / (1.0+float (ClassNodeMap[*it_class]->train_data_num)) ; 
			for(map<string,int>::iterator it_term = local_term_count.begin(); it_term != local_term_count.end(); ++ it_term)
			{
				//added 20120718 ,delete short term
				if (it_term->first.size()<=3)
				{
					DLOG(INFO)<<"term too short ,delete " << it_term->first ;
					continue;
				}
				uint32_t hash_value;
				__gnu_cxx::hash_map<uint32_t, uint32_t>::iterator itMap;
				float term_score;
				hash_value = hashlittle(it_term->first.c_str(),it_term->first.size(),1);
				itMap = TermHashMap.find(hash_value);
				if (itMap == TermHashMap.end())
				{
					DLOG(INFO)<<"have not seen this term " << it_term->first << "hash " << hash_value;
					continue;
				}
				DLOG(INFO)<<"get term " << it_term->first << "hash " << hash_value << "index " << itMap->second;

				tf= mNBTerms[itMap->second].ClassDf[*it_class] * it_term->second  * prior;
				idf=mNBTerms[itMap->second].idf;
				N11 = float(mNBTerms[itMap->second].ClassItemDf[*it_class]);
				N10 = float(mNBTerms[itMap->second].TotalClassItemDf) - N11;
				N01 = float((ClassNodeMap[*it_class])->train_data_num) - N11;
				N = float(TotalLearningItemNum);
				N00 = N- N01 - N10 - N11;
				ChiSquare = GetChiSquare(N,N11,N10,N01,N00);
				term_score =  (float(1.0) + log2f(float(1+tf))) * float(idf) * float(1.0 + log2f(float(1.0) +ChiSquare));
				score += term_score;


				DLOG(INFO)<<"classid [" <<*it_class << "] term [" << mNBTerms[itMap->second].termstr << "] tf [" << tf << " " << it_term->second << "] idf [" << idf << "] N N11 N10 N01 N00 ChiS ["  << N << " " << N11 << " " << N10 << " " << N01 << " " << N00 << " " << ChiSquare << "] score [" << score << "]" << "term_score [" << term_score << "] real tf ["<<log2f(1.0+ (tf)) << "] real chisquare ["<< log2f(1.0+(ChiSquare)) << "]";
			}
			//vClassIdScore.push_back(pair<int,int>(*it_class,score));
			if (score> max_score)
			{
				max_score = score;
				max_id = *it_class;
			}
			score =0;
		}
		if(max_id == 0)
		{
			break;
		}

		tmpNode = ClassNodeMap[max_id];
		DLOG(INFO)<<"classify : change to id " << max_id;
		//sort(vClassIdScore.begin(),vClassIdScore.end(),CmpPairSecond);

	}

	if(max_id ==0)
	{
		
CLASS_FAIL:
		output_profile = "\t0\t其他\t0\t0\t0\t0\t0";
		LOG(ERROR)<<"classify fail:  " << strLine;

		//fout_predict << strline ;
		//fout_predict <<  "\t0\t其他\t0\t0\t0\t0\t0" ;
		fout_predict << strLine<< "\t" <<output_profile << endl;
		LOG(INFO)<<"classify over : bid [" << VStrField[0] << "] id ["<< VStrField[1]<<  "] class_id [0] f_id [0] class_name [其他]";

		return 1;
	}

	if (m_need_test == 1)
	{// output test info
		if(VStrField.size() >= PDATA_FIELD_TEST_CATE && (VStrField[PDATA_FIELD_TEST_CATE]).size() >= 5 )
		{
			string test_cate,predict_cate;
			// the correct category from test data
			test_cate = VStrField[PDATA_FIELD_TEST_CATE];
			// predicted category
			predict_cate = GetClassNameTree(max_id);

			vector<string> test_cate_tree, predict_cate_tree;




			m_test_out << predict_cate << "\t";
			if (predict_cate == test_cate)
			{
				m_test_out << "1\t1\t1\t1\t";
				CorrectNumInTest[0] +=1;
				CorrectNumInTest[1] +=1;
				CorrectNumInTest[2] +=1;
				CorrectNumInTest[3] +=1;
			}
			else
			{
				boost::algorithm::split(test_cate_tree,test_cate,boost::is_any_of(":"));
				boost::algorithm::split(predict_cate_tree,predict_cate,boost::is_any_of(":"));

				int i=0;
				bool match =true;
				//total four level of category tree
				for(i=0;i<4;i++)
				{
					if (match == false)
					{
						m_test_out << "0\t";
						continue;
					}

					if(i >= predict_cate_tree.size() || i >= test_cate_tree.size())
					{// match must be equal to true
						CorrectNumInTest[i] +=1;
						m_test_out << "1\t";
						continue;
					}

					if(test_cate_tree[i] == predict_cate_tree[i])
					{
						CorrectNumInTest[i] +=1;
						m_test_out << "1\t";
					}
					else
					{
						match=false;
						m_test_out << "0\t";
					}
				}
			}
			TotalNumInTest +=1;
			m_test_out <<strLine <<endl;
		}
		else
		{
			m_test_out << "ERROR\t-1\t-1\t-1\t-1\t"<<strLine <<endl;
		}

	}
	// find tags
	std::set<pair<int,int>,setcomp> tmp_st;

	string brand_out;
	string brand_name;
	int brand_off=-1;
	//first: key_str, second:(first: tag_id,second: offset)
	map<string,map<int,int> > all_tags;
	char tmp_brand_buf[128];
	int64_t tag_full_id;


	/** output three column*/
	output_profile =  GetClassFIdTree(max_id) + "\t" +GetClassNameTree(max_id) +"\t" +  GetClassIdTree(max_id) + "\t";

	for(map<string,Aho_Corasick*>::iterator it = ClassNodeMap[max_id]->TagMapAC.begin(); it != ClassNodeMap[max_id]->TagMapAC.end();++it)
	{
		if(it->first.find("eyword") != string::npos)
		{// keyword is not good tag

			DLOG(WARNING)<<"keyword delete : " << it->first ;
			continue;
		}
		tmp_st = it->second->get_match_off(strTagNeeded.c_str());


		if(tmp_st.size()> 0)
		{
			//fout_predict << it->first;
			for(set<pair<int,int>,setcomp>::iterator it2 = tmp_st.begin(); it2 != tmp_st.end(); ++it2)
			{
				//tag_full_id = (int64_t(max_id) << 32) + it2->first; 
				tag_full_id = it2->first; 

				if (TagNameMap[tag_full_id].size() <=2)
				{ //tag_str will be mis-matched if its size less than 2
					DLOG(WARNING)<<"tag too short, delete : " << TagNameMap[tag_full_id] ;
					continue;
				}


				/*
				   if (it->first.find("品牌") != string::npos)
				   {
				   if (brand_out.size() ==0)
				   {
				   snprintf(tmp_brand_buf,128,"%d", it2->first);
				   brand_out = tmp_brand_buf;
				   brand_name = TagNameMap[tag_full_id];
				   brand_off = it2->second;
				   }
				// brand turned out earlier
				else if (it2->second < brand_off)
				{
				snprintf(tmp_brand_buf,128,"%d", it2->first);
				brand_out = tmp_brand_buf ;
				brand_name = TagNameMap[tag_full_id];
				brand_off = it2->second;
				}

				}
				*/

				// no such tag key
				if(all_tags.find(it->first) == all_tags.end())
				{
					map<int,int> tmp_map;
					tmp_map[it2->first]= it2->second;
					all_tags[it->first]= tmp_map; 
				}
				else if (it->first.find("品牌") != string::npos || it->first.find("性别") != string::npos)
				{//this two tag_key should have only one value, according to the offset
					if (all_tags[it->first].begin()->second > it2->second)
					{// get the one turn out earlier
						DLOG(WARNING)<<"replace tag " << all_tags[it->first].begin()->first << " with " << it2->first;
						all_tags[it->first].clear();
						all_tags[it->first][it2->first] = it2->second; 
					}
					else
					{
						DLOG(WARNING)<<"not replace tag " << all_tags[it->first].begin()->first << " with " << it2->first;
					}

				}
				else
				{
					all_tags[it->first][it2->first] = it2->second; 
				}

			}
		}
	}


	// shit begin
	// common tags
	/*
	   std::set<int> tmp_st2;
	   for(map<string,Aho_Corasick*>::iterator it = CommonTagMapAC.begin(); it != CommonTagMapAC.end();++it)
	   {
	   tmp_st2 = it->second->get_match(strTagNeeded.c_str());

	   if(tmp_st2.size()> 0)
	   {
	//fout_predict << it->first;
	for(set<int>::iterator it2 = tmp_st2.begin(); it2 != tmp_st2.end(); ++it2)
	{
	tag_full_id = (int64_t) *it2; 
	//already have this key
	if (all_tags.find(it->first) != all_tags.end())
	{
	continue;
	}
	all_tags[it->first][*it2]=0 ;
	}
	}
	}
	*/
	//shit end
	//output tag
	string all_id_out;
	string all_name_out;
	for(map<string,map<int,int> >::iterator it = all_tags.begin(); it != all_tags.end(); ++it)
	{
		for(map<int,int>::iterator it2= it->second.begin();it2!=it->second.end();++it2)
		{
			//tag_full_id = (int64_t() << 32) + it2->first; 
			if(it == all_tags.begin())
			{
				snprintf(tmp_brand_buf,128,"%d", it2->first);
				all_id_out = tmp_brand_buf;
				all_name_out = TagKeyMap[it2->first] +":" +TagNameMap[it2->first] ;
			}
			else
			{
				snprintf(tmp_brand_buf,128,"%d", it2->first);
				all_id_out = all_id_out + ":" +tmp_brand_buf;
				all_name_out = all_name_out+";"+TagKeyMap[it2->first] +":" +TagNameMap[it2->first] ;
			}

			if (it->first.find("品牌") != string::npos)
			{
				snprintf(tmp_brand_buf,128,"%d", it2->first);
				brand_out = tmp_brand_buf ;
				brand_name = TagNameMap[it2->first];
			}
		}

	}

	// get Perceptual Tag
	string PerceptualOut;
	string PerceptualId;
	if(m_need_perceptual_tag ==1)
	{
		int top_id =  GetAncestorID(max_id);
		tmp_st = ClassNodeMap[top_id]->PerceptualTagAC.get_match_off(strTagNeeded.c_str());
		if(tmp_st.size()> 0)
		{
			//fout_predict << it->first;
			for(set<pair<int,int>,setcomp>::iterator it2 = tmp_st.begin(); it2 != tmp_st.end(); ++it2)
			{
				if(PerceptualOut.find(mPerceptualTag[top_id][it2->first]) == string::npos)
				{
					snprintf(tmp_brand_buf,128,"%d", it2->first);
					PerceptualId = PerceptualId +":" + tmp_brand_buf;
					PerceptualOut += mPerceptualTag[top_id][it2->first] +" " ;
					DLOG(WARNING)<<"Perceptual tag " <<  mPerceptualTag[top_id][it2->first] ;
				}
			}
			all_id_out = all_id_out +PerceptualId;
			all_name_out = all_name_out+";"+"感性标签:" + PerceptualOut;
		}
		//cheat .....
		else if (top_id == 62 || top_id == 2)
		{
			int joke;
			map<int,string>::iterator tmp_it = mPerceptualTag[top_id].begin();
			srand(time(NULL));
			joke = rand() % mPerceptualTag[top_id].size();

			std::advance(tmp_it, joke );

			snprintf(tmp_brand_buf,128,"%d", tmp_it->first);
			PerceptualId = PerceptualId +":" + tmp_brand_buf;

			PerceptualOut += tmp_it->second;
			DLOG(WARNING)<<"no Perceptual tag found, random " << PerceptualOut;

			all_id_out = all_id_out +PerceptualId;
			all_name_out = all_name_out+";"+"感性标签:" + PerceptualOut;
		}
	}


	output_profile = output_profile +  all_id_out + "\t" + all_name_out +"\t" + brand_out + "\t"+ brand_name ;
	fout_predict << strLine << "\t" <<output_profile<<  endl;

	if (m_data_source_type ==1 || m_data_source_type ==2)
	{
		SetDataCache(strLine,m_data_source_type,output_profile);
	}

	LOG(INFO)<<"classify over : bid [" << VStrField[0] << "] id ["<< VStrField[1]<<  "] class_id [" << max_id <<"] f_id [" << ClassIdMap[max_id] <<"] class_name [" << ClassNameMap[max_id] <<"] PerceptualOut [" << PerceptualOut << "]";

	return 0;

}


int NBClassifier::DumpClassFeatureTerm()
{
	string output_model_dir;

	char path[128];
	int tf;
	float N11,N10,N01,N00,N;
	float ChiSquare;
	int idf;

	output_model_dir= m_pConfigInfo->GetValue("model_output_dir");
	snprintf(path,128,"%s/category_terms.txt",output_model_dir.c_str());

	ofstream fp_class_feature(path);
	if(!fp_class_feature.is_open() )
	{
		LOG(ERROR)<<"open " << path << " error";
		return -1;
	}


	vector<TermInfo>::iterator it_term;
	map<int,ClassNode*>::iterator it_class;
	map<int,vector<pair<string,float> > > TermOfClass;

	for(it_term = mNBTerms.begin(); it_term!= mNBTerms.end(); ++ it_term)
	{
		float term_score;
		int class_id;
		for(map<int,int>::iterator it_class= (*it_term).ClassItemDf.begin() ; it_class != (*it_term).ClassItemDf.end(); ++ it_class )
		{
			class_id = it_class->first;
			tf= (*it_term).ClassDf[class_id] ;
			idf=(*it_term).idf;
			N11 = float(it_class->second);
			N10 = float((*it_term).TotalClassItemDf)- N11;
			N01 = float((ClassNodeMap[class_id])->train_data_num) - N11;
			N = float(TotalLearningItemNum);
			N00 = N- N01 - N10 - N11;
			ChiSquare = GetChiSquare(N,N11,N10,N01,N00);
			term_score = (float(1.0) + log2f(float(1+tf))) * float(idf) * float(1.0 + log2f(float(1.0+ChiSquare)));
			TermOfClass[class_id].push_back( pair<string,float>((*it_term).termstr,term_score));
		}

		//DLOG(INFO)<<"classid [" <<*it_class << "] term [" << it_term->first << "] tf [" << tf << " " << it_term->second << "] idf [" << idf << "] N N11 N10 N01 N00 ChiS ["  << N << " " << N11 << " " << N10 << " " << N01 << " " << N00 << " " << ChiSquare << "] score [" << score << "]" << "term_score [" << term_score << "] real tf ["<<log10(1.0+ double(tf)) << "] real chisquare ["<< log(1.0+double(ChiSquare)) << "]";

	}
	for(map<int,vector<pair<string,float> > >::iterator it = TermOfClass.begin() ; it != TermOfClass.end(); ++it)
	{
		sort(it->second.begin(),it->second.end(),CmpPair);
		float max_score;
		for(int i =0 ; (i < it->second.size()/10 ); i++)
		{
			if (i==0)
			{
				max_score = (it->second)[i].second;
			}
			fp_class_feature << it->first << "\t" << ClassIdMap[it->first] << "\t" << ClassNameMap[it->first] << "\t" << (it->second)[i].first << "\t"<< (it->second)[i].second << "\t"<< (it->second)[i].second/ max_score * 100.0<< endl;

		}
	}
	return 0;


}

int NBClassifier::DumpNBModel()
{
	string output_model_dir;
	output_model_dir= m_pConfigInfo->GetValue("model_output_dir");

	char path[128];
	FILE* fp_classnode;
	FILE* fp_term;
	FILE* fp_termhash;
	int ret;
	snprintf(path,128,"%s/classnode.dict",output_model_dir.c_str());
	fp_classnode=fopen( path,"w");

	map<int,ClassNode*>::iterator it_class;
	for(it_class = ClassNodeMap.begin();it_class!= ClassNodeMap.end(); ++ it_class)
	{
		int tmp;
		ret = fwrite(&(it_class->first),sizeof(int),1,fp_classnode);
		//ret = fwrite(&(it_class->second->level),sizeof(int),1,fp_classnode);
		//ret = fwrite(&(it_class->second->id),sizeof(int),1,fp_classnode);
		//ret = fwrite(&(it_class->second->father_id),sizeof(int),1,fp_classnode);
		ret = fwrite(&(it_class->second->term_num),sizeof(int),1,fp_classnode);
		ret = fwrite(&(it_class->second->train_data_num),sizeof(int),1,fp_classnode);
		ret = fwrite(&(it_class->second->son_max_train_data_num),sizeof(int),1,fp_classnode);
		LOG(INFO)<<"in dump index id: " << it_class->first <<" f_id"<< ClassIdMap[it_class->first] << " level:"<< it_class->second->level << " "<<  it_class->second->father_id << " term_num:" << it_class->second->term_num << " actual_train_data_num:" << it_class->second->train_data_num  << " son_max_train_data_num:"  <<  it_class->second->son_max_train_data_num; 
		if ((it_class->second->sons).empty() == true)
		{
			LOG(INFO)<<"in dump index, leaf node id: " << it_class->first <<"\t"<< ClassIdMap[it_class->first] << "\t" << it_class->second->train_data_num  << "\t"  <<  it_class->second->son_max_train_data_num; 
		}

		// write num of sons. in order to read
		/*
		   tmp = it_class->second->sons.size();
		   ret = fwrite(&(tmp),sizeof(int),1,fp_classnode);

		   for (vector<int>::iterator it=it_class->second->sons.begin(); it != it_class->second->sons.end(); ++it)
		   {
		   DLOG(INFO)<<"in dump index, id:" << it_class->first << " sons : " << *it;
		   ret = fwrite(&(*it),sizeof(int),1,fp_classnode);

		   }
		   */
	}
	fclose(fp_classnode);

	snprintf(path,128,"%s/term.dict",output_model_dir.c_str());
	fp_term=fopen(path,"w");
	vector<TermInfo>::iterator it_term;
	char tmp_term[MAX_TERM_LENGTH];

	for(it_term = mNBTerms.begin(); it_term != mNBTerms.end();++it_term)
	{
		int class_df_num;
		snprintf(tmp_term,MAX_TERM_LENGTH,"%s",(*it_term).termstr);
		ret = fwrite((tmp_term),sizeof(char),MAX_TERM_LENGTH,fp_term);
		ret = fwrite(&((*it_term).idf),sizeof(int),1,fp_term);
		ret = fwrite(&((*it_term).df),sizeof(int),1,fp_term);
		ret = fwrite(&((*it_term).TotalClassItemDf),sizeof(int),1,fp_term);

		class_df_num = (*it_term).ClassDf.size();
		ret = fwrite(&(class_df_num),sizeof(int),1,fp_term);
		DLOG(INFO)<<"in dump index, term :" << tmp_term << " " << (*it_term).idf << " " << (*it_term).df << " " << (*it_term).TotalClassItemDf ;

		for (map<int,int>::iterator it = (*it_term).ClassDf.begin(); it != (*it_term).ClassDf.end();++it)
		{
			ret = fwrite(&(it->first),sizeof(int),1,fp_term);
			ret = fwrite(&(it->second),sizeof(int),1,fp_term);
			DLOG(INFO)<<"in dump index, term :" << tmp_term << " class id " << it->first  << " Df " << it->second ;
		}

		class_df_num = (*it_term).ClassItemDf.size();
		ret = fwrite(&(class_df_num),sizeof(int),1,fp_term);

		for (map<int,int>::iterator it = (*it_term).ClassItemDf.begin(); it != (*it_term).ClassItemDf.end();++it)
		{
			ret = fwrite(&(it->first),sizeof(int),1,fp_term);
			ret = fwrite(&(it->second),sizeof(int),1,fp_term);
			DLOG(INFO)<<"in dump index, term :" << tmp_term << " class id " << it->first  << " Item Df " << it->second ;
		}
	}
	fclose(fp_term);

	// dump TermHashMap
	snprintf(path,128,"%s/term_hash.dict",output_model_dir.c_str());
	fp_termhash=fopen(path,"w");
	__gnu_cxx::hash_map<uint32_t, uint32_t>::iterator it_hash;
	for(it_hash = TermHashMap.begin(); it_hash != TermHashMap.end();++it_hash)
	{
		ret = fwrite(&(it_hash->first),sizeof(int),1,fp_termhash);
		ret = fwrite(&(it_hash->second),sizeof(int),1,fp_termhash);
		DLOG(WARNING)<<"dump term hash: hash [" << it_hash->first << "] index [" << it_hash->second <<"]" ;
	}
	fclose(fp_termhash);


	FILE* fp_total;
	char buf[1024];
	snprintf(path,128,"%s/total_info.txt",output_model_dir.c_str());
	fp_total=fopen(path,"w");
	snprintf(buf,1024,"TotalClassNum:%d\nTotalTermNum:%lld\nTotalLearningItemNum:%d\nTotalTermHashNum:%d",TotalClassNum,TotalTermNum,TotalLearningItemNum,TotalTermHashNum);
	ret = fwrite(buf,sizeof(char),strlen(buf),fp_total);
	DLOG(INFO)<<"in dump index " << " TotalClassNum " << TotalClassNum << " TotalTermNum " << TotalTermNum <<  " TotalLearningItemNum " << TotalLearningItemNum << "TotalTermHashNum " << TotalTermHashNum;

	return 0;

}

int NBClassifier::LoadNBModel()
{
	string model_dir;
	model_dir= m_pConfigInfo->GetValue("model_dir");

	FILE* fp_classnode;
	FILE* fp_term;
	FILE* fp_termhash;
	int ret;
	char path[128];
	snprintf(path,128,"%s/classnode.dict",model_dir.c_str());
	fp_classnode=fopen( path,"r");

	while (!feof(fp_classnode))
	{
		int class_id=0;
		int son_num=0;
		ret = fread(&class_id, sizeof(int),1,fp_classnode);

		if (ClassNodeMap.find(class_id) == ClassNodeMap.end())
		{
			LOG(ERROR)<<"error , class id unknown :" << class_id << "load index error";
			return -1;
		}
		ClassNode* tmpClassNode  = ClassNodeMap[class_id];

		//ret = fread(&(tmpClassNode->level), sizeof(int),1,fp_classnode);
		//ret = fread(&(tmpClassNode->id), sizeof(int),1,fp_classnode);
		//ret = fread(&(tmpClassNode->father_id), sizeof(int),1,fp_classnode);
		ret = fread(&(tmpClassNode->term_num), sizeof(int),1,fp_classnode);
		ret = fread(&(tmpClassNode->train_data_num), sizeof(int),1,fp_classnode);
		ret = fread(&(tmpClassNode->son_max_train_data_num), sizeof(int),1,fp_classnode);
		DLOG(WARNING)<<"in read index id: " << class_id << " "<< tmpClassNode->level << " "<< tmpClassNode->id << " "<< tmpClassNode->father_id << " " << tmpClassNode->term_num << " " << tmpClassNode->train_data_num << " " <<tmpClassNode->son_max_train_data_num;
		/*
		   ret = fread(&(son_num), sizeof(int),1,fp_classnode);
		   for(int i =0;i< son_num;i++)
		   {
		   int son;
		   ret = fread(&(son), sizeof(int),1,fp_classnode);
		//error:
		//tmpClassNode->sons.push_back(son);
		DLOG(WARNING)<<"in read index, id:" << class_id << " sons : " << son;
		}
		*/
	}

	snprintf(path,128,"%s/term.dict",model_dir.c_str());
	fp_term=fopen( path,"r");

	uint32_t term_count=0;
	while(!feof(fp_term))
	{
		char  term[80];
		int class_df_num;
		TermInfo tmp_terminfo;
		//ret = fread((term), sizeof(char),MAX_TERM_LENGTH,fp_term);
		ret = fread((term), sizeof(char),MAX_TERM_LENGTH,fp_term);
		snprintf(tmp_terminfo.termstr,MAX_TERM_LENGTH,"%s",term);
		ret = fread(&(tmp_terminfo.idf), sizeof(int),1,fp_term);
		ret = fread(&(tmp_terminfo.df), sizeof(int),1,fp_term);
		ret = fread(&(tmp_terminfo.TotalClassItemDf), sizeof(int),1,fp_term);
		ret = fread(&(class_df_num), sizeof(int),1,fp_term);

		DLOG(WARNING)<<"in read index, term :" << term << " " << tmp_terminfo.idf << " " << tmp_terminfo.df << " " << tmp_terminfo.TotalClassItemDf ;

		for(int i =0; i< class_df_num;i++)
		{
			int tmp1,tmp2;
			ret = fread(&(tmp1), sizeof(int),1,fp_term);
			ret = fread(&(tmp2), sizeof(int),1,fp_term);
			DLOG(WARNING)<<"in read index, term :" << term << " class id " << tmp1  << " Df " << tmp2 ;

			tmp_terminfo.ClassDf.insert(pair<int,int>(tmp1,tmp2));
		}
		ret = fread(&(class_df_num), sizeof(int),1,fp_term);
		for(int i =0; i< class_df_num;i++)
		{
			int tmp1,tmp2;
			ret = fread(&(tmp1), sizeof(int),1,fp_term);
			ret = fread(&(tmp2), sizeof(int),1,fp_term);

			DLOG(WARNING)<<"in read index, term :" << term << " class id " << tmp1  << " ItemDf " << tmp2 ;
			tmp_terminfo.ClassItemDf.insert(pair<int,int>(tmp1,tmp2));
		}

		/*
		   uint32_t hash_value;
		   hash_value = hashlittle(term,strlen(term),1);
		   TermHashMap[hash_value] = term_count;
		   */
		mNBTerms.push_back(tmp_terminfo);
		term_count++;

	}

	snprintf(path,128,"%s/term_hash.dict",model_dir.c_str());
	fp_termhash=fopen( path,"r");

	uint32_t key,value;
	while(!feof(fp_termhash))
	{
		ret = fread(&key, sizeof(int),1,fp_termhash);
		ret = fread(&value, sizeof(int),1,fp_termhash);
		TermHashMap[key] = value;
		DLOG(WARNING)<<"load term hash: hash [" << key << "] index [" << value <<"]" ;

	}


	FILE* fp_total;
	char buf[128];
	string str_tmp;
	vector<string> v_tmp;
	snprintf(path,128,"%s/total_info.txt",model_dir.c_str());
	fp_total=fopen(path,"r");

	fgets(buf,128,fp_total);
	str_tmp = buf;
	boost::algorithm::split(v_tmp,str_tmp,boost::is_any_of(":"));
	if(v_tmp.size() != 2)
	{
		LOG(ERROR)<<"load index error";
		return -1;
	}
	TotalClassNum = CTypeTool<int>::StrTo(v_tmp[1]);

	fgets(buf,128,fp_total);
	str_tmp = buf;
	boost::algorithm::split(v_tmp,str_tmp,boost::is_any_of(":"));
	if(v_tmp.size() != 2)
	{
		LOG(ERROR)<<"load index error";
		return -1;
	}
	TotalTermNum = CTypeTool<long long>::StrTo(v_tmp[1]);

	fgets(buf,128,fp_total);
	str_tmp = buf;
	boost::algorithm::split(v_tmp,str_tmp,boost::is_any_of(":"));
	if(v_tmp.size() != 2)
	{
		LOG(ERROR)<<"load index error";
		return -1;
	}
	TotalLearningItemNum = CTypeTool<int>::StrTo(v_tmp[1]);

	fgets(buf,128,fp_total);
	str_tmp = buf;
	boost::algorithm::split(v_tmp,str_tmp,boost::is_any_of(":"));
	if(v_tmp.size() != 2)
	{
		LOG(ERROR)<<"load index error";
		return -1;
	}
	TotalTermHashNum = CTypeTool<int>::StrTo(v_tmp[1]);

	DLOG(WARNING)<<"in read index " << " TotalClassNum " << TotalClassNum << " TotalTermNum " << TotalTermNum <<  " TotalLearningItemNum " << TotalLearningItemNum << "TotalTermHashNum" << TotalTermHashNum;

	if(term_count != TotalTermHashNum +1)
	{
		LOG(ERROR)<<"term_count do not equal total info: " << term_count << " " << TotalTermHashNum ;
	}

	return 0;

}


bool NBClassifier::GetDataFrmCache(const string line,int source_type,string &outputline)
{

	if (source_type>3 || source_type<1)
	{
		LOG(WARNING) <<"source type error:" << source_type;
		return false;
	}	
	vector<string> vstrField;
	
	boost::algorithm::split(vstrField,line,boost::is_any_of("\t"));
	if (vstrField.size()<TOTAL_FIELD_NUM)
	{
		LOG(WARNING) <<"data format error, string field :" << vstrField.size();
		return false;
	}

	string key,strbuf;
	string data_field,cache_field;

	/** have this key?*/
	key = vstrField[item_base_field_cid] + ":ItemBase:"+ vstrField[item_base_field_iid];

	mLeveldbStatus = mLeveldb->Get(leveldb::ReadOptions(), key, &strbuf);

	/** no such key*/
	if (mLeveldbStatus.IsNotFound() == true)
	{
		return false;
	}
	
	/** compare the itembase of data and cache*/
	vector<string> vcache;
	bool if_match=false;

	boost::algorithm::split(vcache,strbuf,boost::is_any_of("\t"));
	if (vcache.size()<TOTAL_FIELD_NUM)
	{
		LOG(INFO) << "cache data dirty !";
		return false;
	}
	/** js data*/
	if (source_type == 1)
	{
		data_field = js_data_merge(vstrField);
		cache_field = js_data_merge(vcache);
		/** match!*/
		if(data_field == cache_field)
		{
			if_match = true;
		}
	}
	else if (source_type == 2)
	{
		data_field = crawl_data_merge(vstrField);
		cache_field = crawl_data_merge(vcache);
		/** match!*/
		if(data_field == cache_field)
		{
			if_match = true;
		}
	}
	else if (source_type == 3)
	{
		data_field = unknown_data_merge(vstrField);
		cache_field = unknown_data_merge(vcache);
		/** match!*/
		if(data_field == cache_field)
		{
			if_match = true;
		}
	}
	if (if_match == false)
	{
		LOG(INFO) << "item base not match " << key ;
		return false;
	}
	key = vstrField[item_base_field_cid] + ":ItemProfile:"+vstrField[item_base_field_iid];
	mLeveldbStatus = mLeveldb->Get(leveldb::ReadOptions(), key, &outputline);
	if (mLeveldbStatus.IsNotFound() == true)
	{
		LOG(ERROR) << "item base and profile not come along " << key ;
		return false;
	}
	DLOG(INFO) << "get profile from cache " << key << " profile " << outputline;
	return true;
}

void NBClassifier::SetDataCache(const string line,int source_type,const string profile)
{
	if (source_type>3 || source_type<1)
	{
		LOG(WARNING) <<"source type error:" << source_type;
		return ;
	}	

	vector<string> vstrField;
	
	boost::algorithm::split(vstrField,line,boost::is_any_of("\t"));
	if (vstrField.size()<TOTAL_FIELD_NUM)
	{
		LOG(INFO) << "cache line format error" ;
		return ;
	}

	string key,strbuf;
	vector<string> Vstrbuf;

	/** write item base*/
	key = vstrField[item_base_field_cid] + ":ItemBase:"+vstrField[item_base_field_iid];

	mLeveldbStatus = mLeveldb->Get(leveldb::ReadOptions(), key, &strbuf);
	/** no such key*/
	if (mLeveldbStatus.IsNotFound() == true)
	{	
		DLOG(INFO) << "update key " << key << " with value " << line;
		mLeveldbStatus = mLeveldb->Put(leveldb::WriteOptions(), key, line);
		assert(mLeveldbStatus.ok());
	}
	else
	{
		boost::algorithm::split(Vstrbuf,strbuf,boost::is_any_of("\t"));
		if(Vstrbuf.size()<TOTAL_FIELD_NUM)
		{
			LOG(INFO) << "cache line format error" ;
			mLeveldbStatus = mLeveldb->Put(leveldb::WriteOptions(), key, line);
			assert(mLeveldbStatus.ok());
		}
		else
		{
			if(source_type == 1)
			{
				/** js data update*/
				js_data_update(vstrField,Vstrbuf);
			}
			if(source_type == 2)
			{
				/** crawl data update*/
				crawl_data_update(vstrField,Vstrbuf);
			}
			strbuf = boost::algorithm::join(Vstrbuf, "\t");
			DLOG(INFO) << "update key " << key << " with value " << strbuf;
			mLeveldbStatus = mLeveldb->Put(leveldb::WriteOptions(), key, strbuf);
			assert(mLeveldbStatus.ok());
		}
	}

	/** write item profile*/
	key = vstrField[item_base_field_cid] + ":ItemProfile:"+vstrField[item_base_field_iid];
	mLeveldbStatus = mLeveldb->Put(leveldb::WriteOptions(), key, profile);
	assert(mLeveldbStatus.ok());

}

// 清空
void NBClassifier::Clear()
{
	/*
	   ckeywordsmanager::releaseinstance();
	   ctextkeytool::releaseinstance();
	   if(m_pconfiginfo != null)
	   delete m_pconfiginfo;
	   m_pconfiginfo = null;
	   */
}




