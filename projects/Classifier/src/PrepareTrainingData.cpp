#include "PrepareTrainingData.h"
using namespace std;


bool CmpTerm(pair<string,u_int> term1, pair<string,u_int> term2)
{   
    return term1.second > term2.second;
}  
bool CmpPairInt(pair<u_int,u_int> term1, pair<u_int,u_int> term2)
{   
    return term1.first < term2.first;
}  

/**
 * @brief Constructor
 */
PrepareTrainingData::PrepareTrainingData()
{
	m_pConfigInfo= NULL;
}

/**
 * @brief Destructor
 */
PrepareTrainingData::~PrepareTrainingData()
{
	Clear();
}
/**
 * @brief 读入所有微博数据
 *
 * @Param cResource
 *
 * @Returns   
 */
int PrepareTrainingData::WeiboToTrainingData()
{
	int ret=0;
	/*
	ret = ProcessForwardData();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"ProcessForwardData error");
		return -1;
	}
	ret = ProcessCommentData();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"ProcessCommentData error");
		return -1;
		}

	 */

	ret = ProcessOriginalData();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"ProcessOriginalData error");
		return -1;
	}
	return 0;
}

int PrepareTrainingData::GetNBModel()
{
	int ret=0;
	const char * szInputGood;
	const char * szInputBad;
	// 初始化山寨json解析器
	vector<string> json_fields;
	json_fields.push_back("flag");
	json_fields.push_back("id");
	json_fields.push_back("uid");
	json_fields.push_back("filter");
	json_fields.push_back("content");
	json_fields.push_back("yc_source");
	CJParser::InitParser(json_fields);

	szInputGood=m_pConfigInfo->GetValue("good_data").c_str();
	szInputBad=m_pConfigInfo->GetValue("bad_data").c_str();
	ifstream fin_good(szInputGood);
	ifstream fin_bad(szInputBad);
	if(!fin_good.is_open() || !fin_bad.is_open())
	{
		nx_log(NGX_LOG_ERR,"can not open input file");
		return -1;
	}
	string line;
	int line_num=0;
	while(!fin_good.eof())
	{
		++line_num;
		getline(fin_good, line);
		if(line.size() == 0)
		  continue;
		ret = GetNBFrmOriginalLine(line,true);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"original data line parse fail");
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
	}

	while(!fin_bad.eof())
	{
		++line_num;
		getline(fin_bad, line);
		if(line.size() == 0)
		  continue;
		ret = GetNBFrmOriginalLine(line,false);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"original data line parse fail");
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
	}
	fin_good.close();
	fin_bad.close();
	return 0;

}

int PrepareTrainingData::GetTopTerms()
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
		nx_log(NGX_LOG_ERR,"can not open input file");
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
			nx_log(NGX_LOG_ERR,"original data line parse fail");
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
	}
	fin.close();
	return 0;

}
/**
 * @brief 初始化实例
 *
 * @Param cResource
 *
 * @Returns   
 */
int PrepareTrainingData::WBProcessorInit(const char  *pConfigFile)
{
	int ret =0 ;

	m_pConfigInfo = new CConfigInfo(pConfigFile);


	// 装载其他资源
	string strYCConfig = m_pConfigInfo->GetValue("yc_source_conf");
	if(strYCConfig.size() == 0)
	{
		nx_log(NGX_LOG_ERR,"load resource error");
		return -1;
	}

    strYCConfig = m_pConfigInfo->GetValue("daren_resource");
	m_pDarenResource = new CResource(strYCConfig.c_str(),3);

	strYCConfig = m_pConfigInfo->GetValue("vip_resource");
    m_pVipResource = new CResource(strYCConfig.c_str(),4);

	strYCConfig = m_pConfigInfo->GetValue("yc_source_conf");
    m_pYcResource = new CResource(strYCConfig.c_str(),2);

	string strLexiconConf = m_pConfigInfo->GetValue("seg_data");

	ret = LoadLexicon(strLexiconConf.c_str());
	if (ret <0)
	{
		return -1;
	}
	/*
	
	// 加载需要处理的分类信息
	string tmp_string = m_pConfigInfo->GetValue("types_required");
	vector<string> type_string = CStringTool::SpliteByChar(tmp_string,'\t');
	if(type_string.size() < 1)
	{
		nx_log(NGX_LOG_ERR,"classified data format error");
		return -1;
	}
	for(vector<string>::iterator iter = type_string.begin(); iter != type_string.end(); iter++)
	{
		int tmp;
		tmp = CTypeTool<int>::StrTo(*iter);
		m_types.push_back(tmp);
		nx_log(NGX_LOG_DEBUG,"add type: [%d]",tmp);
	}
	*/
	// 初始化文本指纹产生器，用于排重
	return CTextKeyTool::GetInstance()->InitInstance(m_pConfigInfo->GetValue("seg_data").c_str(), 31);
}

