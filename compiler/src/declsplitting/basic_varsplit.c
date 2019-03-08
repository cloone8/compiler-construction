#include "basic_varsplit.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free_node.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

#include "symbol_table.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
    node* initbody;
    node* currentfunbody;
    symbol_table* currentsymtable;
    node* newassignblock;
};

/*
 * INFO macros
 */

#define INITBODY(__arg_info_struct__) ((__arg_info_struct__)->initbody)
#define FUNBODY(__arg_info_struct__) ((__arg_info_struct__)->currentfunbody)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentsymtable)
#define ASSIGNS(__arg_info_struct__) ((__arg_info_struct__)->newassignblock)
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
    result->currentfunbody = NULL;
    result->currentsymtable = NULL;
    result->newassignblock = NULL;

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
            append_statement_to_tail(current_final_statement, new_statement);
        }
    }

    DBUG_RETURN(arg_node);
}

node* BSPbasicvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPbasicvardeclaration");
    DBUG_ASSERT((FUNBODY(arg_info) != NULL), "FUNBODY is NULL, something went really wrong");

    if(BASICVARDECLARATION_INITVAL(arg_node) != NULL) {
        node* initval = BASICVARDECLARATION_INITVAL(arg_node);
        symbol_table_entry* varentry = symbol_table_get_name(CURRENT_SYMBOL_TABLE(arg_info),
                                                             BASICPARAMETER_NAME(arg_node));

        DBUG_ASSERT((varentry != NULL), "Couldn't find symbol table entry");

        BASICVARDECLARATION_INITVAL(arg_node) = NULL;

        node* new_varassign = TBmakeVarassign(varentry->name, initval);
        VARASSIGN_VARPOINTER(new_varassign) = varentry;
        node* new_statement = TBmakeStatements(new_varassign, NULL);

        if(ASSIGNS(arg_info) == NULL) {
            ASSIGNS(arg_info) = new_statement;
        } else {
            append_statement_to_tail(ASSIGNS(arg_info), new_statement);
        }
    }

    DBUG_RETURN(arg_node);
}

node* BSPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPfunctiondefinition");

    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    if(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) != NULL) {
        FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* BSPfunctionbody(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPfunctionbody");

    FUNBODY(arg_info) = arg_node;
    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    // Construct the new assignment block
    if(FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL) {
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);
    }

    node* current_head = FUNCTIONBODY_STATEMENTS(arg_node);

    // Append the new assignments to the start of the statements tree
    node* new_head = ASSIGNS(arg_info);

    node* new_assigns_tail = new_head;

    append_statement_to_tail(new_assigns_tail, current_head);

    ASSIGNS(arg_info) = NULL;

    node* new_body = TBmakeFunctionbody(FUNCTIONBODY_VARDECLARATIONS(arg_node),
                                        FUNCTIONBODY_LOCALFUNCTIONS(arg_node),
                                        new_head);

    if(FUNCTIONBODY_LOCALFUNCTIONS(arg_node) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(arg_node), arg_info);
    }

    FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = NULL;
    FUNCTIONBODY_STATEMENTS(arg_node) = NULL;
    FUNCTIONBODY_VARDECLARATIONS(arg_node) = NULL;

    FREEfunctionbody(arg_node, NULL);

    DBUG_RETURN(new_body);
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
