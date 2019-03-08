#include "basic_varsplit.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

#include "symbol_table.h"

/*
 * INFO structure
 */
struct INFO {
    node* initbody;
};

/*
 * INFO macros
 */

#define INITBODY(__arg_info_struct__) ((__arg_info_struct__)->initbody)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *BSPmakeInfo(void) {
    DBUG_ENTER("BSPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    result->initbody = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *BSPfreeInfo(info *info) {
    DBUG_ENTER("BSPfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* BSPprogram(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPprogram");

    node* init_body = TBmakeFunctionbody(NULL, NULL, NULL);
    node* init_definition = TBmakeFunctiondefinition(FALSE, init_body);
    node* init_function = TBmakeFunction("__init", TP_void, NULL, init_definition);

    node* init_declaration = TBmakeDeclaration(init_function, PROGRAM_FIRST(arg_node));

    PROGRAM_FIRST(arg_node) = init_declaration;

    INITBODY(arg_info) = init_body;

    DBUG_RETURN(arg_node);
}

node* BSPglobal(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPglobal");

    // Only split if the global is both a definition AND has an initialiser value
    if(GLOBAL_DEFINITION(arg_node) != NULL && GLOBALDEFINITION_VALUES(GLOBAL_DEFINITION(arg_node)) != NULL) {
        node* def = GLOBAL_DEFINITION(arg_node);
        symbol_table_entry* varentry = GLOBALDEFINITION_VARPOINTER(def);
        node* initval = GLOBALDEFINITION_VALUES(def);
        GLOBALDEFINITION_VALUES(def) = NULL;

        // Create a new variable assignment statement
        node* new_assign = TBmakeVarassign(varentry->name, initval);
        VARASSIGN_VARPOINTER(new_assign) = varentry;
        node* new_statement = TBmakeStatements(new_assign, NULL);


        node* current_final_statement = FUNCTIONBODY_STATEMENTS(INITBODY(arg_info));

        // If the initialiser function currently has no statements, make this the statement.
        if(current_final_statement == NULL) {
            current_final_statement = new_statement;
        } else {
            // Go to the end of the current statements
            while(STATEMENTS_NEXT(current_final_statement) != NULL) {
                current_final_statement = STATEMENTS_NEXT(current_final_statement);
            }
            // End found, link the new node to the end
            STATEMENTS_NEXT(current_final_statement) = new_statement;
        }
    }

    DBUG_RETURN(arg_node);
}

node* BSPbasicvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPbasicvardeclaration");

    DBUG_RETURN(arg_node);
}

node* BSPfunctionbody(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPfunctionbody");

    DBUG_RETURN(arg_node);
}

node *BSPdo(node *syntaxtree) {
    DBUG_ENTER("BSPdo");

    info* arg_info = BSPmakeInfo();

    TRAVpush(TR_bsp);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = BSPfreeInfo(arg_info);
    DBUG_RETURN(syntaxtree);
}
