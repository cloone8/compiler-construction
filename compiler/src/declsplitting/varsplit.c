#include "varsplit.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free_node.h"
#include "dbug.h"

#include "str.h"
#include "ctinfo.h"
#include "memory.h"

#include "symbol_table.h"

#include "util.h"

#include "declsplitting_lib.h"

#include "varsplit_info.h"

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
    result->newdeclblock = NULL;
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

node* VSPprogram(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPprogram");

    node* old_first = PROGRAM_FIRST(arg_node);
    PROGRAM_NODE(arg_info) = arg_node;
    CURRENT_SYMBOL_TABLE(arg_info) = PROGRAM_VARSYMBOLTABLE(arg_node);

    // Creates a new function called __init() to initialise global variables and
    // makes it the first declaration
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

    fun_symbol_table_entry* entry = fun_symbol_table_add(PROGRAM_FUNSYMBOLTABLE(arg_node), funname, TP_void, 0, NULL);

    FUNCTION_FUNPOINTER(init_function) = entry;

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
        FREEexprarray(GLOBALDEFINITION_VALUES(def), NULL);
        GLOBALDEFINITION_VALUES(def) = NULL;

        // Create a new variable assignment statement
        node* new_assign = TBmakeVarassign(STRcpy(varentry->name), initval);
        VARASSIGN_VARPOINTER(new_assign) = varentry;
        node* new_statement = TBmakeStatements(new_assign, NULL);

        prepend_to_initialiser(new_statement, arg_info);
    }

    DBUG_PRINT("global_basic", ("Done processing %s", GLOBAL_NAME(arg_node)));
    DBUG_RETURN(arg_node);
}

static node* global_array(node* arg_node, info* arg_info) {
    DBUG_ENTER("global_array");

    DBUG_ASSERT((GLOBAL_DIMENSIONS(arg_node) != NULL), "DIMS is NULL");

    if(GLOBAL_DEFINITION(arg_node) != NULL) {
        // Split the dimensions first
        node* dims = GLOBAL_DIMENSIONS(arg_node);

        size_t dimension_count = count_dims(dims);

        node* dims_decls = global_convert_dims_to_vars(GLOBAL_NAME(arg_node), CURRENT_SYMBOL_TABLE(arg_info), dims);

        node* old_first = PROGRAM_FIRST(PROGRAM_NODE(arg_info));

        // Now split the value (Except if the value is a single value for the entire array,
        // we will handle those in a later phase)
        if(GLOBALDEFINITION_VALUES(GLOBAL_DEFINITION(arg_node)) != NULL) {
            node* def = GLOBAL_DEFINITION(arg_node);
            symbol_table_entry* varentry = GLOBALDEFINITION_VARPOINTER(def);

            if(NODE_TYPE(EXPRARRAY_CURRENT(GLOBALDEFINITION_VALUES(def))) != N_expr) {
                // List of values given
                // Clear the value from the global
                node* initval = GLOBALDEFINITION_VALUES(def);
                GLOBALDEFINITION_VALUES(def) = NULL;
                size_t indices[dimension_count];

                for(size_t i = 0; i < dimension_count; i++) {
                    indices[i] = 0;
                }

                node* new_statements = exprarray_to_inits(GLOBAL_NAME(arg_node), varentry, dimension_count, indices, initval);
                if(new_statements != NULL) {
                    prepend_to_initialiser(new_statements, arg_info);
                }
            } else {
                // Single value given
                // Clear the value from the global
                node* initval = GLOBALDEFINITION_VALUES(def);
                GLOBALDEFINITION_VALUES(def) = NULL;

                // Prepare a new basic variable holding the value
                char* init_name = STRcat("tmp-init-", GLOBAL_NAME(arg_node));
                type init_type = array_type_to_type(GLOBAL_TYPE(arg_node));

                // Add this variable to the symbol table
                symbol_table* global_sym_table = PROGRAM_VARSYMBOLTABLE(PROGRAM_NODE(arg_info));
                symbol_table_entry* init_entry = symbol_table_add(global_sym_table, init_name, init_type);

                // Create a new global definition holding the value
                node* new_def = TBmakeGlobaldefinition(FALSE, initval);
                GLOBALDEFINITION_VARPOINTER(new_def) = init_entry;

                // Create a new global declaration from the previous definition
                node* new_init_global = TBmakeGlobal(STRcpy(init_name), init_type, new_def, NULL);

                // Now add the initialiser forloops statements to the __init() function
                node* initloops = create_array_init_loops(GLOBAL_NAME(arg_node), varentry, dims, init_entry, arg_info);
                prepend_to_initialiser(initloops, arg_info);
                // Traverse on this new global, to split it up again and add it to the __init() function
                TRAVdo(new_init_global, arg_info);

                // Put this global into a new declaration, and add it as the first.
                node* new_dec = TBmakeDeclaration(new_init_global, PROGRAM_FIRST(PROGRAM_NODE(arg_info)));
                PROGRAM_FIRST(PROGRAM_NODE(arg_info)) = new_dec;
                old_first = new_dec;

                MEMfree(init_name);
            }
        }

        dims_decls = TRAVdo(dims_decls, arg_info);
        append_declaration_to_tail(dims_decls, old_first);

        PROGRAM_FIRST(PROGRAM_NODE(arg_info)) = dims_decls;
    } else {
        // No global definition given, simply rename the parameters
        rename_extern_global_dims(GLOBAL_NAME(arg_node), GLOBAL_DIMENSIONS(arg_node));
    }

    DBUG_PRINT("global_array", ("Done processing %s", GLOBAL_NAME(arg_node)));
    DBUG_RETURN(arg_node);
}

