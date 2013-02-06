/**
 * @file SegmentLine.cpp
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-08-10
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "string_utils.h"
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>  
#include <boost/lexical_cast.hpp>
#include "CTypeTool.h"
#include "scwdef.h"
#include "BfdSegment.h"
#include <glog/logging.h>

#include "bfd_classtree.h"
#include "utf8.h"
#include "ac.h"

using std::string;
using std::vector;
using std::map;
using std::cout;


CConfigInfo * pConfigInfo;

//extern void write_prop_to_str(scw_property_t& property, char* buffer, int wdtype);


#define MAX_LINE_SIZE 1000*1024 

bool CmpInt(int a, int b )
{
	return a<b;
}


void LoadTag(const char * filename, map<string,map<string,int> >* all_tags)
{

	FILE* fp=fopen(filename,"r");
	vector<string> vs;
	assert(fp!=NULL);
	char buf[1024*16];
	while(fgets(buf,1024*16,fp))
	{
		boost::algorithm::split(vs,buf,boost::is_any_of("\t"));
		if(vs.size()!=6)
		{
			LOG(ERROR)<< "line format error "<< buf << "["  << "]";
			continue;
		}
		(*all_tags)[vs[4]][vs[3]]=1;
	}
	fclose(fp);
	return;

}

void LoadImportantKey(const char * filename, map<string,pair<string,string> >* important_key)
{

	FILE* fp=fopen(filename,"r");
	vector<string> vs;
	assert(fp!=NULL);
	char buf[1024*16];
	string str_buf;
	while(fgets(buf,1024*16,fp))
	{
		str_buf=buf;
		boost::algorithm::trim(str_buf);

		boost::algorithm::split(vs,str_buf,boost::is_any_of("\t"));
		if(vs.size()!=3)
		{
			LOG(ERROR)<< "line format error "<< str_buf << "["  << "]";
			continue;
		}
		(*important_key)[vs[0]]=pair<string,string>(vs[1],vs[2]);
	}
	fclose(fp);
	return;

}

bool InTag(const int curse,const std::multiset<pair<int,int>, setcomp >& match_set, map<int,string>& TagNameMap){
	for(std::multiset<pair<int,int> , setcomp>::iterator it = match_set.begin(); it!=match_set.end();++it){
		int len=TagNameMap[it->first].size();
		// ac machine match the right offset
		if(curse > it->second-len && curse <= it->second)
		{
			return true;
		}
	}
	return false;
}


/**
 * @brief  
 *
 * @Param curse
 * @Param Scwout
 *
 * @Returns 0: beginning of word   
 * 			1: Middle of word
 * 			2: end of word
 */
int InWord(const int curse,scw_out_t *Scwout){
	int pos,len;
	char buf[1024];
	for(int i = 0;i < Scwout->wpbtermcount; i++ ) {
		pos = GET_TERM_POS(Scwout->wpbtermpos[i]);
		len = GET_TERM_LEN(Scwout->wpbtermpos[i]);

		strncpy(buf,Scwout->wpcompbuf + pos , len);

		buf[len]=0;
		// pos + i : because there is a \0 in every end of word
		if(curse > pos + len -1 - i){
			//LOG(INFO) << "curse < pos :" << buf << "curse:" <<  curse  << " pos: " << pos << " len : " << len;
			continue;
		}
		else if (curse == pos + len -1 - i)
		{
			//LOG(INFO) << "end of word :" << buf << "curse:" <<  curse << "pos: " << pos << " len : " << len;
			return 2;
		}
		else if (curse > pos - i && curse < pos + len - 1 - i)
		{
			//LOG(INFO) << "middle of word :" << buf<< "curse:" <<  curse  << " pos: " << pos << " len : " << len;
			return 1;
		}
		else if(curse == pos - i)
		{
			//LOG(INFO) << "beginning of word :" << buf << "curse:" <<  curse  << " pos: " << pos << " len : " << len;
			return 0;
		}
		else {
			LOG(INFO) << "no match of word :" << buf << "curse:" <<  curse << "pos: " << pos << " len : " << len;
			break;
		}
	}
	return 0;
}

void MergeSet(std::multiset<pair<int,int>, setcomp >& dest_set, const std::multiset<pair<int,int>, setcomp > sour_set){
	for(std::multiset<pair<int,int>, setcomp >::iterator it = sour_set.begin(); it!=sour_set.end();++it){
		dest_set.insert(*it);
	}

}




