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
int WeiboProcessor::LoadWeiboAllInfo()
{
	int ret=0;

	ret = ProcessOriginalData();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"ProcessOriginalData error");
		return -1;
	}
	ret = ProcessClassifiedData();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"ProcessClassifiedData error");
		return -1;
	}

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
	ret = CalcWeight();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"CalcWeight error");
		return -1;
	}
	/*
	ret = SortOutput();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"SortOutput error");
		return -1;
	}
	*/
	return 0;
}

/**
 * @brief 初始化实例
 *
 * @Param cResource
 *
 * @Returns   
 */
int WeiboProcessor::WBProcessorInit(CConfigInfo *pConfigInfo)
{
	m_pConfigInfo = pConfigInfo;
	nMaxForwardCount=0;
	nMaxValidCmtCount=0;
	int ret =0 ;

	// 初始化山寨json解析器
	vector<string> json_fields;
	json_fields.push_back("flag");
	json_fields.push_back("id");
	json_fields.push_back("uid");
	json_fields.push_back("filter");
	json_fields.push_back("content");
	json_fields.push_back("yc_source");
	CJParser::InitParser(json_fields);

	// 装载其他资源
	string strYCConfig = m_pConfigInfo->GetValue("yc_source_conf");
	if(strYCConfig.size() == 0)
	{
		nx_log(NGX_LOG_ERR,"load resource error");
		return -1;
	}

	m_pYCConfigInfo	= new CConfigInfo(strYCConfig.c_str(),'\t');

	string strRes = m_pConfigInfo->GetValue("vip_resource");
	m_pVipResource = new CResource(strRes.c_str(),4);

	strRes = m_pConfigInfo->GetValue("daren_resource");
	m_pDarenResource = new CResource(strRes.c_str(),3);

	strRes = m_pConfigInfo->GetValue("type_relation");
	m_pTypeRelation = new CResource(strRes.c_str(),3);

	strRes = m_pConfigInfo->GetValue("IT_white_list");
	m_pITWhiteList = new CResource(strRes.c_str(),2);
	
	strRes = m_pConfigInfo->GetValue("finance_white_list");
	m_pFinanceWhiteList = new CResource(strRes.c_str(),2);

	strRes = m_pConfigInfo->GetValue("sports_white_list");
	m_pSportsWhiteList = new CResource(strRes.c_str(),2);

	string strLexiconConf = m_pConfigInfo->GetValue("seg_data");

	//ret = LoadLexicon(strLexiconConf.c_str());
	//if (ret <0)
	//{
	//	return -1;
	//}
	//return 0;
	
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
	// 初始化文本指纹产生器，用于排重
	return  CTextKeyTool::GetInstance()->InitInstance(m_pConfigInfo->GetValue("seg_data").c_str(), 31);
}

int WeiboProcessor::LoadLexicon(const char *cLexiconPath)
{
	/* 打开词典 */
	m_Lexicon = OpenLexicon_Opt(cLexiconPath, 0x1F);//第一个参数，即根目录，词典
	if(m_Lexicon == NULL)
	{
		nx_log(NGX_LOG_ERR,"load lexicon error");
		return -1;
	}
	/* 设置语种 */
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
	unsigned short lexiconopt = 0x0014;
				
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
 * @brief 装载微博的其他信息资源
 *
 * @Param cResource
 */
int WeiboProcessor::LoadResource(const char* cResource)
{
	int ret;
	m_pConfigInfo = new CConfigInfo(cResource);

	string strYCConfig = m_pConfigInfo->GetValue("yc_source_conf");
	if(strYCConfig.size() == 0)
	{
		nx_log(NGX_LOG_ERR,"load resource error");
		return -1;
	}

	m_pYCConfigInfo	= new CConfigInfo(strYCConfig.c_str(),'\t');

	string strRes = m_pConfigInfo->GetValue("vip_resource");
	m_pVipResource = new CResource(strRes.c_str(),4);

	strRes = m_pConfigInfo->GetValue("daren_resource");
	m_pDarenResource = new CResource(strRes.c_str(),3);

	strRes = m_pConfigInfo->GetValue("type_relation");
	m_pTypeRelation = new CResource(strRes.c_str(),3);

	strRes = m_pConfigInfo->GetValue("IT_white_list");
	m_pITWhiteList = new CResource(strRes.c_str(),2);
	
	strRes = m_pConfigInfo->GetValue("finance_white_list");
	m_pFinanceWhiteList = new CResource(strRes.c_str(),2);

	strRes = m_pConfigInfo->GetValue("sports_white_list");
	m_pSportsWhiteList = new CResource(strRes.c_str(),2);

	string strLexiconConf = m_pConfigInfo->GetValue("seg_data");

	ret = LoadLexicon(strLexiconConf.c_str());
	if (ret <0)
	{
		return -1;
	}
	return 0;

}

/**
 * @brief 处理分类微博数据
 *
 * @Param szInputFile
 *
 * @Returns   
 */
int WeiboProcessor::ProcessClassifiedData()
{
	const char * szInputFile;
	szInputFile=m_pConfigInfo->GetValue("classified_data").c_str();
	ifstream fin(szInputFile);
	if(!fin.is_open())
	{
		nx_log(NGX_LOG_ERR,"open input file error");
		return -1;
	}

	string line;
	int ret;

	while(!fin.eof())
	{
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = ParseLineClassified(line);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"open line error,line: [%s]",line.c_str());
		}
	}
	fin.close();

	return 0;

	//debug:
	/*
	for( map<unsigned long long, WeiboInfo*>::iterator iter= m_mapWeiboInfo.begin();iter != m_mapWeiboInfo.end();iter ++)
	{

		cout << "mid: [" << iter->first << "] " << "keywords: ["  << iter->second->keywords.c_str() << "] " ;
		cout << "score: [ " << iter->second->nSynScore << "]";
		cout << "type: [" << iter->second->type << "]" << endl;
			
	}
	*/
}

