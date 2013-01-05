/**
 * @file TuanDanProcessor.cpp
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-03-14
 */
#include "TuanDanProcessor.h"
using namespace std;


bool Cmp(pair<unsigned long long, int> w1, pair<unsigned long long, int> w2)
{   
    return w1.second > w2.second;
}  

/*
int heapify_r(sort_item_t* input,unsigned int len, int i)
{
    if(input==NULL)
    {
        return -1;
    }

    int min;
	sort_item_t tmp;
	min = i;
	// number of left child must smaller than the length of the heap
	if( (2*i+1) <= len -1 && input[2*i+1].sort_int<input[i].sort_int)
	{
		min = 2*i+1;
	}
	// number of right child must smaller than the length of the heap
	if( (2*i+2) <= len -1 && input[2*i+2].sort_int<input[min].sort_int)
	{
		min = 2*i +2;
	}

	if ( min != i && min <= len -1)
	{
		tmp=input[min];
		input[min]=input[i];
		input[i]=tmp;
		heapify_r(input,len,min);
	}

    return 0;

}

int build_min_heap(sort_item_t* input,unsigned int len)
{
    if(input==NULL)
    {
        return -1;
    }

    int i;

	for(i=(len-1)/2;i>=0;i--)
	{
		heapify_r(input,len,i);
	}
    return 0;
}
*/

/**
 * @brief Constructor
 */
TuanDanProcessor::TuanDanProcessor()
{
	m_pConfigInfo= NULL;
}

/**
 * @brief Destructor
 */
TuanDanProcessor::~TuanDanProcessor()
{
	Clear();
}

/**
 * @brief 初始化
 *
 * @Param cResource
 *
 * @Returns   
 */
int TuanDanProcessor::TDProcessorInit(const char  *pConfigFile)
{
	int ret =0 ;

	m_pConfigInfo = new CConfigInfo(pConfigFile);

	mDmdictLandMartk=NULL;
	mDmdictCity = NULL;

	string strConfig;

	// 装载其他资源
	// 加载city 字典
	strConfig = m_pConfigInfo->GetValue("city_dict");
	if(strConfig.size() == 0)
	{
		nx_log(NGX_LOG_ERR,"load city_data error");
		return -1;
	}

	mDmdictCity = DmDictLoad(strConfig.c_str(), 8196);

	// 加载地标字典
	strConfig = m_pConfigInfo->GetValue("landmark_dict");
	if(strConfig.size() == 0)
	{
		nx_log(NGX_LOG_ERR,"load landmark error");
		return -1;
	}

	mDmdictLandMartk = DmDictLoad(strConfig.c_str(), 6553600);

	// 加载城市和地标所有信息

	strConfig = m_pConfigInfo->GetValue("city_data");
	string strConfig2 = m_pConfigInfo->GetValue("landmark_data");
	if(strConfig.size() == 0 || strConfig2.size()==0)
	{
		nx_log(NGX_LOG_ERR,"load data error");
		return -1;
	}

	ret = LoadInfo(strConfig.c_str(),strConfig2.c_str());
	if (ret< 0)
	{
		nx_log(NGX_LOG_ERR,"load data error");
		return -1;

	}

	return 0;
}


