/**
 * Unicode�ַ������ú���
 * Xu Lubing
 */
#ifndef _USTRING_H
#define _USTRING_H

#include <stdio.h>

typedef unsigned short UCHAR;
#ifdef __cplusplus
extern "C" {
#endif

/* ������Ӧ�ĺ���ȥ��"u"����<string.h>�ṩ�ĺ��� */
size_t ustrlen(const UCHAR *s);
UCHAR* ustrcat(UCHAR *dest, const UCHAR *src);
UCHAR* ustrncat(UCHAR *dest, const UCHAR *src, size_t n);
UCHAR* ustrcpy(UCHAR *dest, const UCHAR *src);
UCHAR* ustrncpy(UCHAR *dest, const UCHAR *src, size_t n);
int ustrcmp(const UCHAR *s1, const UCHAR *s2);
int ustrncmp(const UCHAR *s1, const UCHAR *s2, size_t len);
int ustrcasecmp(const UCHAR *s1, const UCHAR *s2);
int ustrncasecmp(const UCHAR *s1, const UCHAR *s2, size_t len); 
UCHAR* ustrstr(const UCHAR *haystack, const UCHAR *needle);
UCHAR* ustrchr(const UCHAR *s, int c);
UCHAR* ustrrchr(const UCHAR *s, int c);

#ifdef __cplusplus
}
#endif

#endif
