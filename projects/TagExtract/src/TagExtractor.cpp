#include "WeiboProcessor.h"
#include "TagExtractor.h"
#include <iostream>
/**
 * @file TagExtractor.cpp
 * @brief 
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 
 * @date 2011-12-07
 */

using namespace std;
bool CmpTermWeight(type_terminfo *term1, type_terminfo *term2)
{   
    return term1->output_weight > term2->output_weight;
}  

/**
 * @brief 
 *
 * @Param argc
 * @Param argv[0] 配置文件
 *
 * @Returns   
 */

TagExtractor::TagExtractor()
{
	m_pConfigInfo= NULL;
}

TagExtractor::~TagExtractor()
{
	delete m_pConfigInfo;
}

int TagExtractor::TagExtractInit(const char *pConfFile)
{
	int ret=0;
	m_pConfigInfo = new CConfigInfo(pConfFile);
	m_term_num=0;
	m_weibo_num =0 ;


	pWBProcessor = new WeiboProcessor();
	ret = pWBProcessor->WBProcessorInit(m_pConfigInfo);

	//m_spTokenizer =  CTokenizer::GetInstance();
    //ret = m_spTokenizer->InitInstance(m_pConfigInfo->GetValue("seg_data").c_str(), 0);
	if (ret < 0)
	{
		nx_log(NGX_LOG_ERR,"init CTokenizer error");
		return -1;
	}
	string strLexiconConf = m_pConfigInfo->GetValue("seg_data");

	ret = LoadLexicon(strLexiconConf.c_str());
	if (ret <0)
	{
		return -1;
	}
	return 0;

}

int TagExtractor::LoadLexicon(const char *cLexiconPath)
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
		TagExtractor::ReleaseWordSeg();
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


