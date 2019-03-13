#include "typecheck.h"
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
struct INFO {
    fun_symbol_table *current_fun_symbol_table;
    symbol_table *current_symbol_table;
    type final_type;
};


/*
 * INFO macros
 */

#define CURRENT_FUN_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_fun_symbol_table)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_symbol_table)
#define TYPE(__arg_info_struct__) ((__arg_info_struct__)->final_type)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *TCmakeInfo(void) {
    DBUG_ENTER("TCmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_FUN_SYMBOL_TABLE(result) = NULL;
    CURRENT_SYMBOL_TABLE(result) = NULL;
    

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *TCfreeInfo(info *info) {
    DBUG_ENTER("TCfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* TCfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfunctiondefinition");

    fun_symbol_table *f_old = CURRENT_FUN_SYMBOL_TABLE(arg_info);
    symbol_table *s_old = CURRENT_SYMBOL_TABLE(arg_info);

    CURRENT_FUN_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node);
    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    if (FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) != NULL) {
        FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);
    }

    CURRENT_FUN_SYMBOL_TABLE(arg_info) = f_old;
    CURRENT_SYMBOL_TABLE(arg_info) = s_old;

    DBUG_RETURN(arg_node);
}

node* TCvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCvarassign");

    fun_symbol_table* f_table = CURRENT_FUN_SYMBOL_TABLE(arg_info);
    symbol_table* s_table = CURRENT_SYMBOL_TABLE(arg_info);

    symbol_table_entry* entry = symbol_table_get_name(s_table, VARASSIGN_VARNAME(arg_node));
    TYPE(arg_info) = entry->vartype;

    info* val_info = TCmakeInfo();
    CURRENT_FUN_SYMBOL_TABLE(val_info) = f_table;
    CURRENT_SYMBOL_TABLE(val_info) = s_table;

    if (VARASSIGN_VALUE(arg_node) != NULL) {
        TRAVdo(VARASSIGN_VALUE(arg_node), val_info);
    }

    if (TYPE(val_info) != TYPE(arg_info)) {
        CTIerror("Wrong type for %s, expected %s but got %s", entry->name, print_type(TYPE(arg_info)), print_type(TYPE(val_info)));
    }

    DBUG_RETURN(arg_node);
}

node* TCfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfuncall");

    fun_symbol_table *f_table = CURRENT_FUN_SYMBOL_TABLE(arg_info);
    symbol_table *s_table = CURRENT_SYMBOL_TABLE(arg_info);

    fun_symbol_table_entry* entry = fun_symbol_table_get_name(f_table, FUNCALL_FUNCTIONNAME(arg_node));
    TYPE(arg_info) = entry->returntype;

    DBUG_RETURN(arg_node);
}

node *TCbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCBINOP");

    info* left_info = TCmakeInfo();
    info *right_info = TCmakeInfo();

    TRAVdo(BINOP_LEFT(arg_node), left_info);
    TRAVdo(BINOP_RIGHT(arg_node), right_info);

    if (TYPE(left_info) == TYPE(right_info)) {
        TYPE(arg_info) = TYPE(left_info);
    } else {
        CTIerror("Wrong types for binop");
    }

    TCfreeInfo(left_info);
    TCfreeInfo(right_info);

    DBUG_RETURN(arg_node);    
}

node* TCintconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCintconst");

    TYPE(arg_info) = TP_int;

    DBUG_RETURN(arg_node);
}

node *TCfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfloatconst");

    TYPE(arg_info) = TP_float;

    DBUG_RETURN(arg_node);
}

node *TCboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCboolconst");

    TYPE(arg_info) = TP_bool;

    DBUG_RETURN(arg_node);
}

node *TCdo(node *syntaxtree) {
    DBUG_ENTER("TCdo");

    TRAVpush(TR_tc);
    info *arg_info = TCmakeInfo();


    symbol_table* global_symbol_table = PROGRAM_VARSYMBOLTABLE(syntaxtree);
    CURRENT_SYMBOL_TABLE(arg_info) = global_symbol_table;

    fun_symbol_table *global_fun_table = PROGRAM_FUNSYMBOLTABLE(syntaxtree);
    CURRENT_FUN_SYMBOL_TABLE(arg_info) = global_fun_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}