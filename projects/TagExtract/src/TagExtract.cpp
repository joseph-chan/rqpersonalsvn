

#include "TagExtractor.h"

/**
 * @file TagExtract.cpp
 * @brief 
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 
 * @date 2011-12-07
 */
using namespace std;

/**
 * @brief 
 *
 * @Param argc
 * @Param argv[1] ÅäÖÃÎÄ¼ş
 *
 * @Returns   
 */
int main(int argc, char* argv[])
{
	if(argc<2)
	{
		cout<<"usage: ./" << argv[0] <<" conf_file " <<endl;
		return 0;
	}
	int ret=0;
	ret = nx_log_init("./logs/tagextract.");
	if (ret < 0)
	{
		fprintf(stderr,"init log error");
		return -1;
	}

	TagExtractor* pTagEx = new TagExtractor();
	ret = pTagEx->TagExtractInit(argv[1]);
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"Init end sucessfully!");
	ret = pTagEx->LoadWeibo();
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"load weibo sucessfully!");

	ret = pTagEx->ExtractTag();
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"extract tag sucessfully!");

	delete pTagEx;
	nx_log_write(NGX_LOG_WARN,"Program end sucessfully!");


	return 0;
}
