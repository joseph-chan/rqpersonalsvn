/*--------------------------------------------------------------------*
 * Copyright (c) 2007 by Baidu  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 * Any question and suggestion, contact with sharperdavid@hotmail.com.
 *--------------------------------------------------------------------*/

/*
 * API head file for chinese word segmentation library.
 * Author: David Dai
 * Date: 2007/08/20
 */

#ifndef ISEGMENT_H
#define ISEGMENT_H
#include "scwdef.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* subtoken category */
#define TOK_BASIC 1 /* basic term */
#define TOK_SUBPHR 2 /* sub phrase */
#define TOK_PERNAME 3 /* person name */
#define TOK_NEWWORD 4 /*new word*/

/* define the integer type */
#ifndef INT_TYPE_DEFINED
#define INT_TYPE_DEFINED
typedef signed char  int8_t;
typedef unsigned char  uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#endif /* INT_TYPE_DEFINED */

/* define the token type */
#ifndef TOKEN_TYPE_DEFINED
#define TOKEN_TYPE_DEFINED
#define TERM_MAX_LEN 256
typedef struct
{  
  /* term info */
  uint32_t length : 8;  /* length of term, not include the '\0' */  
  uint32_t offset : 24; /* offset of term in input text */  
  uint32_t type : 8;    /* type info, e.g. POS tag. 0 is invalid, used in postag, NOT used in wordseg */ 
  uint32_t weight : 24; /* weight of term */

  /* property */
  struct
  {
    uint32_t m_lprop;  /* high 32 bit */
    uint32_t m_hprop;  /* low 32 bit */
  } prop;

  /* dict index */
  long index; /* point to the address of dict item */  

  /* term buffer */
  char buffer[TERM_MAX_LEN];

} token_t;
#endif /* TOKEN_TYPE_DEFINED */

/* define handle type */
typedef long handle_t;
/* define dictionary pointer */
typedef long dict_ptr_t;


// <Dict-API> --------------------------------------------------------------------------------//
/*!
  Open a handle of the named dictionary.
  @param
    <dictpath> - directory name of dictionary, e.g. ../dict/, ../dict.
  @return
    If succeeds, return the nonzero pointer value; or return 0.

    Look out: This method is NOT thread-safe!
*/
dict_ptr_t seg_dict_open( const char* dictpath );

/*!
  Close a dictionary.
  @param <pdict> - handle of the named dictionary.

  Look out: To a valid dictionary pointer, you can close it 
  ONLY ONCE, then it becomes invalid!
*/
void seg_dict_close( dict_ptr_t pdict );
// </Dict-API> -------------------------------------------------------------------------------//


// <Seg-API> ---------------------------------------------------------------------------------//
/*!
  Open a handle, initialize the inner data.  
  @param <pdict> - dictionary object pointer returned by seg_dict_open().
  @param <size> - size of output buffer used by _segment().
  @return
    If succeeds, return the nonzero handle; or return 0.

  Look out: You must call this method ONLY ONCE in the beginning of thread!
*/
handle_t seg_open( dict_ptr_t pdict, uint32_t size );

/*!
  Do the segmentation task, return the token array.
  @param <handle> - [in][out] handle returned by seg_open().
  @param <text> - input text string to be segmented.
  @param <length> - length of <text>.
  @param <result> - [out] the buffer used to store the segmented result.
  @param <max> - max count of returned tokens.  
  @return
    If succeeds, return the count of tokens; or return 0.
  
  Look out: This method is NOT thread-safe! 
  But the following codes are thread-safe:
    seg_open();
      ...
      seg_segment();
      seg_token();
      ...
    seg_close();
*/
int seg_segment( handle_t handle, const char* text, uint32_t length, token_t result[], uint32_t max );

/*!
  Get the sub-tokens by a segmented result token.
  @param <handle> - handle returned by seg_segment().
  @param <token> - parent token.
  @param <type> - type of token, e.g. TOK_BASIC or TOK_MIXED.
  @param <subtokens> - [out] the buffer used to store the sub-tokens.
  @param <max> - max count of returned tokens.    
  @return
    If succeeds, return the count of sub-tokens; or return 0.

  Look out: This method is NOT thread-safe!
*/
int seg_tokenize( handle_t handle, token_t token, uint32_t type, token_t subtokens[], uint32_t max );


int seg_newword( handle_t handle,token_t newword[], uint32_t max );

/*!
  Close the handle, end the segmentation task, dispose all the resource.
  @param <handle> - the handle returned by seg_segment();

  Look out: You must call this method ONLY ONCE at the end of thread!
  To a valid hanle, you can close it ONLY ONCE, then it becomes invalid!
*/
void seg_close( handle_t handle );

/*!
  Split the mixed segment result.
  @param <phrasedict> - split phrase dict
  @param <pout> - the result of segment
  @param <subtokens> - the result of split
  @param <tcnt> - max count of split  
  @return the count of result tokens, 0 means failed.
*/
int seg_split( Sdict_search* phrasedict, scw_out_t* pout, token_t subtokens[], int tcnt );
/*!                                                                               
 Split one token.                                                 
 @param <phrasedict> - split phrase dict
 @param <handle> - handle returned by seg_segment() 
 @param <token> - the token to be splitted                                           
 @param <subtokens> - the result of split                                   
 @param <tcnt> - max count of split  
 @return the count of result tokens, 0 means failed.                                               
*/
int seg_split_tokenize( Sdict_search* phrasedict, handle_t handle, token_t token, token_t subtokens[], int tcnt);
// </Seg-API> --------------------------------------------------------------------------------//

#ifdef __cplusplus
}
#endif

#endif  /* ISEGMENT_H */

