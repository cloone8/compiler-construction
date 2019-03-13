#include "declsplitting_lib.h"

#include <stdlib.h>
#include "types.h"
#include "node_basic.h"
#include "varsplit.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free_node.h"
#include "dbug.h"

#include "str.h"
#include "ctinfo.h"
#include "memory.h"

#include "symbol_table.h"

#include "util.h"

#include "varsplit_info.h"

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
 * @param current_nesting_level: The current nesting level of the recursive function.
 *                               This should always be 0 when calling the function manually
 * @param indices: An array with the length of the amount of dimensions of the
 *                 array variable.
 * @param exprs: The N_exprarray node containing the list of initial values of
 *               the array
 * @return: A block of statement nodes to be prepended to the function statements
 */
static node* exprarray_to_inits_rec(char* varname, symbol_table_entry* varentry,
                                size_t init_nesting_level, size_t current_nesting_level,
                                size_t* indices, node* exprs) {
    DBUG_ENTER("exprarray_to_inits_rec");
    DBUG_PRINT("exprarray_to_inits_rec", ("Arrived at nesting level %zu of %zu", current_nesting_level + 1, init_nesting_level));

    node* toreturn = NULL;

    node* current_expr = exprs;
    size_t current_index = 0;

    while(current_expr != NULL) {
        // Reset the index of the current nesting level
        // We only do this for > 0 because the first iteration of
        // this algorithm is always the main exprarray containing the actual
        // values, and does not hold any information itself
        if(current_nesting_level > 0) {
            indices[current_nesting_level - 1] = current_index;
        }


        if(current_nesting_level == init_nesting_level) {
            // We have arrived at the deepest nesting level. Time to actually create the statement.
            DBUG_PRINT("exprarray_to_inits_rec", ("Max nesting level of %zu reached, filling", current_nesting_level + 1));

            // We have already gone in too deep, this means that there were too few nested levels
            if(NODE_TYPE(current_expr) != N_exprarray) {
                CTIerror("Invalid initialisation nesting level for variable %s: Expected %d but got %d", varname, init_nesting_level, current_nesting_level - 1);
                DBUG_RETURN(NULL);
            }

            node* value = EXPRARRAY_CURRENT(current_expr);

            // The value is still an N_exprarray, which means that there are still nesting levels to be traversed.
            if(NODE_TYPE(value) == N_exprarray) {
                CTIerror("Invalid initialisation nesting level for variable %s: Expected %d but got %d", varname, init_nesting_level, current_nesting_level + 1);
                DBUG_RETURN(NULL);
            }

            EXPRARRAY_CURRENT(current_expr) = NULL;

            // Create a new arrayvarassign statement assigning this position to the newly
            // retrieved valie
            node* new_varassign = TBmakeArrayvarassign(STRcpy(varname), indices_to_positions(init_nesting_level, indices), value);
            ARRAYVARASSIGN_VARPOINTER(new_varassign) = varentry;
            node* statement = TBmakeStatements(new_varassign, NULL);

            // Append the statement to the current list of statements
            if(toreturn == NULL) {
                toreturn = statement;
            } else {
                append_statement_to_tail(toreturn, statement);
            }
        } else {
            // If the nesting level is not 1, this means that we have to traverse deeper into the expression array
            DBUG_PRINT("exprarray_to_inits_rec", ("Now at nesting level of %zu", current_nesting_level + 1));

            // Enter a new recursive level
            node* new_inits = exprarray_to_inits_rec(varname, varentry, init_nesting_level, current_nesting_level + 1, indices, EXPRARRAY_CURRENT(current_expr));
            if(new_inits == NULL) {
                DBUG_RETURN(NULL);
            }

            if(toreturn == NULL) {
                toreturn = new_inits;
            } else {
                append_statement_to_tail(toreturn, new_inits);
            }
        }

        // Increment the current index and free the EXPRARRAY to prevent
        // memory leaks
        current_index++;
        node* tmp = current_expr;
        current_expr = EXPRARRAY_NEXT(current_expr);
        EXPRARRAY_NEXT(tmp) = NULL;
        EXPRARRAY_CURRENT(tmp) = NULL;
        FREEexprarray(tmp, NULL);
    }

    DBUG_RETURN(toreturn);
}


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
                         size_t init_nesting_level, size_t* indices, node* exprs) {
    DBUG_ENTER("exprarray_to_inits");
    DBUG_PRINT("exprarray_to_inits", ("Processing variable %s", varname));
    print_exprarray(exprs);
    node* result = exprarray_to_inits_rec(varname, varentry, init_nesting_level, 0, indices, exprs);
    DBUG_RETURN(result);
}