int PrepareTrainingData::LoadLexicon(const char *cLexiconPath)
{
	// 打开词典 
	m_Lexicon = OpenLexicon_Opt(cLexiconPath, 0x1F);//第一个参数，即根目录，词典
	if(m_Lexicon == NULL)
	{
		nx_log(NGX_LOG_ERR,"load lexicon error");
		return -1;
	}
	// 设置语种 
	setLexiconWS(m_Lexicon, 0);
	//存放分词结果的空间
	m_Wordseg = InitWordSeg(1024);			//只初始化一次即可	
	if(m_Wordseg==NULL)
	{
		PrepareTrainingData::ReleaseWordSeg();
		return -2;
	}
	//step1:设置分词的参数选项
	unsigned short segopt = SEG_OPT_ALL | SEG_OPT_ZUHEQIYI | SEG_OPT_PER | SEG_OPT_SUFFIX;;
	unsigned short postagopt = 0x7FFF;
	unsigned short appopt = AOP_TERM_ALL;
	unsigned short lexiconopt = 0x0104;
				
	m_WordSegFlags = lexiconopt;
	m_WordSegFlags = m_WordSegFlags<<16;
	m_WordSegFlags = m_WordSegFlags | appopt;
	m_WordSegFlags = m_WordSegFlags<<16;
	m_WordSegFlags = m_WordSegFlags | postagopt;
	m_WordSegFlags = m_WordSegFlags<<16;		
	m_WordSegFlags = m_WordSegFlags | segopt;

	return 0;
}


void PrepareTrainingData::ReleaseWordSeg()
{
	if(m_Wordseg != NULL)
	{
		FreeWordSeg(m_Wordseg);
		m_Wordseg = NULL;
	}
				
	if(m_Lexicon != NULL)
	{		
		CloseLexicon(m_Lexicon);
		m_Lexicon = NULL;
	}
}



/**
 * @brief 处理原始json数据
 *
 * @Returns   >=0 正常
 *            <0  错误
 */
int PrepareTrainingData::ProcessOriginalData()
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
		nx_log(NGX_LOG_ERR,"can not open input file");
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
			nx_log(NGX_LOG_ERR,"original data line parse fail");
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
	}
	fin.close();
	return 0;

}

