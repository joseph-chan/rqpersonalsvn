#include "TagEXtrtProcessor.h"
#include <iostream>

/**
 * @file TagExtract.cpp
 * @brief 提取tag的主程序
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-12-05
 */

using namespace std;

ngx_log_t        *gNxlog;

/**
 * @brief 
 *
 * @Param argc
 * @Param argv[1] 配置文件
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
	ngx_time_init();
	gNxlog = ngx_log_init((unsigned char *)"./");
	gNxlog->log_level = NGX_LOG_DEBUG;
	TagEXtrtProcessor* pTagEx = new TagEXtrtProcessor();
	ret = pTagEx->Init(argv[1]);
	if (ret < 0)
	{
		return -1;
	}
	nx_log_write(NGX_LOG_WARN,gNxlog,"Init end sucessfully!");

	ret = pTagEx->ProcessOriginalData();
	if (ret < 0)
	{
		nx_log_write(NGX_LOG_ERR,gNxlog,"ProcessClassifiedData error");
		return -1;
	}

	delete pTagEx;
	nx_log_write(NGX_LOG_WARN,gNxlog,"Program end sucessfully!");


	return 0;
}