node* VSPglobal(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPglobal");
    DBUG_PRINT("VSPglobal", ("Processing global %s", GLOBAL_NAME(arg_node)));

    switch(GLOBAL_TYPE(arg_node)) {
        case TP_bool:
        case TP_float:
        case TP_int:
            // Basic traversal for basic global types
            DBUG_PRINT("VSPglobal", ("Global has non-array type, doing basic traversal"));
            DBUG_RETURN(global_basic(arg_node, arg_info));
        case TP_bool_array:
        case TP_float_array:
        case TP_int_array:
            // Array traversal for array global types
            DBUG_PRINT("VSPglobal", ("Global has array type, doing array traversal"));
            DBUG_RETURN(global_array(arg_node, arg_info));
        default:
            // Invalid type and thus a programming error
            DBUG_ASSERT(FALSE, "Invalid global type encountered!");
    }
}

node* VSPbasicvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPbasicvardeclaration");
    DBUG_ASSERT((FUNBODY(arg_info) != NULL), "FUNBODY is NULL, something went really wrong");

    // If the basic variable has an initial value, split it into a seperate
    // declaration and assignment
    if(BASICVARDECLARATION_INITVAL(arg_node) != NULL) {
        node* initval = BASICVARDECLARATION_INITVAL(arg_node);
        symbol_table_entry* varentry = symbol_table_get_name(CURRENT_SYMBOL_TABLE(arg_info),
                                                             BASICVARDECLARATION_VARNAME(arg_node));

        DBUG_ASSERT((varentry != NULL), "Couldn't find symbol table entry");

        BASICVARDECLARATION_INITVAL(arg_node) = NULL;

        node* new_varassign = TBmakeVarassign(STRcpy(varentry->name), initval);
        VARASSIGN_VARPOINTER(new_varassign) = varentry;
        node* new_statement = TBmakeStatements(new_varassign, NULL);

        // Add the assignent to the ASSIGNS(arg_info) block, to be used later
        // by the VSPfunctionbody traversal
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

    // Do dimensions first
    node* dims = ARRAYVARDECLARATION_DIMENSIONS(arg_node);
    size_t dimension_count = count_dims(dims);

    fun_convert_dims_to_vars(ARRAYVARDECLARATION_VARNAME(arg_node), dims, arg_info);
    symbol_table_entry* varentry = symbol_table_get_name(CURRENT_SYMBOL_TABLE(arg_info), ARRAYVARDECLARATION_VARNAME(arg_node));

    DBUG_ASSERT((varentry != NULL), "Could not find arrayvardeclaration in symbol table!");

    // Now the the initial value
    if(ARRAYVARDECLARATION_INITVAL(arg_node) != NULL) {
        node* initval = ARRAYVARDECLARATION_INITVAL(arg_node);
        ARRAYVARDECLARATION_INITVAL(arg_node) = NULL;

        if(NODE_TYPE(EXPRARRAY_CURRENT(initval)) != N_expr) {
            // List of values given
            size_t indices[dimension_count];

            for(size_t i = 0; i < dimension_count; i++) {
                indices[i] = 0;
            }

            node* new_statements = exprarray_to_inits(ARRAYVARDECLARATION_VARNAME(arg_node), varentry, dimension_count, indices, initval);

            if(new_statements != NULL) {
                append_statement_to_tail(ASSIGNS(arg_info), new_statements);
            }


        } else {
            // Single value given.

            // Prepare a new basic variable holding the value
            char* init_name = STRcat("tmp-init-", ARRAYVARDECLARATION_VARNAME(arg_node));
            type init_type = array_type_to_type(ARRAYVARDECLARATION_TYPE(arg_node));

            // Add this variable to the symbol table
            symbol_table* sym_table = CURRENT_SYMBOL_TABLE(arg_info);
            symbol_table_entry* init_entry = symbol_table_add(sym_table, init_name, init_type);

            // Create a new basicvardeclaration holding the value
            node* new_dec = TBmakeBasicvardeclaration(STRcpy(init_name), init_type, EXPRARRAY_CURRENT(initval));
            EXPRARRAY_CURRENT(initval) = NULL;
            FREEexprarray(initval, NULL);
            BASICVARDECLARATION_VARPOINTER(new_dec) = init_entry;

            // Create a new functionvardeclaration from the new basicvardec
            node* new_fundec = TBmakeFunctionvardeclaration(new_dec, NULL);

            // Traverse on this new declaration, to split it up again
            TRAVdo(new_fundec, arg_info);

            // Now add the initialiser forloops statements to the function statements block
            node* initloops = create_array_init_loops(ARRAYVARDECLARATION_VARNAME(arg_node), varentry, dims, init_entry, arg_info);

            if(ASSIGNS(arg_info) != NULL) {
                append_statement_to_tail(ASSIGNS(arg_info), initloops);
            } else {
                ASSIGNS(arg_info) = initloops;
            }

            // Add the new declaration to the function
            append_functionvardeclaration_to_tail(DECLS(arg_info), new_fundec);

            MEMfree(init_name);
        }
    }

    DBUG_RETURN(arg_node);
}

node* VSPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPfunctiondefinition");

    symbol_table* old_table = CURRENT_SYMBOL_TABLE(arg_info);
    DBUG_ASSERT((FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node) != NULL), "varsymboltable was NULL!");

    // Changes the current symbol table
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

    node* new_body = arg_node;

    // Construct the new assignment anb declarations block
    if(FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL) {
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);

        node* new_vardecs_head = FUNCTIONBODY_VARDECLARATIONS(arg_node);
        node* new_statements_head = FUNCTIONBODY_STATEMENTS(arg_node);

        if(DECLS(arg_info) != NULL) {
            // Append the current vardeclarations block to the new declarations block,
            // and make the new declarations the first declarations of this function
            append_functionvardeclaration_to_tail(DECLS(arg_info), FUNCTIONBODY_VARDECLARATIONS(arg_node));
            new_vardecs_head = DECLS(arg_info);
            DECLS(arg_info) = NULL;
        }

        if(ASSIGNS(arg_info) != NULL) {
            // Append the current statements block to the new statements block,
            // and make the new statements the first statements of this function
            if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
                append_statement_to_tail(ASSIGNS(arg_info), FUNCTIONBODY_STATEMENTS(arg_node));
            }
            new_statements_head = ASSIGNS(arg_info);
            ASSIGNS(arg_info) = NULL;
        }

        new_body = TBmakeFunctionbody(new_vardecs_head,
                                     FUNCTIONBODY_LOCALFUNCTIONS(arg_node),
                                     new_statements_head);
    }

    if(FUNCTIONBODY_LOCALFUNCTIONS(new_body) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(new_body) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(new_body), arg_info);
    }

    if(new_body != arg_node) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = NULL;
        FUNCTIONBODY_STATEMENTS(arg_node) = NULL;
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = NULL;

        FREEfunctionbody(arg_node, NULL);
    }

    DBUG_RETURN(new_body);
}

node* VSPdeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPdeclaration");

    // Traverse the declarations in reverse order to keep the order of
    // assignments okay

    if(DECLARATION_NEXT(arg_node) != NULL) {
        TRAVdo(DECLARATION_NEXT(arg_node), arg_info);
    }

    TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node* VSPfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("VSPfor");

    if(FOR_CODE(arg_node) != NULL) {
        FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);
    }

    if(FOR_CONDITION(arg_node) != NULL) {
        FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);
    }

    // Purposefully skip forloop counter

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