/*
int PrepareTrainingData::RecordTerm(char * term,WORD_ITEM *seg_info )
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
		nx_log(NGX_LOG_NOTICE,"term [%s] is bad ,postagid [%d] ",term,seg_info->postagid);
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
			nx_log(NGX_LOG_ALERT,"mTermInfo size may be error size [%d] m_term_num [%d]",mTermInfo.size(), m_total_info.term_num);
			return -1;
		}
		nx_log(NGX_LOG_DEBUG,"new term added: term [%s] ",term);
	}
	else
	{
		term_index=iterhm->second;
		mTermInfo[term_index].df_now += is_good_term ;
		nx_log(NGX_LOG_DEBUG,"term update: term [%s] df_now [%d] df_his [%d] relate_term_num [%d]",mTermInfo[term_index].termstr,mTermInfo[term_index].df_now,mTermInfo[term_index].df_history,mTermInfo[term_index].relate_term_num);
	}
	delete local_term_info;
	return 0;

}
*/
int PrepareTrainingData::SortTerms()
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
int PrepareTrainingData::NBPredict()
{
	int ret=0;
	const char * szInputPredict;
	const char * szOutputPredict;
	// 初始化山寨json解析器
	vector<string> json_fields;
	json_fields.push_back("flag");
	json_fields.push_back("id");
	json_fields.push_back("uid");
	json_fields.push_back("filter");
	json_fields.push_back("content");
	json_fields.push_back("yc_source");
	CJParser::InitParser(json_fields);

	szInputPredict=m_pConfigInfo->GetValue("predict_file").c_str();
	szOutputPredict=m_pConfigInfo->GetValue("predict_output").c_str();
	ifstream fin_predict(szInputPredict);
	ofstream fout_predict(szOutputPredict);
	if(!fin_predict.is_open() || !fout_predict.is_open())
	{
		nx_log(NGX_LOG_ERR,"can not open input file");
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
		ret = NBPredictLine(line,fout_predict);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"original data line parse fail");
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
	}

	fin_predict.close();
	return 0;

}
int PrepareTrainingData::GetNBFrmOriginalLine(string strLine,bool if_positive)
{
	map<string, string> mapJson;
	char weibo_content[1000];
	char term[100];
	string strContent ;
	int idf;
	int source_type=0; //json
	int ret;
	source_type = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("NBsource_file_type"));
	//Json::Reader reader;
	//Json::Value value;
	if(source_type == 0)
	{
		int flag=0;
		ret = CJParser::ParseJson(strLine, mapJson);
		if(ret == 0)
		{
		  return -1;
		}

		//  去除转发微博
		flag = CTypeTool<int>::StrTo(mapJson["flag"]);
		if (flag == 1)
		{
			nx_log(NGX_LOG_DEBUG,"forward weibo omitted.");
			return 1;
		}
		strContent = mapJson["content"];
		if ( strContent.size() < 1)
		{
			nx_log(NGX_LOG_NOTICE,"parse weibo error, content is NULL ");
			return 1;
		}
		
	}
	else
	{
		strContent = strLine;
	}

	// 排重
	if(HaveDuplicates(strContent))
	{
		nx_log(NGX_LOG_DEBUG,"duplicates: [%s].",strContent.c_str());
		return 1;
	}
	// 删除一些无用信息

	CMBTextTool::KillNickName(strContent);
	CMBTextTool::KillURL(strContent);
	//static vector<string> vecTopic;
	//vecTopic.clear();
	//CMBTextTool::KillTopic(strContent, vecTopic);
	CMBTextTool::KillEmotion(strContent);
	//CMBTextTool::KillIntrInfo(strContent);
	
	snprintf(weibo_content,1000,"%s",strContent.c_str());

	if (if_positive == true)
	{
		mNBTotalInfo.positive ++;
	}
	else
	{
		mNBTotalInfo.negative ++;
	}

	ret = AnalyTextWithLex2(m_Lexicon,weibo_content , strlen(weibo_content), "GBK", m_Wordseg, m_WordSegFlags,1);
	if(ret < 0)
	{
		nx_log(NGX_LOG_ALERT,"AnalyTextWithLex2 error [%d] weibo [%s]",ret,weibo_content);
		return -1;
	}
	//recurse every term ,record the term info
	//record wordlist

	int len;
	for(int i= 0; i< m_Wordseg->listitem_num; i++)
	{
		len = uniToBytes(m_Wordseg->uni+(m_Wordseg->list[i].word)->wordPos,(m_Wordseg->list[i].word)->wordLen,term,1024,"GBK");
		term[len]='\0';
		
		idf = int (float(m_Wordseg->list[i].word->idf) / 10.0);
		// do not record the term whose idf is zero
		if (idf <= 2 || strlen(term) <3 )
			//||IsGoodTerm(m_Wordseg->list[i].word->postagid) == false)
		{
			nx_log(NGX_LOG_DEBUG,"low idf [%d] len [%d] postagid [%d]",idf,strlen(term),m_Wordseg->list[i].word->postagid,term);
			continue;
		}
		nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->list[i].word->postagid);
		if (mNBTerms.find(term)== mNBTerms.end())
		{
			term_info_t tmp_term_info;
			tmp_term_info.seg_idf=idf;
			tmp_term_info.is_good_term = int(IsGoodTerm(m_Wordseg->list[i].word->postagid));
			if (if_positive == true)
			{
				tmp_term_info.positive = 1;
				tmp_term_info.negative = 0;
			}
			else
			{
				tmp_term_info.negative = 1;
				tmp_term_info.positive = 0;
			}

			mNBTerms.insert(pair<string,term_info_t>(term,tmp_term_info));
		}
		else
		{
			mNBTerms[term].seg_idf = (mNBTerms[term].seg_idf + idf) / 2;
			if (if_positive == true)
			{
				mNBTerms[term].positive +=1;
			}
			else
			{
				mNBTerms[term].negative += 1;
			}
			mNBTerms[term].is_good_term += int(IsGoodTerm(m_Wordseg->list[i].word->postagid));

		}
	}
	
	return 0;

}

