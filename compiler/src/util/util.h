#ifndef __UTIL_H__
#define __UTIL_H__

#include "types.h"
#include "node_appends.h"

#define UNUSED_VAR(x) ((void) x)

// The default size for string buffers. Set to the amount of bits in a size_t, as this way it
// can hold at least a size_t number
#define STR_BUF_SIZE (64)

/**
 * Allocates a char buffer with the given name on the stack, and fills it with
 * the string representation of the given size_t.
 * @param __buffer_name__: The name of char buffer to create
 * @param __to_convert__: The size_t to convert
 */
#define get_size_t_str(__buffer_name__, __to_convert__) char __buffer_name__[STR_BUF_SIZE]; \
                                                        size_t_to_str(__buffer_name__, __to_convert__)


/**
 * Converts a basic type to its respective array type.
 * If the given type is not a basic type, does nothing and
 * just returns the parameter
 * @param current: The type to convert
 * @return: The converted type
 */
extern type type_to_array_type(type current);

/**
 * Converts an array type to its respective basic type.
 * If the given type is not an array type, does nothing and
 * just returns the parameter
 * @param current: The type to convert
 * @return: The converted type
 */
extern type array_type_to_type(type current);

/**
 * Given the name of a function, tries to find it in the current symbol table
 * and all parent tables.
 * @param deepest_table: The table representing the current deepest scope.
 * @param to_find: The name of the function to find.
 * @return The table entry of the function if it was found and is in scope,
 *         NULL otherwise.
 */
extern fun_symbol_table_entry* fun_is_in_scope(fun_symbol_table* deepest_table, char* to_find);

/**
 * Given the name of a variable, tries to find it in the current symbol table
 * and all parent tables.
 * @param deepest_table: The table representing the current deepest scope.
 * @param to_find: The name of the variable to find.
 * @return The table entry of the variable if it was found and is in scope,
 *         NULL otherwise.
 */
extern symbol_table_entry* var_is_in_scope(symbol_table* deepest_table, char* to_find);

/**
 * Prints the given exprarray and all its sub-arrays to the DBUG output. Does nothing if
 * DBUG_OFF is defined.
 *
 * @param exprs: The exprarray to print
 */
extern void print_exprarray(node* exprs);

/**
 * Converts the given type to its string representation.
 * @param to_print: The type to convert
 * @return pointer to the string of the given type. String is
 *         located on the heap, so needs freeing.
 */
extern char* get_type_str(type to_print);

/**
 * Converts the given binop to its string representation.
 * @param bo_type: The binop to convert
 * @return pointer to the string of the given binop. String is
 *         located on the heap, so needs freeing.
 */
extern char* get_binop_str(binop bo_type);

/**
 * Converts the given nodetype to its string representation.
 * @param nt: The nodetype to convert
 * @return pointer to the string of the given nodetype. String is
 *         located on the heap, so needs freeing.
 */
extern char* get_nodetype_string(nodetype nt);

/**
 * Copies the node diagnostic data NODE_FILE, NODE_LINE and NODE_COL
 * from source to target.
 * @param source: Node to copy from
 * @param target: Node to copy to
 */
extern void copy_node_error_data(node* source, node* target);


/**
 * Takes a size_t type integer and stores its string representation in the given buffer
 * WARNING: This function needs a string buffer of at least STR_BUF_SIZE characters
 * long.
 *  @param buffer: The char* buffer (of at least size STR_BUF_SIZE long) to store
 *                 the converted size_t in.
 *  @param toconvert: The size_t to convert
 */
extern void size_t_to_str(char* buffer, size_t toconvert);

#endif
