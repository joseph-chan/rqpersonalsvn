/*----------------------------------------------------------------------*
 * Copyright (c) 2007 by Baidu  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 * Any question and suggestion, contact with sharperdavid@hotmail.com.
 *----------------------------------------------------------------------*/

/**
 * @file iparse.h
 * @brief API header file for chinese chunk parsing library.
 * @author David Dai 
 * @version 1.0
 * @date 2007/12/10
 */

/**
 * @file iparse.h
 * @brief implement the new chunk parsing algorithm.
 * @author Lin Saiqun 
 * @version 2.0
 * @date 2008/9/10
 */

#ifndef IPARSE_H
#define IPARSE_H

#include "ipostag.h"  

/*! maximum count of tokens included in the text to be parsed */
#define MAX_PARSED_COUNT 256 
/*! maximum count of head terms in a chunk */
#define MAX_HEAD_COUNT 32
/*! chunk network node */
typedef struct chunk_t
{
  /// POS tag of head.
  int head_state : 8; 
  /// count of head terms.
  int head_count : 8; 
  /// chunk type, 0 is invalid.
  int chunk_state : 8;
  /// depth of parsing tree, leaf is 0.
  int chunk_depth : 8;
  /// chunk weight.
  double weight;                  
  /// head term buffer.
  int head_buffer[MAX_HEAD_COUNT];
  /// brother pointer.
  struct chunk_t* next;
  /// children pointers.
  struct chunk_t* left;
  struct chunk_t* right;
} chunk_t;

/*!
  Initialize the dictionary and other configure data.  
  @param <dictpath> - directory name of dictionary, e.g. ../dict/, ../dict.
  @return 0 if succeeds, or return the nonzero error code.

  Look out: You must call this method ONLY once in the beginning of process!
*/
int chk_open( const char* dictpath );


/*!
  Do the chunk parsing task.
  @param <tokens> - the segmented tokens processed by using libsegment.
  @param <size> - size of <tokens> array.
  @param <chunks> - buffer to store the parsed result.
  @param <chkcnt> - size of <chunks> array.
  @return If succeeds, return the count of chunks including the count of
  tokens; or return 0.  
*/
int chk_parse( token_t tokens[], uint32_t size, chunk_t chunks[], uint32_t chkcnt );


/*!
  Dispose all the resource.

  Look out: You must call this method ONLY once at the end of process!
*/
void chk_close();


#endif  /* IPARSE_H */