int main(int argc,char* argv[])
{
	if (argc != 2)
	{
		printf("usage:%s config_file\n",argv[0]);
		return 1;
	}
	int ret;
	FILE *FileToBeSeg;
	int omit_short_term=1;
	int omit_digit_term=1;
	map<string,map<string,int> > all_tags;
	pConfigInfo = new CConfigInfo(argv[1]);

	BfdSegment *pSegmentor;
	scw_out_t* Scwout;
	u_int ScwFlag= SCW_OUT_ALL | SCW_OUT_PROP;

	pSegmentor = new BfdSegment;
	pSegmentor->BfdSegment_Init(pConfigInfo);

	Scwout=scw_create_out(100000, ScwFlag);

	BfdClassTree ClassTree;
	ClassTree.BfdClassTree_Init(argv[1]);

	FileToBeSeg = fopen(pConfigInfo->GetValue("file_to_be_segment").c_str(),"r");
	assert(FileToBeSeg != NULL && "file not exist!");

	ofstream output(pConfigInfo->GetValue("output_fname").c_str());
	assert(output.is_open() == true);

	//omit_short_term = boost::lexical_cast<int>(pConfigInfo->GetValue("omit_short_term"));
	//omit_digit_term = boost::lexical_cast<int>(pConfigInfo->GetValue("omit_digit_term"));

	map<string,pair<string,string> > important_key;
	//LoadTag(pConfigInfo->GetValue("tag_file").c_str(),&all_tags);
	//LoadImportantKey(pConfigInfo->GetValue("important_key").c_str(),&important_key);

	string line_label;
	char line_buf[MAX_LINE_SIZE];
	char name_buf[MAX_LINE_SIZE];
	char word[1024];
	char prop[1024];
	int field_name,field_label,field_cate,field_desc,field_tag;
	field_label=0;
	field_name=1;
	field_tag=2;
	field_cate=3;
	uint64_t word_num=0;
	uint64_t label_num=0;
	//global variant
	boost::unordered_map<string,uint64_t> global_word_id;
	boost::unordered_map<string,uint64_t> label_id;
	map<int,string> TagNameMap = ClassTree.GetTagNameMap();


	while(fgets(line_buf,MAX_LINE_SIZE-1,FileToBeSeg) != NULL)
	{
		vector<string> v_s;
		vector<string> v_s_tmp;
		string line= line_buf;
		boost::algorithm::trim(line);
		snprintf(line_buf,MAX_LINE_SIZE-1,"%s",line.c_str());

		boost::algorithm::split(v_s,line_buf,boost::is_any_of("\t"));
		if (v_s.size() <2 )
		{
			LOG(INFO)<<"line format error" ;
			continue;
		}
		snprintf(name_buf,MAX_LINE_SIZE-1,"%s",v_s[1].c_str());
		boost::algorithm::split(v_s_tmp,v_s[0],boost::is_any_of(":"));

	  	ret = pSegmentor->bfd_segment_utf8_orig(v_s[1], Scwout);

		int cate_id = CTypeTool<int>::StrTo(v_s_tmp[v_s_tmp.size()-1]);
		ClassNode* class_node= ClassTree.GetClassNode(cate_id);
		if (class_node == NULL){
			LOG(INFO)<<"no such class id: [" << cate_id << "]";
			continue;
		}
		//string target_key = "颜色";
		string target_key = pConfigInfo->GetValue("no_seg_target_key");
		vector<string> target_key_list;
		boost::algorithm::split(target_key_list,target_key,boost::is_any_of(":"));

		std::multiset<pair<int,int>, setcomp > total_set;
		for(vector<string>::iterator it_key = target_key_list.begin();it_key!=target_key_list.end();++it_key){

			if (class_node->TagMapAC.find(*it_key) != class_node->TagMapAC.end()){
				std::multiset<pair<int,int>, setcomp > match_set;

				match_set = class_node->TagMapAC[*it_key]->get_match_off(name_buf);
				if(match_set.size()> 0){
					MergeSet(total_set,match_set);
				    //LOG(INFO)<<"merging set ,add [" << match_set.size()<<"] item to total";
				}
				else {
			    	LOG(INFO)<<"merging set : no match ";
				}
			}
			else{
				LOG(INFO)<<"in class_id :[" << cate_id <<"] no such tag key: [" << *it_key << "]";
			}
		}
		if(total_set.size()> 0){

			int i=0;
			char* curse=name_buf;
			char buf[16];
			bool Bflag=true;
			int wordBflag=0;
			while ( *curse!='\0')
			{
				i=0;
				u8_inc(curse,&i);
				snprintf(buf,i+1,"%s",curse);
				if (buf[0]==' ' ){
					curse=curse+i;
					continue;
				}

				wordBflag = InWord(curse-name_buf,Scwout);
				//LOG(INFO)<<"token :[" << buf <<"] curse: [" << curse-name_buf<< "] wordflag: [" << wordBflag <<"]" ;
				//LOG(INFO)<<"token :[" << buf <<"] curse: [" << curse-name_buf<< "] wordflag: [" << wordBflag <<"]" ;
				if (wordBflag==0){

					output<<buf<< "\tB";
				}
				else if (wordBflag==1){
					output<< buf<<"\tM";
				}
				else if (wordBflag==2){
					output<< buf<<"\tE";
				}
				else{
					output<< buf<<"\tN";
				}

				if (InTag(curse-name_buf,total_set,TagNameMap)){

					if(Bflag==true){
						output<< "\tB";
					}
					else{
						output<< "\tE";
					}
					Bflag=false;
					//LOG(INFO)<<"token :[" << buf <<"] is B/E in line curse [" << curse-name_buf<< "]" ;
				}
				else{
					output<<  "\tN";
					Bflag=true;
					//LOG(INFO)<<"token :[" << buf <<"] is N in line, curse [" << curse - name_buf<< "]" ;
				}
				output << std::endl;
				curse=curse+i;
			}
		output<< std::endl;
		}
		else{
			LOG(INFO)<<"in line :[" << line_buf <<"] no match tag ";
		}
	}
	return 0;
}