/**
 * @brief 处理一行分类数据
 *
 * @Param szLine
 *
 * @Returns   
 */
int WeiboProcessor::ParseLineClassified(string szLine)
{
	vector<string> vecData = CStringTool::SpliteByChar(szLine, '`');
	if(vecData.size() !=11)
	{
		nx_log(NGX_LOG_ERR,"classified data format error");
		return -1;
	}
	unsigned long long mid;
	string keywords;
	int type;
	int Garbage_score;
	int flag;

	mid = CTypeTool<unsigned long long>::StrTo(vecData[4]); 
	keywords = vecData[2];
	type = CTypeTool<int>::StrTo(vecData[10]); 
	Garbage_score = CTypeTool<int>::StrTo(vecData[9]); 
	flag = CTypeTool<int>::StrTo(vecData[9]); 

	// 1. 丢弃转发微博
	if (flag ==1)
	{
		nx_log(NGX_LOG_DEBUG,"forward weibo, deleted");
		return 1;
	}
	// 2. 查看此条微博是否目标分类
	
	// 高10位保存一级分类
	/*
	type = (type &0xffc00000) >> 22; 

	vector<int>::iterator iter = find(m_types.begin(),m_types.end(),type);

	if (iter == m_types.end())
	{
		//不是需要的分类
		//cout << "type not need " << "mid: [" << mid << "] type: [" << type << "]" << endl;
		nx_log(NGX_LOG_DEBUG,"forward weibo, deleted");
		return 1;
	}
	*/

	// 3. 原始数据中是否有此微博

	map<unsigned long long, WeiboInfo*>::iterator iter_map = m_mapWeiboInfo.find(mid);
	if (iter_map == m_mapWeiboInfo.end())
	{
		nx_log(NGX_LOG_DEBUG,"have not recorded this mid in original data[%uL]",mid);
		return 1;
	}

	// 4. 通过前面两关，保存信息
	iter_map->second->base_info.nGarbageScore = Garbage_score;
	iter_map->second->base_info.strKeywords = keywords;
	iter_map->second->base_info.nType = type;

	return 0;

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
	szInputFile=m_pConfigInfo->GetValue("original_data").c_str();
	ifstream fin(szInputFile);
	if(!fin.is_open())
	{
		nx_log(NGX_LOG_ERR,"can not open input file");
		return -1;
	}
	string line;
	int ret;
	while(!fin.eof())
	{
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = ParseLineOriginal(line);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"original data line parse fail");
		}
	}
	fin.close();
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

	WeiboInfo* pWeibo = new WeiboInfo;
	int ret = CJParser::ParseJson(strLine, mapJson);
	if(ret == 0)
	{
	  return -1;
	}
	unsigned long long mid =0;
	unsigned long long uid =0;
	int filter = 0; 
	string strContent ;
	string strSrc;
	int src_score=0 ;
	int flag=0;
	float fTextScore=0.0;
	char text[1024*1024];
	int len;
	
	mid = CTypeTool<unsigned long long>::StrTo(mapJson["id"]);
	// 1. 先看此条微博是否我们需要的类别
	/*
	map<unsigned long long, WeiboInfo*>::iterator iter = m_mapWeiboInfo.find(mid);
	if(iter == m_mapWeiboInfo.end())
	{
		//不是我们需要的类别
		return 1;	
	}
	*/

	// 2. 去除转发微博
	flag = CTypeTool<int>::StrTo(mapJson["flag"]);
	if (flag == 1)
	{
		nx_log(NGX_LOG_DEBUG,"forward weibo [%d] omitted.",mid);
		return 1;
	}

	//3. 是需要的类别，获取各个参数

	uid = CTypeTool<unsigned long long>::StrTo(mapJson["uid"]);
	filter = CTypeTool<int>::StrTo(mapJson["filter"]);
	strContent = mapJson["content"];
	string strTmp = mapJson["flag"];

	strSrc = m_pYCConfigInfo->GetValue(mapJson["yc_source"].c_str());
	if (strSrc.size() == 0)
	{
		//默认得分，等同于网页发微博的score
		src_score =1;	
	}
	else
	{
		src_score= CTypeTool<int>::StrTo(strSrc);
	}

	// 4. 排重
	if(HaveDuplicates(strContent))
	{
		nx_log(NGX_LOG_DEBUG,"duplicates: [%s].",strContent.c_str());
		return 1;
	}

	// 5. 提取昵称，如果昵称太多，就是一个小范围微博
	// to do: 这里应该仅仅把@删掉。
	map<string, int> mapAt;
	CMBTextTool::ExtractNickName(strContent.c_str(), mapAt);
	// large than 4, delete directly, <=4, ajust weight in CalcWeight
	if( mapAt.size() > 4)
	{
		nx_log(NGX_LOG_DEBUG,"too many At deleted: [%s]. ",strContent.c_str());
		return 1;
	}

	// 6. 计算文本权重
	snprintf(text,1024*1024,"%s",strContent.c_str());
	len = strlen(text);
	ret = AnalyTextWithLex2(m_Lexicon,text , strlen(text), "GBK", m_Wordseg, m_WordSegFlags,1);
	if(ret<0)
	{
		fTextScore =1.0;//出错了，赋予一个较小的值
	}
	fTextScore=checkArticle(m_Wordseg,text,len);
	if(fTextScore <=0 )
	{
		fTextScore =1.0;
	}
	// 7. 筛选完毕, 记录其他字段
	pWeibo->base_info.nUid= uid;
	pWeibo->base_info.nAtNum= mapAt.size();
	pWeibo->base_info.nFilterScore= filter;
	pWeibo->base_info.nYCScore= src_score;
	pWeibo->base_info.strDebugContent= strContent;
	pWeibo->base_info.nContentLen= strContent.size();
	pWeibo->base_info.fTextScore= fTextScore;
	// 5. 初始化其他信息

	pWeibo->base_info.nUid = 0;
	pWeibo->base_info.nYCScore = 0;
	pWeibo->base_info.nFilterScore = 0;
	pWeibo->base_info.nForwardCount = 0;
	pWeibo->base_info.nValidCmtCount = 0;
	pWeibo->base_info.nContentLen = 0;
	pWeibo->nWeight =0;
	
	m_mapWeiboInfo.insert(pair<unsigned long long, WeiboInfo*>(mid, pWeibo));
	return 0;

}