int TagExtractor::LoadWeibo()
{
	int ret;
	char term[100];
	char weibo_str[1000];
	//int weibo_type=0;
	int type_score=0;
	int len;
	int wordvalue;
	int idf;
	int hash_type_str=0;
	int term_index=0;
	unsigned long long mid;
	vector<string> vStrType;
	vector<string>::iterator iter_v;
	ret = pWBProcessor->LoadWeiboAllInfo();
	if (ret < 0)
	{
		nx_log(NGX_LOG_ALERT,"load weibo info error");
		return -1;
	}

	map<unsigned long long, WeiboInfo*> &pAllWeiboInfo  = pWBProcessor->GetAllWeiboInfo();

	map<unsigned long long, WeiboInfo*>::iterator iter_map = pAllWeiboInfo.begin();

	// tranverse every weibo ,record weibo info
	for(;iter_map != pAllWeiboInfo.end();iter_map ++)
	{
		hm_local.clear();

		// get info of this weibo
		snprintf(weibo_str,1000,"%s",iter_map->second->base_info.strDebugContent.c_str());
		mid = iter_map->first;
		//weibo_type= iter_map->second->base_info.nType;
		type_score=iter_map->second->base_info.nTypeScore;
		vStrType =CStringTool::SpliteByChar(iter_map->second->base_info.strType,',');
		

		if(vStrType.size() < 1)
		{
			nx_log(NGX_LOG_ERR,"vStrType error");
			continue;
		}
		for(iter_v = vStrType.begin();iter_v != vStrType.end();iter_v++)
		{
			//if a weibo have n type, add  n weibo_num  
			m_weibo_num ++;

			//map<string,wbtype_info >::iterator iter_map_tmp;
			//if this type turns out for the first time
			//initialize mTypeTermWeight
			if(mTypeTermWeight.find(*iter_v) == mTypeTermWeight.end())
			{
				wbtype_info wbtype_info_tmp;
				wbtype_info_tmp.weibo_num=0;
				mTypeTermWeight.insert(pair<string,wbtype_info>(*iter_v,wbtype_info_tmp));
			}
			// record the relationship of type hash  and type name
			hash_type_str = __gnu_cxx::__stl_hash_string((*iter_v).c_str());
			m_mapTypeInfo[hash_type_str]=*iter_v;
			// weibo num of this type ++
			mTypeTermWeight[*iter_v].weibo_num++;
		}


		//segment weibo
		ret = AnalyTextWithLex2(m_Lexicon,weibo_str , strlen(weibo_str), "GBK", m_Wordseg, m_WordSegFlags,1);
		if(ret < 0)
		{
			nx_log(NGX_LOG_ALERT,"AnalyTextWithLex2 error [%d]",ret);
			continue;
		}

		//recurse every term ,record the term info
		//record wordlist
		for(int i= 0; i< m_Wordseg->word_num; i++)
		{
			len = uniToBytes(m_Wordseg->uni+(m_Wordseg->wordlist[i]).wordPos,(m_Wordseg->wordlist[i]).wordLen,term,1024,"GBK");
			term[len]='\0';
			
			wordvalue = (m_Wordseg->wordlist[i].wordvalue);
			idf = int (float(m_Wordseg->wordlist[i].idf) / 10.0);
			// do not record the term whose idf is zero
			if (idf == 0 || len <=3)
			{
				continue;
			}
			
			nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->wordlist[i].postagid);
			ret= RecordTerm(term,mid,vStrType,m_Wordseg->wordlist[i]);
			if (ret < 0)
			{
				nx_log(NGX_LOG_ALERT,"record term error [%s]",term);
				continue;
			}
		}
		//record appword
		for(int i= 0; i< m_Wordseg->appword_num; i++)
		{
			len = uniToBytes(m_Wordseg->uni+(m_Wordseg->appwordlist[i]).word_item.wordPos,m_Wordseg->appwordlist[i].word_item.wordLen,term,1024,"GBK");
			term[len]='\0';
			
			wordvalue = (m_Wordseg->appwordlist[i].word_item.wordvalue);
			idf = int (float(m_Wordseg->appwordlist[i].word_item.idf) / 10.0);
			// do not record the term whose idf is zero
			if (idf == 0 || len <=3)
			{
				continue;
			}
			nx_log(NGX_LOG_DEBUG,"term [%s] postagid [%d]",term,m_Wordseg->appwordlist[i].word_item.postagid);
			ret= RecordTerm(term,mid,vStrType,m_Wordseg->appwordlist[i].word_item);
			if (ret < 0)
			{
				nx_log(NGX_LOG_ALERT,"record term error [%s]",term);
				continue;
			}
		}

		//nx_log(NGX_LOG_DEBUG,"mid [%uL] content [%s]",iter_map->first,iter_map->second->base_info.strDebugContent.c_str());
	}
	for(int z=0;z<m_term_num;z++)
	{
		float idf;
		vector<pair<string,int > > vec_tmp;
		//int N= pAllWeiboInfo.size();
		int N= m_weibo_num;
		__gnu_cxx::hash_map<int,int>::iterator iter_tmp;
		idf = powf(log10(float (N) / float(vTermInfo[z].nTotalTF)),4);

		for(iter_tmp = vTermInfo[z].TermFreqBin.begin();iter_tmp != vTermInfo[z].TermFreqBin.end();iter_tmp++)
		{
			int weight;
			weight = int(float (iter_tmp->second) * idf);
			float N11,N10,N01,N00;
			int ChiSquare;
			vTermInfo[z].TermScore[iter_tmp->first] = weight;

			//record term info in mTypeTermWeight
			//what a awful code
			mTypeTermWeight[m_mapTypeInfo[iter_tmp->first]].type_term[vTermInfo[z].termstr].term_weight=weight;

			//calculate ChiSquare
			N11=float (iter_tmp->second); 
			N10=float (vTermInfo[z].nTotalTFBin) - N11;
			N01=float ( mTypeTermWeight[m_mapTypeInfo[iter_tmp->first]].weibo_num - mTypeTermWeight[m_mapTypeInfo[iter_tmp->first]].type_term[vTermInfo[z].termstr].term_num);//此类别下，所有微博数量-出现过这个term的微博数量= 此类别下无此term的微博数量
			N00= float(N) - N01 - N10 - N11;
			if(N00<=0)
			{
				nx_log(NGX_LOG_ALERT,"chiSquare calculate error");
				mTypeTermWeight[m_mapTypeInfo[iter_tmp->first]].type_term[vTermInfo[z].termstr].ChiSquare=0;
				continue;
			}
			ChiSquare = int (float(N)* pow((N11 * N00 - N10 * N01),2.0) / ((N11+N01) * (N11 + N10) * (N00 + N10) * (N00+ N01)));
			mTypeTermWeight[m_mapTypeInfo[iter_tmp->first]].type_term[vTermInfo[z].termstr].ChiSquare= ChiSquare;



			nx_log(NGX_LOG_NOTICE,"term [%s] Type [%s] weight [%d] idf [%.2f] tfBin [%d] dfbin [%d] bad_num [%d] N00 [%.2f] N01 [%.2f] N10 [%.2f] N11 [%.2f] ChiSquare [%d] weibo_num [%d]", 
					vTermInfo[z].termstr,
					m_mapTypeInfo[iter_tmp->first].c_str(),
					weight,
					idf,
					iter_tmp->second,
					vTermInfo[z].nTotalTFBin,
					vTermInfo[z].is_bad_num,
					N00,
					N01,
					N10,
					N11,
					ChiSquare,
					N
					);
		}
		
	}
	nx_log(NGX_LOG_NOTICE,"pAllWeiboInfo.size [%d] weibo num [%d] term num [%d]",pAllWeiboInfo.size(),m_weibo_num,vTermInfo.size());

	map<int, string>::iterator x;

	// prepare output
	// copy mTypeTermWeight to a map<vector>
	for(x=m_mapTypeInfo.begin();x!=m_mapTypeInfo.end();x++)
	{
		int hash_value;
		float good_ratio;
		map<string,type_terminfo>::iterator tmp;
		for(tmp = mTypeTermWeight[x->second].type_term.begin(); tmp != mTypeTermWeight[x->second].type_term.end();tmp++)
		{
				hash_value = __gnu_cxx::__stl_hash_string(tmp->second.termstr);
				term_index= mTermIndex[hash_value];
				if (vTermInfo[term_index].nTotalTF > 0)
				{
					good_ratio = powf( (1.0 - float(vTermInfo[term_index].is_bad_num) / float(vTermInfo[term_index].nTotalTF)),5);
				}
				else
				{
					good_ratio = 0;	
				}
			tmp->second.output_weight= int(float (tmp->second.term_weight) * log10f(float(tmp->second.ChiSquare)) * float (strlen(tmp->second.termstr )) * good_ratio);
			m_voutput[x->second].push_back(&(tmp->second));

			nx_log(NGX_LOG_NOTICE,"Type [%s] term [%s] weight [%d] chisqr [%d] output_weight [%d] good_ratio [%.2f]",(x->second).c_str(),(tmp->second).termstr,(tmp->second).term_weight,(tmp->second).ChiSquare,tmp->second.output_weight,good_ratio);
		}
		sort(m_voutput[x->second].begin(),m_voutput[x->second].end(),CmpTermWeight);
	}

	map<string, vector< type_terminfo*> >::iterator y;
	for(y=m_voutput.begin();y!=m_voutput.end();y++)
	{
		ofstream fout;
		int hash_value ;
		fout.open(("./output/" + y->first + ".txt" ).c_str());
		vector<type_terminfo*>::iterator v_tmp;
		for(v_tmp = m_voutput[y->first].begin(); v_tmp != m_voutput[y->first].end();v_tmp++)
		{
			if (strlen((*v_tmp)->termstr) <=2 )
			{
				continue;
				nx_log(NGX_LOG_ALERT,"term too short ! term [%s] ",(*v_tmp)->termstr);
			}
			else
			{
				if ((*v_tmp)->ChiSquare > 100 )
				{
					fout << (*v_tmp)->termstr <<"\t" << (*v_tmp)->output_weight << "\t" << (*v_tmp)->term_weight << "\t" << log10f((*v_tmp)->ChiSquare) << "\t" << endl;
				}
			}
		}
	}

	return 0;
}

