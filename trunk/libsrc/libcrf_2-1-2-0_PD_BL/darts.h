//
//  Darts -- Double-ARray Trie System
//
//  $Id: darts.h,v 1.1 2010/03/05 09:57:24 zhangwei Exp $;
//
//  Copyright(C) 2001-2007 Taku Kudo <taku@chasen.org>
//
#ifndef DARTS_H_
#define DARTS_H_

#define DARTS_VERSION "0.31"
#include <vector>
#include <cstring>
#include <cstdio>


namespace CRFPP {

namespace Darts {



class DoubleArray {
	public:
	struct unit_t {	  
    int   base;
    unsigned int check;
  };
  unit_t        *array_;
  bool          no_delete_;

  public:
  typedef int  value_type;
  typedef char   key_type;
  typedef int  result_type;  // for compatibility

  explicit DoubleArray(): array_(0),
                              no_delete_(0){}
  ~DoubleArray() { clear(); }
  void set_array(void *ptr)
  {
    clear();
    array_ = reinterpret_cast<unit_t *>(ptr);
    no_delete_ = true;
  }
  void clear()
  {
    if (!no_delete_)
      delete [] array_;
    array_ = 0;
    no_delete_ = false;
  }
  inline int exactMatchSearch(const char *key,size_t *node_pos,int *id) const
  {
    *id = -1;
    int  b = array_[*node_pos].base;
    unsigned int p;
    p = b +(unsigned char)(*key) + 1;
    if (static_cast<unsigned int>(b) == array_[p].check)
        b = array_[p].base;
    else
        return -1;
    *node_pos = p;
    p = b;
    int n = array_[p].base;
    if (static_cast<unsigned int>(b) == array_[p].check && n < 0)
    {
       *id = -n - 1;
    }
    return 1;
  }

};
}
}
#endif

