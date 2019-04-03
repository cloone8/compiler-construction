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
    size_t entry_count;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_table)
#define NEW_VARDECS(__arg_info_struct__) ((__arg_info_struct__)->new_vardecs)
#define COUNT(__arg_info_struct__) ((__arg_info_struct__)->entry_count)

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
    COUNT(result) = 0;

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

    node *condition = FOR_CONDITION(arg_node);

    node* vardecl = TBmakeBasicvardeclaration("FOR_COND", TP_bool, condition);

    if(NEW_VARDECS(arg_info) == NULL){
        NEW_VARDECS(arg_info) = TBmakeFunctionvardeclaration(vardecl, NULL);
    } else {
        append_functionvardeclaration_to_tail(NEW_VARDECS(arg_info), TBmakeFunctionvardeclaration(vardecl, NULL));
    }

    symbol_table_add(CURRENT_SYMBOL_TABLE(arg_info), BASICVARDECLARATION_VARNAME(vardecl), BASICVARDECLARATION_TYPE(vardecl), FALSE, FALSE);
    symbol_table_entry* new = symbol_table_get_name(CURRENT_SYMBOL_TABLE(arg_info), "FOR_COND");

    get_size_t_str(table_id_buffer2, new->idx);
    char *new_name_cond = STRcatn(4, "for-", table_id_buffer2, "-condition-", new->name);

    symbol_table_update(CURRENT_SYMBOL_TABLE(arg_info), new->idx, new_name_cond);

    BASICVARDECLARATION_VARPOINTER(vardecl) = new;
    node *new_varread = TBmakeVarread(new->name);
    VARREAD_VARPOINTER(new_varread) = new;
    FOR_CONDITION(arg_node) = TBmakeExpr(new_varread);
    MEMfree(new_name_cond);


    DBUG_ASSERT((FOR_COUNTER(arg_node) != NULL), "FOR_COUNTER was NULL, while it shouldn't yet be!");
    // Lift the counter out of the loop
    node* counter_dec = FOR_COUNTER(arg_node);
    FOR_COUNTER(arg_node) = NULL;
    symbol_table_entry* entry = BASICVARDECLARATION_VARPOINTER(counter_dec);

    // Change the name for clarity
    get_size_t_str(table_id_buffer, entry->table->id);

    char* new_name = STRcatn(4, "for-", table_id_buffer, "-counter-", entry->name);
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

    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);
    append_statement_to_tail(FOR_CODE(arg_node), counter_increment);

    // Return the new assignment, so the FSPstatement traversal function can
    // put it in the right place

    DBUG_RETURN(init_assign);
}

node* FSPstatements(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPstatements");

    if(NODE_TYPE(STATEMENTS_CURRENT(arg_node)) == N_for) {
        // The for traversal returns the new for loop initialiser
        node* for_node = STATEMENTS_CURRENT(arg_node);
        node* for_init = TRAVdo(for_node, arg_info);

        // Insert the initial statement here
        node* for_statement = TBmakeStatements(for_node, STATEMENTS_NEXT(arg_node));
        STATEMENTS_CURRENT(arg_node) = for_init;
        STATEMENTS_NEXT(arg_node) = for_statement;


        // Continue traversing at the original next node
        if(STATEMENTS_NEXT(for_statement) != NULL) {
            STATEMENTS_NEXT(for_statement) = TRAVdo(STATEMENTS_NEXT(for_statement), arg_info);
        }
    } else {
        STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

        if(STATEMENTS_NEXT(arg_node) != NULL) {
            STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), arg_info);
        }
    }

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