int WeiboProcessor::ProcessForwardData()
{
	const char * szInputFile;
	szInputFile=m_pConfigInfo->GetValue("forward_data").c_str();
	ifstream fin(szInputFile);
	if(!fin.is_open())
	{
		nx_log(NGX_LOG_ERR,"input file can not been opend.");
		return -1;
	}
	string line;
	int ret;
	while(!fin.eof())
	{
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = ParseLineForward(line);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"forward data line parse fail");
		}
	}
	fin.close();
	return 0;
	//debug:
	/*
	for( map<unsigned long long, WeiboInfo*>::iterator iter= m_mapWeiboInfo.begin();iter != m_mapWeiboInfo.end();iter ++)
	{

		cout << "mid: [" << iter->first << "] " << "keywords: ["  << iter->second->base_info.strKeywords.c_str() << "] " ;
		cout << "score: [" << iter->second->base_info.nWeiboScore << "]";
		cout << "type: [" << iter->second->base_info.nType << "]";
		cout << "yc_score: [" << iter->second->base_info.nYCScore << "]";
		cout << "filter: [" << iter->second->base_info.nFilterScore << "]";
		cout << "uid: [" << iter->second->base_info.nUid << "]";
		cout << endl;
			
	}
	*/

}

/**
 * @brief 解析一行转发数据
 *
 * @Param szLine
 *
 * @Returns   
 */
