#include "checkArticle.h"
#include "fromuni.h"
#include "touni.h"
#include <malloc.h>
#include <vector>
#include <fstream>
#include <cmath>
#include <map>
#include "minilzo.h"
using namespace std;
struct text_info
{
	int nounnum;//the number of noun
	int verbnum;//the number of verb
	int totalnum;//the number of all words
	int digitnum;//the number of digit 
	unsigned short senWordnum;//the number of sensibility word
	unsigned short noflaglen;//continuous len of no interpunction 
	double totalidf; //the total value of idf
	double English_rate;// the rate of english word 
	int spacenum;//the number of space
};//������Ϣ�ṹ��
int user_analy(WORD_SEGMENT *wordseg,text_info &xx,int beginpos, int endpos);//�û���
double structure_score(text_info &xx);//���ӽṹ����̶ȵ÷�
double single_score(WORD_SEGMENT *wordseg,int beginpos,int endpos);//������
double integer_pid_rationality(WORD_SEGMENT *wordseg,char *text);//�����Ⱥ�����Ե÷�
double body_judge(WORD_SEGMENT *wordseg,char *text);//�ۺϵ����ֵĽ�� 
int sentence_number(WORD_SEGMENT *wordseg, int start, int length, vector<int> &segments);
int quality(WORD_SEGMENT *wordseg,double &score,int sentence_num);
/////////////////////////////////////////////////////////////////////////////////////
double checkArticle(WORD_SEGMENT *wordseg, char *text,int textLen)
{
	if(textLen==0)
		return 0;
	int lc=0,sentence_num=0;
	vector<int>fullposition;
	double compp=0.0;//ѹ���ʣ�Խ������Խ��
	double score=1.0,s1=0;
	lzo_uint lclen=0,llen=0;
	lzo_byte ctext[1024*1024];
	lzo_byte comtext[1024*1024];
	memcpy(ctext,text,textLen+1);
	char buffer[1024*1024];
	text_info xx;
	memset(&xx,0,sizeof(text_info)*1);
	map<string,int>sentence;
////////////////////////////////////////////////
//workspace
	lclen=llen=textLen;
	lc = lzo1x_1_compress(ctext,llen,comtext,&lclen,buffer);//����ѹ���ʽӿ�
	compp=(double)(lclen)/llen;
	if(compp<0.35)//����ѹ����ȥ������������
		return compp;
	sentence_num=sentence_number(wordseg,0,wordseg->word_num,fullposition);
	
	if(sentence_num<=2&&wordseg->word_num<=20)//���±����̵ܶĵ���
	{

		s1=integer_pid_rationality(wordseg,text);
		s1=s1*log(1.0+wordseg->word_num)*0.5;;
		if(compp>1&&wordseg->word_num<=2)
			compp=sqrt(compp);
		score=s1*compp;
	}
	else //  �������Ļ��Ⱥ
	{
		score=body_judge(wordseg,text);//�����������˽��������ϵķ���
		score=score*compp*0.92;
	}
	
	if(score>=80)
		score=80+log(score-80.0);
	return score;
}
///////////////////////////////////////////////////////////////////
int user_analy(WORD_SEGMENT *wordseg,text_info &xx,int beginpos, int endpos)
{
	int fencinum=0,english_word_num=0;
	int len=0,num1=0,postid=0,num2=0,num3=0;
	int no_inter_len=0;
	bool aflag=false;
	char tmp[1024];
	for(int i = beginpos; i < endpos; i++)
	{
		aflag = false;
		num1=0;
		num2=0;
		num3=0;
		memset(tmp,0,1024);
		len = uniToBytes(wordseg->uni+(wordseg->wordlist[i]).wordPos, (wordseg->wordlist[i]).      wordLen, tmp, 1024, "GBK");
		tmp[len] = '\0';
		for(int j=0;j<(wordseg->wordlist[i]).semanticnum;j++)
		{
			if((wordseg->wordlist[i]).semanticid[j]==79)//��д�
			{   
				xx.senWordnum++;
			}
		}
		for(int j = 0;j<len;j++)
		{
			if(isalpha(tmp[j])||isdigit(tmp[j]))
				num1++;
			if(isalpha(tmp[j]))
				num2++;
			if(isdigit(tmp[j])||tmp[j]=='.')
				num3++;
		}
		if(tmp[0]==' '&&len==1)//space
			xx.spacenum++;
		if(num1 == len)//Ӣ��+����
		{
			aflag=true;
		}
		if(num2 == len)//Ӣ��
		{
			english_word_num++;
		}
		if(num3==len&&(num1-num2))
			xx.digitnum++;
		postid = (wordseg->wordlist[i]).postagid;
		no_inter_len++;
		if((postid >= 140)&&(postid <= 146))
			aflag = true;
		if((postid >= 150&&postid <= 156)||postid==230)
		{
			aflag = true;
			if(no_inter_len>xx.noflaglen)
				xx.noflaglen=no_inter_len;
			no_inter_len=0;
		}
		if((postid == 210)||(postid == 212))
			aflag=true;
		if(postid == 160||postid == 60||postid==0)
			aflag=true;
		if(!aflag)
		{
			fencinum++;
			aflag=false;
			if((postid >= 95&&postid <= 104)||postid==123)
				xx.nounnum++;
			if(postid >= 170 && postid <= 176)
				xx.verbnum++;
			if(postid==180||postid==10)
				xx.verbnum++;
			if((wordseg->wordlist[i]).idf==0)
			{
				(wordseg->wordlist[i]).idf=100;
			}
			xx.totalidf = xx.totalidf+(double)(wordseg->wordlist[i]).idf/10.0;
		}
	}
	xx.English_rate=(english_word_num*1.0)/(endpos-beginpos-xx.spacenum);
	return fencinum;
}
////////////////////////////////////////////////////////
/*date:2008.9.18
 * author:guibin
 * describe:�����Ƿ���������ж�
 * supply:	
 * date:2008.9.26 
 * NO.1:��Ϊ�Ծ��ӽ��д��*/
