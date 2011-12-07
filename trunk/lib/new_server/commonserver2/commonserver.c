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


//****************  结构信息和函数 *********************
int g_daemonize = 0;			//1 -- 后台执行，0 -- 前台

int g_port=23202;
int g_threadNum=5; 

TransSrvControl_t *g_servers=NULL; // 定义一个server指针

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

	/* 可选参数分析 */
	if (getParams(argc, argv) != 0)
	{
		usage(argv[0], 1);
	}
	
	
	/* 后台执行 */
	if(1 == g_daemonize)
	{
		daemonize();
	}
	
	//创建多线程传输服务
	//g_threadNum:线程数量
	//g_port:服务端口号
	//DoWork:server的功能性函数
	//CreateWorkingData:装载线程资源回调函数
	//ReleaseWorkingData:释放线程资源
	servers=TransSrvCreate(g_threadNum, g_port, DoWork , CreateWorkingData, ReleaseWorkingData);
	if(servers==NULL)
	{
		fprintf(stderr,"TransSrvCreateEx failed\n");
		return -5;
	}
	g_servers = servers;
		
	//启动服务
	ret = TransSrvStart(servers);
	if(ret<0)
	{
		fprintf(stderr,"TransSrvStart failed\n");
		return -6;
	}
	
	//服务推出
	atexit(ServerExit);
	//服务常驻进程 监听并接收服务请求
	while(1)
	{
		sleep(10000);
	}
	

	return 0;
	
}


/**
* 函数介绍：	程序成为一个daemon
* 输入参数：
* 输出参数： 	
* 返回值  ：	<0		错误代码
*				pid 	进程号	
*/
static int daemonize()
{
	int pid = 0;
	int	saveerrno = 0;
	sigset_t	newmask;

	switch (pid = fork())
	{
		case 0:	/*子进程成为后台进程 */
			break;
		case -1:
			fprintf(stderr, "daemonize: %s\n", strerror(errno));
			exit(1);
		default: /* 父进程自己退出 */
			printf("daemonize:%d\n", pid);
			exit(0);
	}

	/* 阻塞除SIGALRM、SIGKILL、SIGSTOP之外的所有信号 */	
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
* 函数介绍：	命令行帮助信息
* 输入参数：	命令参数，退出代码
* 输出参数： 	
* 返回值  ：	
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
* 函数介绍：	显示版本信息，实现时修改显示的变量信息
* 输入参数：
* 输出参数： 	
* 返回值  ：	
*/
static void showVersion()
{
	
	int first_version = 1;			//主版本号，一位
	int second_version = 0;			//次版本号，两位
	char *author = "search group";			//作者
	char *company = "sina";			//公司
	char *explain = "server services";	//说明
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
* 函数介绍：	解析从命令行传入的启动参数
* 				extern char *optarg;		
* 				extern int optind,opterr,optopt; 
* 输入参数：	进程传入参数
* 输出参数： 	
* 返回值  ：	0			成功
*				<0			失败
*/
static int getParams(int argc, char* argv[])
{
	char ch;
	while ((ch = getopt(argc, argv, "vhdp:")) != -1)
	{
		switch (ch)
		{
			case 'v':		/* 打印版本信息 */
				showVersion();
				exit(0);
				break;
			case 'h':		/* 打印help信息 */
				usage(argv[0], 1);
				break;
			case 'd':		/* 作为一个后台服务运行 */
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
