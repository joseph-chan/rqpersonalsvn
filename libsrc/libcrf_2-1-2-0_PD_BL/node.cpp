//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: node.cpp,v 1.1 2010/03/05 09:57:24 zhangwei Exp $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#include <cmath>
#include "node.h"
#include "common.h"

namespace CRFPP {
	
	
/*
  void Node::calcExpectation(double *expected, double Z, size_t size) const {
  const double c = std::exp(alpha + beta - cost - Z);
  for (int *f = fvector; *f != -1; ++f) expected[*f + y] += c;
  for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it)
  (*it)->calcExpectation(expected, Z, size);
  }
*/
}
