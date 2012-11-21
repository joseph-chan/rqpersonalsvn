#include <string.h>
#include <stdio.h>
#include <ul_dict.h>
#include <ul_sign.h>
#include "scwdef.h"

int main(int argc, char * argv[])
{
	if( argc != 3 )
	{
		fprintf(stderr,"Usage: %s  dictpath  dictname\n",argv[0]);
		return -1;
	}
	char buff[2048];
	char src[1024], out[1024];
	Sdict_search * Dict=NULL;
	if( (Dict=ds_load(argv[1],argv[2]))==NULL )
	{
		fprintf(stderr,"load dict error\n");
		return -1;
	}
	while(fgets(buff, sizeof(buff), stdin))
	{
		if(sscanf(buff, "%s", src)!=1)
			continue;
		int ret=scw_get_goi(Dict,src,strlen(src),out,1024);
		if( ret < 0 )
			printf("scw_get_goi() return error\n");
		else if( ret == 0 )
			printf("no original form: %s\n",src);
		else
			printf("goi:%s\n",out);
	}
	return 0;
}