/**
 * Prepends the given statement to the __init() function body
 * @param new_statement: The node of type N_statements to prepend
 * @param arg_info: The info struct of this traversal
 */
void prepend_to_initialiser(node* new_statement, info* arg_info) {
    node* current_final_statement = FUNCTIONBODY_STATEMENTS(INITBODY(arg_info));

    // If the initialiser function currently has no statements, make this the statement.
    if(current_final_statement == NULL) {
        FUNCTIONBODY_STATEMENTS(INITBODY(arg_info)) = new_statement;
    } else {
        append_statement_to_tail(new_statement, current_final_statement);
        FUNCTIONBODY_STATEMENTS(INITBODY(arg_info)) = new_statement;
    }
}

/**
 * Counts the number of dimensions in the given dimensions node chain.
 * @param dimensions: Node of type N_exprarray containing the dimensions of an
 *                    array type node.
 * @return The amount of dimensions called
 */
size_t count_dims(node* dimensions) {
    DBUG_ENTER("count_dims");
    DBUG_ASSERT((NODE_TYPE(dimensions) == N_exprarray), "count_dims called without exprarray as argument!");
    DBUG_ASSERT((dimensions != NULL), "count_dims called with NULL as argument!");

    size_t count = 0;

    node* current_dim = dimensions;

    while(current_dim != NULL) {
        count++;
        current_dim = EXPRARRAY_NEXT(current_dim);
    }

    DBUG_RETURN(count);
}

/**
 * Converts the given array of indices to an expression array of Intconsts.
 * For example, the array [0, 1, 0] gets converted to a (0) -> (1) -> (0) chain of
 * nodes.
 * @param index_amount: The length of the indices array
 * @param indices: The array containing the indices
 * @return The new N_exprarray chain of intconsts
 */
node* indices_to_positions(size_t index_amount, size_t* indices) {
    DBUG_ENTER("indices_to_positions");
    DBUG_PRINT("indices_to_positions", ("Converting %zu indices to nodes", index_amount));
    node* head = NULL;

    for(size_t i = index_amount; i > 0; i--) {
        head = TBmakeExprarray(head, TBmakeExpr(TBmakeConstant(TBmakeIntconst(indices[i - 1]))));
    }

    DBUG_RETURN(head);
}


/**
 * Takes the dimensions from a global array variable, and converts them to global
 * variables initialised with the value of the dimension.
 * @param global_name: The name of the global variable being processed
 * @param symbatle: The global symbol table
 * @param dimensions: The N_exprarray chain of dimensions
 * @return A chain of new global variable declarations to be prepended before
 *         the declaration of the processed global array.
 */
