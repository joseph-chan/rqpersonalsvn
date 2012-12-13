//============================================================================
// Name        : Configuration.h
// Created on  : Jul 21, 2010
// Author      : Zhijin GUO
// Copyright   : Baifendian Information Technology
// Description :
//============================================================================


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

// system library header file
#include <string>

// third party header file
#include <boost/program_options.hpp>

using namespace std;

/** baifendian tokenizar namespace*/
namespace baifendian_tokenizar {
/** Namespace for the option. */
namespace tokenizar_config {
	struct ConfigVariable {
		// 0 ICTCLAS配置文件名
		string ICTCLAS_conf;
		// 1 TC的存储文件名 存储索引等信息
		string newsdb_file;
		// 2 停用词表文件名
		string stopword_file;
		// 3 用户词典的path
		string userdict_file;
		// 4 关键词的词性
		string keyword_postag;
		// 5 分词方法，目前只有一种，即中科院分词
		int seg_method;
		// 6 抽取关键词返回数量
		int keyword_n;
		// 7 网络新词的长度限制
		int hotword_length;
		// 8 时间间隔(hour)，影响到重分词和去重的group_id
		int expire_time;
		// 9 保存一次索引间隔的新闻条数
		int interval_save_index;
		// 10 去重, 相同关键词数量百分比 f, 超过该比例重复关键词，则认为是重复新闻，即groupid相同
		double group_keyword_factor;
		// 11 关键词标题加权因子
		double title_factor;
	};

	class Config{
	public:
		explicit Config(const string & config_file_path) : m_confPath(config_file_path) {}
		// init m_var_map
		bool initConfigVariable();
		void setConfigVariable(ConfigVariable & cfg_var);
	private:
		const string m_confPath;
		boost::program_options::variables_map m_var_map;
	};

}// end of namespace tokenizar_config
}// end of namespace baifendian_tokenizar

#endif /* CONFIGURATION_H_ */
