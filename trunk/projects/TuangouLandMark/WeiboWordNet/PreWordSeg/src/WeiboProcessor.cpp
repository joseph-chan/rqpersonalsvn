#include "WeiboProcessor.h"
using namespace std;



/**
 * @brief Constructor
 */
WeiboProcessor::WeiboProcessor()
{
	m_pConfigInfo= NULL;
}

/**
 * @brief Destructor
 */
WeiboProcessor::~WeiboProcessor()
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
int WeiboProcessor::LoadWeiboToTerm()
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

/**
 * @brief 初始化实例
 *
 * @Param cResource
 *
 * @Returns   
 */
int WeiboProcessor::WBProcessorInit(const char  *pConfigFile)
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

int WeiboProcessor::LoadLexicon(const char *cLexiconPath)
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
		WeiboProcessor::ReleaseWordSeg();
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


void WeiboProcessor::ReleaseWordSeg()
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
int WeiboProcessor::ProcessOriginalData()
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
int WeiboProcessor::RecordTerm(char * term,WORD_ITEM *seg_info )
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

/**
 * @brief 处理一行json原始数据
 *
 * @Param szIine
 *
 * @Returns   
 */
int WeiboProcessor::ParseLineOriginal(string strLine,ofstream &output)
{
	map<string, string> mapJson;
	char weibo_content[1000];
	char term[100];
	string terms;
	int idf;
	//Json::Reader reader;
	//Json::Value value;

	//WeiboInfo* pWeibo = new WeiboInfo;
	int ret;
	/*
	if ( reader.parse(strLine, value)== false)
	{
		nx_log(NGX_LOG_WARN,"weibo parse fail [%s] ",strLine.c_str());
	  	return -1;
	}
	*/
	ret = CJParser::ParseJson(strLine, mapJson);
	if(ret == 0)
	{
	  return -1;
	}
	unsigned long long mid =0;
	unsigned long long uid =0;
	string strContent ;
	int flag=0;
	int len;
	int forward_count=0;
	int comment_count=0;
	unsigned int hash_value;
	vector<unsigned int> hash_values;

	//  去除转发微博
	//flag = CTypeTool<int>::StrTo(value["flag"].asString());
	flag = CTypeTool<int>::StrTo(mapJson["flag"]);
	if (flag == 1)
	{
		nx_log(NGX_LOG_DEBUG,"forward weibo [%uL] omitted.",mid);
		return 1;
	}
	
	mid = CTypeTool<unsigned long long>::StrTo(mapJson["id"]);
	//mid = CTypeTool<unsigned long long>::StrTo(value["id"].asString());

/*
	map<unsigned long long, int>::iterator iter = m_mapForwardCount.find(mid);
	if (iter != m_mapForwardCount.end())
	{
		forward_count = iter->second;
	}
	iter = m_mapCommentCount.find(mid);
	if (iter != m_mapCommentCount.end())
	{
		comment_count = iter->second;
	}
	*/

	//获取各个参数

	//uid = CTypeTool<unsigned long long>::StrTo(value["uid"].asString());
	uid = CTypeTool<unsigned long long>::StrTo(mapJson["uid"]);
	strContent = mapJson["content"];
	//strContent = value["content"].asString();
	if ( strContent.size() < 1)
	{
		nx_log(NGX_LOG_NOTICE,"parse weibo error, content is NULL [%uL]",mid);
		return 1;
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
	terms = terms + mapJson["id"] + "\t";

	for(int i= 0; i< m_Wordseg->listitem_num; i++)
	{
		len = uniToBytes(m_Wordseg->uni+(m_Wordseg->list[i].word)->wordPos,(m_Wordseg->list[i].word)->wordLen,term,1024,"GBK");
		term[len]='\0';
		
		idf = int (float(m_Wordseg->list[i].word->idf) / 10.0);
		// do not record the term whose idf is zero
		if (idf <= 2 || len <=2 || false == IsGoodTerm(m_Wordseg->list[i].word->postagid))
		{
			nx_log(NGX_LOG_DEBUG,"low idf [%d] or short term [%d] or bad term [%d] term [%s]",idf,len,m_Wordseg->list[i].word->postagid,term);
			continue;
		}
		
		nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->list[i].word->postagid);
		terms = terms + term + "\t";
		
	}
	if(terms.size()>0 and terms[terms.size()-1] == '\t')
	{
		terms.erase(terms.size()-1);
	}
	output << terms <<endl;
	return 0;

}




// 清空
void WeiboProcessor::Clear()
{
	/*
	CKeyWordsManager::ReleaseInstance();
	CTextKeyTool::ReleaseInstance();
	if(m_pConfigInfo != NULL)
	delete m_pConfigInfo;
	m_pConfigInfo = NULL;
	 */
}

//通用的 IsGoodTerm
bool WeiboProcessor::IsGoodTerm(int postagid)
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

// 判断微博是否有重复的
bool WeiboProcessor::HaveDuplicates(string& strContent)
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
