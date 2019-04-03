#include "fortowhile.h"
#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "free_node.h"
#include "traverse.h"
#include "symbol_table.h"
#include "str.h"
#include "ctinfo.h"
#include "copy.h"
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
static info *FTWmakeInfo(void) {
    DBUG_ENTER("FTWmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURR_SYMTABLE(result) = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *FTWfreeInfo(info *info) {
    DBUG_ENTER("FTWfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* FTWfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("FTWfunctiondefinition");

    symbol_table* old_table = CURR_SYMTABLE(arg_info);
    CURR_SYMTABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURR_SYMTABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* FTWfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("FTWfor");
    // The first entry is always the counter entry
    symbol_table_entry* counter_entry = symbol_table_get_idx(FOR_VARSYMBOLTABLE(arg_node), 0);

    symbol_table* table = CURR_SYMTABLE(arg_info);

    // Migrate all the entries
    for(size_t i = 1; i < FOR_VARSYMBOLTABLE(arg_node)->current_entries; i++) {
        symbol_table_entry* entry = symbol_table_get_idx(FOR_VARSYMBOLTABLE(arg_node), i);

        symbol_table_insert(table, entry);
    }

    symbol_table_free_no_entries(FOR_VARSYMBOLTABLE(arg_node));
    FOR_VARSYMBOLTABLE(arg_node) = NULL;

    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);

    // Now transform the for-loop into a while loop
    node* toreturn = NULL;

    node* counter_varread = TBmakeVarread(STRcpy(counter_entry->name));
    VARREAD_VARPOINTER(counter_varread) = counter_entry;

    node* count_up_condition = TBmakeExpr(TBmakeBinop(BO_lt, TBmakeExpr(counter_varread), FOR_CONDITION(arg_node)));
    node* count_down_condition = TBmakeExpr(TBmakeBinop(BO_gt, TBmakeExpr(COPYdoCopy(counter_varread)), COPYdoCopy(FOR_CONDITION(arg_node))));
    node* condition_picker = TBmakeExpr(TBmakeBinop(BO_gt, FOR_STEP(arg_node), TBmakeExpr(TBmakeConstant(TBmakeIntconst(0)))));
    node* condition = TBmakeConditionalexpr(condition_picker, count_up_condition, count_down_condition);

    toreturn = TBmakeWhile(TBmakeExpr(condition), FOR_CODE(arg_node));

    FOR_CONDITION(arg_node) = NULL;
    FOR_CODE(arg_node) = NULL;
    FOR_STEP(arg_node) = NULL;
    FREEfor(arg_node, NULL);

    DBUG_RETURN(toreturn);
}

node* FTWdo(node* syntaxtree) {
    DBUG_ENTER("FTWdo");

    TRAVpush(TR_ftw);
    info *arg_info = FTWmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    FTWfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