int WeiboProcessor::ParseLineForward(string szLine)
{
	vector<string> vecData = CStringTool::SpliteByChar(szLine, '\t');
	if(vecData.size() !=5)
	{
		nx_log(NGX_LOG_ERR,"classified data format error");
		return -1;
	}
	unsigned long long mid;
	unsigned long long uid;
	int zf_count;
	string url;

	mid = CTypeTool<unsigned long long>::StrTo(vecData[4]); 
	uid = CTypeTool<unsigned long long>::StrTo(vecData[2]); 
	zf_count = CTypeTool<int>::StrTo(vecData[3]); 
	url = vecData[1];


	// 1. 此微博是否已经记录过

	map<unsigned long long, WeiboInfo*>::iterator iter_map = m_mapWeiboInfo.find(mid);
	if (iter_map == m_mapWeiboInfo.end())
	{
		nx_log(NGX_LOG_ERR," have not record this in original weibo [%uL].",mid);
		return 1;
	}

	// 2. 有这条微博，保存信息
	iter_map->second->base_info.nForwardCount = zf_count;
	iter_map->second->base_info.strDebugUrl = url;
	if (zf_count > nMaxForwardCount)
	{
		nMaxForwardCount = zf_count;
	}

	return 0;

	

}
int WeiboProcessor::ProcessCommentData()
{
	const char * szInputFile;
	szInputFile=m_pConfigInfo->GetValue("comment_data").c_str();
	ifstream fin(szInputFile);
	if(!fin.is_open())
	{
		nx_log(NGX_LOG_ERR," input file can not been opend.");
		return -1;
	}
	string line;
	int ret;
	while(!fin.eof())
	{
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		ret = ParseLineComment(line);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"comment data line parse fail");
		}
	}
	fin.close();

	return 0;

}

/**
 * @brief 解析一行评论数据
 *
 * @Param strLine
 *
 * @Returns   
 */
