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

    node* old_first = PROGRAM_FIRST(arg_node);
    fun_symbol_table_entry* init_fun = fun_is_in_scope(PROGRAM_FUNSYMBOLTABLE(arg_node), "__init");

    DBUG_ASSERT((init_fun != NULL), "Could not find __init() function!");

    // Go through all declarations, and find the __init() declaration.
    // We do this to find its function body.

    node* current_decl = old_first;
    while(current_decl != NULL) {
        if(NODE_TYPE(DECLARATION_CURRENT(current_decl)) == N_function) {
            node* function_n = DECLARATION_CURRENT(current_decl);
            if(STReq("__init", FUNCTION_NAME(function_n))) {
                INITBODY(arg_info) = FUNCTIONDEFINITION_FUNCTIONBODY(FUNCTION_DEFINITION(function_n));
                break;
            }
        }

        current_decl = DECLARATION_NEXT(current_decl);
    }

    DBUG_ASSERT((INITBODY(arg_info) != NULL), "__init() function body was not found!");

    if (PROGRAM_FIRST(arg_node) != NULL) {
        PROGRAM_FIRST(arg_node) = TRAVdo(PROGRAM_FIRST(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* BSPglobal(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPglobal");

    switch(GLOBAL_TYPE(arg_node)) {
        case TP_bool_array:
        case TP_float_array:
        case TP_int_array:
            DBUG_PRINT("BSPglobal", ("Global has an array type, skipping"));
            DBUG_RETURN(arg_node);
        default:
            break;
    }

    // Only split if the global is both a definition AND has an initialiser value
    if(GLOBAL_DEFINITION(arg_node) != NULL && GLOBALDEFINITION_VALUES(GLOBAL_DEFINITION(arg_node)) != NULL) {
        node* def = GLOBAL_DEFINITION(arg_node);
        symbol_table_entry* varentry = GLOBALDEFINITION_VARPOINTER(def);

        node* initval = EXPRARRAY_CURRENT(GLOBALDEFINITION_VALUES(def));

        GLOBALDEFINITION_VALUES(def) = NULL;

        // Create a new variable assignment statement
        node* new_assign = TBmakeVarassign(varentry->name, initval);
        VARASSIGN_VARPOINTER(new_assign) = varentry;
        node* new_statement = TBmakeStatements(new_assign, NULL);


        node* current_final_statement = FUNCTIONBODY_STATEMENTS(INITBODY(arg_info));

        // If the initialiser function currently has no statements, make this the statement.
        if(current_final_statement == NULL) {
            FUNCTIONBODY_STATEMENTS(INITBODY(arg_info)) = new_statement;
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
                                                             BASICVARDECLARATION_VARNAME(arg_node));

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

    symbol_table* old_table = CURRENT_SYMBOL_TABLE(arg_info);
    DBUG_ASSERT((FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node) != NULL), "varsymboltable was NULL!");

    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    if(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) != NULL) {
        FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);
    }

    CURRENT_SYMBOL_TABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* BSPfunctionbody(node* arg_node, info* arg_info) {
    DBUG_ENTER("BSPfunctionbody");

    FUNBODY(arg_info) = arg_node;

    // Construct the new assignment block
    if(FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL) {
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);
    }

    // Append the new assignments to the start of the statements tree
    node* new_head = ASSIGNS(arg_info);
    ASSIGNS(arg_info) = NULL;

    if(new_head == NULL) {
        DBUG_RETURN(arg_node);
    }

    if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
        node* current_head = FUNCTIONBODY_STATEMENTS(arg_node);

        node* new_assigns_tail = new_head;

        append_statement_to_tail(new_assigns_tail, current_head);

        ASSIGNS(arg_info) = NULL;
    }

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
