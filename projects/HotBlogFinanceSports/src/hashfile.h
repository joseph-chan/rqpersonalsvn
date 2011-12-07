#ifndef HASHFILE_H
#define HASHFILE_H

#include <stdio.h>
#include <stdlib.h>
#include "file_process.h"
#include "hdict.h"
#define FILEBUFFER_LEN_SMALL 100
typedef struct hdict_t Hash;
int mvFileInHash_numValue(char *filename,Hash **hash);

int mvFileInHash(char *filename,Hash **hash,char *type);

int hashIfCreat(Hash *h);

#endif

