/**
 * @file nx_log.c
 * @brief common log
 * @author RuQiang (ruqiang@staff.sina.com.cn)
 * @version 1.0.0
 * @date 2011-11-28
 */

#include "nx_log.h"

static ngx_open_file_t g_file_stderr = {ngx_stderr,{0,NULL} ,NULL, NULL, NULL};
static ngx_log_t  g_log_stderr = {NGX_LOG_NOTICE, &g_file_stderr, 0, 0,NULL,NULL};

static pthread_key_t g_log_fd = PTHREAD_KEYS_MAX;
static pthread_once_t g_log_unit_once = PTHREAD_ONCE_INIT;

static ngx_str_t err_levels[] = {
    ngx_null_string,
    ngx_string("emerg"),
    ngx_string("alert"),
    ngx_string("crit"),
    ngx_string("error"),
    ngx_string("warn"),
    ngx_string("notice"),
    ngx_string("info"),
    ngx_string("debug")
};

static void log_fd_init()
{
	    pthread_key_create(&g_log_fd, NULL);
}

int nx_log_init(const char *prefix)
{

	ngx_log_t  *ngx_log ;
	ngx_open_file_t  *ngx_log_file;
    u_char  *p, *name;
    size_t   nlen, plen;

	pthread_once(&g_log_unit_once, log_fd_init);
	ngx_time_init();

	ngx_log = (ngx_log_t *) malloc (sizeof(ngx_log_t));
	if (ngx_log ==NULL)
	{
		return -1;
	}
	ngx_log_file = (ngx_open_file_t *) malloc (sizeof(ngx_open_file_t));
	if (ngx_log_file ==NULL)
	{
		return -1;
	}
	ngx_log->file= ngx_log_file;
	ngx_log->log_level = NGX_LOG_NOTICE;
	ngx_log->handler = NULL;
	ngx_log->connection = 0;
	ngx_log->data = NULL;

	if (pthread_setspecific(g_log_fd, ngx_log) != 0) 
	{
		free(ngx_log);
		return -1;
	}


	name = (u_char *) NX_LOG_NAME;

	/*
	 * we use ngx_strlen() here since BCC warns about
	 * condition is always false and unreachable code
	 */

	nlen = ngx_strlen(name);

	if (nlen == 0) {
		ngx_log_file->fd = ngx_stderr;
		return 1;
	}

	p = NULL;

	if (name[0] != '/') {

		if (prefix) {
			plen = ngx_strlen(prefix);

        } else {
            plen = 0;
        }

        if (plen) {
            name = (u_char *)malloc(plen + nlen + 2);
            if (name == NULL) {
                return -1;
            }

            p = ngx_cpymem(name, prefix, plen);

			/*
            if (!ngx_path_separator(*(p - 1))) {
                *p++ = '/';
            }
			*/

            ngx_cpystrn(p, (u_char *) NX_LOG_NAME, nlen + 1);

            p = name;
        }
    }

    ngx_log_file->fd = ngx_open_file(name, NGX_FILE_APPEND,
                                    NGX_FILE_CREATE_OR_OPEN,
                                    NGX_FILE_DEFAULT_ACCESS);

    if (ngx_log_file->fd == NGX_INVALID_FILE) {
        ngx_log_stderr(ngx_errno,
                       "[alert] could not open error log file: "
                       ngx_open_file_n " \"%s\" failed", name);
        ngx_log_file->fd = ngx_stderr;
    }

    if (p) {
        ngx_free(p);
    }


    return 0;
}


void nx_log_write(int level,const char *fmt, ...)
{
	ngx_log_t * ngx_log;
	ngx_log = (ngx_log_t *)pthread_getspecific(g_log_fd);
	if (NULL == ngx_log) {
		ngx_log = &g_log_stderr;
	}
	if ((ngx_log)->log_level < level) 
	{ 
		return ;
	}

    va_list  args;
	ngx_err_t err=0;
    u_char  *p, *last, *msg;
    u_char   errstr[NGX_MAX_ERROR_STR];

    if (ngx_log->file->fd == NGX_INVALID_FILE) {
        return;
    }

    last = errstr + NGX_MAX_ERROR_STR;

    ngx_memcpy(errstr, ngx_cached_err_log_time.data,
               ngx_cached_err_log_time.len);

    p = errstr + ngx_cached_err_log_time.len;

    p = ngx_slprintf(p, last, " [%V] ", &err_levels[level]);

    /* pid#tid */
    p = ngx_slprintf(p, last, "%P#" NGX_TID_T_FMT ": ",
                    ngx_log_pid, ngx_log_tid);

    if (ngx_log->connection) {
        p = ngx_slprintf(p, last, "*%uA ", ngx_log->connection);
    }

    msg = p;

    va_start(args, fmt);
    p = ngx_vslprintf(p, last, fmt, args);
    va_end(args);


    if (err) {
        p = ngx_log_errno(p, last, err);
    }

    if (level != NGX_LOG_DEBUG && ngx_log->handler) {
        p = ngx_log->handler(ngx_log, p, last - p);
    }

    if (p > last - NGX_LINEFEED_SIZE) {
        p = last - NGX_LINEFEED_SIZE;
    }

    ngx_linefeed(p);

    (void) ngx_write_fd(ngx_log->file->fd, errstr, p - errstr);

    if (!ngx_use_stderr
        || level > NGX_LOG_WARN
        || ngx_log->file->fd == ngx_stderr)
    {
        return;
    }

    msg -= (7 + err_levels[level].len + 3);

    (void) ngx_sprintf(msg, "nginx: [%V] ", &err_levels[level]);

    (void) ngx_write_console(ngx_stderr, msg, p - msg);
}