node* global_convert_dims_to_vars(char* global_name, symbol_table* symtable, node* dimensions) {
    DBUG_ENTER("global_convert_dims_to_vars");
    DBUG_ASSERT((NODE_TYPE(dimensions) == N_exprarray), "global_convert_dims_to_vars called without exprarray as argument!");
    DBUG_ASSERT((dimensions != NULL), "global_convert_dims_to_vars called with NULL as argument!");

    node* decls = NULL;
    node* current_dim = dimensions;
    size_t dimcount = 0;

    // Go through all dimensions
    while(current_dim != NULL) {
        // Take the value of the dimension (e.g. the 1 and 2 in "int[1, 2] a;") and
        // put it in a new global definition
        node* dim_value = EXPRARRAY_CURRENT(current_dim);
        node* new_definition = TBmakeGlobaldefinition(FALSE, TBmakeExprarray(NULL, dim_value));

        // Convert the dimension into a name of the format "tmp-[dimcount]-[global_name]"
        char name_buffer[STR_BUF_SIZE];
        snprintf(name_buffer, STR_BUF_SIZE, "%zu", dimcount++);
        char *new_global_name = STRcatn(4, "tmp-", name_buffer, "-", global_name);

        // Add the new variable to the symbol table
        symbol_table_entry* var_entry = symbol_table_add(symtable, new_global_name, TP_int);
        GLOBALDEFINITION_VARPOINTER(new_definition) = var_entry;

        // Put the newly created definition into a new global declaration
        node* new_global = TBmakeGlobal(new_global_name, TP_int, new_definition, NULL);
        node* new_decls = TBmakeDeclaration(new_global, NULL);

        // Put the declaration at the end of the declaration chain currently being constructed
        if(decls == NULL) {
            decls = new_decls;
        } else {
            append_declaration_to_tail(decls, new_decls);
        }

        // Replace the current dimension with a varread of the previously created
        // new variable e.g. "int[1, 2] a" -> "int[tmp-0-a, tmp-1-a] a"
        node* new_varread = TBmakeVarread(STRcpy(new_global_name));
        VARREAD_VARPOINTER(new_varread) = var_entry;
        EXPRARRAY_CURRENT(current_dim) = TBmakeExpr(new_varread);

        current_dim = EXPRARRAY_NEXT(current_dim);
    }

    DBUG_RETURN(decls);
}

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
void fun_convert_dims_to_vars(char* var_name, node* dims, info* arg_info) {
    DBUG_ENTER("fun_convert_dims_to_vars");
    DBUG_ASSERT((NODE_TYPE(dims) == N_exprarray), "fun_convert_dims_to_vars called without exprarray as argument!");
    DBUG_ASSERT((dims != NULL), "fun_convert_dims_to_vars called with NULL as argument!");

    symbol_table* cur_table = CURRENT_SYMBOL_TABLE(arg_info);

    node* current_dim = dims;
    size_t dimcount = 0;

    while(current_dim != NULL) {
        // Get the current value of the dimension (e.g. the 1 and 2 in "int[1, 2] a;")
        node* dim_value = EXPRARRAY_CURRENT(current_dim);

        // Create a new variable with the format "tmp-[dimcount]-[var_name]",
        // for example for the first dimension of "int[1, 2] a;" -> "tmp-0-a"
        char name_buffer[STR_BUF_SIZE];
        snprintf(name_buffer, STR_BUF_SIZE, "%zu", dimcount++);
        char *new_var_name = STRcatn(4, "tmp-", name_buffer, "-", var_name);

        // Create the declaration
        node* new_decl = TBmakeFunctionvardeclaration(TBmakeBasicvardeclaration(STRcpy(new_var_name), TP_int, NULL), NULL);

        // Add the new variable to the symbol table
        symbol_table_entry* new_entry = symbol_table_add(cur_table, new_var_name, TP_int);

        // Append it to the current list of new declarations
        if(DECLS(arg_info) == NULL) {
            DECLS(arg_info) = new_decl;
        } else {
            append_functionvardeclaration_to_tail(DECLS(arg_info), new_decl);
        }

        // Create the assignment
        node* new_varassign = TBmakeVarassign(STRcpy(new_var_name), dim_value);
        VARASSIGN_VARPOINTER(new_varassign) = new_entry;
        node* new_statement = TBmakeStatements(new_varassign, NULL);

        // Append the newly created assignment statement to the current list of
        // assignments
        if(ASSIGNS(arg_info) == NULL) {
            ASSIGNS(arg_info) = new_statement;
        } else {
            append_statement_to_tail(ASSIGNS(arg_info), new_statement);
        }

        // Replace the current dimension with a varread of the newly created variable,
        // e.g. "int[1, 2] a;" -> "int[tmp-0-a, tmp-1-a] a;""
        node* new_varread = TBmakeVarread(STRcpy(new_var_name));
        VARREAD_VARPOINTER(new_varread) = new_entry;
        EXPRARRAY_CURRENT(current_dim) = TBmakeExpr(new_varread);

        current_dim = EXPRARRAY_NEXT(current_dim);

        // Free the name of the var, as it has been copied where needed
        MEMfree(new_var_name);
    }

    DBUG_VOID_RETURN;
}

static char* counter_to_name(char* var_name, size_t counter_num) {
    char name_buffer[STR_BUF_SIZE];
    snprintf(name_buffer, STR_BUF_SIZE, "%zu", counter_num);
    return STRcatn(4, "tmp-counter-", name_buffer, "-", var_name);
}

static node* dims_to_positions(char* var_name, node* dims, info* arg_info) {
    DBUG_ENTER("dims_to_positions");
    node* toreturn = NULL;

    node* current_dim = dims;
    size_t dimnum = 0;

    while(current_dim != NULL) {
        char* countername = counter_to_name(var_name, dimnum++);
        symbol_table_entry* counter_entry = var_is_in_scope(CURRENT_SYMBOL_TABLE(arg_info), countername);
        DBUG_ASSERT((counter_entry != NULL), "Couldn't find counter symbol table entry!");

        node* varread = TBmakeVarread(STRcpy(counter_entry->name));
        VARREAD_VARPOINTER(varread) = counter_entry;
        node* new_pos = TBmakeExprarray(NULL, TBmakeExpr(varread));

        if(toreturn == NULL) {
            toreturn = new_pos;
        } else {
            append_exprarray_to_tail(toreturn, new_pos);
        }

        current_dim = EXPRARRAY_NEXT(current_dim);
    }

    DBUG_RETURN(toreturn);
}

