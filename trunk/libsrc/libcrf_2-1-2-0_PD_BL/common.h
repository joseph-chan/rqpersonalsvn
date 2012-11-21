#ifndef CRFPP_COMMON_H__
#define CRFPP_COMMON_H__

#include "ul_log.h"
#include <setjmp.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>


#define MODEL_VERSION 100

#define MAX_FEATURE_FUNCION_LEN 1024

#define MAX_TEMPLS_LEN 100
#define MAX_TEMPLS_NUM 100

#define CRF_MAX_WORD_LEN 100
#define CRF_MAX_WORD_ATTRIBUTE 5

#define CONV_INT_FACTOR 8000

#define MODEL_INIT_UNFINISH 0
#define MODEL_INIT_FINISH 1

#define TAG_INIT_UNFINISH 0
#define TAG_INIT_FINISH 1

#define DEFAULT_CRF_MAX_WORD_NUM 1000
#define DEFAULT_CRF_MAX_WORD_LEN 100
#define DEFAULT_MAX_NBEST 10

#define DEFAULT_PATH_FACTOR 1.0

namespace CRFPP {

template <class T> inline T _min(T x, T y) { return(x < y) ? x : y; }
template <class T> inline T _max(T x, T y) { return(x > y) ? x : y; }
struct whatlog {
  std::ostringstream stream_;
  std::string str_;
  const char* str() {
    str_ = stream_.str();
    return str_.c_str();
  }
  jmp_buf cond_;
};

class wlog {
 public:
  whatlog *l_;
  explicit wlog(whatlog *l): l_(l) { l_->stream_.clear(); }
  ~wlog() { longjmp(l_->cond_, 1); }
  int operator&(std::ostream &) { return 0; }
};
}

#define WHAT what_.stream_

#define CHECK_RETURN(condition, value)                                  \
  if (condition) {} else                                                \
    if (setjmp(what_.cond_) == 1) {                                     \
      return value;                                                     \
    } else                                                              \
      wlog(&what_) & what_.stream_ <<                                   \
          __FILE__ << "(" << __LINE__ << ") [" << #condition << "] "

#define CHECK_0(condition)      CHECK_RETURN(condition, 0)
#define CHECK_FALSE(condition)  CHECK_RETURN(condition, false)

#define CHECK_CLOSE_FALSE(condition)                                    \
  if (condition) {} else                                                \
    if (setjmp(what_.cond_) == 1) {                                     \
      close();                                                          \
      return false;                                                     \
    } else                                                              \
      wlog(&what_) & what_.stream_ <<                                   \
          __FILE__ << "(" << __LINE__ << ") [" << #condition << "] "

#define CHECK_DIE(condition)                                            \
  (condition) ? 0 : die() & std::cerr << __FILE__ <<                    \
                                                     "(" << __LINE__ << ") [" << #condition << "] "

#define CHECK_WARN(condition)                                           \
  (condition) ? 0 : warn() & std::cerr << __FILE__ <<                   \
                                                      "(" << __LINE__ << ") [" << #condition << "] "


#endif