int WeiboProcessor::ParseLineComment(string strLine)
{
	map<string, string> mapJson;
	vector<string> json_fields;
	json_fields.push_back("uid");
	json_fields.push_back("mid");
	json_fields.push_back("text");
	json_fields.push_back("yc_mid");
	CJParser::InitParser(json_fields);

	int ret = CJParser::ParseJson(strLine, mapJson);
	if(ret == 0)
	{
	  return -1;
	}
	unsigned long long mid =0;
	unsigned long long yc_mid =0;
	unsigned long long uid =0;
	string strText ;
	
	yc_mid = CTypeTool<unsigned long long>::StrTo(mapJson["yc_mid"]);
	mid = CTypeTool<unsigned long long>::StrTo(mapJson["mid"]);
	
	// 1. 先看此条微博是否已经记录
	map<unsigned long long, WeiboInfo*>::iterator iter = m_mapWeiboInfo.find(yc_mid);
	if(iter == m_mapWeiboInfo.end())
	{
		nx_log(NGX_LOG_ERR," have not record this mid in original weibo [%uL].",mid);
		//没有这条微博
		return 1;	
	}

	// 2. 这条微博是否一个"回复"
	strText = mapJson["text"];
	/*
	if (strText.find("回复") == 0 || strText.find("回覆") == 0)
	{
		//cout << "reply comment, do not record"<<mid <<endl;
		iter->second->base_info.nValidCmtCount = iter->second->base_info.nValidCmtCount -2;
		if (iter->second->base_info.nValidCmtCount < 0)
		{
			iter->second->base_info.nValidCmtCount =0;
		}
		return 1;
	}
	*/

	//3. 是有效的回复，计数

	(iter->second->base_info.nValidCmtCount)++;
	if (iter->second->base_info.nValidCmtCount > nMaxValidCmtCount)
	{
		nMaxValidCmtCount = iter->second->base_info.nValidCmtCount;
	}
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


/**
 * @brief 计算权重
 *
 * @Param strLine
 *
 * @Returns   
 */
int WeiboProcessor::CalcWeight()
{
	
	int base_weight;
	int filter;
	int ret;
	bool has_video;
	bool has_image;
	int if_vip=0;
	int if_daren=0;
	int if_white=0;
	int fans_num=0;
	float fans_factor=0.0;
	float forward_factor=0.0;
	float comment_factor=0.0;
	stringstream ssBuf;
	string strBuf;
	vector<string> vVipInfo;
	vector<string> vDarenInfo;
	vector<string> vTypeRelation;
	vector<string> vWhiteList;

	if(nMaxForwardCount <=0 || nMaxValidCmtCount <=0)
	{
		nx_log(NGX_LOG_DEBUG,"nMaxForwardCount or nMaxValidCmtCount error");
		return -1;

	}

	nx_log(NGX_LOG_DEBUG,"already recorded weibo num: %d",m_mapWeiboInfo.size());
	map<unsigned long long, WeiboInfo*>::iterator iterWeiboInfo ;
	//calculate weight of every weibo
	for (iterWeiboInfo= m_mapWeiboInfo.begin();iterWeiboInfo != m_mapWeiboInfo.end();iterWeiboInfo ++)
	{
		filter=0;
		ret=0;
		has_video=0;
		has_image=0;
		if_vip=0;
		if_daren=0;
		if_white=0;
		fans_num=0;
		fans_factor=0.0;
		forward_factor=0.0;
		comment_factor=0.0;

		if (iterWeiboInfo -> second->base_info.nContentLen <=0)
		{
			continue;
		}
		if (iterWeiboInfo->second->base_info.nGarbageScore >11)
		{
			iterWeiboInfo->second->base_info.nGarbageScore =11;
		}
		// 1. get info
		//   1.1 get type relation
		ssBuf.str("");
		ssBuf << iterWeiboInfo->second->base_info.nType ;
		m_pTypeRelation->GetValue(ssBuf.str(),vTypeRelation);

		// 2. calculate weight
		//   2.1 set TextScore as base_weight
		base_weight = int(iterWeiboInfo -> second->base_info.fTextScore);
		nx_log(NGX_LOG_DEBUG,"weibo id [%uL] base_weight [%d] begin",iterWeiboInfo->first,base_weight);

		//   2.2 garbage score ajust : garbage score:[1~11] 11 is definitely garbage
		if (iterWeiboInfo->second->base_info.nGarbageScore >=11)
		{
			base_weight = int (float(base_weight) / 3.0);
		}
		else
		{
			base_weight = int (float(base_weight) * ( 1.0 + float( 10.0 - iterWeiboInfo->second->base_info.nGarbageScore )/10.0));
		}
		//   2.3 yc_score : the terminal where this microblog comes from .[1-3]
		base_weight = int (float(base_weight) * ( 1.0 + float(iterWeiboInfo->second->base_info.nYCScore )/10.0));

		//   2.4 forward_count : log10(this)/log10(max) ,multiply by 2
		if (iterWeiboInfo->second->base_info.nForwardCount > 0)
		{
			forward_factor = ( 1.0 + log10(float(iterWeiboInfo->second->base_info.nForwardCount )) / log10(float(nMaxForwardCount )));
			base_weight = (int)(forward_factor * float(base_weight));
		}
		//   2.5 comment_count : log10(this)/log10(max) ,multiply by 2
		if (iterWeiboInfo->second->base_info.nValidCmtCount > 0)
		{
			comment_factor = ( 1.0 + log10(float(iterWeiboInfo->second->base_info.nValidCmtCount )) / log10(float(nMaxValidCmtCount )));
			base_weight = (int)(comment_factor * float(base_weight));
		}

		//  2.6 filter information : if this MB has image or video, if so ,multiply by 2
		filter = iterWeiboInfo->second->base_info.nFilterScore;
		has_video = (filter & 0x20) >> 5;
		has_image = filter & 0x1;
		if (has_video || has_image)
		{
			base_weight = base_weight * 2;
			iterWeiboInfo->second->base_info.has_image=has_image;
			iterWeiboInfo->second->base_info.has_video=has_video;
		}

		//  2.7 vip info : if the user is VIP , multiply by n
		ssBuf.str("");
		ssBuf << iterWeiboInfo->second->base_info.nUid;
		strBuf = ssBuf.str();

		m_pVipResource->GetValue(ssBuf.str(),vVipInfo);
		m_pDarenResource->GetValue(ssBuf.str(),vDarenInfo);
		//vip info have 5 columns
		if (vVipInfo.size() == 3)
		{
			if_vip =1;
			//2.7.1 vip: weight * 2
			nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s]is vip",iterWeiboInfo->first,ssBuf.str().c_str());
			base_weight = int (float(base_weight) * 2);
			fans_num = CTypeTool<int>::StrTo(vVipInfo[0]);

			//2.7.2 fans_num ajust: if the user have many fans ,but this mid have low forward or comment count ,the mid should
			//be punished.
			//assume every mid should have a number of log2(fans_num) comments and forwards, if below , let the weight down.
			//this method is just a guess.
			//a more reasonable way is to establish user information system.
			if (fans_num >0)
			{
				if (iterWeiboInfo->second->base_info.nForwardCount == 0 && iterWeiboInfo->second->base_info.nValidCmtCount == 0)
				{
					fans_factor = ( 1.0 / 3.0);
				}
				else if (iterWeiboInfo->second->base_info.nForwardCount == 0 || iterWeiboInfo->second->base_info.nValidCmtCount == 0)
				{
					fans_factor = ( 1.0 / 2.0);
				}
				else
				{
					fans_factor =  1.0;

				}

				if (iterWeiboInfo->second->base_info.nForwardCount > 0 )
				{
					fans_factor = fans_factor * (1.0 + (float)iterWeiboInfo->second->base_info.nForwardCount / log2(fans_num) / 10);
				}

				if (iterWeiboInfo->second->base_info.nValidCmtCount > 0 )
				{
					fans_factor = fans_factor * (1.0 + (float)iterWeiboInfo->second->base_info.nValidCmtCount / log2(fans_num)/10);
				}

				if (fans_factor > 2.0)
				{
					fans_factor = 2.0;
				}
				if (fans_factor < 0.33)
				{
					fans_factor = 0.33;
				}
				if (fans_factor < 1.0)
				{
					nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s] have been punished by fasn_factor [%.2f]",iterWeiboInfo->first,ssBuf.str().c_str(),fans_factor);
					base_weight = int (float(base_weight) * fans_factor );
				}
				
			}
			//2.7.3 user type ajust : if the user type is same with the mid type ,multiply by 1.5
			
			// difficult to understand
			// vVipInfo[2] is vip first-level classification, m_pTypeRelation[0] is fundamental classification(by fengyoung ) name
			// they not same classification tree, if the name is equal , consider it as same class
			if (vVipInfo[2] == vTypeRelation[0])
			{
				if_vip =2;
				base_weight = int (float(base_weight) * 1.5);
			}
		}
		//  2.8 daren info : if the user is daren , multiply by n
		// a user can not be both VIP and daren at the same time
		else if (vDarenInfo.size() == 2)
		{
			if_daren=1;
			//2.8.1
			base_weight = int (float(base_weight) * 1.5);
			nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s]is daren",iterWeiboInfo->first,ssBuf.str().c_str());

			// fans number
			fans_num = CTypeTool<int>::StrTo(vDarenInfo[1]);

			//2.8.2 fans_num ajust: if the user have many fans ,but this mid have low forward or comment count ,the mid should
			//be punished.
			//assume every mid should have a number of log2(fans_num) comments and forwards, if below , let the weight down.
			//this method is just a guess.
			//a more reasonable way is to establish user information system.
			if (fans_num >0)
			{
				if (iterWeiboInfo->second->base_info.nForwardCount == 0 && iterWeiboInfo->second->base_info.nValidCmtCount == 0)
				{
					fans_factor = ( 1.0 / 3.0);
				}
				else if (iterWeiboInfo->second->base_info.nForwardCount == 0 || iterWeiboInfo->second->base_info.nValidCmtCount == 0)
				{
					fans_factor = ( 1.0 / 2.0);
				}

				if (iterWeiboInfo->second->base_info.nForwardCount > 0 )
				{
					fans_factor = fans_factor * (1.0 +  (float)iterWeiboInfo->second->base_info.nForwardCount / log2(fans_num)/ 10);
				}

				if (iterWeiboInfo->second->base_info.nValidCmtCount > 0 )
				{
					fans_factor = fans_factor * (1.0 + (float)iterWeiboInfo->second->base_info.nValidCmtCount / log2(fans_num)/10);
				}

				if (fans_factor > 2.0)
				{
					fans_factor = 2.0;
				}
				if (fans_factor < 0.33)
				{
					fans_factor = 0.33;
				}
				if (fans_factor < 1.0)
				{
					nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s] have been punished by fasn_factor [%.2f]",iterWeiboInfo->first,ssBuf.str().c_str(),fans_factor);
					base_weight = int (float(base_weight) * fans_factor );
				}
				
			}
			//2.8.3
			// different from vip
			int daren_type = CTypeTool<int>::StrTo(vDarenInfo[0]);

			// use a bit to save daren's type
			// the relation is recorded in a config file
			if ( daren_type >> CTypeTool<int>::StrTo(vTypeRelation[1]) & 0x1 )
			{
				if_daren=2;
				base_weight = int (float(base_weight) * 1.1);
			}

		}
		else
		{
			nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s]is not vip or daren",iterWeiboInfo->first,ssBuf.str().c_str());

		//  2.9 white list : if the user is in whitelist , multiply by 1.2
		// ssBuf should save the Uid, do not modify it
			switch (iterWeiboInfo->second->base_info.nType)
			{
				case 1:
					ret = m_pITWhiteList->GetValue(ssBuf.str(),vWhiteList);
					if (ret == 0)
					{
						if_white=1;
						base_weight = int (float(base_weight) * 1.5);
						nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s]is in whilte list",iterWeiboInfo->first,ssBuf.str().c_str());
					}
					break;
				case 2:
					ret = m_pFinanceWhiteList->GetValue(ssBuf.str(),vWhiteList);
					if (ret == 0)
					{
						if_white=1;
						base_weight = int (float(base_weight) * 1.5);
						nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s]is in whilte list",iterWeiboInfo->first,ssBuf.str().c_str());
					}
					break;
				case 16:
					ret = m_pSportsWhiteList->GetValue(ssBuf.str(),vWhiteList);
					if (ret == 0)
					{
						if_white=1;
						base_weight = int (float(base_weight) * 1.5);
						nx_log(NGX_LOG_DEBUG,"weibo id [%uL] user [%s]is in whilte list",iterWeiboInfo->first,ssBuf.str().c_str());
					}
					break;
				default :
					break;
			}
		}
		//2.10 : AtNum: if the weibo AT two many people , make the weight down

		if (iterWeiboInfo->second->base_info.nAtNum >0 )
		{
			base_weight = int (float(base_weight) / float(iterWeiboInfo->second->base_info.nAtNum));
		}

		iterWeiboInfo->second->nWeight = base_weight;

		nx_log(NGX_LOG_DEBUG , "mid [%uL] bw [%d] uid [%uL] gsc [%d] tsc [%.2f] tp [%d] "\
				"ycsc [%d] img [%d] video [%d] fwn [%d] fwsc [%.2f] fwnmx [%d] cmtn [%d] cmtsc [%.2f] cmtnmx [%d] atn [%d] vip [%d] "\
				"dr [%d] wht [%d] fann [%d] fansc [%.2f]",
				iterWeiboInfo->first,
				iterWeiboInfo->second->nWeight,
				iterWeiboInfo->second->base_info.nUid,
				iterWeiboInfo->second->base_info.nGarbageScore,
				iterWeiboInfo->second->base_info.fTextScore,
				iterWeiboInfo->second->base_info.nType,
				iterWeiboInfo->second->base_info.nYCScore ,
				has_image,
				has_video,
				iterWeiboInfo->second->base_info.nForwardCount,
				forward_factor,
				nMaxForwardCount,
				iterWeiboInfo->second->base_info.nValidCmtCount,
				comment_factor,
				nMaxValidCmtCount,
				iterWeiboInfo->second->base_info.nAtNum,
				if_vip,
				if_daren,
				if_white,
				fans_num,
				fans_factor
				);


		/*
		cout << "in calcweight: ";
		cout << "mid: [" << iterWeiboInfo->first << "] ";
		cout << "uid: [" << iterWeiboInfo->second->base_info.nUid << "]";
		cout << "garbage_score: [" << iterWeiboInfo->second->base_info.nGarbageScore << "]";
		cout << "textscore: [" << iterWeiboInfo->second->base_info.fTextScore << "]";
		cout << "type: [" << iterWeiboInfo->second->base_info.nType << "]";
		cout << "yc_score: [" << iterWeiboInfo->second->base_info.nYCScore << "]";
		cout << "filter: [" << iterWeiboInfo->second->base_info.nFilterScore << "]";
		cout << "forward_count: [" << iterWeiboInfo->second->base_info.nForwardCount << "]";
		cout << "forward_count_max: [" << nMaxForwardCount << "]";
		cout << "comment_count: [" << iterWeiboInfo->second->base_info.nValidCmtCount << "]";
		cout << "comment_count_max: [" << nMaxValidCmtCount << "]";
		cout << "keywords: ["  << iterWeiboInfo->second->base_info.strKeywords.c_str() << "] " ;
		cout << "content: ["  << iterWeiboInfo->second->base_info.strDebugContent.c_str() << "] " ;
		cout << endl;
		*/


	}
		
	return 0;
}