int PrepareTrainingData::NBPredictLine(string strLine,ofstream &fout_predict)
{
	map<string, string> mapJson;
	map<string,u_int> local_term_count;
	char weibo_content[1000];
	char term[100];
	string strContent,strUid;
	string strBuf;
    vector<string> vVipInfo;
    vector<string> vDarenInfo;
    vector<string> vBuf;
	int idf;
	int source_type=0; //json
	int ret;
	unsigned long long mid;
	source_type = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("predict_file_type"));
	//Json::Reader reader;
	//Json::Value value;
	if(source_type == 0)
	{
		int flag=0;
		ret = CJParser::ParseJson(strLine, mapJson);
		if(ret == 0)
		{
		  return -1;
		}

		//  去除转发微博
		flag = CTypeTool<int>::StrTo(mapJson["flag"]);
		if (flag == 1)
		{
			nx_log(NGX_LOG_DEBUG,"forward weibo omitted.");
			return 1;
		}
		
		strUid = mapJson["uid"];

        m_pVipResource->GetValue(strUid.c_str(),vVipInfo);
        m_pDarenResource->GetValue(strUid.c_str(),vDarenInfo);
		if (vVipInfo.size() == 3)
		{
			string VipType;
			int fans_num;
			fans_num = CTypeTool<int>::StrTo(vVipInfo[0]);
			VipType = vVipInfo[1];

			if(fans_num > 10000 || VipType != "名人")
			{
				nx_log(NGX_LOG_NOTICE,"vip weibo,too many fans [%d] or not people [%s],uid [%s] ",fans_num,VipType.c_str(),strUid.c_str());
				return 1;

			}
		}
		else if (vDarenInfo.size() == 2)
		{
			int fans_num;
			int mutual_fans_num;

			fans_num = CTypeTool<int>::StrTo(vDarenInfo[0]);
			mutual_fans_num = CTypeTool<int>::StrTo(vDarenInfo[1]);
			if (mutual_fans_num <=0 || fans_num <=0 ||float(mutual_fans_num)/float(fans_num) < 0.02)
			{
				nx_log(NGX_LOG_NOTICE,"daren weibo,fans illegal,fans_num [%d] mutual_fans [%d] uid %s",fans_num,mutual_fans_num,strUid.c_str());
				return 1;
			}
		}

		strBuf = mapJson["yc_source"];
        m_pYcResource->GetValue(strBuf.c_str(),vBuf);
		if (vBuf.size() == 1)
		{
			nx_log(NGX_LOG_NOTICE,"illegal yc_source [%s],uid %s",strBuf.c_str(),strUid.c_str());
			return 1;
		}


		strContent = mapJson["content"];
		if ( strContent.size() < 1)
		{
			nx_log(NGX_LOG_NOTICE,"parse weibo error, content is NULL ");
			return 1;
		}
		mid = CTypeTool<unsigned long long>::StrTo(mapJson["id"]);
		
	}
	else
	{
		strContent = strLine;
	}
	// 排重
	if(HaveDuplicates(strContent))
	{
		nx_log(NGX_LOG_DEBUG,"duplicates: [%s].",strContent.c_str());
		return 1;
	}

	// 删除一些无用信息

	CMBTextTool::KillNickName(strContent);
	CMBTextTool::KillURL(strContent);
	//static vector<string> vecTopic;
	//vecTopic.clear();
	//CMBTextTool::KillTopic(strContent, vecTopic);
	CMBTextTool::KillEmotion(strContent);
	//CMBTextTool::KillIntrInfo(strContent);
	
	snprintf(weibo_content,1000,"%s",strContent.c_str());

	ret = AnalyTextWithLex2(m_Lexicon,weibo_content , strlen(weibo_content), "GBK", m_Wordseg, m_WordSegFlags,1);
	if(ret < 0)
	{
		nx_log(NGX_LOG_ALERT,"AnalyTextWithLex2 error [%d] weibo [%s]",ret,weibo_content);
		return -1;
	}
	//recurse every term ,record the term info
	//record wordlist

	int len;
	double score;
	score= log(1.0 + (double)mNBTotalInfo.positive /((double)mNBTotalInfo.positive + (double)mNBTotalInfo.negative));

	nx_log(NGX_LOG_DEBUG,"mid [%uL] predict begin [%.4f] total_pos [%d] total_neg [%d]",mid,score,mNBTotalInfo.positive,mNBTotalInfo.negative);
	local_term_count.clear();
	for(int i= 0; i< m_Wordseg->listitem_num; i++)
	{
		len = uniToBytes(m_Wordseg->uni+(m_Wordseg->list[i].word)->wordPos,(m_Wordseg->list[i].word)->wordLen,term,1024,"GBK");
		term[len]='\0';
		
		if (local_term_count.find(term) == local_term_count.end())
		{
			local_term_count[term] =1;
		}
		else
		{
			++local_term_count[term];
			if(local_term_count[term] >=3)
			{
				continue;
			}

		}
		if (mNBTerms.find(term)== mNBTerms.end())
		{
			nx_log(NGX_LOG_DEBUG,"do not have this term [%s] in model ",term);
		}
		else
		{
			if ((mNBTerms[term].positive + mNBTerms[term].negative) < 10 || mNBTerms[term].positive == 0)
			{
				continue;
				nx_log(NGX_LOG_DEBUG,"too little term [%s] [%d]",term,mNBTerms[term].positive + mNBTerms[term].negative);
			}
			double term_score;
			term_score = log (1.0 + ((double)mNBTerms[term].positive )/ ( (double)mNBTerms[term].positive + (double)mNBTerms[term].negative));
			score += term_score;
			nx_log(NGX_LOG_DEBUG,"mid [%uL] term [%s] term_score [%.4f] score [%.4f] term_pos [%d] term_neg [%d]",mid,term,term_score,score,mNBTerms[term].positive,mNBTerms[term].negative);
		}
	}
	
	fout_predict << score << "\t" << strLine << endl;
	
	return 0;

}
int PrepareTrainingData::GetTermFrmOriginalLine(string strLine)
{
	map<string, string> mapJson;
	char weibo_content[1000];
	char term[100];
	string strContent ;
	int idf;
	int source_type=0; //json
	int ret;
	source_type = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("source_file_type"));
	//Json::Reader reader;
	//Json::Value value;
	if(source_type == 0)
	{
		int flag=0;
		ret = CJParser::ParseJson(strLine, mapJson);
		if(ret == 0)
		{
		  return -1;
		}

		//  去除转发微博
		flag = CTypeTool<int>::StrTo(mapJson["flag"]);
		if (flag == 1)
		{
			nx_log(NGX_LOG_DEBUG,"forward weibo omitted.");
			return 1;
		}
		strContent = mapJson["content"];
		if ( strContent.size() < 1)
		{
			nx_log(NGX_LOG_NOTICE,"parse weibo error, content is NULL ");
			return 1;
		}
		
	}
	else
	{
		strContent = strLine;
	}

	// 排重
	if(HaveDuplicates(strContent))
	{
		nx_log(NGX_LOG_DEBUG,"duplicates: [%s].",strContent.c_str());
		return 1;
	}
	// 删除一些无用信息

	CMBTextTool::KillNickName(strContent);
	CMBTextTool::KillURL(strContent);
	//static vector<string> vecTopic;
	//vecTopic.clear();
	//CMBTextTool::KillTopic(strContent, vecTopic);
	CMBTextTool::KillEmotion(strContent);
	//CMBTextTool::KillIntrInfo(strContent);
	
	snprintf(weibo_content,1000,"%s",strContent.c_str());

	ret = AnalyTextWithLex2(m_Lexicon,weibo_content , strlen(weibo_content), "GBK", m_Wordseg, m_WordSegFlags,1);
	if(ret < 0)
	{
		nx_log(NGX_LOG_ALERT,"AnalyTextWithLex2 error [%d] weibo [%s]",ret,weibo_content);
		return -1;
	}
	//recurse every term ,record the term info
	//record wordlist

	int len;
	for(int i= 0; i< m_Wordseg->listitem_num; i++)
	{
		len = uniToBytes(m_Wordseg->uni+(m_Wordseg->list[i].word)->wordPos,(m_Wordseg->list[i].word)->wordLen,term,1024,"GBK");
		term[len]='\0';
		
		idf = int (float(m_Wordseg->list[i].word->idf) / 10.0);
		if (idf ==0)
		{
			++idf;
		}
		// do not record the term whose idf is zero
		//if (idf <= 3 || strlen(term) <3 ||IsGoodTermTmp(m_Wordseg->list[i].word->postagid) == false)
		if (strlen(term) <3 ||IsGoodTermTmp(m_Wordseg->list[i].word->postagid) == false)
		{
			nx_log(NGX_LOG_DEBUG,"low idf [%d] len [%d] postagid [%d]",idf,strlen(term),m_Wordseg->list[i].word->postagid,term);
			continue;
		}
		nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->list[i].word->postagid);
		if (mTopTerms.find(term)== mTopTerms.end())
		{
			mTopTerms.insert(pair<string,u_int>(term,idf));
		}
		else
		{
			mTopTerms[term] += idf;
		}
	}
	for(int i= 0; i< m_Wordseg->appword_num; i++)
	{
		len = uniToBytes(m_Wordseg->uni+(m_Wordseg->appwordlist[i].word_item).wordPos,(m_Wordseg->appwordlist[i].word_item).wordLen,term,1024,"GBK");
		term[len]='\0';
		
		idf = int (float(m_Wordseg->appwordlist[i].word_item.idf) / 10.0);
		// do not record the term whose idf is zero
		if (idf <= 3 || IsGoodTermTmp(m_Wordseg->appwordlist[i].word_item.postagid) == false)
		{
			nx_log(NGX_LOG_DEBUG,"low idf [%d] postagid [%d]",idf,m_Wordseg->appwordlist[i].word_item.postagid,term);
			continue;
		}
		nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->appwordlist[i].word_item.postagid);
		if (mTopTerms.find(term)== mTopTerms.end())
		{
			mTopTerms.insert(pair<string,u_int>(term,idf));
		}
		else
		{
			mTopTerms[term]+= idf;
		}
	}
	
	return 0;

}
/**
 * @brief 处理一行json原始数据
 *
 * @Param szIine
 *
 * @Returns   
 */
