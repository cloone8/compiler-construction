#ifndef __UTIL_H__
#define __UTIL_H__
#include "types.h"

#define IS_UNUSED(x) ((void) x)

type type_to_array_type(type current);
void append_statement_to_tail(node* current_final_statement, node* new_final_statement);
fun_symbol_table_entry* fun_is_in_scope(fun_symbol_table* deepest_table, char* to_find);
symbol_table_entry* var_is_in_scope(symbol_table* deepest_table, char* to_find);

#endif
