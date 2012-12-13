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
#include "scwdef.h"
#include "BfdSegment.h"
#include <glog/logging.h>

#include "bfd_classtree.h"

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

	BfdClassTree ClassTree;
	ClassTree.BfdClassTree_Init(argv[1]);
	BfdSegment *pSegmentor;
	pSegmentor = new BfdSegment;
	pSegmentor->BfdSegment_Init(pConfigInfo);

	FileToBeSeg = fopen(pConfigInfo->GetValue("file_to_be_segment").c_str(),"r");
	assert(FileToBeSeg != NULL && "file not exist!");

	ofstream output(pConfigInfo->GetValue("output_fname").c_str());
	assert(output.is_open() == true);

	omit_short_term = boost::lexical_cast<int>(pConfigInfo->GetValue("omit_short_term"));
	omit_digit_term = boost::lexical_cast<int>(pConfigInfo->GetValue("omit_digit_term"));

	map<string,pair<string,string> > important_key;
	LoadTag(pConfigInfo->GetValue("tag_file").c_str(),&all_tags);
	LoadImportantKey(pConfigInfo->GetValue("important_key").c_str(),&important_key);

	string line;
	string line_label;
	char line_buf[MAX_LINE_SIZE];
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


	while(fgets(line_buf,MAX_LINE_SIZE-1,FileToBeSeg) != NULL)
	{
		// variant for a line
		u_int pos,len;
		int word_count;
		vector<string> seg_out;
		vector<pair<string,int> > new_word;
		map<string,int> word_in_line;


		line=line_buf;
		vector<string> VStrField;

		if (pConfigInfo->GetValue("data_format_split") == "1")
		{
			VStrField = boost::algorithm::split(VStrField,line,boost::is_any_of("\t"));
			if (VStrField.size()!=5)
			{
				LOG(ERROR)<< "line format error "<< line << "[" << line.size() << "]";
				continue;
			}
			snprintf(line_buf,MAX_LINE_SIZE-1,"%s %s",VStrField[field_name].c_str(),VStrField[field_cate].c_str());
			line_label= VStrField[field_label];
		}

		word_count=pSegmentor->bfd_segment_utf8(line_buf,seg_out,new_word);

		if(word_count <=0)
		{
			continue;
		}
		bool has_key=false;
		string out_line;
		for(int i=0;i<word_count;i++)
		{
			if ( (omit_short_term && seg_out[i].size()<3 )  || (omit_digit_term && is_digit(seg_out[i])))
			{
				LOG(INFO) << "term too short,delete [" << seg_out[i] << "]";
				continue;
			}
			if (all_tags.find(seg_out[i]) == all_tags.end())
			{
				out_line += seg_out[i] + "\tOO\tOO" +"\n";
			}
			else
			{
				bool key_found=false;
				map<string,int>::iterator it_impo_key=all_tags[seg_out[i]].begin();
				for (;it_impo_key!=all_tags[seg_out[i]].end();++it_impo_key)
				{
					//we can find a key in our important key
					if (important_key.find(it_impo_key->first) != important_key.end())
					{
						has_key=true;
						key_found=true;
						out_line += seg_out[i] + "\t" + important_key[it_impo_key->first].first + "\t" +important_key[it_impo_key->first].second + "\n";
						break;
					}

				}
				if (key_found == false)
				{
					out_line += seg_out[i] + "\tOO\tOO" +"\n";
				}

			}
		}
		if (has_key == true)
		{
			output << out_line <<std::endl;
		}
		else
		{
			cout << "no key found in this line " << line << std::endl;

		}
	}

	return 0;
}
