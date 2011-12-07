/**
 * @file ProcessWeibo.cpp
 * @brief main process routine
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-10
 */

#include "WeiboProcessor.h"
#include <iostream>

using namespace std;

// ���������
// None
//
int main(int argc, char* argv[])
{

	if(!WeiboProcessor::GetInstance()->InitInstance(argv[1]))
	{
		cout<<"WeiboProcessor initialized failed."<<endl;
		return 0;
	}
	WeiboProcessor::GetInstance()->Process(argv[2], argv[3]);

	// �ͷ�ʵ��
	CWeiboPreProcessor::ReleaseInstance();
}
