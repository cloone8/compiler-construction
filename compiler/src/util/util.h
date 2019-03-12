#ifndef __UTIL_H__
#define __UTIL_H__
#include "types.h"

#define IS_UNUSED(x) ((void) x)
#define STR_BUF_SIZE (50)

type type_to_array_type(type current);
type array_type_to_type(type current);
void append_statement_to_tail(node* block_to_append_to, node* statement_to_append);
void append_declaration_to_tail(node *block_to_append_to, node *declaration_to_append);
void append_functionvardeclaration_to_tail(node *block_to_append_to, node *declaration_to_append);
void append_exprarray_to_tail(node *block_to_append_to, node *new_final_exprarray);
fun_symbol_table_entry* fun_is_in_scope(fun_symbol_table* deepest_table, char* to_find);
symbol_table_entry* var_is_in_scope(symbol_table* deepest_table, char* to_find);
void print_exprarray(node* exprs);
#endif
