
/*
 * Chunk parsing≤‚ ‘≥Ã–Ú°£
 * David Dai
 * 2007.12.10
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "isegment.h"
#include "ipostag.h"
#include "iparse.h"

#define MAX_TEXT_LENGTH 4096
#define MAX_TERM_COUNT 4096

void print_result(token_t tokens[], int count);
void print_result(token_t tokens[], int size, chunk_t chunks[], int cnt);
extern const char* get_pos_str( unsigned int nPOS ); // from libpostag.a


int main(int argc,char** argv)
{
  token_t tokens[MAX_TERM_COUNT];
  //token_t subtokens[MAX_TERM_COUNT];
  chunk_t chunks[MAX_TERM_COUNT];
  char line[MAX_TEXT_LENGTH] = {0}; 
  
  if(argc < 4)
  {
    fprintf(stderr,"usage: %s  segDictPath tagDictPath chkDictPath \n",argv[0]);
    exit(-1);
  }

  // open dict
  dict_ptr_t dictPtr = seg_dict_open(argv[1]);
  if( !dictPtr )
  {
    fprintf(stderr,"error: can't load the segment dictionary %s.\n",argv[1]);
    exit(-1);
  }
  if( tag_open( argv[2]) )
  {
    fprintf(stderr,"error: can't load the postag dictionary %s.\n",argv[2]);
    exit(-1);
  }
  if( chk_open( argv[3]) )
  {
    fprintf(stderr,"error: can't load the chunk dictionary %s.\n",argv[3]);
    exit(-1);
  }
  fprintf(stderr,"All dicts are loaded successfully\n");

  // start ...
  handle_t handle = seg_open(dictPtr,MAX_TERM_COUNT);
  if( !handle )
  {
    fprintf(stderr,"error: can't create handle for segment.\n");
    exit(-1);
  }
  
  // start timing
  clock_t start = clock();    
  
  while(fgets(line,sizeof(line),stdin))
  {    
    int len=strlen(line);
    while((line[len-1]=='\r') ||(line[len-1]=='\n'))
    {
      line[--len]=0;
    }

    if( !len )
      continue;
    
    // segment
    int count = seg_segment(handle,line,len,tokens,MAX_TERM_COUNT);
    if( !count )
    {
      fprintf(stderr, "error: when segmenting, jump over:\n%s\n",line);
      continue;
    } 
    if( count < 2 )
      continue;
    // postag
    int ret = tag_postag(tokens,count);
    if( !ret )
    {
      fprintf(stderr, "error: when pos tagging, jump over:\n%s\n",line);
      continue;
    }      
    // output mixed terms        
    print_result(tokens,count);     

    // chunk parsing
    ret = chk_parse(tokens,count,chunks,MAX_TERM_COUNT);
    if( !ret )
    {
      fprintf(stderr, "error: when chunk parsing, jump over:\n%s\n",line);
      continue;
    }      
    // output chunks
    print_result(tokens,count,chunks,ret);

    // get basic terms
    /*int cnt = 0;
    for(int i=0; i<count; i++)
		{			
			cnt += seg_tokenize(handle,tokens[i],TOK_BASIC,subtokens+cnt,32);	
		}
    // postag
    ret = tag_postag(subtokens,cnt);
    if( !ret )
    {
      fprintf(stderr, "error: when pos tagging, jump over:\n%s\n",line);
      continue;
    }      
    // output basic terms
    print_result(subtokens,cnt);  

    // chunk parsing
    ret = chk_parse(subtokens,cnt);*/

  }    

  // end timing
  double time = (double)( clock() - start ) / CLOCKS_PER_SEC;
  fprintf(stderr,"The elapsed time: %f seconds.\n", time);

  // end ...
  seg_close(handle);
  
  // close dict
  chk_close();
  tag_close();
  seg_dict_close(dictPtr);

  return 0;
}

/// for output chunk
static void print(chunk_t* root, token_t* tokens) 
{
  if( !root || !tokens )
    return;

  if( root->left && root->right )
  {
    /*if(root->weight <= 0.000001)
      return;*/
    /*if( !root->chunk_state )
      return;*/
   
    chunk_t* p = root->left;    
    while( p )
    {
      for( int i=0; i<p->head_count; i++ )
      {
        int j = p->head_buffer[i];
        printf("%s",tokens[j].buffer);
      }        
      p = p->next;
    }    

    printf("(%f,%d,%d)[",root->weight,root->head_state,root->chunk_state);
    for( int i=0; i<root->head_count; i++ )
    {
      int j = root->head_buffer[i];
      printf("%s",tokens[j].buffer);
    }
    printf("]  ");    
  }
  else
  {
    int j = root->head_buffer[0];
    printf("%s(%f)  ",tokens[j].buffer,root->weight);
  }
}

void print_result(token_t tokens[], int count)
{  
  for(int i=0; i<count; i++)
  {    
    const char* stag = get_pos_str(tokens[i].type);
    if( stag )
      printf("%s/%s  ",tokens[i].buffer,stag);    
    else
      printf("%s  ",tokens[i].buffer);    
  }  
  printf("\n");
}

void print_result(token_t tokens[], int size, chunk_t chunks[], int cnt)
{  
  int i = 0;
  
  // term
  printf("term(%d): ",size);
  for( i=0; i<size && i<cnt; i++ )
  {
    print(&chunks[i],tokens);
  }
  printf("\n");
  
  // head term
  printf("head(%.2f): ",3.0);
  for( i=0; i<size && i<cnt; i++ )
  {
    if( 3.0 - chunks[i].weight < 0.000001f )
      print(&chunks[i],tokens);
  }
  printf("\n");
  /*double var = 0.0, avg = 0.0;
  for( i=0; i<size && i<cnt; i++ )
  {
    avg += chunks[i].weight;    
  }
  avg /= size;
  for( i=0; i<size && i<cnt; i++ )
  {
    double diff = fabs(chunks[i].weight-avg);    
    var += diff*diff;
  }
  var = sqrt(var/size);
  printf("head2(%.2f): ",var);
  for( i=0; i<size && i<cnt; i++ )
  {
    if( chunks[i].weight > var )
      print(&chunks[i],tokens);
  }
  printf("\n");*/

  // chunk
  printf("chunk(%d): ",cnt-size); 
  for( i=size; i<cnt; i++ )
  {
    print(&chunks[i],tokens);
  }
  printf("\n");
}

