#ifndef _CIVCC_MYTYPES_H_
#define _CIVCC_MYTYPES_H_

/*
 * This enumeration defines all monops
 */
typedef enum { MO_not, MO_neg, MO_unknown } monop;

/*
 * This enumeration defines all binops
 */

typedef enum
{
    BO_add,
    BO_sub,
    BO_mul,
    BO_div,
    BO_mod,
    BO_lt,
    BO_le,
    BO_gt,
    BO_ge,
    BO_eq,
    BO_ne,
    BO_and,
    BO_or,
    BO_unknown
} binop;

typedef enum { TP_void, TP_int, TP_float, TP_bool, TP_unknown } type;

#include "varsymboltable.h"
#include "funsymboltable.h"

#endif  /* _CIVCC_MYTYPES_H_ */
