#ifndef __UTIL_H__
#define __UTIL_H__
#include "types.h"

#define IS_UNUSED(x) ((void) x)

type type_to_array_type(type current);
void append_statement_to_tail(node* current_final_statement, node* new_final_statement);
#endif
