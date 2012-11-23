/*----------------------------------------------------------------------*
 * Copyright (c) 2007 by Baidu  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 * Any question and suggestion, contact with sharperdavid@hotmail.com.
 *----------------------------------------------------------------------*/

/**
 * @file ipostag.h
 * @brief API header file for chinese word POS tagging library.
 * @author  David Dai 
 * @version 1.0
 * @date 2007/08/20
 */

/**
 * @brief add a new interface.
 * @author  David Dai 
 * @version 2.0
 * @date 2009/02/17
 */

#ifndef IPOSTAG_H
#define IPOSTAG_H

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
  uint32_t weight : 24; /* weight of term, NOT used */

  /* property */
  struct
  {
    uint32_t m_lprop;  /* high 32 bit */
    uint32_t m_hprop;  /* low 32 bit */
  } prop;

  /* dict index */
  long index; /* point to the address of dict item, used by wordseg */ 

  /* term buffer */
  char buffer[TERM_MAX_LEN];

} token_t;
#endif /* TOKEN_TYPE_DEFINED */


// API I
//------------------------------------------------------------------------------//
/*!
  Initialize the dictionary and other configure data.  
  @param <dictpath> - directory name of dictionary, e.g. ../dict/, ../dict.
  @return If succeeds, return 0; or return the nonzero error code.

  Look out: You must call this method ONLY once in the beginning of process!
*/
int tag_open( const char* dictpath );

/*!
  Do the POS tagging task.
  @param <tokens> - the segmented tokens processed by using libsegment.
  @param <size> - size of <tokens> array.
  @return If succeeds, return the count of tagged tokens; or return 0.
    
  Look out: POS is tagged in "type" field of token_t, this method is thread-safe!
*/
int tag_postag( token_t tokens[], uint32_t size );

/*!
  Dispose all the resource.

  Look out: You must call this method ONLY once at the end of process!
*/
void tag_close();


// API II
//------------------------------------------------------------------------------//
#include <ul_dict.h>

/*! sign of out of tag set */
#define OUT_OF_TAG 0x800000
/*! type of postag dict */
typedef struct tag_dict_t;

/*!
  Create the dictionary and other configure data.  
  @param <dictpath> - directory name of dictionary, e.g. ../dict/, ../dict.
  @return NULL if failed, or return the dict pointer.

  Look out: You must call this method ONLY once in the beginning of process!
*/
tag_dict_t* tag_create(const char* dictpath);

/*!
  Do the POS tagging task.
  @param <pdcit> - POS tag dict.
  @param <tokens> - the segmented tokens processed by using libsegment.
  @param <size> - size of <tokens> array.
  @return If succeeds, return the count of tagged tokens; or return 0.
    
  Look out: POS tag value is saved in "type" field of token_t, this method is thread-safe!
*/
int tag_postag( tag_dict_t* pdict, token_t tokens[], uint32_t size );

/*!
  Dispose all the resource.
  @param <p> - the pointer of dict to be deleted.
*/
void tag_destroy( tag_dict_t* p );


#endif  /* IPOSTAG_H */




