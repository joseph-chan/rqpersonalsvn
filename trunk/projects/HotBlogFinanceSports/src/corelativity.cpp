#include"corelativity.h"
#include<fstream>
using namespace std;
#define FILENAME_LEN 1024
Hash *hd=NULL;
int get_filelineCount(const char *filename);
double get_relativity(vector<keyword> &keyseg1,vector<keyword>&keyseg2);

int get_corelativity(WORD_SEGMENT *wordseg_t,WORD_SEGMENT *wordseg_r,char *topic,char * reply,     Core_Information &ans)
{
	vector<keyword>keyseg1,keyseg2;
	int integrate_score=0;
///////////////////////////////////////////////////////////
//topic
	ans.topic_score=checkArticle(wordseg_t,topic,strlen(topic));//正文打分
	if(ans.topic_score>28)//正文根据阈值评价
		ans.topic_value=1;
	if(ans.topic_score<10)
		ans.topic_value=2;
	get_keyword(wordseg_t,keyseg1,20);//得到正文的权重最高的前20个关键词
//////////////////////////////////////////////////////////
//reply
	ans.reply_score=checkArticle(wordseg_r,reply,strlen(reply));//评论打分
	get_keyword(wordseg_r,keyseg2,10);//去关键词
	if(strlen(reply))
		ans.corelativity=get_relativity(keyseg1,keyseg2);//计算相关度
	if(ans.reply_score>15||(ans.reply_score>10&&ans.corelativity))//评论判断直观结果
		ans.reply_value=1;
	if(ans.reply_score<3||(ans.reply_score<5&&ans.corelativity==0))
		ans.reply_value=2;
	//ans.reply_score=ans.reply_score*(1+log(ans.corelativity+1));
///////////////////////////////////////////////////////////////
//integrate
	integrate_score=(ans.topic_value)*(ans.reply_value);
	return integrate_score;
}
////////////////////////////////////////////////////////////////
//相关信息的初始化
int Init_CoreInfor(Core_Information &ans)
{
	ans.topic_score=0.0;
	ans.reply_score=0.0;
	ans.corelativity=0;
	ans.topic_value=0;
	ans.reply_value=0;
	return 0;
}
//////////////////////////////////////////////////////////////////
int mvFileInHash_Value(char *filename)
{
	Hash **hash=&hd;
	int num = get_filelineCount(filename);
	*hash = hdict_create(num * 3, 0, sizeof(double));
	if(!(*hash))
	{
		fprintf(stderr, "NOMEM, hashtable from \"%s\" can not create.\n",filename);
		return FALSE;
	}
#ifdef DEBUG
	fprintf(stderr,"name~~~~~~~~~~~~~~~~~~~~~~~~``:%s,%d\n",filename,num);
#endif
	ifstream fin;
	fin.open(filename);
	if(!fin.is_open())
	{
		cout<<filename<<" can not be opened!"<<endl;
		return -1;
	}
	char buffer[1024];
	char *temp=NULL;
	double core_value=0.0;
	void *handle=NULL;
	while(!fin.eof())
	{
		temp=NULL;
		fin.getline(buffer,1024);
		temp=strrchr(buffer,'\t');
		if(temp==NULL||NULL==temp+1)
			continue;
		core_value=atof(temp+1);
		*temp='\0';
#ifdef DEBUG
		//遍历hash_map，输出
#endif
		handle=handle = hdict_insert(*hash,buffer);
		*(double *)handle=core_value;;
		memset(temp,0,strlen(temp));
	}
	fin.close();
	return 1;
}

int file_getline(FILE *fp,char *buffer,int length)
{
	if( fgets(buffer,length,fp)!=NULL ){
		buffer[length-1]=0;
		buffer[strlen(buffer)-1]='\0';
		return TRUE;
	}
	return FALSE;
}
//////////////////////////////////////////////////////////
int get_filelineCount(const char *filename)
{
	if( strlen(filename) > FILENAME_LEN )
	{
		return FALSE;
	}
	FILE *pp=NULL;
	char tmp[FILENAME_LEN + 7]="";
	char pop_out[1024]="";
	sprintf(tmp,"wc -l %s",filename);
	pp=popen(tmp , "r");
	file_getline(pp,pop_out,1024);
	pclose(pp);
	return atoi(pop_out);
}
////////////////////////////////////////////////////////////
double get_relativity(vector<keyword> &keyseg1,vector<keyword>&keyseg2)
{
	double relativity=0.0,medans=0.0;
	double *pos=NULL;
	void *posv=NULL;
	int yuyinum=0;
	int len1=keyseg1.size();
	int len2=keyseg2.size();
#ifdef DEBUG	//在编译的DEBUG模式下，输出
	cout<<len1<<"\t"<<len2<<endl;
#endif
	char two_word[64];
	if(len2==0)
		return 0;
	for(int i=0;i<len1;i++)
	{
		for(int j=0;j<len2;j++)
		{
			medans=0;
			memset(two_word,0,64);
			if(keyseg1[i].word==keyseg2[j].word)
			{
				relativity+=1;
				continue;
			}
			strcpy(two_word,keyseg1[i].word.c_str());
			strcat(two_word,"\t");
			strcat(two_word,keyseg2[j].word.c_str());
			posv=(hdict_test(hd,two_word));
			if(posv!=NULL)
			{
				pos=(double*)posv;
				medans=(*pos);
#ifdef DEBUG
				cout<<two_word<<":"<<medans<<endl;
#endif
			}
			relativity+=medans;
			if(keyseg1[i].yuyi==keyseg2[j].yuyi)
				yuyinum++;
		}
	}
	relativity+=(yuyinum*1.0)/10;
	return relativity;
}
void hash_map_clear()
{
	hdict_clear(hd);
}
double test_hash(char *key)
{
	double value=0;
	double *pos=NULL;
	pos=(double*)(hdict_test(hd,key));
	if(pos!=NULL)
	{
		value=*pos;
		cout<<key<<"\t"<<value<<"\tInit hash ok!"<<endl;
		return value;
	}
	return 0;
}