dm_dict_t* TuanDanProcessor::DmDictLoad(const char* fullpath, int lemma_num)
{
	FILE* fp=NULL;
	char buff[2048];
	char word[256];
	char prop[256];
	dm_dict_t* pdict=NULL;
	
	pdict = dm_dict_create(lemma_num);
	if(!pdict){
		nx_log(NGX_LOG_STDERR, "error , create dict failed");
		goto failed;;
	}

	fp = fopen(fullpath, "rb");
	if(!fp){
		nx_log(NGX_LOG_STDERR, "error , file %s open failed", fullpath);
		goto failed;
	}

	while(fgets(buff, sizeof(buff), fp)){
		if(sscanf(buff, "%s%s", word, prop)!=2){
			nx_log(NGX_LOG_NOTICE, "warning , bad format %s", buff);
			continue;
		}
		
		CutTail(word);

		dm_lemma_t lm;
		lm.pstr = word;
		lm.len 	= strlen(word);
		if(lm.len < 6)
		{
			nx_log(NGX_LOG_NOTICE, "warning , bad format %s", buff);
			continue;
		}
		//lm.prop	= dm_prop_str2int_p(prop);
		lm.prop	= zatoi(prop);

		if(dm_add_lemma(pdict, &lm)<0){
			nx_log(NGX_LOG_STDERR, "error , add lemma failed");
			goto failed;
		}
	}
	fclose(fp);

	for(u_int i=0;i<pdict->lmpos;i++)	{
		pdict->lmlist[i].pstr=pdict->strbuf + pdict->lmlist[i].bpos;
	}

	return pdict;

failed:
	if(fp)
		fclose(fp);
	dm_dict_del(pdict);
	return NULL;
}



void TuanDanProcessor::CutTail(char* word)
{
	if (word== NULL)
	{
		return;
	}
	int len;
	len = strlen(word);
	if (strncmp("省",&(word[len-3]),3) ==0 ||
		strncmp("市",&(word[len-3]),3) ==0 ||
		strncmp("区",&(word[len-3]),3) ==0 ||
		strncmp("县",&(word[len-3]),3) ==0
		)
	{
		nx_log(NGX_LOG_DEBUG,"end with tail ,cut it [%s]" ,word);
		word[len-3]='\0';
		nx_log(NGX_LOG_DEBUG,"end with tail ,cut it [%s]" ,word);
		return ;
	}
	nx_log(NGX_LOG_DEBUG," did not end with tail , [%s]" ,word);
	return;
}

int TuanDanProcessor::LoadInfo(const char * city_data, const char * landmark_data)
{
	ifstream fin_city(city_data);
	ifstream fin_landmark(landmark_data);
	if(!fin_city.is_open() || !fin_landmark.is_open())
	{
		nx_log(NGX_LOG_ERR,"can not open input file");
		return -1;
	}
	string line;
	int ret;
	int line_num=0;
	while(!fin_city.eof())
	{
		getline(fin_city, line);
		if(line.size() == 0)
		{
		  continue;
		}
		ret = TDProcessLineCity(line);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"original data line parse fail ! line [%s]",line.c_str());
			continue;
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
		++line_num;
	}

	while(!fin_landmark.eof())
	{
		getline(fin_landmark, line);
		if(line.size() == 0)
		{
		  continue;
		}
		ret = TDProcessLineLM(line);
		if(ret < 0 )
		{
			nx_log(NGX_LOG_ERR,"original data line parse fail ! line [%s]",line.c_str());
		}
		nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
		++line_num;
	}
	fin_city.close();
	fin_landmark.close();
	return 0;
}

int TuanDanProcessor::TDProcessLineCity(string strLine)
{
	
	vector<string> term_string = CStringTool::SpliteByChar(strLine,'\t');
	vector<string>::iterator iterstr;

	if (term_string.size()!=4)
	{
		nx_log(NGX_LOG_NOTICE,"no term in line,check data");
		return -1;
	}

	strict_item_t tmp_strict;
	snprintf(tmp_strict.name,MAX_NAME_LEN,"%s",term_string[0].c_str());
	tmp_strict.father_id = CTypeTool<int>::StrTo(term_string[2]);
	tmp_strict.type = CTypeTool<int>::StrTo(term_string[3]);

	mLandmarkInfo.insert(pair<u_int,strict_item_t>(CTypeTool<u_int>::StrTo(term_string[1]), tmp_strict));
	return 0;


}

