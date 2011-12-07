#ifndef FILE_PROCESS
#define FILE_PROCESS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
//#include "define.h"

FILE* file_open(FILE **fp,const char *filename,char *wr);

int file_open_noErrExit(FILE **fp,const char *filename,char *wr);

int file_getFileLength(const char *filename);

int file_getline(FILE *fp,char *buffer,int length);

int file_lineCount(const char *filename);

int file_getAllFileContent(FILE *fp,char *buffer,int length);

int file_close(FILE *fp);

#endif
