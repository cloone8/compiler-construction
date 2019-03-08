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

node *CFSfunctiondefinition(node *arg_node, info *arg_info) {

}

node *CFSfuncall(node *arg_node, info *arg_info) {

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
