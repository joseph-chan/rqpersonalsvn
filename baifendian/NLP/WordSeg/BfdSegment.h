/**
 * @file BfdSegment.h
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-08-16
 */

#ifndef __BFDSEGMENT_H__
#define __BFDSEGMENT_H__


#include <map>
#include <set>
#include <vector>
#include <iconv.h>
#include "CStringTool.h"
#include "CTypeTool.h"
#include "CConfigInfo.h"
#include "scwdef.h"

#define NDEBUG 1
#include <glog/logging.h>


#define NEW_TERM_ATOM 1
#define NEW_TERM_MULTI 2
#define MAX_LINE_LEGNTH 1024*512
#define ASSERT(x) { if (!(x)) {LOG(ERROR) << "assertion " << #x  << " failed in file "<<  __FILE__ << " line " << __LINE__ ;exit(1);}}


/** word segment of bfd
 *  a wrap of scw_segment
 *  */
class BfdSegment 
{
public:

	/** default constructor*/
	//BfdSegment();

	/**
	 * @brief 
	 *
	 * @Param path
	 *
	 * @Returns   
	 */
	void BfdSegment_Init(CConfigInfo* pConfigInfo);

	/**
	 * @brief segment
	 *
	 * @Param line
	 * @Param seg_out  segment out terms
	 * @Param new_word  new_word if have
	 *
	 * @Returns   
	 */
	int bfd_segment(string line,vector<string> &seg_out, vector<pair<string,int> > &new_word);

	/**
	 * @brief 
	 *
	 * @Param line
	 * @Param seg_out
	 * @Param new_word
	 *
	 * @Returns   
	 */
	int bfd_segment_utf8(string line,vector<string> &seg_out, vector<pair<string,int> > &new_word);

	int Process_encoding(iconv_t cd,char *addr, size_t len,char *outbuf);
	
	/**
	 * @brief deconstructor
	 */
	~BfdSegment();

private:
	/** config */
	CConfigInfo *mpConfigInfo;
	/** segment dict*/
	scw_worddict_t *mpScwdict;
	/** segment out*/
	scw_out_t * mpScwout;
	/** segment flag*/
	u_int mScwFlag;
	/** iconv buf*/
	char * mpInbuf;
	char * mpOutbuf;

};
#endif
