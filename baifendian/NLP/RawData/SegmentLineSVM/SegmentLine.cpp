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

using namespace std;


CConfigInfo * pConfigInfo;

//extern void write_prop_to_str(scw_property_t& property, char* buffer, int wdtype);


#define MAX_LINE_SIZE 1000*1024 

bool CmpInt(int a, int b )
{
	return a<b;
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
	pConfigInfo = new CConfigInfo(argv[1]);

	BfdSegment *pSegmentor;
	pSegmentor = new BfdSegment;
	pSegmentor->BfdSegment_Init(pConfigInfo);


	FileToBeSeg = fopen(pConfigInfo->GetValue("file_to_be_segment").c_str(),"r");
	assert(FileToBeSeg != NULL && "file not exist!");


	ofstream output(pConfigInfo->GetValue("output_fname").c_str());
	assert(output.is_open() == true);

	ofstream output_hash(pConfigInfo->GetValue("output_hash_fname").c_str());
	assert(output_hash.is_open() == true);

	ofstream new_word_output(pConfigInfo->GetValue("new_word_output_fname").c_str());
	assert(new_word_output.is_open() == true);

	ofstream worddict_output(pConfigInfo->GetValue("worddict_output_fname").c_str());
	assert(worddict_output.is_open() == true);

	ofstream catedict_output(pConfigInfo->GetValue("category_dict_fname").c_str());
	assert(catedict_output.is_open() == true);

	ofstream label_output(pConfigInfo->GetValue("label_output_fname").c_str());
	assert(label_output.is_open() == true);

	omit_short_term = boost::lexical_cast<int>(pConfigInfo->GetValue("omit_short_term"));
	omit_digit_term = boost::lexical_cast<int>(pConfigInfo->GetValue("omit_digit_term"));

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
			output<< line_label <<endl;
			continue;
		}
		bool no_word=true;
		for(int i=0;i<word_count;i++)
		{
			std::cout << seg_out[i] << "\t";
			if ( (omit_short_term && seg_out[i].size()<=3 )  || (omit_digit_term && is_digit(seg_out[i])))
			{
				DLOG(INFO) << "term too short,delete [" << seg_out[i] << "]";
				continue;
			}
			no_word=false;
			// do not have this word
			if(global_word_id.find(seg_out[i])==global_word_id.end())
			{
				global_word_id[seg_out[i]] = word_num;
				word_num++;
			}

			// count word in line
			// do not have this word in current line
			if(word_in_line.find(seg_out[i]) == word_in_line.end())
			{
				word_in_line[seg_out[i]]=1;
			}
			else
			{
				word_in_line[seg_out[i]] +=1;
			}
		}
		std::cout << std::endl;
		if (no_word == true)
		{
			continue;
		}

		if (label_id.find(line_label) == label_id.end())
		{
			label_id[line_label] =label_num;
			label_num++;
		}
		//output the words in current line;
		output << line_label << "\t";
		label_output << label_id[line_label] << "\n";

		vector<int> word_hash;
		map<int,int> wordhash_in_line;
		for(map<string,int>::iterator it = word_in_line.begin(); it != word_in_line.end(); ++it)
		{
			output << it->first << ":" << it->second << " ";
			word_hash.push_back(global_word_id[it->first]);
			wordhash_in_line[global_word_id[it->first]] = it->second;
		}

		//output words hash sorted
		sort(word_hash.begin(),word_hash.end(),CmpInt);

		for(vector<int>::iterator it = word_hash.begin();it != word_hash.end();it++)
		{
			output_hash << *it << ":" << wordhash_in_line[*it] << " " ;
		}
		output << endl;
		output_hash << endl;

		for(int i=0; i <new_word.size() ; i++)
		{
			new_word_output << new_word[i].first << ":" << new_word[i].second << " ";
		}
		new_word_output << endl;
	}
	for(boost::unordered_map<string,uint64_t>::iterator it = global_word_id.begin();it!=global_word_id.end();it++)
	{
		worddict_output << it->first << "\t" << it->second << endl;
	}
	for(boost::unordered_map<string,uint64_t>::iterator it = label_id.begin();it!=label_id.end();it++)
	{
		catedict_output << it->first << "\t" << it->second<< endl;
	}
	return 0;
}