int PrepareTrainingData::ParseLineOriginal(string strLine,ofstream &output)
{
	map<string, string> mapJson;
	char weibo_content[1000];
	char term[100];
	string strContent ;
	int idf;
	int source_type=0; //json
	int ret;
	source_type = CTypeTool<int>::StrTo(m_pConfigInfo->GetValue("source_file_type"));
	//Json::Reader reader;
	//Json::Value value;
	if(source_type == 0)
	{
		int flag=0;
		ret = CJParser::ParseJson(strLine, mapJson);
		if(ret == 0)
		{
		  return -1;
		}

		//  去除转发微博
		flag = CTypeTool<int>::StrTo(mapJson["flag"]);
		if (flag == 1)
		{
			nx_log(NGX_LOG_DEBUG,"forward weibo omitted.");
			return 1;
		}
		strContent = mapJson["content"];
		if ( strContent.size() < 1)
		{
			nx_log(NGX_LOG_NOTICE,"parse weibo error, content is NULL ");
			return 1;
		}
		
	}
	else
	{
		strContent = strLine;
	}

	// 排重
	if(HaveDuplicates(strContent))
	{
		nx_log(NGX_LOG_DEBUG,"duplicates: [%s].",strContent.c_str());
		return 1;
	}
	// 删除一些无用信息

	CMBTextTool::KillNickName(strContent);
	CMBTextTool::KillURL(strContent);
	//static vector<string> vecTopic;
	//vecTopic.clear();
	//CMBTextTool::KillTopic(strContent, vecTopic);
	CMBTextTool::KillEmotion(strContent);
	//CMBTextTool::KillIntrInfo(strContent);
	
	snprintf(weibo_content,1000,"%s",strContent.c_str());

	ret = AnalyTextWithLex2(m_Lexicon,weibo_content , strlen(weibo_content), "GBK", m_Wordseg, m_WordSegFlags,1);
	if(ret < 0)
	{
		nx_log(NGX_LOG_ALERT,"AnalyTextWithLex2 error [%d] weibo [%s]",ret,weibo_content);
		return -1;
	}
	//recurse every term ,record the term info
	//record wordlist
	map<u_int,u_int> line_output;
	int hash_value;
	int weight;
	int len;

	for(int i= 0; i< m_Wordseg->listitem_num; i++)
	{
		len = uniToBytes(m_Wordseg->uni+(m_Wordseg->list[i].word)->wordPos,(m_Wordseg->list[i].word)->wordLen,term,1024,"GBK");
		term[len]='\0';
		
		idf = int (float(m_Wordseg->list[i].word->idf) / 10.0);
		// do not record the term whose idf is zero
		if (idf <= 3 )
		{
			nx_log(NGX_LOG_DEBUG,"low idf [%d] postagid [%d]",idf,m_Wordseg->list[i].word->postagid,term);
			continue;
		}
		nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->list[i].word->postagid);
		hash_value = hashlittle(term,strlen(term),1);
		weight = 1;
		if (line_output.find(hash_value)== line_output.end())
		{
			line_output[hash_value] = weight;
		}
		else
		{
			line_output[hash_value] += weight;
		}
	}
	vector<pair<u_int,u_int> > vtmp;
	map<u_int, u_int>::iterator iter_output;
	for(iter_output = line_output.begin();iter_output != line_output.end(); iter_output++)
	{
		vtmp.push_back(pair<u_int,u_int>( (iter_output->first % 2147483647),iter_output->second));
	}

	sort(vtmp.begin(),vtmp.end(),CmpPairInt);

	vector<pair<u_int,u_int> >::iterator iter_vtmp;
	output << m_pConfigInfo->GetValue("data_class") << " ";
	for(iter_vtmp = vtmp.begin();iter_vtmp != vtmp.end(); iter_vtmp++)
	{
		output<< iter_vtmp->first << ":" << iter_vtmp->second << " ";
	}

	output << endl;
	
	return 0;

}




