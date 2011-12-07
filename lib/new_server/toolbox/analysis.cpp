#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <fstream>
#include <iterator>
#include <vector>
#include "lexicon.h"
#include "touni.h"
#include "fromuni.h"
#include "analysis.h"
#include "minilzo.h"
#include "normalize_str.h"
using namespace maxent;

map<unsigned short,unsigned short> complex2simple;//unicode形式的繁简体对应表
/***************************************************
 * date:2009.11.03
 * author:guibin
 * describe:变种广告的最大熵模型的装载信息
 * input: path 资源文件路径
 * return:==0 sucess
 *        ==-1 failed
 * *************************************************/
MaxentModel *loadMaxentModel(const char *file)
{
	if(file==NULL)
		return NULL;
	MaxentModel *model = new MaxentModel;
	try
	{
		model->load(file);
	}
	catch(...)
	{
		return NULL;
	}
	fprintf(stderr,"finish loading %s!\n",file);
	return model;
}

//map<string,int>型资源的通用装载函数
int loadMapResource(const char *file,map<string,int> &rMap,int type)
{
	ifstream fin;
	fin.open(file);
	if(!fin.is_open())
	{
		cout<<file<<" can not be opened!"<<endl;
		return -1;
	}
	char buffer[1024];
	char *head=NULL;
	char *tail=NULL;
	map<string,int>::iterator it;
	rMap.clear();
	while(!fin.eof())
	{
		if(fin.getline(buffer,1024).good()==false)
			break;
		if(type==1)//只有1列数据
		{
			it=rMap.find(buffer);
			if(it==rMap.end())
				rMap.insert(map<string,int>::value_type(buffer,1));
		}
		else if(type==2)//有2列数据
		{
			head=strtok_r(buffer,"\t",&tail);
			if(head==NULL||tail==NULL)
				continue;
			it=rMap.find(head);
			if(it==rMap.end())
				rMap.insert(map<string,int>::value_type(head,atoi(tail)));
		}
	}
	fin.close();
	fprintf(stderr,"finish loading %s:%d\n",file,rMap.size());
	return 0;
}
/////////////////////////////////////////////////////////////////
//论坛监控词用map<string,int>型map装载函数
int loadStrIntMap(const char *file,map<string,int> &wmap,int value)
{
	if(file == NULL)
	{
		fprintf(stderr,"file is NULL\n");
		return -1;
	}
	ifstream fin;
	fin.open(file);
	if(!fin.is_open())
	{
		fprintf(stderr,"%s can not be opened!\n",file);
		return -2;
	}
	int iret = 0,wLen = 0;
	char *pos = NULL;
	char buf[1024]="";;
	map<string,int>::iterator it;
	while(!fin.eof())
	{
		if(fin.getline(buf,1024).good() == false)
			break;
		wLen = strlen(buf);
		if(wLen > 0)
			wmap[buf] = 1;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////
//通用的装在map<string,string>型函数
int loadStrStrMap(const char *file,map<string,string> &rMap)
{
	ifstream fin;
	fin.open(file);
	if(!fin.is_open())
	{
		cout<<file<<" can not be opened!"<<endl;
		return -1;
	}
	char buffer[1024];
	char *head=NULL;
	char *tail=NULL;
	map<string,string>::iterator it;
	rMap.clear();
	while(!fin.eof())
	{
		if(fin.getline(buffer,1024).good()==false)
			break;
		head=strtok_r(buffer,"\t",&tail);
		if(head==NULL||tail==NULL)
			continue;
		it=rMap.find(head);
		if(it==rMap.end())
			rMap.insert(map<string,string>::value_type(head,tail));
	}
	fin.close();
	fprintf(stderr,"finish loading %s --> %d\n",file,rMap.size());
	return 0;
}
//白名单资源的装载
int loadWhiteList(const char *file,map<string,string> &white)
{
	ifstream fin;
	fin.open(file);
	if(!fin.is_open())
	{
		fprintf(stderr,"%s can not be opened!\n",file);
		return -1;
	}
	char buf[1024];
	char *head=NULL;
	char *tail=NULL;
	map<string,string>::iterator it;
	while(!fin.eof())
	{
		if(fin.getline(buf,1024).good()==false)
			break;
		head=strtok_r(buf,"\t",&tail);
		if(head == NULL && tail == NULL)
			continue;
		it=white.find(head);
		if(it==white.end())
			white.insert(map<string,string>::value_type(head,tail));
	}
	return 0;
}
//核心层资源装载
int loadCoreData(const char *path,coreData *data)
{
	fprintf(stderr,"begin coreData loading!\n");
	char file[1024],file2[1024];
	int iret = 0;
	//简体汉字表
	sprintf(file,"%s/simpleCh.txt",path);
	iret = loadMapResource(file,data->hanzi,1);
	if(iret != 0)
	{
		fprintf(stderr,"%s Init failed!\n",file);
		return -4;
	}

	//重要英文词表
	sprintf(file,"%s/importantEnglish.txt",path);
	iret = loadMapResource(file,data->englishWord,1);
	if(iret != 0)
	{
		fprintf(stderr,"%s Init failed!\n",file);
		return -4;
	}

	//各种数字表示形式
	sprintf(file,"%s/chTodigit.txt",path);
	iret = loadStrStrMap(file,data->chTodigit);
	if(iret < 0)
	{
		fprintf(stderr,"%s Init failed!\n",file);
		return -4;
	}

	sprintf(file,"%s/interpunction.txt",path);
	iret = loadMapResource(file,data->interpunction,1);
	if(iret != 0)
	{
		fprintf(stderr,"%s Init failed!\n",file);
		return -4;
	}

	sprintf(file,"%s/blackList.txt",path);
	data->blackList = loadMachine(file);
	if(data->blackList == NULL)
	{
		fprintf(stderr,"%s Init failed!\n",file);
		return -4;
	}

	//3个最大熵模型的装载
	sprintf(file,"%s/variationAdv.mdl",path);
	data->maxent_variationAdv = loadMaxentModel(file);
	if(data->maxent_variationAdv == NULL)
	{
		fprintf(stderr,"%s Init failed!\n",file);
		return -5;
	}   

	//贝叶斯广告模型的装载
	sprintf(file,"%s/bayesHM_adv.mdl",path);
	data->bayesHM_adv = loadBayesModel(file);
	if(data->bayesHM_adv == NULL)
	{
		fprintf(stderr,"%s Init failed!\n",file);
		return -6;
	}
	fprintf(stderr,"coreData loading sucess!\n\n");
	return 0;
}

/*******************************************************
 * date:2009.11.02
 * author:guibin
 * describe:重复率过高的垃圾文本的识别
 * input:
 * return: ==-1 文本重复率过高
 *         ==0 文本正常
 * ****************************************************/
int IS_overlap(const char *text,double &compressRate)
{
	double score=1.0,s1=0,s2=0;
	double qscore=0.0;
	unsigned int len=strlen(text);
	lzo_uint lclen=0,llen=0;
	lzo_byte ctext[1024*1024];
	lzo_byte comtext[1024*1024];
	memcpy(ctext,text,strlen(text)+1);
	char buffer[1024*1024];
	lclen=llen=len;
	int lc = lzo1x_1_compress(ctext,llen,comtext,&lclen,buffer);//文章压缩率接口
	compressRate=lclen*1.0/llen;
	return 0;
}
/**************************************************************
 * date:2009.11.23
 * author:guibin
 * describe:新的色情信息的判断函数
 * intput: wordseg 分词结构体
 *         sexRate 文本可能为色情的概率
 * return:==0 色情
 *        ==1 正常
 * ***********************************************************/
int MaxentModeRate(map<string,wordItem> &wordtf, double &badRate,maxent::MaxentModel *model)
{
	if(model == NULL )
	{
		fprintf(stderr,"model is NULL\n");
		return -1;
	}
	vector<pair<string, float> > vec_feat;
	pair<string,float>pp;
	vec_feat.clear();
	map<string,wordItem>::iterator it;
	for(it=wordtf.begin();it!=wordtf.end();it++)
	{
		pp.first=it->first;
		pp.second = it->second.idf*log(it->second.tf+1.0);
		vec_feat.push_back(pp);
	}
	double rankg = model->eval(vec_feat , "g");
	badRate = model->eval(vec_feat , "b");

	return 0;
}
/************************************************
 * date:2010.05.27
 * author:guibin
 * version:2.0
 * describe:抹掉HTML和表情符号的通用接口
 * flags:
 *      0001 抹掉HTML标签
 *      0010 抹掉表情符号
 * supply:先把所有的HTML标签挑出来，然后决定是否该留
 * **********************************************/
int eraseHtmlEmoticons(const char *src,char *text,int &htmlLen,map<string,int> &urlMap,coreData *cdata,int flags)
{
	bool sqBracketFlag=false;//方括号
	bool flag=false,escFlag=false;//转义字符
	bool triBracketFlag=false;//尖括号
	bool outputFlag=false;//HTML标签是否要输出
	bool emoticonsFlag=false;//是否是表情符号
	char esc[16],html[128];
	int ei=0,hi=0,ascii=0;
	int ti=0,srcLen=strlen(src);
	string srcCh="",escCh="";
	htmlLen=0;
	urlMap.clear();
	map<string,int>::iterator it;
	map<string,string>::iterator sit;
	for(int i=0;i<srcLen;i++)
	{
		if(escFlag==true)//转义字符
		{
			if(ei<16 && (isdigit(src[i]) || isalpha(src[i]) || src[i]=='#'))
			{
				htmlLen++;
				if(src[i]!='#')
				{	
					esc[ei++]=src[i];
				}
				else
					flag=true;
				continue;
			}
			else
			{
				ascii=atoi(esc);
				if(ascii>=65 && ascii<=90)
					text[ti++]='A'+(ascii-65);
				else if(ascii>=97 && ascii<=122)
					text[ti++]='a'+(ascii-97);
				else if(ascii==46)
					text[ti++]='.';
				else
				{
					escCh="&";
					if(flag == true)
						escCh += "#";
					escCh += esc;
					escCh += ";";
					sit = cdata->chTodigit.find(escCh);
					if(sit != cdata->chTodigit.end())
						text[ti++]=sit->second.c_str()[0];
				}
				escFlag=false;
				if(src[i]==';')
					continue;
			}
		}
		if(src[i]=='&' && testbit(flags,0))
		{
			memset(esc,0,8);
			ei=0;
			escFlag=true;
			flag=false;
			htmlLen++;
			continue;
		}
		if(sqBracketFlag==true)//方括号
		{
			if(hi<127 && src[i]!=']')
			{
				html[hi++]=src[i];
			}
			else
			{
				if(strstr(html,"http:")!=NULL && strstr(html,"sina.com.cn")==NULL)
				{
					it = urlMap.find(html);
					if(it == urlMap.end())
						urlMap.insert(map<string,int>::value_type(html,1));
					else
						it->second++;
				}
				if(src[i]==']')
				{
					html[hi++]=']';
					htmlLen++;
				}
				else
					outputFlag = true;
				if(!testbit(flags,0))//没有方括号的过滤需求
					outputFlag=true;
				else if(hi <= 10)
				{
					int eChNum = 0;
					for(int j=0;j<hi;j++)
					{
						if(isalpha(html[j]) || html[j]=='=')
							eChNum ++;
					}
					if(2*eChNum < hi)
						outputFlag = true;
				}
				if(testbit(flags,1) && strstr(html,"emoticions")!=NULL)
					outputFlag=false;
				if(outputFlag == true)
				{
					for(int j=0;j<hi;j++)
						text[ti++]=html[j];
					outputFlag=false;
					htmlLen--;
				}
#ifdef DEBUG
				fprintf(stderr,"sequar bracket:%s\n",html);
#endif
				if(strstr(html,"emoticons=")!=NULL)
					emoticonsFlag=true;
				else if(strstr(html,"emoticons]")!=NULL)
					emoticonsFlag=false;
				sqBracketFlag=false;
			}
			continue;
		}
		if(src[i]=='[')
		{
			sqBracketFlag=true;
			memset(html,0,128);
			hi=0;
			html[hi++]=src[i];
			htmlLen++;
			continue;
		}
		if(triBracketFlag==true)
		{
			if(hi<127 && src[i]!='>')
			{
				html[hi++]=src[i];
			}
			else
			{
				if(strstr(html,"http:")!=NULL && strstr(html,"sina.com.cn")==NULL)
				{
					it = urlMap.find(html);
					if(it == urlMap.end())
						urlMap.insert(map<string,int>::value_type(html,1));
					else
						it->second++;
				}
				if(src[i]=='>')
				{
					html[hi++]=src[i];
					htmlLen++;
				}
				else
					outputFlag=true;
				if(!testbit(flags,0))
					outputFlag=true;
				if(outputFlag==true)
				{
					for(int j=0;j<hi;j++)
						text[ti++]=html[j];
					outputFlag=false;
					htmlLen--;
				}
#ifdef DEBUG
				fprintf(stderr,"triangle bracket:%s\n",html);
#endif
				triBracketFlag=false;
			}
			continue;
		}
		if(src[i]=='<')
		{
			triBracketFlag=true;
			memset(html,0,128);
			hi=0;
			html[hi++]=src[i];
			htmlLen++;
			continue;
		}
		//表情符号全部删除
		if(testbit(flags,1) && emoticonsFlag==true)
			continue;
		text[ti++]=src[i];
	}
	if(escFlag == true)
	{
		ascii=atoi(esc);
		if(ascii>=65 && ascii<=90)
			text[ti++]='A'+(ascii-65);
		else if(ascii>=97 && ascii<=122)
			text[ti++]='a'+(ascii-97);
		else if(ascii==46)
			text[ti++]='.';
	}
	else if(sqBracketFlag==true || triBracketFlag==true)
	{
		if(strstr(html,"http:")!=NULL && strstr(html,"sina.com.cn")==NULL)
		{
			it = urlMap.find(html);
			if(it == urlMap.end())
				urlMap.insert(map<string,int>::value_type(html,1));
			else
				it->second++;
		}
		if(flags==0 || strstr(html,">")==NULL)
		{
			for(int j=0;j<hi;j++)
				text[ti++]=html[j];
			htmlLen--;
		}
	}
	//在文本的结尾也要进行处理
	return 0;
}
/*************************************************
 * date:2010.12.27
 * author:guibin
 * version:1.0
 * describe:根据字符分割文本
 * ***********************************************/
int splitTextbyCh(const char *src,vector<string> &chVec)
{
	if(src == NULL || strlen(src) == 0 || strlen(src) >= 65536)
		return -1;
	int sLen = strlen(src),chlen = 0;
	unsigned short uni[65536];
	char tmp[4]="";
	int num = bytesToUni(src,sLen,uni,sLen+1,"GBK");
	for(int i=0;i<num;i++)
	{
		chlen = uniToBytes(uni+i,1,tmp,2,"GBK");
		tmp[chlen] = '\0';
		chVec.push_back(tmp);
	}
	return 0;
}
/************************************************
 * date:2010.05.27
 * author:guibin
 * version:2.0
 * describe:去掉文中的标点符号
 * labelRate 标点符号总长度
 * disturbRate 打扰符的比例
 * flags:00000001 各种数字的表达方式归一化
 *       00000010 抹掉文中的标点符号
 *       00000100 去掉汉字间隔的标点符号
 * ************************************************/
int rubout(const char *src,char *out,coreData *data,int rank)
{
	if(src == NULL || out == NULL)
		return -1;
	vector<string> ch;
	int chlen=0,len=0;
	int hanziNum = 0,borderHanzi = 0;//汉字及相邻汉字的标点的数量
	int frontType = 0;//上一个字符的类型
	string temp,sret,front;
	char tmp[4]="";
	vector<int> chType;//字符类型的标识
	map<string,int>::iterator it;
	map<string,string>::iterator sit;
	len=strlen(src);
	unsigned short uni[len+1];
	int num = bytesToUni(src, len, uni, len+1, "GBK");
	for(int i=0;i<num;i++)
	{
		memset(tmp,0,4);
		chlen = uniToBytes(uni+i,1,tmp,2,"GBK");
		if(chlen <= 0)
		{
			fprintf(stderr,"something wrong\n");
			continue;
		}
		tmp[chlen]='\0';
		temp = tmp;
		//标准字符转化
		sit = data->chTodigit.find(temp);
		if(sit != data->chTodigit.end())//变种字符到标准字符的转化
			temp = sit->second;
		ch.push_back(temp);
		it = data->interpunction.find(ch[i]);
		if(it != data->interpunction.end())
		{
			if(frontType == 0)
				borderHanzi ++;
			chType.push_back(-1);//标点为-1
			frontType = -1;
			continue;
		}
		it = data->hanzi.find(ch[i]);//汉字
		if(it != data->hanzi.end())
		{
			hanziNum ++;
			chType.push_back(0);//汉字为0
			frontType = 0;
			continue;
		}
		if(ch[i].length()==1)
		{
			frontType = 2;
			char c = ch[i].c_str()[0];
			if(c >= '0' && c <= '9')
				chType.push_back(1);//数字
			else if((c >= 'a' && c <= 'z')||(c>='A' && c<= 'Z'))
				chType.push_back(2);//英文
			else
				chType.push_back(-1);//其他未收录的标点符号
			continue;
		}
		frontType = 3;
		chType.push_back(3);//其他情况均为3
	}
#ifdef DEBUG
	if(hanziNum > 0)
		fprintf(stderr,"border hanzi rate:%f\n",(borderHanzi*1.0/hanziNum));
#endif
	if(hanziNum == 0 || (borderHanzi*1.0/hanziNum) < 0.3)
	{
		strcpy(out,src);
		return 0;
	}
	num=ch.size();sret.clear();
	int conValueChNum=0;//标点后连续的英文和汉字的长度
#ifdef DEBUG
	fprintf(stderr,"@ch:");
#endif
	for(int i=0;i<num && i<chType.size();i++)
	{
#ifdef DEBUG
		fprintf(stderr,"%s:%d\t",ch[i].c_str(),chType[i]);
#endif
		if(chType[i] == 0)//汉字
		{
			sret += ch[i];
			conValueChNum++;
		}
		else //其他
		{
			if(chType[i] == -1)//标点，有选择性保留
			{
				//汉字不可以用空格做分割符
				if(conValueChNum >= 2 && i>0 && ((chType[i-1]==0 && ch[i]!=" ")||chType[i-1]==2))
				{
					sret += ch[i];
				}
				conValueChNum = 0;
				continue;
			}
			if(chType[i] == 1)//数字 汉字夹缝中的数字去掉
			{
				if(rank == 4 && ((i>=2 && chType[i-2]==0)||(i>=1 && chType[i-1]==0)) && ((i+1<num && chType[i+   1]==0)||(i+2<num && chType[i+2]==0)))
					continue;
				sret += ch[i];
				continue;
			}
			if(chType[i] == 2)//英文 汉字夹缝中的英文字母去掉
			{
				conValueChNum ++;
				if(rank == 4 && ((i>=2 && chType[i-2]==0)||(i>=1 && chType[i-1]==0)) && ((i+   1<num && chType[i+1]==0)||(i+2<num && chType[i+2]==0)))
					continue;
				sret += ch[i];
				continue;
			}
			if(chType[i]==3)//其他
				sret += ch[i];
		}
	}
	strcpy(out,sret.c_str());
	return 0;
}
///////////////////////////////////////////////////////////////
MATCHENTRY * loadMachine(const char *file)
{
	MATCHENTRY *machine_ww = NULL;
	if(!(machine_ww = strMatchInit(2)))
	{
		fprintf(stderr,"Init machine err!\n");
		return NULL;
	}
	ifstream fin;
	fin.open(file);
	if(!fin.is_open())
	{
		fprintf(stderr,"%s can not be opened!\n",file);
		return NULL;
	}
	char text[1024]="";
	char unitary[1024]="";
	char *pos = NULL;
	int tLen = 0,iret = 0;
	double labelLen=0,disturbRate=0;
	map<string,int> machineMap;
	map<string,int>::iterator it;
	while(!fin.eof())
	{
		if(fin.getline(text,1024).good()==false)
			break;
		tLen = strlen(text);
		if(tLen < 2)
			continue;
		it = machineMap.find(text);
		if(it == machineMap.end())
		{
			machineMap.insert(map<string,int>::value_type(text,1));
			Addword(text,0,machine_ww,0);
		}
	}
	fin.close();
	Prepare(0, machine_ww);
	return machine_ww;
}
////////////////////////////////////////////////////////////////////
//适用于文本垃圾过滤的多用途文本归一化接口
int TextNormalization(char *src,int srcLen,char *out,int outLen,coreData *data,int flags)
{
	if(src == NULL || srcLen >65536 || strlen(src)>65536 || srcLen > outLen)
		return -1;
#ifdef DEBUG
	fprintf(stderr,"src:%s\n",src);
#endif
	char text[65536]="",text2[65536]="";
	int iret = 0;
	if(srcLen > 0)
	{
		iret = normalize_str(src,srcLen,text,srcLen+1,14);
		if(iret < 0)
			return -3;
#ifdef DEBUG
		fprintf(stderr,"after normailize:%s\n",text);
#endif
	}
	//标点 数字 英文 
	iret = rubout(text,text2,data,3);
	if(iret < 0)
		return -4;
	strcpy(out,text2);
	return 0;
}
///////////////////////////////////////////////////////////////
//文本中是否有联系方式
int ExistContactWay(WORD_SEGMENT *wordseg, double &badrate)
{
	if(wordseg==NULL)
		return -1;
	int isQQ=0,longD=0,isContact=0; //联系方式
	int len,dlen=0;
	char tmp[1024]="";
	for(int i=0; i< wordseg->word_num; i++)
	{
		len = uniToBytes(wordseg->uni+(wordseg->wordlist[i]).wordPos, (wordseg->wordlist[i]).wordLen, tmp, 1023, "GBK");
		tmp[len] = 0;
		if(wordseg->wordlist[i].postagid==90)
		{
			dlen=0;
			for(int j=0;j<len;j++)
			{
				if(isdigit(tmp[j]))
					dlen++;
			}
#ifdef DEBUG
			fprintf(stderr,"num word:%s\t%d\t%d\n",tmp,dlen,len);
#endif
			if(dlen>=10 && len>=10 && tmp[0]=='1' && strstr(tmp,"11111111")==NULL)
				isContact++;
			if(dlen>=8 && strstr(tmp,"5555")==NULL && strstr(tmp,"201")!=tmp)
				longD++;//时间信息要去掉
		}
		if((strcmp(tmp,"qq")==0 || strcmp(tmp,"在线咨询")==0))
			isQQ=1;    
	}
	if(isQQ || longD > 0)
		badrate=0.6;
	if(isContact > 0)
		badrate=isContact;
#ifdef DEBUG
	fprintf(stderr,"isContact:%f\n",badrate);
#endif
	return 0;
}
////////////////////////////////////////////////////////////////////
//20101231 生产贝叶斯模型所需要的资源
int buildBayesData(map<string,wordItem> &wordtf,vector<pair<string,double> > &bayes_word)
{
	bayes_word.clear();
	int oneGramNum = 0;
	pair<string,double> sd;
	map<string,wordItem>::iterator wit;
	for(wit = wordtf.begin();wit!=wordtf.end();wit++)
	{
		sd.first = wit->first;
		sd.second = log(wit->second.tf+0.5)*wit->second.idf*0.1;
		bayes_word.push_back(sd);
		if(wit->second.wordLen == 1)
			oneGramNum++;
	}
	if(oneGramNum > 0.6*wordtf.size())
	{
		sd.first = "oneGram";
		sd.second = (oneGramNum*1.0/wordtf.size()-0.6)*10.0;
		bayes_word.push_back(sd);
	}
	return 0;
}
//////////////////////////////////////////////////////////////
//20110310 通用特征值计算函数
int eigenvalueCompute(map<string,wordItem> &wordtf,vector<pair<string,double> > &eigenvalueVec,int *oneGramNum)
{
	eigenvalueVec.clear();
	pair<string,double> sd;
	map<string,wordItem>::iterator wit;
	for(wit = wordtf.begin();wit!=wordtf.end();wit++)
	{
		sd.first = wit->first;
		sd.second = log(wit->second.tf+1.0)*wit->second.idf;
		eigenvalueVec.push_back(sd);
		if(wit->second.wordLen == 1)
			oneGramNum++;
	}
	return 0;																	    
}
//////////////////////////////////////////////////////////////
//创建原始文本到归一化文本的的字符对照索引
//flags == 1 做繁简体对照处理,前提是前面的繁简体对应表已装载完毕
int CreateChFormatIndex(const char *src,char *standard,map<int,string> &FormatIndex,int flags)
{
	if(src == NULL || standard == NULL)
		return -1;
	int tLen1 = strlen(src),tLen2 = strlen(standard);
	if(tLen1 >= 65535 || tLen2 >= 65535)
		return -1;
#ifdef DEBUG
	fprintf(stderr,"@complex:%s\n",standard);
#endif
	char tmp[4]="";
	bool fanflag = false;//文本中是否存在繁体字
	unsigned short uni1[65536],uni2[65536];
	int ulen1 = 0, ulen2 = 0;
	ulen1 = bytesToUni(src,tLen1,uni1,tLen1+1,"GBK");
	ulen2 = bytesToUni(standard,tLen2,uni2,tLen2+1,"GBK");
	if(ulen1 != ulen2)
	{
#ifdef DEBUG
		fprintf(stderr,"src != standard,off-normal\n");
#endif
		return -2;
	}
	map<unsigned short,unsigned short>::iterator it;
	for(int i=0;i<tLen1;i++)
	{
		if(flags == 1)//需要做繁简体对应
		{
			it = complex2simple.find(uni2[i]);
			if(it != complex2simple.end())
			{
				uni2[i] = it->second;
				fanflag = true;
			}
		}
		if(uni1[i] != uni2[i])
		{
			memset(tmp,0,4);
			uniToBytes(uni1+i,1,tmp,2,"GBK");
			FormatIndex[i] = tmp;
		}
	}
	if(flags == 1 && fanflag == true)//需要做繁简体转化且文中确实存在繁体字符
	{
		tLen2 = uniToBytes(uni2,ulen2,standard,ulen2*2+1,"GBK");
		if(tLen2 >= ulen2)
			standard[tLen2] = '\0';
#ifdef DEBUG
		fprintf(stderr,"@simple:%s-->%d\n",standard,tLen2);
#endif
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
//20101229字符串的繁简体转化
int transComplex2simple(char *src,int slen,char *out,int olen)
{
	if(src == NULL || out == NULL || olen < slen)
		return -1;
	unsigned short uni[slen+1];
	int ulen = 0,tlen = 0;
	ulen = bytesToUni(src,slen,uni,slen+1,"GBK");
	if(ulen <= 0)//没有成功转化的字符 
		return -2;
	for(int i=0;i<ulen;i++)
	{
		if(complex2simple.find(uni[i]) != complex2simple.end())
			uni[i] = complex2simple[uni[i]];
	}
	memset(out,0,olen);
	tlen = uniToBytes(uni,ulen,out,olen,"GBK");
	if(tlen <= 0)
		return -3;
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////
//下载繁体字到简体字的对应表unicode形式
int loadComplex2simple(const char *path)
{
	if(path == NULL)
		return -1;
	char file[1024]="";
	sprintf(file,"%s/simplified_complex.txt",path);
	ifstream fin;
	fin.open(file);
	if(!fin.is_open())
	{
		fprintf(stderr,"%s can not be opened!\n",file);
		return -2;
	}
	char buf[1024]="";
	char *first = NULL;
	char *second = NULL;
	unsigned short uni1[4],uni2[4];
	int iret1 = 0,iret2 = 0;
	while(!fin.eof())
	{
		if(fin.getline(buf,1024).good()==false)
			break;
		first = strtok_r(buf,"\t",&second);
		if(first == NULL || second == NULL)
			continue;
		iret1 = bytesToUni(first,strlen(first),uni1,strlen(first)+1,"GBK");
		iret2 = bytesToUni(second,strlen(second),uni2,strlen(second)+1,"GBK");
		if(iret1!=1 || iret2!=1)
			continue;
		complex2simple[uni1[0]] = uni2[0];
	}
	fprintf(stderr,"finish loading complex2simple:%d\n",complex2simple.size());
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////
int wordsegSimple(WORD_SEGMENT *wordseg,vector<wordItem> &wordvec,double &engDigitRate,int wordLevel)
{
	int wordlen = 0,totalLen = 0;//文本总长度
	int engDigitLen = 0;//数字+英文词的长度
	bool nickflag = false;//昵称开始标志位
	char tmp[1024]="";
	int eLen = 0,dLen = 0;
	string nickname = "";
	wordItem xx;
	wordvec.clear();
#ifdef DEBUG
	fprintf(stderr,"@word:");
#endif
	for(int i=0;i<wordseg->word_num;i++)
	{
		wordlen = uniToBytes(wordseg->uni+(wordseg->wordlist[i]).wordPos,(wordseg->wordlist[i]).wordLen, tmp,    1023, "GBK");
		tmp[wordlen] = '\0';
		totalLen += wordlen;
		//去掉昵称对文本相似计算的干扰
		if((i==0 && strcmp(tmp,"你好")==0)||(i==1 && strcmp(tmp,"好")==0))
		{
			nickflag = true;
			nickname = "";
			continue;
		}
		xx.isEngOrDigit = xx.isErase = false;
		xx.pid=wordseg->wordlist[i].postagid;
		xx.idf=wordseg->wordlist[i].idf;
		xx.wordLen = wordseg->wordlist[i].wordLen;
		if((xx.pid >= 150 && xx.pid <=156)|| xx.pid == 230)
		{
			nickflag = false;
			continue;
		}
		if(nickflag == true)//可能是昵称
		{
			if(nickname.size() <= 12)
			{
				nickname += tmp;
				continue;
			}
			else
				nickflag = false;
		}
		//20110124 语气词、拟声词、叹词、时间词均去掉
		if(wordLevel>=1 && (xx.pid==160 || xx.pid == 50 || xx.pid == 132 ||xx.pid == 107 || (xx.pid>=140 && xx.  pid<=146) || wordlen <= 1))
			continue;
		if(wordLevel>=2 && ((xx.pid>=30 && xx.pid<90)||(xx.pid>=107 && xx.pid<=127)||(xx.pid>=173&&xx.pid<=176)))
			continue;
		eLen = dLen = 0;
		for(int j=0;j<wordlen;j++)
		{
			if(isdigit(tmp[j]))
				dLen ++;
			else if(isalpha(tmp[j]))
				eLen ++;
		}
		if((eLen + dLen) == wordlen)//该词由数字英文组成
		{
			xx.isEngOrDigit = true;
			engDigitLen += wordlen;
		}
		strncpy(xx.word,tmp,64);
#ifdef DEBUG
		fprintf(stderr,"%s:%d:%d\t",xx.word,xx.pid,xx.wordLen);
#endif
		wordvec.push_back(xx);
	}
#ifdef DEBUG
	fprintf(stderr,"\n");
#endif
	engDigitRate = (engDigitLen*1.0) / totalLen;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
int get_wordtf(vector<wordItem> &wSimple,map<string,wordItem>&wordtf)
{
	wordtf.clear();
	map<string,wordItem>::iterator it;
	int wordnum=wSimple.size(),pid=0,idf=0,wlen=0;
	wordItem xx;
	for(int i=0;i<wordnum;i++)
	{
		pid=wSimple[i].pid;
		idf=wSimple[i].idf;
		wlen=strlen(wSimple[i].word);
		it=wordtf.find(wSimple[i].word);
		if(it!=wordtf.end())
			it->second.tf++;//first : tf
		else
		{
			xx.tf=1;
			xx.idf=wSimple[i].idf;//second : idf
			xx.pid=wSimple[i].pid;
			xx.wordLen=wSimple[i].wordLen;
			wordtf.insert(map<string,wordItem>::value_type(wSimple[i].word,xx));
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////
//关键词识别函数
int findBlackWord(char *text,int &watchNum,MATCHENTRY *machine_ww,vector<pair<string,int> > &watchWord)
{
	if(text == NULL || machine_ww == NULL || machine_ww->wordsnum == 0)
		return -1;
	FindAllUntilNull(text,0,machine_ww);
	watchNum=0;
	watchWord.clear();
	pair<string,int> pp;
	for(int i = 0; i< machine_ww->wordsnum; i++)
	{
		if (machine_ww->wordlist[i].findnum > 0)
		{
			pp.first = (char*)machine_ww->wordlist[i].word;
			pp.second = machine_ww->wordlist[i].findnum;
#ifdef DEBUG
			fprintf(stderr,"findBlackWord:%s--%d\n", pp.first.c_str(),machine_ww->wordlist[i].findnum);
#endif
			watchWord.push_back(pp);
			watchNum ++;
			machine_ww->wordlist[i].findnum = 0;
		}
	}
	return 0;
}

