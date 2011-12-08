#include "WeiboProcessor.h"
#include <iostream>
/**
 * @file TagExtractor.cpp
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
 * @Param argv[0] ÅäÖÃÎÄ¼þ
 *
 * @Returns   
 */

TagExtractor::TagExtractor()
{
	m_pConfigInfo= NULL;
}

TagExtractor::TagExtractor()
{
	clear();
}

TagExtractor::TagExtractInit(const char *pConfFile)
{
	int ret=0;
	m_pConfigInfo = new CConfigInfo(cResource);


	pWBProcessor = new WeiboProcessor();
	ret = pWBProcessor->WBProcessorInit();

}
