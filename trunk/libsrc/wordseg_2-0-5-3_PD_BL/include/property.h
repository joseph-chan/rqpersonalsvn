#ifndef PROPERTY_H
#define PROPERTY_H

#include "scwdef.h"

#define INIT_PROPERTY(property) property.m_hprop = 0;property.m_lprop = 0;

//
//  the property table of the lemma
//

#define IS_PHRASE(property)     ((property.m_hprop)& 0x00000001)   
#define DEL_PHRASE(property)    ((property.m_hprop)& 0x11111110)
#define SET_PHRASE(property)    ((property.m_hprop)|= 0x00000001)

#define IS_MARK(property)       ((property.m_hprop)& 0x00000002)  
#define SET_MARK(property)      ((property.m_hprop)|= 0x00000002)

#define IS_NEWWORD(property)    ((property.m_lprop)& 0x04000000)
#define SET_NEWWORD(property)   ((property.m_lprop)|= 0x04000000)

#define IS_MULTI_TERM(property)  ((property.m_lprop)& 0x08000000)
#define SET_MULTI_TERM(property) ((property.m_lprop)|= 0x08000000)

#define IS_ALNUM(property)      ((property.m_hprop)& 0x00000004) 
#define SET_ALNUM(property)     ((property.m_hprop)|= 0x00000004)

#define IS_ONEWORD(property)    ((property.m_hprop)& 0x00000008) 
#define SET_ONEWORD(property)   ((property.m_hprop)|= 0x00000008)

#define IS_ASCIIWORD(property)  ((property.m_hprop)& 0x00000010) 
#define SET_ASCIIWORD(property) ((property.m_hprop)|= 0x00000010)

#define IS_ASCIINUM(property)   ((property.m_hprop)&0x00000020) 
#define SET_ASCIINUM(property)  ((property.m_hprop)|=0x00000020)

#define IS_HUMAN_NAME(property) ((property.m_hprop)& 0x00000040)  
#define SET_HUMAN_NAME(property)((property.m_hprop)|=0x00000040)

#define IS_TIME(property)       ((property.m_hprop)& 0x00000080) 
#define SET_TIME(property)      ((property.m_hprop)|= 0x00000080)

#define IS_CNUMBER(property)    ((property.m_hprop)& 0x00000100)
#define SET_CNUMBER(property)   ((property.m_hprop)|= 0x00000100)

#define IS_SURNAME(property)    ((property.m_hprop)& 0x00000200)
#define SET_SURNAME(property)   ((property.m_hprop)|= 0x00000200)

#define IS_QUANTITY(property)   ((property.m_hprop)& 0x00000400)
#define SET_QUANTITY(property)  ((property.m_hprop)|=0x00000400)

#define IS_LOC(property)        ((property.m_hprop)& 0x00000800)
#define SET_LOC(property)       ((property.m_hprop)|= 0x00000800)

#define IS_ORGNAME(property)    ((property.m_hprop)& 0x00001000)
#define SET_ORGNAME(property)   ((property.m_hprop)|= 0x00001000)

#define IS_PLACE_SUFFIX(property)      ((property.m_hprop)& 0x00800000)
#define SET_PLACE_SUFFIX(property)     ((property.m_hprop)|= 0x00800000)

#define IS_HZ(property)      ((property.m_hprop)& 0x01000000)
#define SET_HZ(property)     ((property.m_hprop)|= 0x01000000)

#define IS_DI(property)  ((property.m_hprop)& 0x02000000)
#define SET_DI(property)  ((property.m_hprop)|=0x02000000)

#define IS_BOOKNAME(property)   ((property.m_hprop)& 0x08000000)
#define SET_BOOKNAME(property)  ((property.m_hprop)|= 0x08000000)

#define IS_BOOKNAME_START(property) ((property.m_hprop)&0x20000000)
#define SET_BOOKNAME_START(property) ((property.m_hprop)|=0x20000000)

#define IS_BOOKNAME_END(property)       ((property.m_hprop)&0x40000000)
#define SET_BOOKNAME_END(property)      ((property.m_hprop)|=0x40000000)

