#ifndef __UTIL_H__
#define __UTIL_H__

#include "types.h"
#include "node_appends.h"

#define UNUSED(x) ((void) x)
#define STR_BUF_SIZE (64)

#define get_size_t_str(__buffer_name__, __to_convert__) char __buffer_name__[STR_BUF_SIZE]; \
                                                        size_t_to_str(__buffer_name__, __to_convert__)

// The default size for string buffers. Set to 64 characters, as this way it
// can hold most number types when they are converted to a string

type type_to_array_type(type current);
type array_type_to_type(type current);
fun_symbol_table_entry* fun_is_in_scope(fun_symbol_table* deepest_table, char* to_find);
symbol_table_entry* var_is_in_scope(symbol_table* deepest_table, char* to_find);
void print_exprarray(node* exprs);
char* get_type_str(type to_print);
char* get_binop_str(binop bo_type);
char* get_nodetype_string(nodetype nt);
void copy_node_error_data(node* source, node* target);


/**
 * Takes a size_t type integer and stores its string representation in the given buffer
 * WARNING: This function needs a string buffer of at least STR_BUF_SIZE characters
 * long.
 *  @param buffer: The char* buffer (of at least size STR_BUF_SIZE long) to store
 *                 the converted size_t in.
 *  @param toconvert: The size_t to convert
 */
void size_t_to_str(char* buffer, size_t toconvert);

#endif