// 清空
void PrepareTrainingData::Clear()
{
	/*
	CKeyWordsManager::ReleaseInstance();
	CTextKeyTool::ReleaseInstance();
	if(m_pConfigInfo != NULL)
	delete m_pConfigInfo;
	m_pConfigInfo = NULL;
	*/
}
bool PrepareTrainingData::IsGoodTermTmp(int postagid)
{
	if ( (postagid >= POSTAG_ID_N && postagid <= POSTAG_ID_N_RB && postagid != POSTAG_ID_NS_Z) || 
			(postagid >= POSTAG_ID_V && postagid <=POSTAG_ID_V_E)  ||
			(postagid == POSTAG_ID_V_Q)  ||
			(postagid == POSTAG_ID_AN)  ||
			(postagid == POSTAG_ID_Z)  ||
			(postagid == POSTAG_ID_SP )  ||
			(postagid == POSTAG_ID_VD  )  ||
			(postagid == POSTAG_ID_VN  ) 
	   )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//通用的 IsGoodTerm
bool PrepareTrainingData::IsGoodTerm(int postagid)
{
	if ( (postagid <= POSTAG_ID_A) || 
			(postagid >= POSTAG_ID_H_T && postagid <= POSTAG_ID_H_N)  ||
			(postagid >= POSTAG_ID_N && postagid <= POSTAG_ID_N_RB)  ||
			(postagid == POSTAG_ID_Q_H)  ||
			(postagid == POSTAG_ID_T_Z)  ||
			(postagid >= POSTAG_ID_V && postagid <= POSTAG_ID_V_Q)  ||
			(postagid == POSTAG_ID_Z)  ||
			(postagid >= POSTAG_ID_X && postagid <= POSTAG_ID_VN)
	   )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
bool PrepareTrainingData::IsGoodTerm_20120109(int postagid)
{
	if (postagid <= POSTAG_ID_B || 
			(postagid == POSTAG_ID_T_Z)  ||
			(postagid == POSTAG_ID_Z)  ||
			(postagid == POSTAG_ID_AN)  ||
			(postagid == POSTAG_ID_VN)  ||
			(postagid >= POSTAG_ID_M &&  postagid <= POSTAG_ID_NS_Z))
	{
		return true;
	}
	else
	{
		return false;
	}
}
*/

// 判断微博是否有重复的
bool PrepareTrainingData::HaveDuplicates(string& strContent)
{
	static unsigned long long key = 0l;
	key = 0l;
	if(!CTextKeyTool::GetInstance()->GetKey(strContent.c_str(), key, 8))
		return false;
	if(m_setContentKey.count(key))
		return true;
	m_setContentKey.insert(key);
	return false;
}