#define IS_UNIVERSITY(property)        ((property.m_lprop)& 0x00000001)
#define SET_UNIVERSITY(property)       ((property.m_lprop) |= 0x00000001)

#define IS_SOFTNAME(property)        ((property.m_lprop)& 0x00000002)
#define SET_SOFTNAME(property)       ((property.m_lprop) |= 0x00000002)

#define IS_FMNAME(property)        ((property.m_lprop)& 0x00000004)
#define SET_FMNAME(property)       ((property.m_lprop) |= 0x00000004)

#define IS_LOC_SMALL(property)        ((property.m_lprop)& 0x00000008)
#define SET_LOC_SMALL(property)       ((property.m_lprop) |= 0x00000008)

#define IS_BRAND(property)        ((property.m_lprop)& 0x00000010)
#define SET_BRAND(property)       ((property.m_lprop) |= 0x00000010)

#define IS_TEMP(property)        ((property.m_lprop)& 0x00000020)
#define SET_TEMP(property)       ((property.m_lprop) |= 0x00000020)

#define IS_SPEC_ENSTR(property)  ((property.m_lprop)& 0x00000080)
#define SET_SPEC_ENSTR(property)  ((property.m_lprop)|= 0x00000080)

#define IS_NAMEMID(property)      ((property.m_hprop)& 0x00002000)
#define SET_NAMEMID(property)     ((property.m_hprop)|= 0x00002000)

#define IS_NAMEEND(property)     ((property.m_hprop)& 0x00004000)
#define SET_NAMEEND(property)        ((property.m_hprop)|= 0x00004000)

#define IS_SPACE(property)        ((property.m_lprop)& 0x00000040)
#define SET_SPACE(property)       ((property.m_lprop) |= 0x00000040)

#define IS_MANWORD(property)	((property.m_lprop) & 0x00100000)
#define SET_MANWORD(property)	((property.m_lprop) |= 0x00100000)
//#define IS_WORDNOTDYN(property)	((property.m_lprop) & 0x00010000)
//#define SET_WORDNOTDYN(property)	((property.m_lprop) |= 0x00010000)
//#define CLOSE_WORDNOTDYN(property)	((property.m_lprop) &= 0xFFFEFFFF)

/* property for Chinese */
#define IS_NAME_STICK(property)     ((property.m_hprop)& 0x00008000)
#define SET_NAME_STICK(property)    ((property.m_hprop)|= 0x00008000)

#define IS_QIANZHUI(property)       ((property.m_hprop)& 0x00010000)
#define SET_QIANZHUI(property)      ((property.m_hprop)|= 0x00010000)

#define IS_HOUZHUI(property)        ((property.m_hprop)& 0x00020000)
#define SET_HOUZHUI(property)       ((property.m_hprop)|= 0x00020000)

#define IS_QIANDAO(property)        ((property.m_hprop)& 0x00040000)
#define SET_QIANDAO(property)       ((property.m_hprop)|= 0x00040000)

#define IS_HOUDAO(property)         ((property.m_hprop)& 0x00080000)
#define SET_HOUDAO(property)        ((property.m_hprop)|= 0x00080000)

#define IS_CFOREIGN(property)       ((property.m_hprop)& 0x00100000)
#define SET_CFOREIGN(property)      ((property.m_hprop)|= 0x00100000)

#define IS_STOPWORD(property)       ((property.m_hprop)& 0x00200000)
#define SET_STOPWORD(property)      ((property.m_hprop)|= 0x00200000)

#define IS_SPECIAL(property)        ((property.m_hprop)& 0x00400000)
#define SET_SPECIAL(property)       ((property.m_hprop)|= 0x00400000)

#define IS_QYPD(property)       ((property.m_hprop)& 0x04000000)
#define SET_QYPD(property)       ((property.m_hprop)|=0x04000000)

#define IS_2WHPREFIX(property) ((property.m_hprop)& 0x10000000)
#define SET_2WHPREFIX(property) ((property.m_hprop)|= 0x10000000)
 
#define IS_CH_NAME(property)  ((property.m_hprop)&0x80000000)
#define SET_CH_NAME(property)  ((property.m_hprop)|=0x80000000)

