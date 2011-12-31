/**
 * @file MakeWordNetData.cpp
 * @brief 
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 
 * @date 2011-12-22
 */

#include "WeiboProcessor.h"


using namespace std;

/**
 * @brief 
 *
 * @Param argc
 * @Param argv[1] ≈‰÷√Œƒº˛
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

	WeiboProcessor* pWBProcessor = new WeiboProcessor();;
	ret = pWBProcessor->WBProcessorInit(argv[1]);
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"Init end sucessfully!");
	ret = pWBProcessor->LoadWeiboToIndex();
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"load weibo sucessfully!");
	ret = pWBProcessor->DumpIndex();
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"dump index sucessfully!");


	return 0;
}

