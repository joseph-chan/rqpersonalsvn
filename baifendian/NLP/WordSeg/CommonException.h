//============================================================================
// Name        : Exception.h
// Created on  : Jul 22, 2010
// Author      : Zhijin GUO
// Copyright   : Baifendian Information Technology
// Description :
//============================================================================


#ifndef EXCEPTION_H_
#define EXCEPTION_H_

// system library header file
#include <iostream>

using namespace std;

namespace baifendian_common {


	class CommonException : public exception {
	public:
		explicit CommonException(const string & msg) : m_msg(msg) {
		}
		virtual ~CommonException() throw() {}

		virtual const char* what() const throw() {
			return m_msg.c_str();
		}
	private:
		string m_msg;
	};

} // end of namespace baifendian_common
#endif /* EXCEPTION_H_ */
