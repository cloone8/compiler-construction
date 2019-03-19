#include "forsymtablemigrate.h"
#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "traverse.h"
#include "symbol_table.h"
#include "str.h"
#include "ctinfo.h"
#include "memory.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
    symbol_table* curr_symtable;
};

/*
 * INFO macros
 */
#define CURR_SYMTABLE(__arg_info_struct__) ((__arg_info_struct__)->curr_symtable)
/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *FSMmakeInfo(void) {
    DBUG_ENTER("FSMmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURR_SYMTABLE(result) = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *FSMfreeInfo(info *info) {
    DBUG_ENTER("FSMfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* FSMfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSMfunctiondefinition");

    symbol_table* old_table = CURR_SYMTABLE(arg_info);
    CURR_SYMTABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURR_SYMTABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* FSMfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSMfor");

    // As the forloop symbol table has only a single entry for the counter, get that
    // entry.
    symbol_table_entry* counter_entry = symbol_table_get_idx(FOR_VARSYMBOLTABLE(arg_node), 0);

    symbol_table* table = CURR_SYMTABLE(arg_info);
    symbol_table_insert(table, counter_entry);

    symbol_table_free_no_entries(FOR_VARSYMBOLTABLE(arg_node));
    FOR_VARSYMBOLTABLE(arg_node) = NULL;

    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node* FSMdo(node* syntaxtree) {
    DBUG_ENTER("FSMdo");

    TRAVpush(TR_fsm);
    info *arg_info = FSMmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    FSMfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
