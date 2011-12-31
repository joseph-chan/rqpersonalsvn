/**
 * @file nx_log.h
 * @brief common log
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-28
 */

#ifndef NX_LOG_H
#define NX_LOG_H


extern "C"
{
	#include <ngx_config.h>
	#include <ngx_core.h>
}

#include <pthread.h>
#include <stdarg.h>


#define NX_LOG_NAME "log"

int nx_log_init(const char *prefix,int log_level= NGX_LOG_NOTICE);


void nx_log_write(int level,const char *fmt,...);
#define nx_log(level,fmt,...) \
	do { \
		ngx_time_update();\
		nx_log_write(level, fmt" [%s:%d]", \
			##__VA_ARGS__,\
			__FILE__,__LINE__ \
				); \
		} while (0)
#endif
