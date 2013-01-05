/**
 * @file MakeWordNetData.cpp
 * @brief 
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 
 * @date 2011-12-22
 */

#include "PrepareTrainingData.h"


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
	ret = nx_log_init("./logs/prewordseg.",g_log_level);
	if (ret < 0)
	{
		fprintf(stderr,"init log error");
		return -1;
	}

	PrepareTrainingData* pWBProcessor = new PrepareTrainingData();;
	ret = pWBProcessor->WBProcessorInit(g_conf_file);
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"Init end sucessfully!");
	ret = pWBProcessor->WeiboToTrainingData();
	if (ret < 0)
	{
		return -1;
	}
	nx_log(NGX_LOG_WARN,"wordseg sucessfully!");


	return 0;
}

