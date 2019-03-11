#include "varsplit.h"

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
    node* programnode;
};

/*
 * INFO macros
 */

#define INITBODY(__arg_info_struct__) ((__arg_info_struct__)->initbody)
#define FUNBODY(__arg_info_struct__) ((__arg_info_struct__)->currentfunbody)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentsymtable)
#define ASSIGNS(__arg_info_struct__) ((__arg_info_struct__)->newassignblock)
#define PROGRAM_NODE(__arg_info_struct__) ((__arg_info_struct__)->programnode)
/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *VSPmakeInfo(void) {
    DBUG_ENTER("VSPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    result->initbody = NULL;
    result->currentfunbody = NULL;
    result->currentsymtable = NULL;
    result->newassignblock = NULL;
    result->programnode = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *VSPfreeInfo(info *info) {
    DBUG_ENTER("VSPfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

static node* convert_dims_to_vars(char* global_name, symbol_table* symtable, node* dimensions) {
    DBUG_ENTER("convert_dims_to_vars");
    DBUG_ASSERT((NODE_TYPE(dimensions) == N_exprarray), "convert_dims_to_vars called without exprarray as argument!");
    DBUG_ASSERT((dimensions != NULL), "convert_dims_to_vars called with NULL as argument!");

    node* decls = NULL;
    node* current_dim = dimensions;
    size_t dimcount = 0;

    while(current_dim != NULL) {
        node* dim_value = EXPRARRAY_CURRENT(current_dim);
        node* new_definition = TBmakeGlobaldefinition(FALSE, TBmakeExprarray(NULL, dim_value));
        char name_buffer[STR_BUF_SIZE];
        snprintf(name_buffer, STR_BUF_SIZE, "%zu", dimcount++);
        char *new_global_name = STRcatn(4, "tmp-", name_buffer, "-", global_name);

        symbol_table_entry* var_entry = symbol_table_add(symtable, new_global_name, TP_int);
        GLOBALDEFINITION_VARPOINTER(new_definition) = var_entry;

        node* new_global = TBmakeGlobal(new_global_name, TP_int, new_definition, NULL);
        node* new_decls = TBmakeDeclaration(new_global, NULL);

        if(decls == NULL) {
            decls = new_decls;
        } else {
            append_declaration_to_tail(decls, new_decls);
        }

        node* new_varread = TBmakeVarread(STRcpy(new_global_name));
        VARREAD_VARPOINTER(new_varread) = var_entry;
        EXPRARRAY_CURRENT(current_dim) = new_varread;

        current_dim = EXPRARRAY_NEXT(current_dim);
    }

    DBUG_RETURN(decls);
}

node* VSPprogram(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPprogram");

    node* old_first = PROGRAM_FIRST(arg_node);
    PROGRAM_NODE(arg_info) = arg_node;
    CURRENT_SYMBOL_TABLE(arg_info) = PROGRAM_VARSYMBOLTABLE(arg_node);

    node* init_body = TBmakeFunctionbody(NULL, NULL, NULL);
    node* init_definition = TBmakeFunctiondefinition(FALSE, init_body);
    FUNCTIONDEFINITION_FUNSYMBOLTABLE(init_definition) = fun_symbol_table_make(PROGRAM_FUNSYMBOLTABLE(arg_node), 1);
    FUNCTIONDEFINITION_VARSYMBOLTABLE(init_definition) = symbol_table_make(CURRENT_SYMBOL_TABLE(arg_info), 1);
    char* funname = STRcpy("__init");
    node* init_function = TBmakeFunction(funname, TP_void, NULL, init_definition);

    node* init_declaration = TBmakeDeclaration(init_function, old_first);

    PROGRAM_INITFUN(arg_node) = init_function;
    INITBODY(arg_info) = init_body;

    PROGRAM_FIRST(arg_node) = init_declaration;

    fun_symbol_table_add(PROGRAM_FUNSYMBOLTABLE(arg_node), funname, TP_void, 0, NULL);

    if (PROGRAM_FIRST(arg_node) != NULL) {
        TRAVdo(PROGRAM_FIRST(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

static node* global_basic(node* arg_node, info* arg_info) {
    DBUG_ENTER("global_basic");

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

static node* global_array(node* arg_node, info* arg_info) {
    DBUG_ENTER("global_array");

    DBUG_ASSERT((GLOBAL_DIMENSIONS(arg_node) != NULL), "DIMS is NULL");

    // Split the dimensions first
    if(GLOBAL_DEFINITION(arg_node) != NULL) {
        node* dims = GLOBAL_DIMENSIONS(arg_node);

        node* dims_decls = convert_dims_to_vars(GLOBAL_NAME(arg_node), CURRENT_SYMBOL_TABLE(arg_info), dims);

        node* old_first = PROGRAM_FIRST(PROGRAM_NODE(arg_info));
        append_declaration_to_tail(dims_decls, old_first);

        PROGRAM_FIRST(PROGRAM_NODE(arg_info)) = dims_decls;
    }

    DBUG_RETURN(arg_node);
}

node* VSPglobal(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPglobal");

    switch(GLOBAL_TYPE(arg_node)) {
        case TP_bool:
        case TP_float:
        case TP_int:
            DBUG_PRINT("VSPglobal", ("Global has non-array type, doing basic traversal"));
            DBUG_RETURN(global_basic(arg_node, arg_info));
        case TP_bool_array:
        case TP_float_array:
        case TP_int_array:
            DBUG_PRINT("VSPglobal", ("Global has array type, doing array traversal"));
            DBUG_RETURN(global_array(arg_node, arg_info));
        default:
            DBUG_ASSERT(FALSE, "Invalid global type encountered!");
    }
}

node* VSPbasicvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPbasicvardeclaration");
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

node* VSParrayvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSParrayvardeclaration");
    DBUG_ASSERT((FUNBODY(arg_info) != NULL), "FUNBODY is NULL, something went really wrong");

    DBUG_RETURN(arg_node);
}

node* VSPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPfunctiondefinition");

    symbol_table* old_table = CURRENT_SYMBOL_TABLE(arg_info);
    DBUG_ASSERT((FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node) != NULL), "varsymboltable was NULL!");

    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    if(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) != NULL) {
        FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);
    }

    CURRENT_SYMBOL_TABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* VSPfunctionbody(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPfunctionbody");

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

node* VSPdeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPdeclaration");

    if(DECLARATION_NEXT(arg_node) != NULL) {
        DECLARATION_NEXT(arg_node) = TRAVdo(DECLARATION_NEXT(arg_node), arg_info);
    }

    DECLARATION_CURRENT(arg_node) = TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *VSPdo(node *syntaxtree) {
    DBUG_ENTER("VSPdo");

    info* arg_info = VSPmakeInfo();

    TRAVpush(TR_vsp);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = VSPfreeInfo(arg_info);
    DBUG_RETURN(syntaxtree);
}
