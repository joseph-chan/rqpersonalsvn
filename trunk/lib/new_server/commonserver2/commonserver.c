#include "transsrvcontrol.h"
#include "searchdef.h"
#include "commonworker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>


//****************  �ṹ��Ϣ�ͺ��� *********************
int g_daemonize = 0;			//1 -- ��ִ̨�У�0 -- ǰ̨

int g_port=23202;
int g_threadNum=5; 

TransSrvControl_t *g_servers=NULL; // ����һ��serverָ��

static void usage(const char* exe, int exit_code	);
static int getParams(int argc, char* argv[]);
static void showVersion();
static int daemonize();
//******************************************************

void sigExit(int sig);
void ServerExit();

int  main(int argc, char **argv)
{
	int ret;
	TransSrvControl_t *servers=NULL;

	/* ��ѡ�������� */
	if (getParams(argc, argv) != 0)
	{
		usage(argv[0], 1);
	}
	
	
	/* ��ִ̨�� */
	if(1 == g_daemonize)
	{
		daemonize();
	}
	
	//�������̴߳������
	//g_threadNum:�߳�����
	//g_port:����˿ں�
	//DoWork:server�Ĺ����Ժ���
	//CreateWorkingData:װ���߳���Դ�ص�����
	//ReleaseWorkingData:�ͷ��߳���Դ
	servers=TransSrvCreate(g_threadNum, g_port, DoWork , CreateWorkingData, ReleaseWorkingData);
	if(servers==NULL)
	{
		fprintf(stderr,"TransSrvCreateEx failed\n");
		return -5;
	}
	g_servers = servers;
		
	//��������
	ret = TransSrvStart(servers);
	if(ret<0)
	{
		fprintf(stderr,"TransSrvStart failed\n");
		return -6;
	}
	
	//�����Ƴ�
	atexit(ServerExit);
	//����פ���� ���������շ�������
	while(1)
	{
		sleep(10000);
	}
	

	return 0;
	
}


/**
* �������ܣ�	�����Ϊһ��daemon
* ���������
* ��������� 	
* ����ֵ  ��	<0		�������
*				pid 	���̺�	
*/
static int daemonize()
{
	int pid = 0;
	int	saveerrno = 0;
	sigset_t	newmask;

	switch (pid = fork())
	{
		case 0:	/*�ӽ��̳�Ϊ��̨���� */
			break;
		case -1:
			fprintf(stderr, "daemonize: %s\n", strerror(errno));
			exit(1);
		default: /* �������Լ��˳� */
			printf("daemonize:%d\n", pid);
			exit(0);
	}

	/* ������SIGALRM��SIGKILL��SIGSTOP֮��������ź� */	
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGHUP);
	sigaddset(&newmask, SIGINT);
	sigaddset(&newmask, SIGQUIT);
	sigaddset(&newmask, SIGILL);
	sigaddset(&newmask, SIGABRT);
	sigaddset(&newmask, SIGIOT);
	sigaddset(&newmask, SIGFPE);
	sigaddset(&newmask, SIGPIPE);
	sigaddset(&newmask, SIGTSTP);
	
	//signal(SIGKILL, sigExit);
	//signal(SIGSTOP, sigExit);
	signal(SIGTERM, sigExit);
	
	if (sigprocmask(SIG_SETMASK, &newmask, NULL) < 0)
	{
		saveerrno = errno;	
		fprintf(stderr, "sig_block sigprocmask: %s", strerror(saveerrno));
		return -1;
	}
	
	return pid;
}

/**
* �������ܣ�	�����а�����Ϣ
* ���������	����������˳�����
* ��������� 	
* ����ֵ  ��	
*/
static void usage(const char* exe, int exit_code)
{
	printf("Usage: %s [options] para ++ \n", exe);
	printf("Options are:\n"						
			"  -d, --daemon				run as a server.\n"
			"  -p port				server port.\n"
			"  -v, --version			show version.\n"
			"  -h, --help				show help,this message.\n"
			"  -c\n"
		  );
	exit(exit_code);
}

/**
* �������ܣ�	��ʾ�汾��Ϣ��ʵ��ʱ�޸���ʾ�ı�����Ϣ
* ���������
* ��������� 	
* ����ֵ  ��	
*/
static void showVersion()
{
	
	int first_version = 1;			//���汾�ţ�һλ
	int second_version = 0;			//�ΰ汾�ţ���λ
	char *author = "search group";			//����
	char *company = "sina";			//��˾
	char *explain = "server services";	//˵��
	time_t curtime;
	struct tm *nowtime;
	
	time(&curtime);
	nowtime = localtime(&curtime);
	printf("Ver  %d.%02d.%04d%02d%02d \n\n", first_version,
											 second_version,
											 nowtime->tm_year,
											 nowtime->tm_mon,
											 nowtime->tm_mday);
	printf("Written by %s.\n", author);
	printf("Copyright (C) %4d %s, Inc.\n", nowtime->tm_year, company);
	printf("%s\n", explain);

}

/**
* �������ܣ�	�����������д������������
* 				extern char *optarg;		
* 				extern int optind,opterr,optopt; 
* ���������	���̴������
* ��������� 	
* ����ֵ  ��	0			�ɹ�
*				<0			ʧ��
*/
static int getParams(int argc, char* argv[])
{
	char ch;
	while ((ch = getopt(argc, argv, "vhdp:")) != -1)
	{
		switch (ch)
		{
			case 'v':		/* ��ӡ�汾��Ϣ */
				showVersion();
				exit(0);
				break;
			case 'h':		/* ��ӡhelp��Ϣ */
				usage(argv[0], 1);
				break;
			case 'd':		/* ��Ϊһ����̨�������� */
				g_daemonize = 1;
				break;		
			case 'p':
				g_port = atoi(optarg);
				break;
			default:
				usage(argv[0], 1);
		}
	}

	return CreateGlobalData(argv+optind, argc-optind);
}


void sigExit(int sig)
{
	exit(0);
}

void ServerExit()
{
	printf("ServerExit\n");
	TransSrvDestroy(g_servers);
	g_servers=NULL;
	ReleaseGlobalData();
}
