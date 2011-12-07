/* $XJG: xslib/net.h,v 1.17 2007/01/09 03:01:00 jiagui Exp $ */
/*
   Author: XIONG Jiagui
   Date: 2005-06-20
 */
#ifndef NET_H_
#define NET_H_ 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define net_listen	net_tcp_listen
#define net_connect	net_tcp_connect


int net_tcp_listen(unsigned short port);


int net_tcp_connect(const char *host, unsigned short port);


int net_tcp_connect_nonblock(const char *host, unsigned short port);


int net_unix_listen(const char *pathname);


int net_unix_connect(const char *pathname);


/* Same as read(), except that it will restart read() when EINTR.
 */
int net_read_n(int fd, void *buf, int len);


/* Same as write(), except that it will restart write() when EINTR.
 */
int net_write_n(int fd, const void *buf, int len);


/* Non-block read. 
 * The return value has a different meanings than read() function.
 * 0 for would-block, -1 for error, -2 for end-of-file (read() returning 0).
 */ 
int net_read_nonblock(int fd, void *buf, int len);


/* Non-block peek.
 * The return value: 
 * 0 for would-block, -1 for error, -2 for end-of-file (read() returning 0).
 */
int net_peek_nonblock(int fd, void *buf, int len);


/* Non-block write.
   The return value has a different meaning than write() function.
   0 for would-block, -1 for error, -2 for end-of-file (EPIPE).
 */
int net_write_nonblock(int fd, const void *buf, int len);



int net_get_so_error(int fd);


int net_set_nonblock(int fd);
int net_set_block(int fd);


int net_set_close_on_exec(int fd);
int net_clear_close_on_exec(int fd);


uint16_t net_get_sock_ip_port(int sock, char ip[16]);
uint16_t net_get_peer_ip_port(int sock, char ip[16]);


/* Return 1 for big-endian, 0 for little-endian. 
 */
int net_get_endian();


/* Return the number of IPs that the current machine has, not including 
   the local loop ip (127.x.x.x).  
   The returned number may be greater than the 2nd argument "num".
   A negative number is returned is some error occured.
   The IPs get are returned in the 1st argument.
   The returned ips are in network byte order.
 */
int net_get_ip(uint32_t ip[], int num);


void net_swap2(void *p);
void net_swap4(void *p);
void net_swap8(void *p);

void net_swap_int16(int16_t *p);
void net_swap_uint16(uint16_t *p);

void net_swap_int32(int32_t *p);
void net_swap_uint32(uint32_t *p);

void net_swap_int64(int64_t *p);
void net_swap_uint64(uint64_t *p);

void net_swap_float(float *p);
void net_swap_double(double *p);


#ifdef __cplusplus
}
#endif

#endif

