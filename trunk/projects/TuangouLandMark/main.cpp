/**
 * @file main.cpp
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-03-14
 */

#include "TuanDanProcessor.h"


using namespace std;

int g_log_level = NGX_LOG_NOTICE;
char g_conf_file[32] ;

static int getParams(int argc, char* argv[])
{
	int ch;
	while ((ch = getopt(argc, argv, "vhl:f:")) != -1)
	{
		switch (ch)
		{
			/*
			case 'v':       // 打印版本信息 
				showVersion();
				exit(0);
				break;
			case 'h':       // 打印help信息 
				usage(argv[0], 1);
				break;
			*/
			case 'l':
				g_log_level = atoi(optarg);
				if (g_log_level < 0 || g_log_level > NGX_LOG_DEBUG)
				{
					g_log_level = NGX_LOG_NOTICE;
				}
				break;
			case 'f':
				snprintf(g_conf_file,32,"%s",optarg);
				break;
			default:
				break;
		}
	}
	return 0;
}
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
	ret = getParams(argc,argv);
	if (ret < 0)
	{
		fprintf(stderr,"init log error");
		return -1;
	}
	ret = nx_log_init("./logs/TuangouLandMark.",g_log_level);
	if (ret < 0)
	{
		fprintf(stderr,"init log error");
		return -1;
	}

	TuanDanProcessor* pProcessor = new TuanDanProcessor();;
	ret = pProcessor->TDProcessorInit(g_conf_file);
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"Init end sucessfully!");
	ret = pProcessor->TDProcess();
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"get landmark sucessfully!");


	return 0;
}