//////////////////////////////////////////////////////
double structure_score(text_info &xx)
{
	if(xx.English_rate>0.70&&xx.spacenum>2)//ר�����ڴ���Ӣ�ĵ����
		return 6;
	double score1=0;
	double idf=0,imp=0;//imp Ȩ��ϵ��
	idf=log(1+xx.totalidf/20);//Ҫ���� idf==0 ʱ�������
	if(idf>6)
		idf=2;
	if(xx.nounnum>=4&&xx.verbnum>=2)
		imp=4;
	else 
	{
		if(xx.nounnum>=2&&xx.verbnum>=1)
			imp=3;
		else
		{
			if((xx.nounnum+xx.verbnum)>=3||(xx.nounnum>=1&&xx.verbnum>=1))
				imp=2;
			else
				if((xx.nounnum>=1)&&xx.verbnum==0)
					imp=1;
			if((xx.nounnum==0)&&xx.verbnum>=2)
				imp=0.5;
			if((xx.nounnum==0)&&xx.verbnum<2)
				imp=0;
		}
	}
#ifdef DEBUG
	cout<<imp<<"\t"<<idf<<"sigle"<<endl;
#endif
	score1=imp*idf;
	score1=score1*sqrt(1+0.2*xx.senWordnum);//�����̬�ʵļӷ�
	return score1;
}
/////////////////////////////////////////////////////////////
/*date:2008.9.18
 * auther:guibin
 * todo:single_score sentence checkArticle analy
 * supply:   */
/////////////////////////////////////////////////////////////
double single_score(WORD_SEGMENT *wordseg,int beginpos,int endpos)
{
	double ans=0.0;
	if(endpos==beginpos)
		return ans;
	text_info sxx;
	memset(&sxx,0,sizeof(text_info)*1);
	sxx.totalnum=user_analy(wordseg,sxx,beginpos,endpos);//�û���
	ans=structure_score(sxx);
	return ans;
}
/////////////////////////////////////////////////////////////
/*date:2008.9.19
 * author:guibin
 * describe:��ȫ�ĵļ�ֵ�����ж�
 * supply:		*/
