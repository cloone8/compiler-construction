#ifndef __DECLSPLITTING_LIB_H__
#define __DECLSPLITTING_LIB_H__

#include "types.h"
#include <stdlib.h>

/**
 * A recursive function that given a variable declaration of the
 * form "int[2 ,2] a = [[1, 2], [3, 4]];" will return a chain of N_statements
 * initialising each index of the array individually e.g.:
 *
 * a[0, 0] = 1;
 * a[0, 1] = 2;
 * a[1, 0] = ...
 *
 * @param varname: The name of the variable being processed
 * @param varentry: The symbol table entry of the array variable being processed
 * @param init_nesting_level: The nesting level expected. This would be 1 for a one
 *                            dimensional array, 2 for a two dimensional array etc.
 * @param indices: An array with the length of the amount of dimensions of the
 *                 array variable.
 * @param exprs: The N_exprarray node containing the list of initial values of
 *               the array
 * @return: A block of statement nodes to be prepended to the function statements
 */
node* exprarray_to_inits(char* varname, symbol_table_entry* varentry,
                         size_t init_nesting_level, size_t* indices, node* exprs);


/**
 * Prepends the given statement to the __init() function body
 * @param new_statement: The node of type N_statements to prepend
 * @param arg_info: The info struct of this traversal
 */
void prepend_to_initialiser(node* new_statement, info* arg_info);

/**
 * Counts the number of dimensions in the given dimensions node chain.
 * @param dimensions: Node of type N_exprarray containing the dimensions of an
 *                    array type node.
 * @return The amount of dimensions called
 */
size_t count_dims(node* dimensions);

/**
 * Converts the given array of indices to an expression array of Intconsts.
 * For example, the array [0, 1, 0] gets converted to a (0) -> (1) -> (0) chain of
 * nodes.
 * @param index_amount: The length of the indices array
 * @param indices: The array containing the indices
 * @return The new N_exprarray chain of intconsts
 */
node* indices_to_positions(size_t index_amount, size_t* indices);


/**
 * Takes the dimensions from a global array variable, and converts them to global
 * variables initialised with the value of the dimension.
 * @param global_name: The name of the global variable being processed
 * @param symbatle: The global symbol table
 * @param dimensions: The N_exprarray chain of dimensions
 * @return A chain of new global variable declarations to be prepended before
 *         the declaration of the processed global array.
 */
node* global_convert_dims_to_vars(char* global_name, symbol_table* symtable, node* dimensions);
/**
 * Takes the dimensions from a function array variable, and converts them to new
 * basic variables initialised with the value of the dimension.
 *
 * The new declarations and assignments are placed in the DECLS(arg_info) and
 * ASSIGNS(arg_info) macros
 * @param var_name: The name of the array variable being processed
 * @param dims: The N_exprarray chain of dimensions
 * @param arg_info: The traversal arg_info struct (to be used to get the symtable from)
 */
void fun_convert_dims_to_vars(char* var_name, node* dims, info* arg_info);

#endif
