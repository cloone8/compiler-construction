#include "array_varsplit.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "free_node.h"

#include "str.h"
#include "memory.h"

#include "util.h"

#include "symbol_table.h"
#include "fun_symbol_table.h"

// Must fit at least as many characters as the amount of numbers in a size_t
#define BUF_SIZE (50)

/*
 * INFO structure
 */
struct INFO {
    node* initbody;
    node* currentfunbody;
    symbol_table* currentsymtable;
    node* programnode;
};

/*
 * INFO macros
 */

#define INITBODY(__arg_info_struct__) ((__arg_info_struct__)->initbody)
#define FUNBODY(__arg_info_struct__) ((__arg_info_struct__)->currentfunbody)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentsymtable)
#define PROGRAM_NODE(__arg_info_struct__) ((__arg_info_struct__)->programnode)
/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *ASPmakeInfo(void) {
    DBUG_ENTER("ASPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    result->initbody = NULL;
    result->currentfunbody = NULL;
    result->currentsymtable = NULL;
    result->programnode = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *ASPfreeInfo(info *info) {
    DBUG_ENTER("ASPfreeInfo");

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
        char name_buffer[BUF_SIZE];
        snprintf(name_buffer, BUF_SIZE, "%zu", dimcount++);
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

        node* new_varread = TBmakeVarread(new_global_name);
        VARREAD_VARPOINTER(new_varread) = var_entry;
        EXPRARRAY_CURRENT(current_dim) = new_varread;

        current_dim = EXPRARRAY_NEXT(current_dim);
    }

    DBUG_RETURN(decls);
}


node* ASPprogram(node* arg_node, info* arg_info) {
    DBUG_ENTER("ASPprogram");

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

    if (DECLARATION_NEXT(init_declaration) != NULL) {
        DECLARATION_NEXT(init_declaration) = TRAVdo(DECLARATION_NEXT(init_declaration), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* ASPglobal(node* arg_node, info* arg_info) {
    DBUG_ENTER("ASPglobal");

    switch(GLOBAL_TYPE(arg_node)) {
        case TP_bool:
        case TP_float:
        case TP_int:
            DBUG_PRINT("ASPglobal", ("Global has no array type, skipping"));
            DBUG_RETURN(arg_node);
        default:
            break;
    }

    DBUG_ASSERT((GLOBAL_DIMENSIONS(arg_node) != NULL), "DIMS is NULL");


    // Split the dimensions first
    if(GLOBAL_DEFINITION(arg_node) != NULL) {
        node* dims = GLOBAL_DIMENSIONS(arg_node);
        node* def = GLOBAL_DEFINITION(arg_node);

        node* dims_decls = convert_dims_to_vars(GLOBAL_NAME(arg_node), CURRENT_SYMBOL_TABLE(arg_info), dims);

        node* old_first = PROGRAM_FIRST(PROGRAM_NODE(arg_info));
        append_declaration_to_tail(dims_decls, old_first);

        PROGRAM_FIRST(PROGRAM_NODE(arg_info)) = dims_decls;
    }

    DBUG_RETURN(arg_node);
}

node* ASParrayvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("ASParrayvardeclaration");
    DBUG_ASSERT((FUNBODY(arg_info) != NULL), "FUNBODY is NULL, something went really wrong");

    DBUG_RETURN(arg_node);
}

node* ASPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("ASPfunctiondefinition");

    symbol_table* old_table = CURRENT_SYMBOL_TABLE(arg_info);
    DBUG_ASSERT((FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node) != NULL), "varsymboltable was NULL!");

    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    if(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) != NULL) {
        FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);
    }

    CURRENT_SYMBOL_TABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* ASPfunctionbody(node* arg_node, info* arg_info) {
    DBUG_ENTER("ASPfunctionbody");

    FUNBODY(arg_info) = arg_node;

    DBUG_RETURN(arg_node);
}

node* ASPdeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("ASPdeclaration");

    if(DECLARATION_NEXT(arg_node) != NULL) {
        DECLARATION_NEXT(arg_node) = TRAVdo(DECLARATION_NEXT(arg_node), arg_info);
    }

    DECLARATION_CURRENT(arg_node) = TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASPdo(node *syntaxtree) {
    DBUG_ENTER("ASPdo");

    info* arg_info = ASPmakeInfo();

    TRAVpush(TR_asp);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = ASPfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