/////////////////////////////////////////////////////////////
double body_judge(WORD_SEGMENT *wordseg,char *text)
{
	int sentence_num=0,sn=0,vsn=0;
	double score=0.0,pidS=0,maxS=0;//maxS ��ȫ����Ѿ�����������Ч�������ʵ������õ��ĵ÷�
	double gsp=0,effectNum=0;//�м�ֵ���ӵ�����
	double senlen=0;//average length of sentence
	double	ss[1024];//�������Ӧ�ĵ÷�
	vector<int>segments;
	pidS=integer_pid_rationality(wordseg,text);//ȫ�Ĵ��Խṹ����̶ȵ÷�
	if(pidS==-1)
		return 0;
	sentence_num=sentence_number(wordseg,0,wordseg->word_num,segments);
	senlen=wordseg->word_num/sentence_num;//���ӳ���
	if(senlen<=25||(sentence_num>20&&senlen<=50))//��һ���ж�ģʽ
	{
		for(int i=0;i<sentence_num&&i<1024;i++)
		{
			ss[i]=single_score(wordseg,segments[i],segments[i+1]);
			if(ss[i]>maxS)
				maxS=ss[i];
			if(ss[i]>0)
			{
				vsn+=(segments[i+1]-segments[i]);
				effectNum++;
			}
		}
		gsp=(double)(vsn)/(wordseg->word_num);//gsp ��Ч�ʵı���
		maxS=maxS*gsp;
	}
	else									  //���ڹ����ľ��Ӱ������ؾ���ٽ��е������
	{
		for(int j=0;j<wordseg->word_num;j++)
		{
			if(j%15==0&&j!=0)
			{
				ss[sn++]=single_score(wordseg,j-15,j);
				if(ss[sn-1]>maxS)
					maxS=ss[sn-1];
				if(ss[sn-1]>1)
					vsn+=15;
			}
		}
		if(wordseg->word_num%15!=0)
		{
			ss[sn++]=single_score(wordseg,(wordseg->word_num/15)*15,wordseg->word_num);
			if(ss[sn-1]>maxS)
				maxS=ss[sn-1];
			if(ss[sn-1]>1)
				vsn+=wordseg->word_num-(wordseg->word_num/15)*15;
		}
		gsp=(double)(vsn)/wordseg->word_num;
		maxS=maxS*gsp;
	}
#ifdef DEBUG
	cout<<text<<endl;
	cout<<"body"<<pidS<<"\t"<<maxS<<endl;
#endif
	score=maxS*sqrt(pidS);
	return score;
}

double integer_pid_rationality(WORD_SEGMENT *wordseg,char *text)
{
	double score=0,sl=0,snv=0;//snv �����ʶ��ʱ����õ��ĵ÷�
	double np=0,vp=0,averagei=0,lajip=0;
	double nounp=0,no_interp=0;
	text_info xx;
	memset(&xx,0,sizeof(text_info)*1);
	if(wordseg->word_num==0)
		return score;
	xx.totalnum=user_analy(wordseg,xx,0,wordseg->word_num);
	np=(double)xx.nounnum/wordseg->word_num;
	vp=(double)xx.verbnum/wordseg->word_num;
	/****************�������ʵõ��ķ�ֵ�ж�*************/
	lajip=(double)(wordseg->word_num-xx.totalnum)/wordseg->word_num;
	averagei=(double)xx.totalidf/xx.totalnum;//ƽ����IDFֵ
	if(xx.English_rate>0.7)//is an english article
	{
		if(xx.spacenum>wordseg->word_num*0.3)
			return 3;
		else
			return 0.1;
	}
	if(lajip>0.86&&xx.English_rate<0.65)//���ߵ�������
	{
		return 0;
	}

	sl=0.5*pow(10,((1-lajip)));
	/*********���ʶ��ʱ�����ֵ�ж�********/
	if(np>0.1||((vp>0.05)&&(np+vp)>0.2))
		snv=3;
	else
	{
		if(np>0.05||(np+vp)>0.1)
			snv=2;
		else
			snv=1;
	}
	score=snv*sl;
	if(xx.totalnum)//Ϊ��ʱ���ܳ�����ֹ����nan�����
		score=score*sqrt(1+xx.senWordnum*10.0/xx.totalnum);//�����̬�ʳɷ�
	nounp=xx.nounnum*1.0/xx.totalnum;
	if(nounp>0.5)
		score=score*0.6;//����������ʱ�������ǹ��
	if((xx.spacenum*1.0/wordseg->word_num)>0.25&&xx.English_rate<0.3)
		score=score*0.04;//���˶���ִʵĹ��
	if((xx.digitnum*10)>xx.totalnum)//��������ֿ����ǹ��
	{
		score=(0.5*0.1*xx.totalnum/xx.digitnum)*score;
	}
	no_interp=100-(xx.noflaglen-10)*(xx.noflaglen-10);
	if(no_interp<=0)
		no_interp=5;
	if(xx.noflaglen>10&&strlen(text)<200)
		score=score*log(no_interp)/log(100.0);
	return score;
}
//////////////////////////////////////////////////////////////////////////////
/*************************************************************
 * date:2008.10.27
 * author:gongjin
 * description:���ݾ�Ⱥ�ִʵ�idf�ֲ�������ʣ����Ծ��ӽ�������
 * version:1.0
 * input:
 * wordseg ��Ⱥ�ִʽṹ��
 * score ���ʼ�����
 * output:����ֵ�õ��ľ�Ⱥ�������۵��������
 ************************************************************/
