 
/**
 * @file pub.h
 * @brief 
 *  
 **/


#ifndef  __PUB_H_
#define  __PUB_H_



#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <signal.h>
#include <map>
#include "myopt.h"

#define FILEPATH_MAXSIZE 1024

using namespace std;



typedef struct options_data_t
{
	int input;
	char infile[FILEPATH_MAXSIZE];
	int threadnum;
	char modelfilename[FILEPATH_MAXSIZE];
} options_data_t;


extern options_data_t opt_data;


int load_options(int argc, char** argv );



#endif  //__PUB_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