static node* get_current_dim(node* dims, size_t iters_left) {
    DBUG_ENTER("get_current_dim");
    node* toreturn = dims;
    size_t dimcount = count_dims(dims);

    for(size_t i = dimcount; i > iters_left; i--) {
        DBUG_ASSERT((toreturn != NULL), "toreturn is NULL! Too many iters for the amount of dimensions!");

        toreturn = EXPRARRAY_NEXT(toreturn);
    }

    DBUG_ASSERT((toreturn != NULL), "toreturn is NULL! Too many iters for the amount of dimensions!");
    DBUG_RETURN(toreturn);
}

static node* array_init_loop_iter(char* var_name, node* dims, symbol_table_entry* var_entry, size_t iters_left, info* arg_info) {
    DBUG_ENTER("array_init_loop_iter");
    node* toreturn = NULL;

    if(iters_left == 0) {
        DBUG_PRINT("array_init_loop_iter", ("Creating varassign statement"));
        // No more for loops to create, simply add the assignment statement.
        // Get the position
        node* position = dims_to_positions(var_name, dims, arg_info);
        // The value will be the variable read from var_entry
        node* value = TBmakeVarread(STRcpy(var_entry->name));
        VARREAD_VARPOINTER(value) = var_entry;

        node* assign = TBmakeArrayvarassign(STRcpy(var_name), position, TBmakeExpr(value));
        toreturn = TBmakeStatements(assign, NULL);
    } else {
        DBUG_PRINT("array_init_loop_iter", ("Creating forloop"));
        // Create the new counter
        size_t dimcount = count_dims(dims);
        char* new_counter_name = counter_to_name(var_name, dimcount - iters_left);
        node* counterdec = TBmakeBasicvardeclaration(STRcpy(new_counter_name), TP_int, TBmakeExpr(TBmakeConstant(TBmakeIntconst(0))));

        // Create new symbol table for the forloop and add the new counter to it
        symbol_table* new_forloop_table = symbol_table_make(CURRENT_SYMBOL_TABLE(arg_info), CURRENT_SYMBOL_TABLE(arg_info)->table_depth + 1);
        symbol_table_entry* new_counter_entry = symbol_table_add(new_forloop_table, new_counter_name, TP_int);
        IS_UNUSED(new_counter_entry);

        // Get the current dimension varread
        node* current_dim = get_current_dim(dims, iters_left);
        print_exprarray(current_dim);
        printf("%s\n", get_nodetype_string(NODE_TYPE(EXPRARRAY_CURRENT(current_dim))));
        node* current_dim_varread = EXPR_CURRENT(EXPRARRAY_CURRENT(current_dim));
        node* dim_varread = TBmakeVarread(STRcpy(VARREAD_VARNAME(current_dim_varread)));
        VARREAD_VARPOINTER(dim_varread) = VARREAD_VARPOINTER(current_dim_varread);

        // Get the inner statements. We change the symbol table so that the recursive function can change create
        // their own symbol table properly.

        symbol_table* old_table = CURRENT_SYMBOL_TABLE(arg_info);
        CURRENT_SYMBOL_TABLE(arg_info) = new_forloop_table;
        node* inner_statements = array_init_loop_iter(var_name, dims, var_entry, iters_left - 1, arg_info);
        CURRENT_SYMBOL_TABLE(arg_info) = old_table;

        // Create the for loop
        node* forloop = TBmakeFor(counterdec, TBmakeExpr(dim_varread), NULL, inner_statements);

        // Create the statement
        toreturn = TBmakeStatements(forloop, NULL);
    }

    DBUG_RETURN(toreturn);
}

node* create_array_init_loops(char* var_name, node* dims, symbol_table_entry* var_entry, info* arg_info) {
    DBUG_ENTER("create_array_init_loops");
    node* toreturn = NULL;

    size_t dimcount = count_dims(dims);

    toreturn = array_init_loop_iter(var_name, dims, var_entry, dimcount, arg_info);

    DBUG_RETURN(toreturn);
}
