//============================================================================
// Name        : Converter.h
// Created on  : Jul 22, 2010
// Author      : Zhijin GUO
// Copyright   : Baifendian Information Technology
// Description : Convert character set, e.g: utf8 and GBK
//============================================================================


#ifndef CONVERTER_H_
#define CONVERTER_H_

// system library header file
#include <iostream>
#include <iconv.h>
#include <errno.h>

// local header file
#include "CommonException.h"

using namespace std;

namespace baifendian_common {

	class CodeConverter {
	// reference http://www.gnu.org/s/libc/manual/html_node/Generic-Conversion-Interface.html
	public:
		CodeConverter(const char * from_charset, const char * to_charset) {
			cd = iconv_open(to_charset, from_charset);
			if (cd == (iconv_t)-1 ) {
				//iconv_close(cd);
				throw CommonException("Character Set Converter Initialize ERROR");
			}
		}
		~CodeConverter() {
			iconv_close(cd);
		}

		int convert(char * inbuf, size_t inlen, char * outbuf, size_t outlen)
		{
			char * outptr = outbuf;
			char * inptr = inbuf;
			size_t inleft = inlen;
			try {
				// 如果字符集存在不可转换字符, 则跳过继续转换。
				while (inleft > 0) {
					char * tmp = inptr;
					size_t n = iconv(cd, &inptr, &inleft, &outptr, &outlen);

					if ( (n != (size_t)-1) && inleft == 0) {
						// no invalid character, all inbuf being converted.
						break;
					}
					if (n == (size_t)-1 && errno == EILSEQ) {
						// inbuf has invalid character.
						if (inptr == tmp) {
							inptr++;
						}
					} else { // error
						throw -1;
					}
				}
			} catch (...) {
				//throw CommonException("Character Set Convert ERROR!");
				return -1;
			}

			return 0;
		}
	private:
		iconv_t cd;
	};

} // end of namespace baifendian_common

#endif /* CONVERTER_H_ */
