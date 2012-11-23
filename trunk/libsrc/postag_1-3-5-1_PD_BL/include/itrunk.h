/*----------------------------------------------------------------------*
 * Copyright (c) 2009 by Baidu  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 * Any question and suggestion, contact with sharperdavid@hotmail.com.
 *----------------------------------------------------------------------*/

/**
 * @file itrunk.h
 * @brief API header file for trunk parsing library.
 * @author David Dai 
 * @version 1.0
 * @date 2009/1/19
 */

/**
 * @file itrunk.h
 * @brief implement the new algorithm.
 * @author David Dai 
 * @version 2.0
 * @date 2009/5/7
 */

#ifndef ITRUNK_H
#define ITRUNK_H

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

#include <ul_dict.h>

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

/*! maximum count of tokens to be processed */
#define MAX_TOKEN_COUNT 256

/*! type of term rank */
typedef enum term_rank_t
{
  TERM_RANK_OMIT = 0,
  TERM_RANK_MODIFY = 1,
  TERM_RANK_TRUNK = 2
}TERM_RANK;

/*! type of term modifier */
typedef enum modifier_t
{
  TERM_MODI_LOW = 1,
  TERM_MODI_MID = 2,
  TERM_MODI_HIGH = 3,
  TERM_MODI_UNESCAPE = 4,
}MODI_TYPE;

#define DEFAULT_REDUCE_TYPE 7
/*! type of trunk result */
typedef struct trunk_t
{   
  uint8_t rank : 4; // TERM_RANK
  uint8_t type : 4; // MODI_TYPE
  uint8_t reduce_type: 3;
  uint8_t term_level : 3;
  uint8_t reserve : 2;
  float weight;  
} trunk_t;

typedef struct cmp_trunk_t
{
    unsigned int ind;
    float weight;
} cmp_trunk_t;

/*! type declaration of trunk dict */
typedef struct trunk_dict_t;
/*! type of word rank dict */
typedef struct rank_dict_t;
/*! type of postag dict */
typedef struct tag_dict_t;

/*!
  Create the dictionary and other configure data.  
  @param[in] <dictpath> - directory name of dictionary, e.g. ../dict/, ../dict.
  @return NULL if failed, or return the dict pointer.

  Look out: You must call this method ONLY once in the beginning of process!
*/
trunk_dict_t* trk_create( const char* dictpath );

/*!
  Do the trunk parsing task.
  @param[in] <trkdict> - trunk dict.
  @param[in] <wdrdict> - word rank dict.
  @param[in] <tagdict> - postag dict, NULL is OK.
  @param[in] <tokens> - the pos tagged tokens.
  @param[in] <size> - size of <tokens> array.  
  @param[out] <trunks> - array to store the result.
  @param[in] <trksize> - size of <trunks> array.  
  @return If succeeds, return the count of processed terms; or return 0.  
*/
int trk_parse( trunk_dict_t* trkdict, 
               rank_dict_t* wdrdict, 
               tag_dict_t* tagdict,
               Sdict_build * pword_tag_dict,
               token_t tokens[], uint32_t size, 
               trunk_t trunks[], uint32_t trksize ); 

/*!
  Dispose all the resource.
  @param[in] <p> - the pointer of dict to be deleted.
*/
void trk_destroy( trunk_dict_t* p );


#endif  /* ITRUNK_H */


