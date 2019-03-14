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

static void check_valid_parameter_count(fun_symbol_table_entry* fun_entry, node* funcall_node, info* arg_info) {
    DBUG_ENTER("check_parameter_count");

    DBUG_ASSERT((NODE_TYPE(funcall_node) == N_funcall), "valid_parameter_count called with an invalid node type!");
    DBUG_ASSERT((fun_entry != NULL), "NULL fun_symbol_table_entry given to valid_parameter_count!");

    size_t count_expected = fun_entry->paramamount;
    size_t count_actual = 0;

    node* param = FUNCALL_PARAMS(funcall_node);

    while(param != NULL) {
        count_actual++;
        // Scope any funcalls here as well
        TRAVdo(param, arg_info);
        param = FUNCALLPARAMETER_NEXT(param);
    }

    if(count_actual != count_expected) {
        CTIerror("Invalid parameter count in function call for %s: Expected %zu but got %zu",
                  FUNCALL_FUNCTIONNAME(funcall_node), count_expected, count_actual);
    }

    DBUG_VOID_RETURN;
}

node *CFSfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("CFSfuctiiondefinition");

    fun_symbol_table* old = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node);
    CURRENT_SCOPE(arg_info)++;

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURRENT_SCOPE(arg_info)--;
    CURRENT_SYMBOL_TABLE(arg_info) = old;

    DBUG_RETURN(arg_node);
}

node *CFSfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("CFSfuncall");

    char* fun_name = FUNCALL_FUNCTIONNAME(arg_node);

    fun_symbol_table_entry* entry = fun_is_in_scope(CURRENT_SYMBOL_TABLE(arg_info), fun_name);

    if (entry == NULL) {
        CTIerror("Fun %s not in scope", fun_name);
    } else {
        FUNCALL_FUNPOINTER(arg_node) = entry;
    }

    check_valid_parameter_count(entry, arg_node, arg_info);

    DBUG_RETURN(arg_node);
}

node *CFSdo(node *syntaxtree) {
    DBUG_ENTER("CFSdo");

    TRAVpush(TR_cfs);
    info *arg_info = CFSmakeInfo();

    fun_symbol_table *global_fun_table = PROGRAM_FUNSYMBOLTABLE(syntaxtree);
    CURRENT_SYMBOL_TABLE(arg_info) = global_fun_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    CFSfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