int TagExtractor::RecordTerm(char* term,unsigned long long mid, vector<string> vStrType ,WORD_ITEM seg_info)
{
	if (term == NULL)
	{
		return -1;
	}
	int hash_value=0;
	int hash_type_str=0;
	int term_index=0;
	int is_bad_term=0;
	term_info local_term_info ;
	vector<string>::iterator iter_v;
	__gnu_cxx::hash_map<int,int>::iterator iterhm;
	__gnu_cxx::hash_map<int,int>::iterator iterhm_local;
	hash_value=__gnu_cxx::__stl_hash_string(term);

	iterhm = mTermIndex.find(hash_value);
	iterhm_local = hm_local.find(hash_value);

	if (IsBadTerm(seg_info.postagid) == true)
	{
		is_bad_term = 1;
		nx_log(NGX_LOG_NOTICE,"term [%s] is bad ,postagid [%d] mid [%uL]",term,seg_info.postagid,mid);
	}

	// have never record this term,brand new
	if (iterhm == mTermIndex.end())
	{
		//record global info of this term
		snprintf(local_term_info.termstr,MAX_TERM_LENGTH,"%s",term);
		local_term_info.postagid= seg_info.postagid;
		local_term_info.nTotalTF =0;
		local_term_info.nTotalTFBin =0;
		local_term_info.is_bad_num = 0;

		//record type info of this term
		for(iter_v = vStrType.begin();iter_v != vStrType.end();iter_v++)
		{
			hash_type_str = __gnu_cxx::__stl_hash_string((*iter_v).c_str());
			local_term_info.TermFreq[hash_type_str]=1;
			local_term_info.TermFreqBin[hash_type_str]=1;

			//this type encount this term for the first time
			//if(mTypeTermWeight[*iter_v].type_term.find(term) == mTypeTermWeight[*iter_v].type_term.end())
			//{
			snprintf(mTypeTermWeight[*iter_v].type_term[term].termstr,MAX_TERM_LENGTH,"%s",term);
			mTypeTermWeight[*iter_v].type_term[term].term_num=1;
			mTypeTermWeight[*iter_v].type_term[term].postagid=seg_info.postagid ;
			mTypeTermWeight[*iter_v].type_term[term].term_weight=0;
			mTypeTermWeight[*iter_v].type_term[term].ChiSquare=0;
			mTypeTermWeight[*iter_v].type_term[term].output_weight=0;
			//}
			// if the  weibo has n type, every term should add n times
			local_term_info.nTotalTF ++;
			local_term_info.is_bad_num += is_bad_term;
			local_term_info.nTotalTFBin ++;
		}

		hm_local[hash_value] = 1;
		mTermIndex[hash_value] = m_term_num;

		// term count ++
		vTermInfo.push_back(local_term_info);
		m_term_num ++;
		if (vTermInfo.size() != m_term_num)
		{
			nx_log(NGX_LOG_ALERT,"vTermInfo size may be error size [%d] m_term_num [%d]",vTermInfo.size(), m_term_num);
			return -1;
		}
		term_index= mTermIndex[hash_value];

		nx_log(NGX_LOG_NOTICE,"new term [%s] ,totaltf [%d] totaltfbin [%d] type [%s] typetfbin [%d] mid [%uL]",term,vTermInfo[term_index].nTotalTF,vTermInfo[term_index].nTotalTFBin,m_mapTypeInfo[hash_type_str].c_str(),vTermInfo[term_index].TermFreqBin[hash_type_str],mid);
	}
	// the term turn out in this weibo for the first time.but have been found in other weibo
	else if (iterhm_local == hm_local.end() && iterhm != mTermIndex.end())
	{
		//get term index
		term_index= mTermIndex[hash_value];
		if(term_index < 0)
		{
			nx_log(NGX_LOG_ALERT,"term_index error term [%s]",term);
			return -1;
		}
		// modify type info
		for(iter_v = vStrType.begin();iter_v != vStrType.end();iter_v++)
		{
			hash_type_str = __gnu_cxx::__stl_hash_string((*iter_v).c_str());
			vTermInfo[term_index].TermFreq[hash_type_str]++;
			vTermInfo[term_index].TermFreqBin[hash_type_str]++;

			//this type encount this term for the first time
			if(mTypeTermWeight[*iter_v].type_term.find(term) == mTypeTermWeight[*iter_v].type_term.end())
			{
				snprintf(mTypeTermWeight[*iter_v].type_term[term].termstr,MAX_TERM_LENGTH,"%s",term);
				mTypeTermWeight[*iter_v].type_term[term].term_num=1;
				mTypeTermWeight[*iter_v].type_term[term].postagid= seg_info.postagid;
				mTypeTermWeight[*iter_v].type_term[term].term_weight=0;
				mTypeTermWeight[*iter_v].type_term[term].ChiSquare=0;
				mTypeTermWeight[*iter_v].type_term[term].output_weight=0;
			}
			else
			{
				mTypeTermWeight[*iter_v].type_term[term].term_num ++;
			}
			// modify global info
			vTermInfo[term_index].nTotalTF++;
			vTermInfo[term_index].is_bad_num += is_bad_term;
			vTermInfo[term_index].nTotalTFBin++;

			nx_log(NGX_LOG_NOTICE,"term [%s] have been found in this weibo first time,totaltf [%d] totaltfbin [%d] type [%s] typetfbin [%d] type_term_num [%d] mid [%uL]",term,vTermInfo[term_index].nTotalTF,vTermInfo[term_index].nTotalTFBin,m_mapTypeInfo[hash_type_str].c_str(),vTermInfo[term_index].TermFreqBin[hash_type_str],mTypeTermWeight[*iter_v].type_term[term].term_num,mid);
		}

		//record this term in this weibo
		hm_local[hash_value] = 1;

	}
	//this term have been found both in this weibo and global
	// say,this term  turn out more than one time in this weibo
	else if (iterhm_local != hm_local.end() && iterhm != mTermIndex.end())
	{
		//get term index
		term_index= mTermIndex[hash_value];
		if(term_index < 0)
		{
			nx_log(NGX_LOG_ALERT,"term_index error term [%s]",term);
			return -1;
		}
		// modify global info
		// modify type info
		for(iter_v = vStrType.begin();iter_v != vStrType.end();iter_v++)
		{
			hash_type_str = __gnu_cxx::__stl_hash_string((*iter_v).c_str());
			vTermInfo[term_index].TermFreq[hash_type_str]++;

			vTermInfo[term_index].nTotalTF++;
			vTermInfo[term_index].is_bad_num += is_bad_term;
			//nx_log(NGX_LOG_NOTICE,"term [%s] have been found both in this weibo and global,totaltf [%d] totaltfbin [%d] type [%s] typetfbin [%d] mid [%uL]",term,vTermInfo[term_index].nTotalTF,vTermInfo[term_index].nTotalTFBin,m_mapTypeInfo[hash_type_str].c_str(),vTermInfo[term_index].TermFreqBin[hash_type_str],mid);
		}
	}
	/*
	   for(iterhm =  vTermInfo[mTermIndex[hash_value]].TermFreqBin.begin();iterhm !=  vTermInfo[mTermIndex[hash_value]].TermFreqBin.end();iterhm++)
	   {
	   nx_log(NGX_LOG_NOTICE,"term [%s] totalTF [%d] nTotalTFBin [%d] Type [%s] TermFreqBin [%d]",vTermInfo[mTermIndex[hash_value]].termstr,vTermInfo[mTermIndex[hash_value]].nTotalTF,vTermInfo[mTermIndex[hash_value]].nTotalTFBin,m_mapTypeInfo[iterhm->first].c_str(),iterhm->second);
	   }
	 */
	return 0;
}	

bool TagExtractor::IsBadTerm(int postagid)
{
	if (postagid <= POSTAG_ID_B || 
			(postagid == POSTAG_ID_T_Z)  ||
			(postagid == POSTAG_ID_Z)  ||
			(postagid == POSTAG_ID_AN)  ||
			(postagid == POSTAG_ID_VN)  ||
			(postagid >= POSTAG_ID_M &&  postagid <= POSTAG_ID_NS_Z))
	{
		return false;
	}
	else
	{
		return true;
	}
}


void TagExtractor::ReleaseWordSeg()
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

int TagExtractor::ExtractTag()
{

	nx_log(NGX_LOG_NOTICE,"in ExtractTag");
	return 0;

}
