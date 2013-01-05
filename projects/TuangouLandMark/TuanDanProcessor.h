/**
 * @file TuanDanProcessor.h
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-03-14
 */

#ifndef TUANDANPROCESSOR_H
#define TUANDANPROCESSOR_H

#include "CStringTool.h"
#include "CTypeTool.h"
#include "CConfigInfo.h"
#include "CResource.h"
#include <dictmatch.h>
//#include "ustring.h"
#include "nx_log.h"
#include <map>
#include <set>
#include <vector>
#include <math.h>
#include <fstream>
#include <iostream>
#include "ac.h"


#define CUSTOMER_SLOT 0
#define ITEM_ID_SLOT  1
#define	ITEM_NAME_SLOT  2
#define	ITEM_LINK_SLOT  3
#define	IMAGE_LINK_SLOT  4
#define	DESCRIPTION_SLOT  5
#define	PRICE_SLOT  6
#define	CATEGORY_SLOT  7
#define	IS_ACTIVE_SLOT  8
#define	CREATION_TIME_SLOT  9
#define	L_CUSTOMER_SLOT  10

#define MAX_NAME_LEN 128
#define MAX_TYPE_LEN 32
#define MAX_ADDR_LEN 256

//#include "lookup.h"

using namespace std;

typedef struct _strict_item_t
{
	int id;
	int father_id;
	char name[MAX_NAME_LEN];
	int type;
}strict_item_t;

typedef struct _shop_item_t
{
    int id;
    char type[MAX_TYPE_LEN];
    char name[MAX_NAME_LEN];
    char addr[MAX_ADDR_LEN];
    char tag[MAX_NAME_LEN];
}shop_item_t;

class TuanDanProcessor
{
	public:

		TuanDanProcessor();
		~TuanDanProcessor();

	public:
		/**
		 * @brief 初始化数据、字典
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int TDProcessorInit(const char  *pConfigFile);
		/**
		 * @brief 所有数据
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */
		int TDProcess();

	private:
		/**
		 * @brief 一行数据
		 *
		 * @Param cResource
		 *
		 * @Returns   
		 */

		int TDProcessLineCity(string line);
		int TDProcessLineLM(string line);
		int TDProcessLineShop(string line);

		int LoadInfo(const char * city_data, const char * landmark_data,const char * shop_data);
		//int DMSearch(dm_dict_t * dm_dict, dm_pack_t * dm_pack,const char * query);
		int IsFatherSon(int  father,int son);
		dm_dict_t* DmDictLoad(const char* fullpath, int lemma_num);
		void CutTail(char * word);

		void Clear();

/****************************以下为数据成员区**************************************/
	private:

		// 基本配置信息
		CConfigInfo* m_pConfigInfo;

		map<int,strict_item_t> mLandmarkInfo;
		map<int,strict_item_t> mCityInfo;
		map<int,shop_item_t> mShopInfo;

		Aho_Corasick* acCity;
		Aho_Corasick* acLM;
		Aho_Corasick* acShop;
		
};
#endif