#define IS_LOC_FOLLOW(property)  ((property.m_lprop)&0x00000100)
#define SET_LOC_FOLLOW(property)  ((property.m_lprop)|=0x00000100)

#define IS_FNAMEBGN(property)      ((property.m_lprop)& 0x00000200)
#define SET_FNAMEBGN(property)     ((property.m_lprop)|= 0x00000200)

#define IS_FNAMEMID(property)      ((property.m_lprop)& 0x00000400)
#define SET_FNAMEMID(property)     ((property.m_lprop)|= 0x00000400)

#define IS_FNAMEEND(property)     ((property.m_lprop)& 0x00000800)
#define SET_FNAMEEND(property)        ((property.m_lprop)|= 0x00000800)

#define IS_2FNAMEEND(property)     ((property.m_lprop)& 0x00001000)
#define SET_2FNAMEEND(property)        ((property.m_lprop)|= 0x00001000)

#define IS_1W_NAMEEND(property)  ((property.m_hprop&0x00000008) && (property.m_hprop&0x00004000))

#define IS_1W_SUFFIX(property) ((property.m_lprop) & 0x01000000)
#define SET_1W_SUFFIX(property) ((property.m_lprop) |= 0x01000000)

// add property for U V W c v 11.13.2008
#define IS_VIDEO(property)    ((property.m_lprop) & 0x80000000)
#define SET_VIDEO(property)    ((property.m_lprop) |= 0x80000000)

#define IS_GAME(property)     ((property.m_lprop) & 0x40000000)
#define SET_GAME(property)     ((property.m_lprop) |= 0x40000000)

#define IS_SONG(property)     ((property.m_lprop) & 0x20000000)
#define SET_SONG(property)    ((property.m_lprop) |= 0x20000000)

#define IS_SCW_PRODUCT(property)      ((property.m_lprop) & 0x10000000)
#define SET_SCW_PRODUCT(property)      ((property.m_lprop) |= 0x10000000)

#define IS_STATIC_BOOK(property)      ((property.m_lprop) & 0x02000000)
#define SET_STATIC_BOOK(property)      ((property.m_lprop) |= 0x02000000)


/* property for Japanese */
#define IS_NOUN(property)       ((property.m_hprop)&0x00010000)
#define SET_NOUN(property)      ((property.m_hprop)|=0x00010000)

#define IS_GIVENNAME(property)       ((property.m_hprop)&0x00020000)
#define SET_GIVENNAME(property)      ((property.m_hprop)|=0x00020000)

#define IS_PINGJM(property)       ((property.m_hprop)&0x00040000)
#define SET_PINGJM(property)      ((property.m_hprop)|=0x00040000)

#define IS_PIANJM(property)       ((property.m_hprop)&0x00080000)
#define SET_PIANJM(property)      ((property.m_hprop)|=0x00080000)

#define IS_PAJMSTR(property)       ((property.m_hprop)&0x00100000)
#define SET_PAJMSTR(property)      ((property.m_hprop)|=0x00100000)

#define IS_VERB(property)       ((property.m_hprop)&0x00200000)
#define SET_VERB(property)      ((property.m_hprop)|=0x00200000)

#define IS_PJM(property)        (((property.m_hprop)&0x00040000) || ((property.m_hprop)&0x00080000))

#define IS_JPNAME_BLANK(property)       ((property.m_lprop)&0x00200000)
#define SET_JPNAME_BLANK(property)      ((property.m_lprop)|=0x00200000)

#define IS_BIG_PHRASE(property)       ((property.m_lprop)&0x00400000)
#define SET_BIG_PHRASE(property)      ((property.m_lprop)|=0x00400000)

// wuzhongqin add 2009.05.13 for strong suffix after name ,such as "˜”¡¢¤µ¤Þ¡¢µî¡¢¤Á¤ã¤ó¡¢ÊÏ"
#define IS_NAME_SUFFIX(property)       ((property.m_lprop)&0x00800000)
#define SET_NAME_SUFFIX(property)      ((property.m_lprop)|=0x00800000)

#endif

