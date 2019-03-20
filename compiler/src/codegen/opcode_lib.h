#ifndef __OPCODE_LIB_H__
#define __OPCODE_LIB_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef INT64_MAX
// Platform support 64 bit ints

/**
 * Type to be used for all constants given to the opcode lib
 */
typedef int64_t const_t;
#define PRIcon "%" PRId64
#else
// Platform only supports 32 bit ints

/**
 * Type to be used for all constants given to the opcode lib
 */
typedef int32_t const_t;
#define PRIcon "%" PRId32
#endif

/**
 * The file struct to write all the code to.
 */
extern FILE* ofile;

#endif
