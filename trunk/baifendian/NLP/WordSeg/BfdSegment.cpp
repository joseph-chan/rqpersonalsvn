/**
 * @file BfdSegment.cpp
 * @brief 
 * @author RuQiang (qiang.ru@baifendian.com)
 * @version 
 * @date 2012-08-16
 */

#include "BfdSegment.h"

using namespace std;

/**
 * @brief 
 *
 * @Param path
 *
 * @Returns   
 */
void BfdSegment::BfdSegment_Init(CConfigInfo * pConfigInfo)
{
	mpScwdict=NULL;
	mpScwout=NULL;
	mScwFlag= SCW_OUT_ALL | SCW_OUT_PROP;


	ASSERT(pConfigInfo!=NULL);

	mpConfigInfo = pConfigInfo;

	mpScwdict=scw_load_worddict(mpConfigInfo->GetValue("wordseg_dict").c_str());
	ASSERT(mpScwdict!=NULL);

	mScwFlag = SCW_OUT_ALL | SCW_OUT_PROP;
	mpScwout=scw_create_out(100000, mScwFlag);
	ASSERT(mpScwout !=NULL);

	mpInbuf = (char*) malloc(sizeof(char) * MAX_LINE_LEGNTH);
	mpOutbuf = (char*) malloc(sizeof(char) * MAX_LINE_LEGNTH);
	ASSERT(mpInbuf !=NULL && mpOutbuf !=NULL);

}

int BfdSegment::bfd_segment_utf8(string line,vector<string> &seg_out, vector<pair<string,int> > &new_word)
{
	int len,ret,term_num;
	iconv_t cd;
	len = line.size();
	if (len <=0) return 0;

    cd = iconv_open ("gbk//IGNORE","utf8");
  	if (cd == (iconv_t) -1) 
	{
		LOG(WARNING) << "iconv_open error";
		return -1;
	}

	snprintf(mpInbuf,len+1,"%s",line.c_str());
	
	DLOG(INFO) << "segment utf8 in " << mpInbuf;
	ret = Process_encoding(cd, mpInbuf,len,mpOutbuf);
	ASSERT(ret >=0);

	term_num = bfd_segment(mpOutbuf,seg_out,new_word);
	if (term_num<=0 || term_num!=seg_out.size())
	{
		LOG(WARNING) << "word segment may be error";
		return 0;
	}

    cd = iconv_open ("utf8//IGNORE","gbk");
  	if (cd == (iconv_t) -1) 
	{
		LOG(WARNING) << "iconv_open error";
		return -1;
	}
	//convert seg_out to utf8
	for(int i = 0 ;i<seg_out.size();i++)
	{
		len = seg_out[i].size();
		strncpy(mpInbuf,seg_out[i].c_str(),len+1);
		ret = Process_encoding(cd,mpInbuf,len+1,mpOutbuf);
		ASSERT(ret >=0);
		seg_out[i]=mpOutbuf;
		DLOG(INFO) << "segment out " << i << "th term: [" << mpOutbuf << "]";
	}
	//convert new_word to utf8
	for(int i = 0 ;i<new_word.size();i++)
	{
		len = new_word[i].first.size();
		strncpy(mpInbuf,new_word[i].first.c_str(),len+1);
		ret = Process_encoding(cd,mpInbuf,len+1,mpOutbuf);
		ASSERT(ret >=0);

		new_word[i].first=mpOutbuf;
		DLOG(INFO) << "new word out " << i << "th term: [" << mpOutbuf << "]";
	}
	iconv_close(cd);
	return term_num;
}

int BfdSegment::bfd_segment(string line,vector<string> &seg_out, vector<pair<string,int> > &new_word)
{
	int ret,pos,len;
	char word[MAX_WORD_LEN];
	ret = scw_segment_words(mpScwdict,mpScwout,line.c_str(),line.size());

	if(mpScwout->wpbtermcount <=0)
	{
		return 0;
	}
	seg_out.clear();
	new_word.clear();

	for(int i=0; i < mpScwout->wpbtermcount ; i++)
	{
		pos = GET_TERM_POS(mpScwout->wpbtermpos[i]);
		len = GET_TERM_LEN(mpScwout->wpbtermpos[i]);
		strncpy(word, mpScwout->wpcompbuf + pos, len);
		word[len] = 0;
		seg_out.push_back(word);
	}

	scw_newword_t* pnewword = mpScwout->pnewword;
	for(int i = 0; i<pnewword->newwordbtermcount; i++)
	{
		pos = GET_TERM_POS(pnewword->newwordbtermpos[i]);
		len = GET_TERM_LEN(pnewword->newwordbtermpos[i]);
		strncpy(word, pnewword->newwordbuf + pos, len);
		word[len] = 0;
		if(IS_MULTI_TERM(pnewword->newwordbtermprop[i])) 
		{
			new_word.push_back(pair<string,int>(word,NEW_TERM_MULTI));
		}
		if(IS_NEWWORD(pnewword->newwordbtermprop[i])) 
		{
			new_word.push_back(pair<string,int> (word,NEW_TERM_ATOM));
		}
	}
	return mpScwout->wpbtermcount;
}

int BfdSegment::Process_encoding(iconv_t cd, char *addr, size_t len,char *outbuf)
{
	const char *start = addr;
	char *outptr;
	size_t n;
	int ret = 0;
	size_t outlen=0;
	int out_cur=0;
	int omit_invalid;

	omit_invalid=1;


	while (len > 0)
	{
		outptr = outbuf;
		outlen = MAX_LINE_LEGNTH;
		n = iconv (cd, &addr, &len, &outptr, &outlen);

		// invalid multibyte and need omit
		if (n == (size_t) -1 && omit_invalid && errno == EILSEQ)
		{
			ret = 1;
			// inbytesleft =0, means all inbuf is converted
			// set n=0, can output now
			if (len == 0)
				n = 0;
			else
				errno = E2BIG; // do not enter the switch-case
		}

		if (outptr != outbuf)
		{
			out_cur = outptr-outbuf;
			outbuf[out_cur]='\0';
			break;
		}

		if (n != (size_t) -1)
		{
			/* All the input test is processed.  For state-dependent
			   character sets we have to flush the state now.  */
			outptr = outbuf;
			outlen = MAX_LINE_LEGNTH;
			n = iconv (cd, NULL, NULL, &outptr, &outlen);

			if (outptr != outbuf)
			{
				out_cur = outptr-outbuf;
				outbuf[out_cur]='\0';
				break;
			}

			if (n != (size_t) -1)
				break;

			if (omit_invalid && errno == EILSEQ)
			{
				ret = 1;
				break;
			}
		}

		if (errno != E2BIG)
		{
			/* iconv() ran into a problem.  */
			switch (errno)
			{
				case EILSEQ:
					break;
				case EINVAL:
					break;
				case EBADF:
					break;
				default:
					break;
			}
			return -1;
		}
	}

	return ret;
}
