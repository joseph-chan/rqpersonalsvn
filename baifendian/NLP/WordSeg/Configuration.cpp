//============================================================================
// Name        : Configuration.cpp
// Created on  : Jul 22, 2010
// Author      : Zhijin GUO
// Copyright   : Baifendian Information Technology
// Description :
//============================================================================

// system library header file
#include <fstream>
#include <iostream>

// local header file
#include "Configuration.h"


/** baifendian tokenizar namespace*/
namespace baifendian_tokenizar {
/** Namespace for the option. */
namespace tokenizar_config {
	bool Config::initConfigVariable() {
		boost::program_options::options_description config_file_options;
		boost::program_options::options_description conf("parameter");
		conf.add_options()("ICTCLAS_conf","conf file name of ICTCLAS")
						("NewsDB_File","file name of news database")
						  ("StopWord_File", "file name of stopword")
						  ("UserDict_File", "user dictionary for segmentation")
						  ("Keyword_Postag", "the postags for the selection of keywords")
						  ("SegMethod", "the method of segmentation")
						  ("Keyword_n", "top n of the keywords")
						  ("HotWord_Length", "the length for hotword constraint")
						  ("Expire_Time",	"expire time for news collection, calc by hours")
						  ("Interval_SaveIndex",	"the interval to save index once")
						  ("GroupKeyword_Factor", "the ratio of same keywords used to identify groupid")
						  ("Title_Factor", "the weight factor for keywords in title");

		config_file_options.add(conf);

		std::ifstream ifs(m_confPath.c_str());
		if (!ifs) {
			std::cout << "cannot open config file!" << std::endl;
			return false;
		} else {
			//std::cout << vm.size() << std::endl;
			boost::program_options::store(boost::program_options::parse_config_file(ifs, config_file_options), m_var_map);
			boost::program_options::notify(m_var_map);
		}

		ifs.close();
		return true;
	}

	void Config::setConfigVariable(ConfigVariable & cfg_var) {
		cfg_var.ICTCLAS_conf = m_var_map["ICTCLAS_conf"].as<string>();
		cfg_var.newsdb_file = m_var_map["NewsDB_File"].as<string>();
		cfg_var.stopword_file = m_var_map["StopWord_File"].as<string>();
		cfg_var.userdict_file = m_var_map["UserDict_File"].as<string>();
		cfg_var.keyword_postag = m_var_map["Keyword_Postag"].as<string>();
		cfg_var.seg_method = boost::lexical_cast<int>(m_var_map["SegMethod"].as<string>());
		cfg_var.keyword_n = boost::lexical_cast<int>(m_var_map["Keyword_n"].as<string>());
		cfg_var.hotword_length = boost::lexical_cast<int>(m_var_map["HotWord_Length"].as<string>());
		cfg_var.expire_time = boost::lexical_cast<int>(m_var_map["Expire_Time"].as<string>());
		cfg_var.interval_save_index = boost::lexical_cast<int>(m_var_map["Interval_SaveIndex"].as<string>());
		cfg_var.group_keyword_factor = boost::lexical_cast<double>(m_var_map["GroupKeyword_Factor"].as<string>());
		cfg_var.title_factor = boost::lexical_cast<double>(m_var_map["Title_Factor"].as<string>());
	}
}// end of namespace tokenizar_config
}// end of namespace baifendian_tokenizar
