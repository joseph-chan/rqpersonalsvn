#include <stdio.h>
#include <ext/hash_map>
#include <string>
#include <queue>
#include <set>

#include "test.h"
//#include <ext/hash_fun.h>
#include <map>
#include <vector>
#include <utility>
//#include "nx_log.h"
#include "math.h"
#include <iconv.h>
//#include "lookup.h"
#include "json/json.h"
#include "json/reader.h"

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>   

#include <assert>
#include "leveldb/db.h"

using namespace std;

int omit_invalid=1;
#define int2ll(a,b) (((long long) a << 32) + b)
bool cmpr(int i, int j) { return i<j; }
struct tst{
	int a;
	long b;
	char c;
	char * d;
};

	static int
process_block (iconv_t cd, char *addr, size_t len ,char* outputbuf, size_t *outputlen)
{
#define OUTBUF_SIZE	32768
	const char *start = addr;
	char outbuf[32768];
	char *outptr;
	size_t n;
	size_t outlen;
	int ret = 0;

	outputbuf[0]='\0';

	while (len > 0)
	{
		cout << "in encoding input " << addr << "outbuf " << outbuf << " outlen " << outlen <<endl;
		outptr = outbuf;
		outlen = OUTBUF_SIZE;
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
			*outputlen = outptr-outbuf;
			snprintf(outputbuf,outptr-outbuf,"%s",outbuf);
		}

		if (n != (size_t) -1)
		{
			/* All the input test is processed.  For state-dependent
			   character sets we have to flush the state now.  */
			outptr = outbuf;
			outlen = OUTBUF_SIZE;
			n = iconv (cd, NULL, NULL, &outptr, &outlen);

			if (outptr != outbuf)
			{
				*outputlen = outptr-outbuf;
				snprintf(outputbuf,outptr-outbuf,"%s",outbuf);
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

typedef struct _relate_term_info_t
{
	u_int num_history; //only record good_term
	u_int num_now;
	u_int relation;
}relate_term_info_t;

class mycomparison
{
	bool reverse;
	public:
	mycomparison(const bool& revparam=false)
	{reverse=revparam;}
	bool operator() (const int& lhs, const int&rhs) const
	{
		if (reverse) return (lhs>rhs);
		else return (lhs<rhs);
	}
	bool operator() (tst& lhs, tst&rhs) const
	{
		if (reverse) return (lhs.a>rhs.a);
		else return (lhs.a<rhs.a);
	}
};


class setcomp {
	public:
		bool operator() (const int& lhs, const int& rhs) const
		{return lhs>rhs;}
		bool operator() (const tst& lhs, const tst& rhs) const
		{return lhs.a>rhs.a;}
};



int main(int argc,char * argv[])
{
	unsigned int nx=10;
	int temp,tmp;
	nx_log_init("logs/temp.",NGX_LOG_DEBUG);
	nx_log(NGX_LOG_NOTICE,"a [%c]" ,nx);



	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	//options.error_if_exists = true;
	leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(status.ok());

	std::string value;
	leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);
	if (s.ok()) s = db->Put(leveldb::WriteOptions(), key2, value);
	if (s.ok()) s = db->Delete(leveldb::WriteOptions(), key1);


	/*

	boost::hash<std::string> string_hash;

	size_t h;
	h = string_hash("Hash me");
	cout << string_hash("Hash me") <<endl;
	cout << h << endl;

	string test ="{\"id\":1,\"name\":\"kurama\"}";
	Json::Reader reader;
	Json::Value value;
	if(reader.parse(test,value))
	{
		if(!value["id"].isNull())
		{
			cout << value["id"].asInt() << endl;
			cout << value["name"].asString()<< endl;
		}
	}
	*/

	return 0;

	/*
	   iconv_t cd;
	   FILE *fp;
	   size_t len=0,outlen=0;
	   cd = iconv_open ("gbk//IGNORE","utf8");
	   if (cd == (iconv_t) -1) cout<< "iconv_open error";

	   fp = fopen(argv[1],"r");
	   char *addr;
	   addr = (char*) malloc(sizeof(char) *32768);
	   char outputbuf[65536];
	   while(fgets(addr,65535,fp)!=NULL)
	   {
	   len = strlen(addr);

	   process_block(cd,addr,len,outputbuf,&outlen);
	   outputbuf[outlen]='\0';
	   cout << "inbuf "<< addr << " outlen "<< outlen<< "outputbuf :[" <<outputbuf <<"]" <<endl;
	   }
	   */
	/*
	   set<string> myset;
	   set<string> myset2;
	   set<string>::iterator it;
	   pair<set<string>::iterator,bool> ret;

	//set some initial values:

	ret = myset.insert("aaa");               // no new element inserted
	ret = myset.insert("aaa");               // no new element inserted

	if (ret.second==false) it=ret.first;  // "it" now points to element 20

	myset.insert (it,"ddd");                 // no max efficiency inserting
	myset.insert (it,"中共");                 // no max efficiency inserting
	myset.insert (it,"中国");                 // max efficiency inserting
	myset.insert (it,"陌生");                 // max efficiency inserting
	myset2.insert (it,"酒店");                 // no max efficiency inserting
	myset2.insert (it,"夏威夷");                 // no max efficiency inserting

	it = myset.find("中国");
	cout << *it <<endl;

	myset.insert(myset2.begin(),myset2.end());

	for (it=myset.begin(); it!=myset.end(); it++)
	cout << " " << *it;
	cout << endl;

	cout << sqrtf(5) << endl;

	return 0;








	char input[]="int.txt";
	vector<int > a;
	vector<int>::iterator b;
	string line;
	ifstream fin(input);
	while(!fin.eof())
	{
	getline(fin, line);
	if(line.size() == 0)
	{
	continue;
	}
	tmp = atoi(line.c_str());
	a.push_back(tmp);
	}
	nx=a.size();
	cout << a.size()<<endl;



	long long ll;
	int a,b;
	a=2;
	b=16;
	ll = int2ll(a,b);
	ll = ((long long) a << 32) + b;
	printf("%x %x %llx",a,b,ll);


	/////////////////////////////////////
	//set
	///////////////////////////////
	multiset<tst,setcomp> fifth;                 // class as Compare
	multiset<tst,setcomp>::iterator iter;                 // class as Compare
	tst a;

	a.a=18;
	fifth.insert(a);
	a.a=20;
	fifth.insert(a);
	a.a=1;
	a.b=2;
	fifth.insert(a);
	a.a=15;
	a.b=15;
	fifth.insert(a);
	a.a=1;
	a.b=3;
	fifth.insert(a);
	a.a=15;
	a.b=15;
	fifth.insert(a);
	a.a=15;
	a.b=15;
	fifth.insert(a);

	tst b;
	b.a=1;
	b.b=4;

	iter = fifth.find(b);
	if ( iter == fifth.end())
	{
		cout<<"not find" << endl;
	}
	else
	{
		for (;iter != fifth.end(); iter++)
		{
			cout << (*( iter )).a << endl;
			cout << (*( iter )).b << endl;
		}
	}

	fifth.clear();

	iter = fifth.find(b);
	if ( iter == fifth.end())
	{
		cout<<"not find" << endl;
	}
	else
	{
		for (;iter != fifth.end(); iter++)
		{
			cout << (*( iter )).a << endl;
			cout << (*( iter )).b << endl;
		}
	}

	//cout<< "##########" << (*fifth.begin()).a<< endl;
	//for(iter = fifth.begin(); iter!=fifth.end();iter++)
	//cout<< (*iter).a <<endl;


	///////////////////////////////////
	//优先队列
	///////////////////////////////


	int myints[]= {10,60,50,20};

	tst a;

	// using mycomparison:
	priority_queue< tst, vector<tst>, mycomparison > mypq(mycomparison(false));

	a.a=1;
	mypq.push(a);
	a.a=18;
	mypq.push(a);
	a.a=14;
	mypq.push(a);
	a.a=12;
	mypq.push(a);
	while (!mypq.empty())
	{
		cout << " " << mypq.top().a;
		cout << " " << mypq.top().b;
		mypq.pop();
	}
	cout << endl;

	return 0;
	*/


		/*
		   nx = hashlittle("刘冬", 4, 1);
		   printf("hash %u \n",nx);
		   nx = hashlittle("刘冬", 4, 2);
		   printf("hash %u \n",nx);
		   nx =  __gnu_cxx::__stl_hash_string("建筑师");
		   cout << nx <<endl;
		   cout << __gnu_cxx::__stl_hash_string("理念") <<endl;



		//std::string strValue = "{\"key1\":\"value1\",\"array\":[{\"key2\":\"value2\"},{\"key2\":\"value3\"},{\"key2\":\"value4\"}]}";
		string strValue = "{\"flag\":0,\"id\":\"3394640411870445\",\"uid\":\"2010072132\",\"time\":\"2011-12-26 00:00:07\",\"timestamp\":\"1324828807\",\"filter\":\"0\",\"topic\":\"\",\"ttopic\":\"\",\"content\":\"天青色等烟雨，而我在等你~               \",\"frommid\":\"\",\"fromuid\":\"\",\"text\":\"\",\"url_yc\":\"\",\"url_zf\":\"\",\"yc_source\":\"皮皮时光机\",\"zf_source\":\"\"}\n";

		Json::Reader reader;
		Json::Value value;


		if (true==reader.parse(strValue, value))
		{
		std::string out = value["xxkey2"].asString();
		std::cout << value["content"].asString() << std::endl;
		cout << value["flag"].asString() <<endl;
		const Json::Value arrayobj = value["array"];
		for (int i=0; i<arrayobj.size(); i++)
		{
		out = arrayobj[i]["key2"].asString();
		std::cout << out;
		if (i != arrayobj.size() - 1)
		std::cout << std::endl;
		}
		}
		return 0;



		string config_doc;
		char buf[2048];
		FILE* fp;

		fp=fopen("json.txt","r");

		fgets(buf,2048,fp);
		config_doc = buf;
		Json::Value root;   // will contains the root value after parsing.
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( config_doc, root );
		if ( !parsingSuccessful )
		{
		// report to the user the failure and their locations in the document.
		std::cout  << "Failed to parse configuration\n"
		<< reader.getFormattedErrorMessages();
		return;
		}

		// Get the value of the member of root named 'encoding', return 'UTF-8' if there is no
		// such member.
		std::string encoding = root.get("encoding", "UTF-8" ).asString();
		// Get the value of the member of root named 'encoding', return a 'null' value if
		// there is no such member.
		const Json::Value plugins = root["plug-ins"];
		for ( int index = 0; index < plugins.size(); ++index )  // Iterates over the sequence elements.
		loadPlugIn( plugins[index].asString() );

		setIndentLength( root["indent"].get("length", 3).asInt() );
		setIndentUseSpace( root["indent"].get("use_space", true).asBool() );

		// ...
		// At application shutdown to make the new configuration document:
		// Since Json::Value has implicit constructor for all value types, it is not
		// necessary to explicitly construct the Json::Value object:
		root["encoding"] = getCurrentEncoding();
	root["indent"]["length"] = getCurrentIndentLength();
	root["indent"]["use_space"] = getCurrentIndentUseSpace();

	Json::StyledWriter writer;
	// Make a new JSON document for the configuration. Preserve original comments.
	std::string outputConfig = writer.write( root );

	// You can also use streams.  This will put the contents of any JSON
	// stream at a particular sub-value, if you'd like.
	std::cin >> root["subtree"];

	// And you can write to a stream, using the StyledWriter automatically.
	std::cout << root;
	*/


		/*
		   relate_term_info_t a;
		   vector<int> myvector;
		   vector<relate_term_info_t> myvector2;

		// set some content in the vector:
		for (int i=0;i<100000 ; i++) 
		{
		if(myvector2.size() == myvector2.capacity() )
		{
		try
		{
		myvector.push_back(i);
		myvector2.push_back(a);
		}
		catch (bad_alloc err)
		{
		cout << "catch error "<< err.what() <<endl;
		cout << "size: " << myvector2.size() << "\n";
		cout << "capacity: " << myvector2.capacity() << "\n";
		cout << "max_size: " << myvector2.max_size() << "\n";
		myvector2.erase(myvector2.begin(),myvector2.begin()+100);
		}
		}
		else
		{
		myvector.push_back(i);
		myvector2.push_back(a);
		}

		cout << "size: " << myvector.size() << "\n";
		cout << "capacity: " << myvector.capacity() << "\n";
		cout << "max_size: " << myvector.max_size() << "\n";
		cout << "size: " << myvector2.size() << "\n";
		cout << "capacity: " << myvector2.capacity() << "\n";
		cout << "max_size: " << myvector2.max_size() << "\n";
		}	

		long long unsigned int a;
		a=3393190835322539ll;

		printf("%llu\n",a);

		cout << log10f(174053.0/3395.0) << " " << log10f(174053.0/33.0) <<endl;
		cout << pow(3.22,5) << " " << pow(1.67,5) <<endl;

		string test="ggg";
		const char * test2="ggg";
		map<string,string>  aaa;
		aaa["ggg"]="ok";
		if (aaa.find(test) != aaa.end() && aaa.find(test2)!= aaa.end())
		{
		cout << "both ok";
		}
		else
		{
		cout << "no";
		}

		map<string,vector<pair<string,int > > > m;
		vector<pair<string,int > > n;
		m["bcd"].push_back(pair<string,int>("Liaoyuanqing",123));
		cout << m.size() << m["bcd"][0].first << endl;
		cout << (test + "123").c_str() <<endl;

		vector<tst> v;
		struct tst a={1,2,3,NULL};
		v.push_back(a);
		a.a=2;
		a.b=3;
		v.push_back(a);
		cout << v.size()<<endl;
	cout << v[1].a <<v[1].b << v[0].a << v[0].b <<endl;
	*/


		//hash_map
		/*
		   int temp;
		   __gnu_cxx::hash_map<int, int > hm;
		   temp = __gnu_cxx::__stl_hash_string("sdkjf");
		   hm[temp]=1;
		   hm[2]=2;
		   hm[3]=3;
		   __gnu_cxx::hash_map<int,int>::iterator iterhm;
		   for(iterhm = hm.begin();iterhm != hm.end();iterhm++)
		   {
		   cout << iterhm->first << " " << iterhm->second << endl;

		   }
		   iterhm = hm.find(temp);
		   if(iterhm!= hm.end())
		   {
		   nx_log(NGX_LOG_ALERT,"find %d elemnt: %d ",iterhm->first,iterhm->second);

		   }
		   ngx_log_t*  pNxlog;
		   pNxlog = ngx_log_init((unsigned char *)"temp");
		   int ret;

		   dm_dict_t* _dm_dict=NULL;
		   dm_pack_t* _dm_pack=NULL;

		   nx_log_init("logs/test.");
		   _dm_dict = dm_dict_load("./tag.txt", 655360);
		   _dm_pack = dm_pack_create(8);

		   const char *query = "大明星大明星从军记明星经纪人大明星运动会";
		   dm_search(_dm_dict, _dm_pack, query, strlen(query), DM_OUT_FMM);
		   for(int i = 0; i < (int)_dm_pack->ppseg_cnt; ++i)
		   {
		   nx_log(NGX_LOG_NOTICE,"dm match query: [%s] num [%d]",  ((_dm_pack->ppseg)[i])->pstr,((_dm_pack->ppseg)[i])->prop);
		   }
		   */




		/*
		   vector<struct tst> tst_v;
		   struct tst tsts={1,2,'c'};
		   tst_v.push_back(tsts);

		   nx_log(NGX_LOG_ALERT,"vector.size()  %d vector[0].a %d",tst_v.size(),tst_v[0].a);
		   ret = nx_log_init("logs/test.");
		   if (ret < 0) {
		   return 0;
		   }
		   ret = 3;

		   __gnu_cxx::hash_map<int,int>::iterator iterhm;
		   iterhm = hm.find(temp);
		   if(iterhm!= hm.end())
		   {
		   nx_log(NGX_LOG_ALERT,"find %d elemnt: %d ",iterhm->first,iterhm->second);

		   }

*/


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
