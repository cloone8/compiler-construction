#ifndef _CIVCC_MYTYPES_H_
#define _CIVCC_MYTYPES_H_


/*
 * This enumeration defines all monops
 */
typedef enum { MO_not, MO_neg, MO_unknown } monop;

/*
 * This enumeration defines all binops
 */

typedef enum { AO_add, AO_sub, AO_mul, AO_div, AO_mod, AO_unknown } arithop;

typedef enum { RO_lt, RO_le, RO_gt, RO_ge, RO_eq, RO_ne, RO_unknown } relop;

typedef enum { LO_and, LO_or, LO_unknown } logop;

typedef enum { FT_void, FT_int, FT_float, FT_bool, BO_unknown } type;


#endif  /* _CIVCC_MYTYPES_H_ */
