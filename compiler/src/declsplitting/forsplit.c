#include "forsplit.h"
#include "symbol_table.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "copy.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
    symbol_table* current_table;
    node* new_vardecs;
    node* for_init_statements;
    size_t entry_count;
    bool containsfuncall;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_table)
#define NEW_VARDECS(__arg_info_struct__) ((__arg_info_struct__)->new_vardecs)
#define INIT_STATEMENTS(__arg_info_struct__) ((__arg_info_struct__)->for_init_statements)
#define COUNT(__arg_info_struct__) ((__arg_info_struct__)->entry_count)
#define HASFUNCALL(__arg_info_struct__) ((__arg_info_struct__)->containsfuncall)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *FSPmakeInfo(void) {
    DBUG_ENTER("FSPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_SYMBOL_TABLE(result) = NULL;
    NEW_VARDECS(result) = NULL;
    INIT_STATEMENTS(result) = NULL;
    COUNT(result) = 0;
    HASFUNCALL(result) = FALSE;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *FSPfreeInfo(info *info) {
    DBUG_ENTER("FSPfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* FSPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPfunctiondefinition");

    symbol_table* old_table = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURRENT_SYMBOL_TABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* FSPfunctionbody(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPfunctionbody");

    node* old_decls = NEW_VARDECS(arg_info);
    NEW_VARDECS(arg_info) = NULL;

    if(FUNCTIONBODY_LOCALFUNCTIONS(arg_node) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
        FUNCTIONBODY_STATEMENTS(arg_node) = TRAVdo(FUNCTIONBODY_STATEMENTS(arg_node), arg_info);

        // For loops may have resulted in new variable declarations, append them
        // to the current ones
        if(NEW_VARDECS(arg_info) != NULL) {
            if(FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL) {
                append_functionvardeclaration_to_tail(FUNCTIONBODY_VARDECLARATIONS(arg_node), NEW_VARDECS(arg_info));
            } else {
                FUNCTIONBODY_VARDECLARATIONS(arg_node) = NEW_VARDECS(arg_info);
            }
        }
    }

    NEW_VARDECS(arg_info) = old_decls;

    DBUG_RETURN(arg_node);
}

node* FSPfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPfor");

    HASFUNCALL(arg_info) = FALSE;

    node *condition = FOR_CONDITION(arg_node);

    node* vardecl = TBmakeBasicvardeclaration(STRcpy("FOR_COND"), TP_int, NULL);

    if(NEW_VARDECS(arg_info) == NULL){
        NEW_VARDECS(arg_info) = TBmakeFunctionvardeclaration(vardecl, NULL);
    } else {
        append_functionvardeclaration_to_tail(NEW_VARDECS(arg_info), TBmakeFunctionvardeclaration(vardecl, NULL));
    }

    symbol_table_entry* cond_entry = symbol_table_add(FOR_VARSYMBOLTABLE(arg_node), BASICVARDECLARATION_VARNAME(vardecl), TP_int, FALSE, FALSE);

    // Change the name for clarity
    get_size_t_str(table_id_buffer, cond_entry->table->id);

    char *new_name_cond = STRcatn(3, "for-", table_id_buffer, "-condition");

    symbol_table_update(FOR_VARSYMBOLTABLE(arg_node), cond_entry->idx, new_name_cond);

    BASICVARDECLARATION_VARPOINTER(vardecl) = cond_entry;
    node *cond_varread = TBmakeVarread(STRcpy(cond_entry->name));
    VARREAD_VARPOINTER(cond_varread) = cond_entry;
    FOR_CONDITION(arg_node) = TBmakeExpr(cond_varread);

    node* init_stat = TBmakeVarassign(STRcpy(new_name_cond), condition);
    VARASSIGN_VARPOINTER(init_stat) = cond_entry;

    MEMfree(new_name_cond);

    if(INIT_STATEMENTS(arg_info) == NULL) {
        INIT_STATEMENTS(arg_info) = TBmakeStatements(init_stat, NULL);
    } else {
        append_statement_to_tail(INIT_STATEMENTS(arg_info), TBmakeStatements(init_stat, NULL));
    }

    DBUG_ASSERT((FOR_COUNTER(arg_node) != NULL), "FOR_COUNTER was NULL, while it shouldn't yet be!");
    // Lift the counter out of the loop
    node* counter_dec = FOR_COUNTER(arg_node);
    FOR_COUNTER(arg_node) = NULL;
    symbol_table_entry* entry = BASICVARDECLARATION_VARPOINTER(counter_dec);

    char* new_name = STRcatn(3, "for-", table_id_buffer, "-counter");
    symbol_table_update(FOR_VARSYMBOLTABLE(arg_node), entry->idx, new_name);


    // Lift the initial value out of the declaration
    node* initval = BASICVARDECLARATION_INITVAL(counter_dec);
    BASICVARDECLARATION_INITVAL(counter_dec) = NULL;

    // Add the declaration of the counter to the innermost function declarations
    if(NEW_VARDECS(arg_info) == NULL) {
        NEW_VARDECS(arg_info) = TBmakeFunctionvardeclaration(counter_dec, NULL);
    } else {
        append_functionvardeclaration_to_tail(NEW_VARDECS(arg_info), TBmakeFunctionvardeclaration(counter_dec, NULL));
    }

    // Create a new var assignment, assigning the init value to the counter
    node* init_assign = TBmakeVarassign(STRcpy(new_name), initval);
    VARASSIGN_VARPOINTER(init_assign) = entry;

    node* counter_init_statement = TBmakeStatements(init_assign, NULL);

    if(INIT_STATEMENTS(arg_info) == NULL) {
        INIT_STATEMENTS(arg_info) = counter_init_statement;
    } else {
        append_statement_to_tail(INIT_STATEMENTS(arg_info), counter_init_statement);
    }

    // Create a new var assignment, which increments the counter with the given stepsize.
    node* counter_inc_varread = TBmakeVarread(STRcpy(new_name));
    VARREAD_VARPOINTER(counter_inc_varread) = entry;

    // Read the step value and create a varassign node which increments the
    // For loop counter with the given step size.
    node* counter_inc_step;
    if(FOR_STEP(arg_node) != NULL) {
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
        counter_inc_step = COPYdoCopy(FOR_STEP(arg_node));
        // FOR_STEP(arg_node) = NULL;
    } else {
        counter_inc_step = TBmakeExpr(TBmakeConstant(TBmakeIntconst(1)));
        FOR_STEP(arg_node) = COPYdoCopy(counter_inc_step);
    }

    node* incr_binop = TBmakeBinop(BO_add, TBmakeExpr(counter_inc_varread), counter_inc_step);

    node* counter_inc_varassign = TBmakeVarassign(STRcpy(new_name), TBmakeExpr(incr_binop));
    VARASSIGN_VARPOINTER(counter_inc_varassign) = entry;

    node* counter_increment = TBmakeStatements(counter_inc_varassign, NULL);

    MEMfree(new_name);

    // Traverse through the body of the loop, and append any inner-loop init statements
    // to the body of this loop
    node* new_statements = INIT_STATEMENTS(arg_info);

    INIT_STATEMENTS(arg_info) = NULL;
    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);

    if(INIT_STATEMENTS(arg_info) != NULL) {
        append_statement_to_tail(INIT_STATEMENTS(arg_info), FOR_CODE(arg_node));
        FOR_CODE(arg_node) = INIT_STATEMENTS(arg_info);
        INIT_STATEMENTS(arg_info) = NULL;
    }

    append_statement_to_tail(FOR_CODE(arg_node), counter_increment);

    INIT_STATEMENTS(arg_info) = new_statements;

    // Return the new assignment, so the FSPstatement traversal function can
    // put it in the right place
    DBUG_RETURN(arg_node);
}

node* FSPstatements(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPstatements");

    if(NODE_TYPE(STATEMENTS_CURRENT(arg_node)) == N_for) {
        INIT_STATEMENTS(arg_info) = NULL;
        STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

        if(INIT_STATEMENTS(arg_info) != NULL) {
            append_statement_to_tail(INIT_STATEMENTS(arg_info), arg_node);
        }
        node* toreturn = INIT_STATEMENTS(arg_info);
        INIT_STATEMENTS(arg_info) = NULL;

        if(STATEMENTS_NEXT(arg_node) != NULL) {
            STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), arg_info);
        }

        DBUG_RETURN(toreturn);
    } else {
        STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

        if(STATEMENTS_NEXT(arg_node) != NULL) {
            STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), arg_info);
        }
    }

    DBUG_RETURN(arg_node);
}

node* FSPfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPfuncall");

    HASFUNCALL(arg_info) = TRUE;

    DBUG_RETURN(arg_node);
}

node *FSPdo(node* syntaxtree) {
    DBUG_ENTER("FSPdo");

    info* arg_info = FSPmakeInfo();

    TRAVpush(TR_fsp);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FSPfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