int quality(WORD_SEGMENT *wordseg,double &score,int sentence_num)
{
	map<int, int> map_entropy;
	map<int, int>::iterator iter_int_int;
	int idf = 0;
	int sum = 0;
	int postagid=0;
	double atten=0.0;
	for(int i = 0; i < wordseg->word_num; i++)
	{
		postagid=wordseg->wordlist[i].postagid;
		if(postagid >= 150 && postagid <= 156)
		{
			continue;//���ȥ��
		}
		if(postagid == 0 || postagid == 90 || postagid==230)
		{
			continue;//���ʺ��޴��Ե�ȥ��
		}
		if(wordseg->wordlist[i].idf > 0)
		{
			idf = wordseg->wordlist[i].idf/10;
		}
		else
		{
			continue;
		}
		sum ++;
		iter_int_int = map_entropy.find(idf);
		if(iter_int_int != map_entropy.end())
		{
			iter_int_int->second++;
		}
		else
		{
			map_entropy.insert(pair<int,int>(idf,1));
		}
	}
	double entropy = 0.0;
	for(iter_int_int = map_entropy.begin(); iter_int_int != map_entropy.end(); iter_int_int++)
	{
		double prob = (double)(iter_int_int->second)/sum;
		entropy += -prob*log(prob)/log(2.0);
	}
	score = entropy;
	atten=100-(sentence_num-12)*(sentence_num-12);
	if(atten<=1)
	{
		if(sentence_num>22)
			atten=10;
		else
			atten=5;
	}
	score=score*log(atten)/log(100.0);
	if(score > 2)
	{
		return 1;//high quality
	}
	else
	{
		return 0;//low quality
	}
}
///////////////////////////////////////////////////////////////////////////////
int sentence_number(WORD_SEGMENT *wordseg, int start, int length, vector<int> &segments)
{
	char a[3]="��";
	char b[3]="��";
	char bb[2]="?";
	char c[3]="��";
	char cc[2]="!";
	char d[3]="��";
	char dd[2]=";";
	char tmp[1024];
	int len = 0;
	int sentenceLen = 0;
	int end = start+length;
	int sentencenum=0;
	segments.push_back(0);
	for(int i = start; i < end-1; i++)
	{
		if(wordseg->wordlist[i].postagid < 150 || wordseg->wordlist[i].postagid > 156)
		{
			sentenceLen += wordseg->wordlist[i].wordLen;
			continue;
		}
		len = uniToBytes(wordseg->uni+(wordseg->wordlist[i]).wordPos, (wordseg->wordlist[i]).      wordLen, tmp, 1023, "GBK");
		tmp[len] = 0;
		if((!strcmp(tmp,a))||(!strcmp(tmp,b))||(!strcmp(tmp,bb))||(!strcmp(tmp,c))||(!strcmp(tmp,  cc))||(!strcmp(tmp,d))||(!strcmp(tmp,dd)))
		{
			len = uniToBytes(wordseg->uni+(wordseg->wordlist[i+1]).wordPos, (wordseg->wordlist[i+  1]).wordLen, tmp, 1023, "GBK");
			tmp[len] = 0;
			if((!strcmp(tmp,a))||(!strcmp(tmp,b))||(!strcmp(tmp,bb))||(!strcmp(tmp,c))||(!         strcmp(tmp,cc))||(!strcmp(tmp,d))||(!strcmp(tmp,dd)))
			{
				continue;
			}
			else
			{
				segments.push_back(i+1);
				sentenceLen = 0;
			}
		}
	}
	sentencenum=segments.size()-1;
	if(segments[sentencenum]!=(length-1))
	{
		segments.push_back(length);
		sentencenum++;
	}
	return sentencenum;
}

