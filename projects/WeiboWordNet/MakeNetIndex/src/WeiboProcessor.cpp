#include "WeiboProcessor.h"
using namespace std;

/**
 * @brief 排序算法比较函数
 *
 * @Param weibo1
 * @Param weibo2
 *
 * @Returns   
 */
bool CmpWeibo(pair<unsigned long long, int> weibo1, pair<unsigned long long, int> weibo2)
{   
    return weibo1.second > weibo2.second;
}  


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
int WeiboProcessor::LoadWeiboToIndex()
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
	m_term_num=0;
	m_relate_term_num=0;
	m_weibo_num=0;


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
	if(!fin.is_open())
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
		ret = ParseLineOriginal(line);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"original data line parse fail");
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
	}
	fin.close();
	return 0;

}

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

		mTermIndex[hash_value] = m_term_num;

		// term count ++
		mTermInfo.push_back(*local_term_info);
		m_term_num ++;
		if (mTermInfo.size() != m_term_num)
		{
			nx_log(NGX_LOG_ALERT,"mTermInfo size may be error size [%d] m_term_num [%d]",mTermInfo.size(), m_term_num);
			return -1;
		}
		nx_log(NGX_LOG_NOTICE,"new term added: term [%s] ",term);
	}
	else
	{
		term_index=iterhm->second;
		mTermInfo[term_index].df_now += is_good_term ;
		nx_log(NGX_LOG_NOTICE,"term update: term [%s] df_now [%d] df_his [%d] relate_term_num [%d]",mTermInfo[term_index].termstr,mTermInfo[term_index].df_now,mTermInfo[term_index].df_history,mTermInfo[term_index].relate_term_num);
	}
	delete local_term_info;
	return 0;

}

/**
 * @brief 处理一行json原始数据
 *
 * @Param szIine
 *
 * @Returns   
 */
int WeiboProcessor::ParseLineOriginal(string strLine)
{
	map<string, string> mapJson;
	char weibo_content[1000];
	char term[100];
	int idf;

	//WeiboInfo* pWeibo = new WeiboInfo;
	int ret;
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
	flag = CTypeTool<int>::StrTo(mapJson["flag"]);
	if (flag == 1)
	{
		nx_log(NGX_LOG_NOTICE,"forward weibo [%uL] omitted.",mid);
		return 1;
	}
	
	mid = CTypeTool<unsigned long long>::StrTo(mapJson["id"]);

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

	uid = CTypeTool<unsigned long long>::StrTo(mapJson["uid"]);
	strContent = mapJson["content"];

	// 排重
	/*
	if(HaveDuplicates(strContent))
	{
		nx_log(NGX_LOG_DEBUG,"duplicates: [%s].",strContent.c_str());
		return 1;
	}
	*/
	// 删除一些无用信息

	static map<string, int> mapNames;
	mapNames.clear();
	CMBTextTool::ExtractNickName(strContent, mapNames, true);
	mapNames.clear();
	CMBTextTool::KillURL(strContent, mapNames);
	static vector<string> vecTopic;
	vecTopic.clear();
	//CMBTextTool::KillTopic(strContent, vecTopic);
	CMBTextTool::KillEmotion(strContent);
	//CMBTextTool::KillIntrInfo(strContent);

	__gnu_cxx::hash_map<u_int,u_int>::iterator iterhm_local;

	snprintf(weibo_content,1000,"%s",strContent.c_str());
	hmInWeibo.clear();
	//vTermInWeibo.clear();

	ret = AnalyTextWithLex2(m_Lexicon,weibo_content , strlen(weibo_content), "GBK", m_Wordseg, m_WordSegFlags,1);
	if(ret < 0)
	{
		nx_log(NGX_LOG_ALERT,"AnalyTextWithLex2 error [%d] weibo [%s]",ret,weibo_content);
		return -1;
	}
	//recurse every term ,record the term info
	//record wordlist
	for(int i= 0; i< m_Wordseg->listitem_num; i++)
	{
		len = uniToBytes(m_Wordseg->uni+(m_Wordseg->list[i].word)->wordPos,(m_Wordseg->list[i].word)->wordLen,term,1024,"GBK");
		term[len]='\0';
		
		idf = int (float(m_Wordseg->list[i].word->idf) / 10.0);
		// do not record the term whose idf is zero
		if (idf <= 3 || len <=3)
		{
			nx_log(NGX_LOG_NOTICE,"low idf [%d] or short term [%d] term [%s]",idf,len,term);
			continue;
		}
		
		nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->list[i].word->postagid);

		hash_value=__gnu_cxx::__stl_hash_string(term);
		
		iterhm_local = hmInWeibo.find(hash_value);
		if (iterhm_local == hmInWeibo.end())
		{ //this term turned out first time in this weibo
			ret= RecordTerm(term,m_Wordseg->list[i].word);
			if (ret < 0)
			{
				nx_log(NGX_LOG_ALERT,"record term error [%s]",term);
				continue;
			}

			hash_values.push_back(hash_value);

			hmInWeibo[hash_value] = 1;
		}
	}

	AddCooc(hash_values);

	m_weibo_num++;
	return 0;

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

bool WeiboProcessor::IsGoodTerm(int postagid)
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

