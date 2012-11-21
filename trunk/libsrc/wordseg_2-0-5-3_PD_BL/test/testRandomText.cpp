/* 
 * Generating a random string automatically to test program.
 * David Dai
 * 2007.8.22
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "isegment.h"
#include "property.h"


#define MAX_TEXT_LENGTH 1024
#define MAX_TERM_COUNT 1024

int generate_chtext(char buffer[]);
int generate_entext(char buffer[]);

void print_result(token_t tokens[], int count);
void print_result(const char* text, token_t tokens[], int count);


int main(int argc,char** argv)
{
  if(argc < 2)
  {
    fprintf(stderr,"Error: invalid parameters.\nUsage: %s  dictpath  [time(minute)]\n",argv[0]);
    return -1;
  }

  // open dict
  dict_ptr_t dictPtr = seg_dict_open(argv[1]);
  if( !dictPtr )
  {
    fprintf(stderr,"Error: can't load the dictionary %s.\n",argv[1]);
    return -1;
  }

  srand( (unsigned)time( NULL ) ); // for generating the random numbers

  // start ...
  handle_t handle = seg_open(dictPtr,MAX_TERM_COUNT);
  
  // start timing
  clock_t start = clock();
  long timeLimit = 60*CLOCKS_PER_SEC;  // 1 minute
  if( argc > 2)
    timeLimit = atol(argv[2])*60*CLOCKS_PER_SEC;
  
  // segment
  token_t tokens[MAX_TERM_COUNT];
  char line[MAX_TEXT_LENGTH] = {0};  
  while(  (clock() - start) < timeLimit )
  {  
    int len = 0;
    len += generate_chtext(line);
    len += generate_entext(line+len);
    line[len] = 0;

    //fprintf(stderr,"%s\n",line);  // for debug
    
    int count = seg_segment(handle,line,len,tokens,MAX_TERM_COUNT);
    if( !count )
    {
      fprintf(stderr, "Error: when segmenting, jump over:\n%s\n",line);
      continue;
    }  
    
    // output    
    printf("============== Segment Result =============\n");
    print_result(tokens,count);  
    printf("\n");
    
    printf("============== Basic Word Result =============\n");
    for(int i=0; i<count; i++)
    {
      token_t subtokens[32];
      int cnt = seg_tokenize(handle,tokens[i],TOK_BASIC,subtokens,32);
      print_result(subtokens,cnt);  
    }
    printf("\n");

    printf("============== Sub Phrase Result =============\n");
    for(int i=0; i<count; i++)
    {
      token_t subtokens[32];
      int cnt = seg_tokenize(handle,tokens[i],TOK_SUBPHR,subtokens,32);
      print_result(subtokens,cnt);  
    }    
    printf("\n");
    
    printf("============== Human Name Result =============\n");
    for(int i=0; i<count; i++)
    {
      token_t subtokens[32];
      int cnt = seg_tokenize(handle,tokens[i],TOK_PERNAME,subtokens,32);
      print_result(subtokens,cnt);  
    }  
    printf("\n");    

    printf("============== Book Name Result =============\n");  
    for(int i=0; i<count; i++)
    {
      if( IS_BOOKNAME(tokens[i].prop) )
      {
        printf("%s  ", tokens[i].buffer);
      }
    }  
    printf("\n");
  }

  // end timing
  double time = (double)timeLimit / CLOCKS_PER_SEC;
  fprintf(stderr,"The elapsed time: %f seconds.\n", time);

  // end ...
  seg_close(handle);

  // close dict
  seg_dict_close(dictPtr);

  return 0;
}

void print_result(token_t tokens[], int count)
{  
  for(int i=0; i<count; i++)
  {    
    printf("%s  ",tokens[i].buffer);
  }  
}

void print_result(const char* text, token_t tokens[], int count)
{
  char term[256] = {0};  
  for(int i=0; i<count; i++)
  {
    int pos = tokens[i].offset;
    int len = tokens[i].length;
    memcpy(term,text+pos,len);
    term[len] = 0;
    printf("%s  ",term);
  }
  printf("\n");
}

int generate_number(int min, int max)
{  
  int randnum = (int)( ( (double)rand() / (double)RAND_MAX ) * (max - min) + min );
  return randnum;
}

int generate_chtext(char buffer[])
{
  int randlen = generate_number(10,1000);
  if( randlen % 2 )
    randlen++;

  for(int i=0; i<randlen; i+=2)
  {
    buffer[i] = generate_number(176,247);
    buffer[i+1] = generate_number(161,254);
  }
  

  
  /*for(int i=0; i<randlen; i+=2)
  {
    if(i % 3)
      buffer[i] = generate_number(176,247);
    else
      buffer[i] = generate_number(32,126);

    if( buffer[i] & 0x80 )
      buffer[i+1] = generate_number(161,254);
    else
      buffer[i+1] = generate_number(32,126);
  }
  if( randlen % 2 )
  {
    buffer[randlen-1] = generate_number(32,126);
    buffer[randlen] = 0;
  }
  else
    buffer[randlen] = 0;*/

  return randlen;
}

int generate_entext(char buffer[])
{
  int randlen = generate_number(10,500);

  for(int i=0; i<randlen; i++)
  {
    buffer[i] = generate_number(32,126);
  }  

  return randlen;
}

