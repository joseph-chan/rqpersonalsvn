/**
* @file WordNetIndexor.cpp
* @brief 
* @author RuQiang (ruqiang@staff.sina.com.cn)
* @version 
* @date 2011-12-30
*/
#include "WordNetIndexor.h"
using namespace std;


/**
 * @brief Constructor
 */
WordNetIndexor::WordNetIndexor()
{
}

/**
 * @brief Destructor
 */
WordNetIndexor::~WordNetIndexor()
{
	//Clear();
}

/*
int WordNetIndexor::DumpIndex()
{
	FILE* fp_index1;
	FILE* fp_index2;
	FILE* fp_indexn;
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
		ret1 = fwrite(&(m_term_num),sizeof(int),1,fp_indexn);


	FILE* fp_term_info;
	vector<term_info>::iterator iterv;

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
	vector<relate_term_info>::iterator iterRelateInfo;

	fp_relate_info=fopen("relate_info.index","w");
	if (NULL ==fp_relate_info)
	{
		nx_log(NGX_LOG_ERR,"dump index error: can't not open file");
		return -1;
	}
	for(iterRelateInfo= mRelateInfo.begin();iterRelateInfo!=mRelateInfo.end();iterRelateInfo++)
	{
		ret1= fwrite(&(*iterRelateInfo),sizeof(relate_term_info),1,fp_relate_info);
		if(ret1 != 1)
		{
			nx_log(NGX_LOG_ERR,"dump index error: can't not open file");
			return -1;
		}
	}
	return 0;

}
*/
int WordNetIndexor::LoadIndex(const char* word_index,const char* term_info,const char * relate_info,__gnu_cxx::hash_map<u_int,u_int> & TermIndex, vector<term_info_t> &TermInfo,vector<relate_term_info_t> &RelateInfo,total_index_info_t *total_info)
{
	FILE* fp_index1;
	FILE* fp_index2;
	FILE* fp_indexn;
	FILE* fp_term_info;
	FILE* fp_relate_info;
	int ret1,ret2;
	char buf[256];

	snprintf(buf,100,"%s.ind1",word_index);
	fp_index1=fopen(buf,"r");
	
	snprintf(buf,100,"%s.ind2",word_index);
	fp_index2=fopen(buf,"r");

	snprintf(buf,100,"%s.n",word_index);
	fp_indexn=fopen(buf,"r");

	snprintf(buf,100,"%s",term_info);
	fp_term_info=fopen(buf,"r");

	snprintf(buf,100,"%s",relate_info);
	fp_relate_info=fopen(buf,"r");

	if (NULL ==fp_index1 || NULL == fp_index2 || NULL == fp_indexn || NULL == fp_term_info || NULL == fp_relate_info)
	{
		nx_log(NGX_LOG_ERR,"load index error: can't not open file");
		return -1;
	}

	// 获取四个总数
	string str_tmp;
	vector<string> v_tmp;
	fgets(buf,256,fp_indexn);
	str_tmp = buf;
	v_tmp = CStringTool::SpliteByChar(str_tmp,':');
	if(v_tmp.size() != 2)
	{
		nx_log(NGX_LOG_ERR,"load index error: can't not open file");
		return -1;
	}
	total_info->term_num= CTypeTool<u_int>::StrTo(v_tmp[1]);

	fgets(buf,256,fp_indexn);
	str_tmp = buf;
	v_tmp = CStringTool::SpliteByChar(str_tmp,':');
	if(v_tmp.size() != 2)
	{
		nx_log(NGX_LOG_ERR,"load index error: can't not open file");
		return -1;
	}
	total_info->relate_term_num= CTypeTool<u_int>::StrTo(v_tmp[1]);

	fgets(buf,256,fp_indexn);
	str_tmp = buf;
	v_tmp = CStringTool::SpliteByChar(str_tmp,':');
	if(v_tmp.size() != 2)
	{
		nx_log(NGX_LOG_ERR,"load index error: can't not open file");
		return -1;
	}
	total_info->sorted_relate_term_num= CTypeTool<u_int>::StrTo(v_tmp[1]);
	//a patch. sorted_relate_term_num and relate_term_num should be one;
	total_info->relate_term_num= total_info->sorted_relate_term_num;


	fgets(buf,256,fp_indexn);
	str_tmp = buf;
	v_tmp = CStringTool::SpliteByChar(str_tmp,':');
	if(v_tmp.size() != 2)
	{
		nx_log(NGX_LOG_ERR,"load index error: can't not open file");
		return -1;
	}
	total_info->weibo_num= CTypeTool<unsigned long long>::StrTo(v_tmp[1]);



	//读取term index
	for(u_int i =0; i < total_info->term_num; i++)
	{
		u_int hash_value;
		u_int index_num;
		ret1 = fread(&(hash_value),sizeof(u_int),1,fp_index1);
		ret2 = fread(&(index_num),sizeof(u_int),1,fp_index2);
		if(ret1 !=  1 || ret2 != 1)
		{
			nx_log(NGX_LOG_ERR,"load index error: can't not load file [%d] [%d]",ret1,ret2);
			return -1;
		}
		if (index_num>=total_info->term_num)
		{
			nx_log(NGX_LOG_ALERT,"load index error: index num exceed max num [%d] [%d]",index_num,total_info->term_num);
			return -1;
		}
		TermIndex.insert(pair<u_int,u_int>(hash_value,index_num));
		nx_log(NGX_LOG_DEBUG,"load index:term index: term_hash [%ud] index [%ud]",hash_value,index_num);
	}

	//读取term info
	for(u_int i = 0; i< total_info->term_num;i++)
	{
		term_info_t tmp_term;
		u_int tmp_hash;
		ret1 = fread(tmp_term.termstr,sizeof(char),MAX_TERM_LENGTH,fp_term_info);
		ret1 += fread(&(tmp_term.df_history),sizeof(int),1,fp_term_info);
		ret1 += fread(&(tmp_term.df_now),sizeof(int),1,fp_term_info);
		ret1 += fread(&(tmp_term.relate_term_num),sizeof(int),1,fp_term_info);
		if (ret1 != ( MAX_TERM_LENGTH + 3))
		{
			nx_log(NGX_LOG_ERR,"load index error: read index error");
			return -1;
		}
		tmp_hash = hashlittle(tmp_term.termstr,strlen(tmp_term.termstr),1);
		//merge history info and now info, different from server
		tmp_term.df_history += tmp_term.df_now;
		tmp_term.df_now =0;
		//merge end

		for(u_int j=0;j<tmp_term.relate_term_num;j++)
		{
			u_int hash_value;
			u_int index_num;
			ret1 = fread(&(hash_value),sizeof(u_int),1,fp_term_info);
			ret2 = fread(&(index_num),sizeof(u_int),1,fp_term_info);
			if(ret1 != 1 || ret2 !=  1)
			{
				nx_log(NGX_LOG_ERR,"dump index error: write file error");
				return -1;
			}
			if (index_num>=total_info->sorted_relate_term_num)
			{
				nx_log(NGX_LOG_ALERT,"load index error: relate index num exceed max num [%d] [%d]",index_num,total_info->sorted_relate_term_num);
				return -1;
			}

			tmp_term.cooc_term.insert(pair<u_int,u_int>(hash_value,index_num));
			nx_log(NGX_LOG_DEBUG,"load index,relate info: term [%s] [%ud] index df_his [%ud] df_now [%ud] term_total_relate_num [%ud] relate_hash [%ud] relate_index [%ud] now_relate_num [%ud]",tmp_term.termstr,tmp_hash,tmp_term.df_history,tmp_term.df_now,tmp_term.relate_term_num,hash_value,index_num,j);

		}
		TermInfo.push_back(tmp_term);
		nx_log(NGX_LOG_DEBUG,"load index,term info: push_back term [%s] [%ud] df_his [%ud] df_now [%ud] relate_num [%ud]",tmp_term.termstr,tmp_hash,tmp_term.df_history,tmp_term.df_now,tmp_term.relate_term_num);
	}

	for(u_int i =0; i <total_info->sorted_relate_term_num;i++ )
	{
		relate_term_info_t tmp_relate_info;
		ret1= fread(&(tmp_relate_info),sizeof(relate_term_info_t),1,fp_relate_info);
		if(ret1 != 1 || feof(fp_relate_info) >0)
		{
			nx_log(NGX_LOG_ERR,"read index error: can't not open file ! ret [%d]",ret1);
			return -1;
		}
		//merge history info and now info, different from server
		tmp_relate_info.num_history += tmp_relate_info.num_now;
		tmp_relate_info.num_now=0;
		//merge end

		RelateInfo.push_back(tmp_relate_info);	
		nx_log(NGX_LOG_DEBUG,"load relate index:relate info: relate_index [%d] num_his [%ud] num_now [%ud] relation [%ud]",i,tmp_relate_info.num_history,tmp_relate_info.num_now,tmp_relate_info.relation);
	}
	nx_log(NGX_LOG_NOTICE,"load index end ,total info:total_weibo [%uL] total_term [%d] total_relation [%d]",total_info->weibo_num,total_info->term_num,total_info->sorted_relate_term_num);
	return 0;

}	