int WeiboProcessor::AddCooc(vector<unsigned int> &hash_values)
{
	int ret;
	u_int term_index;
	__gnu_cxx::hash_map<u_int,u_int>::iterator iterhm;
	for(int i=0;i<hash_values.size();i++)
	{
		term_index=mTermIndex[hash_values[i]];	
		for(int j=0;j<hash_values.size();j++)
		{
			if (i==j)
			{
				continue;
			}
			else
			{
				iterhm = mTermInfo[term_index].cooc_term.find(hash_values[j]);
				
				if(iterhm == mTermInfo[term_index].cooc_term.end())
				{
					//i and j co-occurrence for the first time
					relate_term_info_t tmp_relate_info;

					tmp_relate_info.num_now=1;
					tmp_relate_info.num_history=0;
					tmp_relate_info.relation=0;

					m_relate_term_num++;
					mRelateInfo.push_back(tmp_relate_info);

					mTermInfo[term_index].cooc_term[hash_values[j]] = m_relate_term_num;
					mTermInfo[term_index].relate_term_num++ ;

					nx_log(NGX_LOG_NOTICE,"term [%s] term [%s] cooc for the first time, relate_term_num [%d]",mTermInfo[term_index].termstr,mTermInfo[mTermIndex[hash_values[j]]].termstr,mTermInfo[term_index].relate_term_num);
				}
				else
				{
					int relate_index;
					relate_index=iterhm->second;
					mRelateInfo[relate_index].num_now ++;
					nx_log(NGX_LOG_NOTICE,"term [%s] term [%s] cooc [%d]",mTermInfo[term_index].termstr,mTermInfo[mTermIndex[hash_values[j]]].termstr,mRelateInfo[relate_index].num_now);
				}

			}
		}

	}
	return 0;
}

int WeiboProcessor::DumpIndex()
{
	FILE* fp_index1;
	FILE* fp_index2;
	FILE* fp_indexn;
	char buf[128];
	int ret1,ret2;

	fp_index1=fopen("word_net.ind1","w");
	fp_index2=fopen("word_net.ind2","w");
	fp_indexn=fopen("word_net.n","w");
	if (NULL ==fp_index1 || NULL == fp_index2 || NULL == fp_indexn )
	{
		nx_log(NGX_LOG_ERR,"dump index error: can't not open file");
		return -1;
	}

	__gnu_cxx::hash_map<u_int,u_int>::iterator hmIndex;


	for(hmIndex= mTermIndex.begin();hmIndex!= mTermIndex.end();hmIndex++)
	{
		ret1 = fwrite(&(hmIndex->first),sizeof(int),1,fp_index1);
		ret2 = fwrite(&(hmIndex->second),sizeof(int),1,fp_index2);
		if(ret1 !=  1 || ret2 != 1)
		{
			nx_log(NGX_LOG_ERR,"dump index error: can't not write file [%d] [%d]",ret1,ret2);
			return -1;

		}
	}
		snprintf(buf,128,"total_term:%u\ntotal_relate_term:%u\ntotal_weibo:%llu",m_term_num,m_relate_term_num,m_weibo_num);
		ret1 = fwrite(buf,sizeof(char),strlen(buf),fp_indexn);


	FILE* fp_term_info;
	vector<term_info_t>::iterator iterv;

	fp_term_info=fopen("term_info.index","w");
	if (NULL ==fp_term_info)
	{
		nx_log(NGX_LOG_ERR,"dump index error: can't not open file");
		return -1;
	}
	for(iterv= mTermInfo.begin();iterv!=mTermInfo.end();iterv++)
	{
		ret1 = fwrite((*iterv).termstr,sizeof(char),MAX_TERM_LENGTH,fp_term_info);
		ret1 += fwrite(&((*iterv).df_history),sizeof(int),1,fp_term_info);
		ret1 += fwrite(&((*iterv).df_now),sizeof(int),1,fp_term_info);
		ret1 += fwrite(&((*iterv).relate_term_num),sizeof(int),1,fp_term_info);
		
		if (ret1 != ( MAX_TERM_LENGTH + 3))
		{
			nx_log(NGX_LOG_ERR,"dump index error: write index error");
			return -1;
		}
		__gnu_cxx::hash_map<u_int,u_int>::iterator iterhm;
		for(iterhm= (*iterv).cooc_term.begin();iterhm != (*iterv).cooc_term.end();iterhm++)
		{
			ret1 = fwrite(&(iterhm->first),sizeof(int),1,fp_term_info);
			ret2 = fwrite(&(iterhm->second),sizeof(int),1,fp_term_info);
			if(ret1 != 1 || ret2 !=  1)
			{
				nx_log(NGX_LOG_ERR,"dump index error: write file error");
				return -1;
			}
		}
	}

	FILE* fp_relate_info;
	vector<relate_term_info_t>::iterator iterRelateInfo;

	fp_relate_info=fopen("relate_info.index","w");
	if (NULL ==fp_relate_info)
	{
		nx_log(NGX_LOG_ERR,"dump index error: can't not open file");
		return -1;
	}
	for(iterRelateInfo= mRelateInfo.begin();iterRelateInfo!=mRelateInfo.end();iterRelateInfo++)
	{
		ret1= fwrite(&(*iterRelateInfo),sizeof(relate_term_info_t),1,fp_relate_info);
		if(ret1 != 1)
		{
			nx_log(NGX_LOG_ERR,"dump index error: can't not open file");
			return -1;
		}
	}
	return 0;

}