/**
 * @brief 排序
 *
 * @Param strLine
 *
 * @Returns   
 */
int WeiboProcessor::SortOutput()
{
	map<unsigned long long, WeiboInfo*>::iterator iterWeiboInfo ;
	vector <pair <unsigned long long, int> > VectorSports;
	vector <pair <unsigned long long, int> > VectorFinance;
	vector <pair <unsigned long long, int> > VectorIT;
	pair <unsigned long long, int> tmpPair;
	
	for (iterWeiboInfo= m_mapWeiboInfo.begin();iterWeiboInfo != m_mapWeiboInfo.end();iterWeiboInfo ++)
	{
		if (iterWeiboInfo->first >0 && iterWeiboInfo->second->nWeight>0 && iterWeiboInfo->second->base_info.nType > 0)
		{
			tmpPair.first = iterWeiboInfo ->first;
			tmpPair.second = iterWeiboInfo ->second -> nWeight;
			switch (iterWeiboInfo->second->base_info.nType)
			{
				case 1:
					VectorIT.push_back(tmpPair);
					break;
				case 2:
					VectorFinance.push_back(tmpPair);
					break;
				case 16:
					VectorSports.push_back(tmpPair);
					break;
				default :
					continue;
			}
		}
	}
	sort(VectorIT.begin(),VectorIT.end(),CmpWeibo);
	sort(VectorFinance.begin(),VectorFinance.end(),CmpWeibo);
	sort(VectorSports.begin(),VectorSports.end(),CmpWeibo);

	ofstream fout_sports(m_pConfigInfo->GetValue("output_sports").c_str());
	ofstream fout_finance(m_pConfigInfo->GetValue("output_finance").c_str());
	ofstream fout_IT(m_pConfigInfo->GetValue("output_IT").c_str());
	if(fout_sports.is_open())
	{
		for(vector <pair <unsigned long long, int> >::iterator iter = VectorSports.begin(); iter != VectorSports.end();iter ++)
		{
			fout_sports <<iter->first;
			//debug
			fout_sports << "\t" <<iter->second;
			map<unsigned long long, WeiboInfo*>::iterator iter2= m_mapWeiboInfo.find(iter->first);
			fout_sports << "\t" << iter2->second->base_info.has_image;
			fout_sports << "\t" << iter2->second->base_info.has_video;
			fout_sports << "\t" << iter2->second->base_info.strDebugContent;
			fout_sports << "\t" << iter2->second->base_info.strDebugUrl;
			fout_sports << endl;
		}
	}
	if(fout_finance.is_open())
	{
		for(vector <pair <unsigned long long, int> >::iterator iter = VectorFinance.begin(); iter != VectorFinance.end();iter ++)
		{
			fout_finance <<iter->first;
			//debug
			fout_finance << "\t" <<iter->second;
			map<unsigned long long, WeiboInfo*>::iterator iter2 = m_mapWeiboInfo.find(iter->first);
			fout_finance << "\t" << iter2->second->base_info.has_image;
			fout_finance << "\t" << iter2->second->base_info.has_video;
			fout_finance << "\t" << iter2->second->base_info.strDebugContent;
			fout_finance << "\t" << iter2->second->base_info.strDebugUrl;
			fout_finance << endl;
		}
	}
	if(fout_IT.is_open())
	{
		for(vector <pair <unsigned long long, int> >::iterator iter = VectorIT.begin(); iter != VectorIT.end();iter ++)
		{
			fout_IT <<iter->first;
			//debug
			fout_IT << "\t" <<iter->second;
			map<unsigned long long, WeiboInfo*>::iterator iter2 = m_mapWeiboInfo.find(iter->first);
			fout_IT << "\t" << iter2->second->base_info.has_image;
			fout_IT << "\t" << iter2->second->base_info.has_video;
			fout_IT << "\t" << iter2->second->base_info.strDebugContent;
			fout_IT << "\t" << iter2->second->base_info.strDebugUrl;
			fout_IT << endl;
		}
	}

	return 0;
}
