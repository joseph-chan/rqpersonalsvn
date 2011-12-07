#include "WeiboProcessor.h"
#include <iostream>
/**
 * @file HotBlog.cpp
 * @brief 获取财经和体育类热门微博的主程序
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

using namespace std;


ngx_log_t        *gNxlog;

/**
 * @brief 
 *
 * @Param argc
 * @Param argv[0] 配置文件
 *
 * @Returns   
 */
int main(int argc, char* argv[])
{
	if(argc<2)
	{
		cout<<"usage:<0_HotBlog> <1_config_file> "<<endl;
		return 0;
	}
	int ret=0;
	ngx_time_init();
	gNxlog = ngx_log_init((unsigned char *)"./");
	gNxlog->log_level = NGX_LOG_DEBUG;
	WeiboProcessor* pWbPrcss = new WeiboProcessor();
	ret = pWbPrcss->Init(argv[1]);
	if (ret < 0)
	{
		return -1;
	}
	nx_log_write(NGX_LOG_WARN,gNxlog,"Init end sucessfully!");

	ret = pWbPrcss->ProcessClassifiedData();
	if (ret < 0)
	{
		nx_log_write(NGX_LOG_ERR,gNxlog,"ProcessClassifiedData error");
		return -1;
	}

	ret = pWbPrcss->ProcessOriginalData();
	if (ret < 0)
	{
		nx_log_write(NGX_LOG_ERR,gNxlog,"ProcessOriginalData error");
		return -1;
	}

	ret = pWbPrcss->ProcessForwardData();
	if (ret < 0)
	{
		nx_log_write(NGX_LOG_ERR,gNxlog,"ProcessForwardData error");
		return -1;
	}
	ret = pWbPrcss->ProcessCommentData();
	if (ret < 0)
	{
		nx_log_write(NGX_LOG_ERR,gNxlog,"ProcessCommentData error");
		return -1;
	}
	ret = pWbPrcss->CalcWeight();
	if (ret < 0)
	{
		nx_log_write(NGX_LOG_ERR,gNxlog,"CalcWeight error");
		return -1;
	}
	ret = pWbPrcss->SortOutput();
	if (ret < 0)
	{
		nx_log_write(NGX_LOG_ERR,gNxlog,"SortOutput error");
		return -1;
	}
	delete pWbPrcss;
	nx_log_write(NGX_LOG_WARN,gNxlog,"Program end sucessfully!");


	return 0;
}
