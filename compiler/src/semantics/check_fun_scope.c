#include "check_fun_scope.h"
#include "fun_symbol_table.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO
{
    fun_symbol_table *current_table;
    size_t paramcount;
    size_t current_scope;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_table)
#define CURRENT_SCOPE(__arg_info_struct__) ((__arg_info_struct__)->current_scope)
#define PARAMCOUNT(__arg_info_struct__) ((__arg_info_struct__)->paramcount)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *CFSmakeInfo(void) {
    DBUG_ENTER("CFSmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_SYMBOL_TABLE(result) = NULL;
    CURRENT_SCOPE(result) = 0;
    PARAMCOUNT(result) = 0;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *CFSfreeInfo(info *info) {
    DBUG_ENTER("CFSfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

static fun_symbol_table_entry* is_in_scope(fun_symbol_table* deepest_table, char* to_find, size_t scope) {
    DBUG_ENTER("is_in_scope");
    fun_symbol_table* current_table = deepest_table;

    for(size_t scopes_left = scope + 1; scopes_left > 0; scopes_left--) {
        fun_symbol_table_entry* result = fun_symbol_table_get_name(current_table, to_find);

        if(result != NULL) {
            DBUG_PRINT("is_in_scope", ("Function %s is in scope", to_find));
            DBUG_RETURN(result);
        }

        DBUG_PRINT("is_in_scope", ("Function %s not found, checking next table"));

        current_table = fun_symbol_table_move_up(current_table, 1);
    }

    DBUG_PRINT("is_in_scope", ("Function %s is NOT in scope", to_find));
    DBUG_RETURN(NULL);
}

static bool valid_parameter_count(fun_symbol_table_entry* fun_entry, node* funcall_node) {
    DBUG_ENTER("check_parameter_count");

    DBUG_ASSERT((NODE_TYPE(funcall_node) == N_funcall), "valid_parameter_count called with an invalid node type!");
    DBUG_ASSERT((fun_entry != NULL), "NULL fun_symbol_table_entry given to valid_parameter_count!");

    size_t count_expected = fun_entry->paramamount;
    size_t count_actual = 0;

    node* param = FUNCALL_PARAMS(funcall_node);

    while(param != NULL) {
        count_actual++;
        param = FUNCALLPARAMETER_NEXT(param);
    }

    DBUG_RETURN(count_actual == count_expected);
}

node *CFSfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("CFSfuctiiondefinition");

    DBUG_RETURN(NULL);
}

node *CFSfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("CFSfuncall");

    DBUG_RETURN(NULL);
}

node *CFSdo(node *syntaxtree) {
    DBUG_ENTER("CFSdo");

    // TRAVpush(TR_cfs);
    // info *arg_info = STBmakeInfo();

    // symbol_table *global_var_table = symbol_table_make(NULL);
    // PROGRAM_VARSYMBOLTABLE(syntaxtree) = global_var_table;
    // CURRENT_SYMBOL_TABLE(arg_info) = global_var_table;

    // syntaxtree = TRAVdo(syntaxtree, arg_info);

    // STBfreeInfo(arg_info);
    // TRAVpop();

    DBUG_RETURN(syntaxtree);
}
