/* $XJG: xslib/jenkins.h,v 1.2 2006/06/26 06:30:29 jiagui Exp $ */
#ifndef JENKINS_H_
#define JENKINS_H_ 1

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


uint32_t jenkins_hash(const void *key, size_t length, uint32_t initval);

uint32_t jenkins_hash4(const uint32_t *k, size_t length, uint32_t initval);


#ifdef __cplusplus
}
#endif

#endif