int TuanDanProcessor::TDProcessLineLM(string strLine)
{
	
	vector<string> term_string = CStringTool::SpliteByChar(strLine,'\t');
	vector<string>::iterator iterstr;

	if (term_string.size()!=4)
	{
		nx_log(NGX_LOG_NOTICE,"no term in line,check data");
		return -1;
	}


	if (mLandmarkInfo.find(CTypeTool<u_int>::StrTo(term_string[1])) != mLandmarkInfo.end())
	{//already have this lm, must be city
		nx_log(NGX_LOG_DEBUG,"already have this land mark %s",term_string[1].c_str());
		return 0;
	}
	else
	{
		strict_item_t tmp_strict;
		snprintf(tmp_strict.name,MAX_NAME_LEN,"%s",term_string[0].c_str());
		tmp_strict.father_id = CTypeTool<int>::StrTo(term_string[2]);
		tmp_strict.type = CTypeTool<int>::StrTo(term_string[3]);

		mLandmarkInfo.insert(pair<u_int,strict_item_t>(CTypeTool<u_int>::StrTo(term_string[1]), tmp_strict));
	}

	return 0;
}
/**
 * @brief 处理原始数据
 *
 * @Returns   >=0 正常
 *            <0  错误
 */
int TuanDanProcessor::TDProcess()
{
	const char * szInputFile;
	vector<string> term_string ;

	szInputFile=m_pConfigInfo->GetValue("tuangou_data").c_str();
	ifstream fin(szInputFile);
	szInputFile=m_pConfigInfo->GetValue("output_data").c_str();
	ofstream fout(szInputFile);
	if(!fin.is_open() || !fout.is_open())
	{
		nx_log(NGX_LOG_ERR,"can not open input file");
		return -1;
	}



	string line;
	string name,cata,des,tuangou_id;
	string str_tmp;
	dm_pack_t* _dm_pack = dm_pack_create(32);
	char buf[10240];
	int line_num=0;
	u_int city_id,lm_id,tmp_id;
	bool confirm_city;

	while(!fin.eof())
	{
		++line_num;
		getline(fin, line);
		if(line.size() == 0)
		  continue;
		term_string = CStringTool::SpliteByChar(line,'\t');
		if(term_string.size() !=11)
		{
			nx_log(NGX_LOG_WARN,"original data line error ! line [%s]",line.c_str());
			continue;
		}

		name = term_string[ITEM_NAME_SLOT];
		cata = term_string[CATEGORY_SLOT];
		des = term_string[DESCRIPTION_SLOT];
		tuangou_id = term_string[ITEM_ID_SLOT];
		
		//search in cata item first, using city dict
		snprintf(buf,10240,"%s",cata.c_str());
		dm_search(mDmdictCity,_dm_pack,buf,strlen(buf),DM_OUT_FMM);
		nx_log(NGX_LOG_DEBUG,"dict match in  line [%s] cnt[%d]",buf,_dm_pack->ppseg_cnt);

		city_id=0;
		tmp_id=0;
		confirm_city = false;
		for(int i = 0; i < (int)_dm_pack->ppseg_cnt; ++i)
		{
			tmp_id = ((_dm_pack->ppseg)[i])->prop;
			
			if((mLandmarkInfo[tmp_id]).type == 2 || (mLandmarkInfo[tmp_id]).type ==3 )
			{
				if (city_id !=0 && IsFatherSon(city_id,tmp_id) ==0)
				{
					nx_log(NGX_LOG_NOTICE,"city conflicts: old[%d] new[%d]",  tmp_id,city_id);
					continue;
				}
				city_id = tmp_id;
				confirm_city = true;
				nx_log(NGX_LOG_NOTICE,"confirm city : [%d]",city_id);
			}

		}

		// search city in name + des
		str_tmp = name+des;
		if(confirm_city == false)
		{
			snprintf(buf,10240,"%s",str_tmp.c_str());
			dm_search(mDmdictCity,_dm_pack,buf,strlen(buf),DM_OUT_FMM);
			nx_log(NGX_LOG_DEBUG,"dict match in  line [%s] cnt[%d]",buf,_dm_pack->ppseg_cnt);

			city_id=0;
			tmp_id=0;
			for(int i = 0; i < (int)_dm_pack->ppseg_cnt; ++i)
			{
				tmp_id = ((_dm_pack->ppseg)[i])->prop;
				
				if((mLandmarkInfo[tmp_id]).type == 2 || (mLandmarkInfo[tmp_id]).type ==3 )
				{

					if (city_id !=0 && IsFatherSon(city_id,tmp_id) ==0)
					{
						nx_log(NGX_LOG_NOTICE,"city conflicts: old[%d] new[%d]",  tmp_id,city_id);
						continue;
					}
					city_id = tmp_id;

					nx_log(NGX_LOG_NOTICE,"ambiguous city : [%d]",city_id);
				}

			}

		}

		if(city_id <=0)
		{
			nx_log(NGX_LOG_NOTICE,"no city ");
		}

		// find lank mark
		str_tmp = name + des;
		snprintf(buf,10240,"%s",str_tmp.c_str());
		dm_search(mDmdictLandMartk,_dm_pack,buf,strlen(buf),DM_OUT_FMM);
		nx_log(NGX_LOG_DEBUG,"dict match in  line [%s] cnt[%d]",buf,_dm_pack->ppseg_cnt);

		//if (city_id<=0 && _dm_pack->ppseg_cnt <=0)
		if (city_id<=0 )
		{
			fout << tuangou_id << "\t" << "None" << "\t" << "None" <<  "\t" << "None" << endl;
			continue;
		}
		else if (city_id>0 && _dm_pack->ppseg_cnt <=0)
		{
			if(confirm_city ==true)
			{
				fout << tuangou_id << "\t" << mLandmarkInfo[city_id].name << "\t" << "None" <<  "\t" << "sure" << endl;
			}
			else
			{
				fout << tuangou_id << "\t" << mLandmarkInfo[city_id].name << "\t" << "None" <<  "\t" << "probably" << endl;
			}
			continue;
		}

		lm_id=0;
		tmp_id=0;
		for(int i = 0; i < (int)_dm_pack->ppseg_cnt; ++i)
		{
			tmp_id = ((_dm_pack->ppseg)[i])->prop;

			if (IsFatherSon(city_id,tmp_id) == 1)
			{
				lm_id = tmp_id;
				nx_log(NGX_LOG_NOTICE,"get city and land mark: city [%d] lm[%d]",city_id,lm_id);
				break;
			}
		}

		if (lm_id > 0)
		{
			fout << tuangou_id << "\t" << mLandmarkInfo[city_id].name << "\t" << mLandmarkInfo[lm_id].name<<  "\t" << "sure" << endl;
		}
		else
		{
			fout << tuangou_id << "\t" << mLandmarkInfo[city_id].name << "\t" << "None"<<  "\t" << "probably" << endl;
		}


		//nx_log(NGX_LOG_NOTICE,"original data line [%d] parsed over",line_num);
	}
	fin.close();
	fout.close();
	return 0;

}

/**
 * @brief 
 *
 * @Param father
 * @Param son
 *
 * @Returns   0: false  1: true and father is below province  2: father is province
 */
int TuanDanProcessor::IsFatherSon(u_int father,u_int son)
{
	if (mLandmarkInfo[father].type <=1 || mLandmarkInfo[son].type <=1)
	{
		return 0;
	}
	u_int tmp_id;
	tmp_id = son;
	int count =0;
	while (mLandmarkInfo[tmp_id].type >1 && count < 10)
	{
		if (mLandmarkInfo[tmp_id].father_id == father)
		{
			if(mLandmarkInfo[father].type ==1)
			{
				return 2;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			tmp_id = mLandmarkInfo[tmp_id].father_id;
			count ++;
		}

	}
	return 0;
}

// 清空
void TuanDanProcessor::Clear()
{
	/*
	CKeyWordsManager::ReleaseInstance();
	CTextKeyTool::ReleaseInstance();
	if(m_pConfigInfo != NULL)
	  delete m_pConfigInfo;
	m_pConfigInfo = NULL;
	*/
}

