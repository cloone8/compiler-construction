#ifndef __UTIL_H__
#define __UTIL_H__

#include "types.h"
#include "node_appends.h"

#define UNUSED(x) ((void) x)
#define STR_BUF_SIZE (2048)

type type_to_array_type(type current);
type array_type_to_type(type current);
fun_symbol_table_entry* fun_is_in_scope(fun_symbol_table* deepest_table, char* to_find);
symbol_table_entry* var_is_in_scope(symbol_table* deepest_table, char* to_find);
void print_exprarray(node* exprs);
char* print_type(type to_print);
char* get_nodetype_string(nodetype nt);
void copy_node_error_data(node* source, node* target);

#endif
