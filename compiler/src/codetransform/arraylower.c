#include "arraylower.h"

#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "symbol_table.h"
#include "tree_basic.h"
#include "free_node.h"
#include "traverse.h"
#include "str.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
};

/*
 * INFO macros
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *LADmakeInfo(void) {
    DBUG_ENTER("LADmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *LADfreeInfo(info *info) {
    DBUG_ENTER("LADfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

static size_t pos_count(node* positions) {
    DBUG_ENTER("pos_count");
    DBUG_ASSERT((positions != NULL), "Positions was NULL!");
    DBUG_ASSERT((NODE_TYPE(positions) == N_exprarray), "Positions was not an Exprarray!");

    size_t count = 0;
    node* current_pos = positions;

    while(current_pos != NULL) {
        count++;
        current_pos = EXPRARRAY_NEXT(current_pos);
    }

    DBUG_RETURN(count);
}

static node* get_nth_position(size_t n, node* positions) {
    DBUG_ENTER("get_nth_position");
    DBUG_ASSERT((positions != NULL), "Positions was NULL!");
    DBUG_ASSERT((NODE_TYPE(positions) == N_exprarray), "Positions was not an Exprarray!");

    node* toreturn = positions;

    for(size_t i = 0; i < n; i++) {
        toreturn = EXPRARRAY_NEXT(toreturn);
        DBUG_ASSERTF((toreturn != NULL), ("n higher than dimensioncount! Only found %zu dimensions but asked for %zu", i - 1, n));
    }

    DBUG_RETURN(toreturn);
}

static size_t get_array_dims(symbol_table_entry* array_entry, symbol_table_entry** dim_entries, size_t expected) {
    DBUG_ENTER("get_array_dims");
    size_t count = 0;

    symbol_table* array_table = array_entry->table;
    print_symbol_table(array_table);
    while(TRUE) {
        get_size_t_str(name_buffer, count);
        char* dim_var_name = STRcatn(4, "tmp-", name_buffer, "-", array_entry->name);

        symbol_table_entry* dim_var = symbol_table_get_name(array_table, dim_var_name);
        if(dim_var == NULL) {
            break;
        }

        if(count < expected) {
            dim_entries[count] = dim_var;
        }
        count++;
    }

    DBUG_RETURN(count);
}

static node* get_nd_to_1d_exprs_rec(char* var_name, symbol_table_entry** dim_entries, node* positions, size_t dimcount, size_t current_dim) {
    DBUG_ENTER("get_nd_to_1d_exprs_rec");
    DBUG_ASSERT((positions != NULL), "Positions was NULL!");
    DBUG_ASSERT((dim_entries != NULL), "dim_entries was NULL!");
    DBUG_ASSERT((NODE_TYPE(positions) == N_exprarray), "Positions was not an Exprarray!");
    DBUG_ASSERT((dimcount != 0), "Zero dimensions not possible!");

    DBUG_PRINT("flatten", ("Handling dimension %zu", current_dim));
    size_t current_index = (dimcount - 1) - current_dim;
    DBUG_PRINT("flatten", ("Current index is %zu", current_index));

    if(current_index == 0) {
        node* current_pos = get_nth_position(current_index, positions);
        // Sanity checks
        DBUG_ASSERT((EXPRARRAY_CURRENT(current_pos) != NULL), "EXPRARRAY_CURRENT was null!");

        node* toreturn = EXPRARRAY_CURRENT(current_pos);
        EXPRARRAY_CURRENT(current_pos) = NULL;

        DBUG_ASSERT((NODE_TYPE(toreturn) == N_expr), "Expected toreturn to be an expr type node!");
        DBUG_RETURN(toreturn);
    } else {
        // Get the current position expression.
        node* current_pos_exprarray = get_nth_position(current_index, positions);
        node* current_pos = EXPRARRAY_CURRENT(current_pos_exprarray);
        // Sanity checks
        DBUG_ASSERT((current_pos != NULL), "Current position was NULL!");
        DBUG_ASSERT((NODE_TYPE(current_pos) == N_expr), "Current position was not an N_expr type node!");
        DBUG_ASSERT((dim_entries[current_index] != NULL), "Couldn't get dimension var entry!");

        EXPRARRAY_CURRENT(current_pos_exprarray) = NULL;

        // Get the inner expression
        node* inner_dims = get_nd_to_1d_exprs_rec(var_name, dim_entries, positions, dimcount, current_dim + 1);
        node* current_dim_read = TBmakeVarread(STRcpy(dim_entries[current_index]->name));
        VARREAD_VARPOINTER(current_dim_read) = dim_entries[current_index];
        node* dim_mult = TBmakeExpr(TBmakeBinop(BO_mul, TBmakeExpr(current_dim_read), inner_dims));

        node* final_binop = TBmakeExpr(TBmakeBinop(BO_add, dim_mult, current_pos));
        DBUG_RETURN(final_binop);
    }
}

static node* get_nd_to_1d_exprs(char* var_name, symbol_table_entry** dim_entries, node* positions, size_t dimcount) {
    DBUG_ENTER("get_nd_to_1d_exprs");
    DBUG_ASSERT((positions != NULL), "Positions was NULL!");
    DBUG_ASSERT((dim_entries != NULL), "dim_entries was NULL!");
    DBUG_ASSERT((var_name != NULL), "var_name was NULL!");
    DBUG_ASSERT((NODE_TYPE(positions) == N_exprarray), "Positions was not an Exprarray!");
    DBUG_ASSERT((dimcount != 0), "Zero dimensions not possible!");

    node* toreturn = get_nd_to_1d_exprs_rec(var_name, dim_entries, positions, dimcount, 0);
    if(toreturn == NULL) {
        // On error
        DBUG_RETURN(NULL);
    }

    DBUG_ASSERT((NODE_TYPE(toreturn) == N_expr), "Did not get expression back from recursive function!");

    FREEexprarray(positions, NULL);

    DBUG_RETURN(toreturn);
}

static node* flatten_position_exprarray(symbol_table_entry* array_entry, node* positions) {
    DBUG_ENTER("flatten_position_exprarray");
    DBUG_ASSERT((positions != NULL), "Positions was NULL!");
    DBUG_ASSERT((array_entry != NULL), "array_entry was NULL!");

    DBUG_PRINT("flatten", ("Flattening %s", array_entry->name));

    size_t given_position_count = pos_count(positions);
    print_exprarray(positions);
    node* toreturn = NULL;

    symbol_table_entry* dim_entries[given_position_count];

    // Fill the dimension entries table
    size_t expected_position_count = get_array_dims(array_entry, dim_entries, given_position_count);

    if(expected_position_count != given_position_count) {
        CTIerrorFile(NODE_FILE(positions), NODE_LINE(positions),
                     "Invalid number of array position arguments for array \"%s\"! Expected %zu but got %zu",
                      array_entry->name, expected_position_count, given_position_count);
        DBUG_RETURN(NULL);
    }

    toreturn = get_nd_to_1d_exprs(array_entry->name, dim_entries, positions, expected_position_count);

    DBUG_RETURN(toreturn);
}

node* LADarrayvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("LADarrayvarread");
    symbol_table_entry* array_entry = ARRAYVARREAD_VARPOINTER(arg_node);
    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    node* read_position = ARRAYVARREAD_POSITION(arg_node);
    ARRAYVARREAD_POSITION(arg_node) = NULL;

    node* flatten_result = flatten_position_exprarray(array_entry, read_position);
    node* flattened_position = TBmakeExprarray(NULL, flatten_result);
    ARRAYVARREAD_POSITION(arg_node) = flattened_position;

    DBUG_RETURN(arg_node);
}

node* LADarrayvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("LADarrayvarassign");

    symbol_table_entry* array_entry = ARRAYVARASSIGN_VARPOINTER(arg_node);
    ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);
    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    node* assign_position = ARRAYVARASSIGN_POSITION(arg_node);
    ARRAYVARASSIGN_POSITION(arg_node) = NULL;

    node* flatten_result = flatten_position_exprarray(array_entry, assign_position);
    node* flattened_position = TBmakeExprarray(NULL, flatten_result);
    ARRAYVARASSIGN_POSITION(arg_node) = flattened_position;

    DBUG_RETURN(arg_node);
}

node* LADdo(node* syntaxtree) {
    DBUG_ENTER("LADdo");

    info* arg_info = LADmakeInfo();

    TRAVpush(TR_lad);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = LADfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
