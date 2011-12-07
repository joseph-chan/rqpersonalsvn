#include <stdio.h>

#include "test.h"
//#include "nx_log.h"
#include "math.h"
//using namespace std;
int main()
{

	int ret;
    ngx_log_t*  pNxlog;
    pNxlog = ngx_log_init((unsigned char *)"temp");

	ret = nx_log_init("logs/test.");
	if (ret < 0) {
		return 0;
	}
	ret = 3;

	nx_log_write(NGX_LOG_ALERT, "failed before execut%s %d ing new binary process path %d","agc",3,ret);
	//nx_log(NGX_LOG_NOTICE, "failed b new binary process path %d",ret);
	ngx_log_error(NGX_LOG_ALERT, pNxlog,0,"failed before executing new binary process path %d",ret);

	nx_log(NGX_LOG_ALERT,"failed before executing new binary process path %d",ret);


	sleep(1);
	nx_log_write(NGX_LOG_ALERT, "failed before executing new binary process path %d",ret);
	sleep(1);
	nx_log_write(NGX_LOG_ALERT, "failed before executing new binary process path %d",ret);
	sleep(1);
	nx_log_write(NGX_LOG_ALERT, "failed before executing new binary process path %d",3);
	//nx_log(NGX_LOG_NOTICE, "failed b new binary process path %d",ret);
	/*
	   int a=0x40000000;
	   int b;
	   b = (a & 0xffc00000) >> 22;

	   int c=32,d=1;
	   stringstream ss;
	   ss.str("");
	   vector<string> vOutput;


	   ss<< a;


	vOutput.push_back(ss.str());
	ss<< b;
	cout << vOutput.size() << endl;

	
	 vOutput.clear();
	cout << vOutput.size() << endl;

	ss.clear();
	ss.str("");
	ss<< c;

	bool e = (c & 0x20) >>5;
	bool f = (d & 0x1);

	c= c * (((float(d)/10.0) + 1.0));
	string g="abcdefgkhtl";
	*/


}
