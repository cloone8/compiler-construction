#include "typecheck.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
    type final_type;
    size_t paramdepth;
    type* given_params;
    type expected_returntype;
    bool has_return;
};


/*
 * INFO macros
 */

#define TYPE(__arg_info_struct__) ((__arg_info_struct__)->final_type)
#define DEPTH(__arg_info_struct__) ((__arg_info_struct__)->paramdepth)
#define GIVEN(__arg_info_struct__) ((__arg_info_struct__)->given_params)
#define RETURNTYPE(__arg_info_struct__) ((__arg_info_struct__)->expected_returntype)
#define HASRETURN(__arg_info_struct__) ((__arg_info_struct__)->has_return)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *TCmakeInfo(void) {
    DBUG_ENTER("TCmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    DEPTH(result) = 0;
    GIVEN(result) = NULL;
    TYPE(result) = TP_unknown;
    RETURNTYPE(result) = TP_unknown;
    HASRETURN(result) = FALSE;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *TCfreeInfo(info *info) {
    DBUG_ENTER("TCfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* TCfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfunctiondefinition");

    if (FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) != NULL) {
        FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* TCfunction(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfunction");
    UNUSED(arg_info);

    if (FUNCTION_DEFINITION(arg_node) != NULL) {
        info* new_info = TCmakeInfo();
        RETURNTYPE(new_info) = FUNCTION_TYPE(arg_node);

        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), new_info);

        if(!HASRETURN(new_info) && FUNCTION_TYPE(arg_node) != TP_void) {
            CTIerrorLine(NODE_LINE(arg_node), "Non void function %s missing return statement!", FUNCTION_NAME(arg_node));
        }

        TCfreeInfo(new_info);
    }

    DBUG_RETURN(arg_node);
}

node* TCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCreturn");
    type funtype = RETURNTYPE(arg_info);

    DBUG_ASSERT((funtype != TP_unknown), "Unknown return type encountered!");
    DBUG_ASSERT((funtype != TP_int_array), "Invalid return type encountered!");
    DBUG_ASSERT((funtype != TP_float_array), "Invalid return type encountered!");
    DBUG_ASSERT((funtype != TP_bool_array), "Invalid return type encountered!");

    HASRETURN(arg_info) = TRUE;

    if(funtype != TP_void) {
        if(RETURN_VALUE(arg_node) == NULL) {
            CTIerror("Return statement without value not permitted in non-void function!");
        }

        info* new_info = TCmakeInfo();

        TRAVdo(RETURN_VALUE(arg_node), new_info);

        if(TYPE(new_info) != funtype) {
            CTIerror("Return statement with invalid type! Expected %s but got %s", get_type_str(funtype), get_type_str(TYPE(new_info)));
        }

        TCfreeInfo(new_info);
    } else {
        if(RETURN_VALUE(arg_node) != NULL) {
            CTIerror("Return statement with value not permitted in void function!");
        }
    }

    DBUG_RETURN(arg_node);
}

node* TCfunctionbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfunctionbody");

    if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
        FUNCTIONBODY_STATEMENTS(arg_node) = TRAVdo(FUNCTIONBODY_STATEMENTS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL) {
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_LOCALFUNCTIONS(arg_node) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* TCvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCvarassign");

    symbol_table_entry* entry = VARASSIGN_VARPOINTER(arg_node);
    TYPE(arg_info) = entry->vartype;

    info* val_info = TCmakeInfo();

    if (VARASSIGN_VALUE(arg_node) != NULL) {
        TRAVdo(VARASSIGN_VALUE(arg_node), val_info);
    }

    if (TYPE(val_info) != TYPE(arg_info)) {
        CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Wrong type for %s, expected %s but got %s", entry->name, get_type_str(TYPE(arg_info)), get_type_str(TYPE(val_info)));
    }

    TCfreeInfo(val_info);

    DBUG_RETURN(arg_node);
}

node* TCarrayvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCarrayvarassign");

    symbol_table_entry* entry = ARRAYVARASSIGN_VARPOINTER(arg_node);
    TYPE(arg_info) = array_type_to_type(entry->vartype);

    info* val_info = TCmakeInfo();

    if (ARRAYVARASSIGN_VALUES(arg_node) != NULL) {
        TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), val_info);
    }

    if (TYPE(val_info) != TYPE(arg_info)) {
        CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Wrong type for %s, expected %s but got %s", entry->name, get_type_str(TYPE(arg_info)), get_type_str(TYPE(val_info)));
    }

    TCfreeInfo(val_info);

    DBUG_RETURN(arg_node);
}

node* TCfuncallparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfuncallparameter");

    DEPTH(arg_info)++;
    if (FUNCALLPARAMETER_NEXT(arg_node) != NULL) {
        FUNCALLPARAMETER_NEXT(arg_node) = TRAVdo(FUNCALLPARAMETER_NEXT(arg_node), arg_info);
    }
    DEPTH(arg_info)--;

    if (FUNCALLPARAMETER_VALUE(arg_node) != NULL) {
        FUNCALLPARAMETER_VALUE(arg_node) = TRAVdo(FUNCALLPARAMETER_VALUE(arg_node), arg_info);
        GIVEN(arg_info)[DEPTH(arg_info)] = TYPE(arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *TCvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCvarread");

    symbol_table_entry* entry = VARREAD_VARPOINTER(arg_node);
    TYPE(arg_info) = entry->vartype;

    DBUG_RETURN(arg_node);
}

node *TCarrayvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCarrayvarread");


    symbol_table_entry* entry = ARRAYVARREAD_VARPOINTER(arg_node);
    TYPE(arg_info) = array_type_to_type(entry->vartype);

    info* array_pos = TCmakeInfo();

    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), array_pos);
    if (TYPE(array_pos) != TP_int) {
        CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node),
                     "Array subscript of \"%s[]\" must be of type int.", ARRAYVARREAD_ARRAYVARNAME(arg_node));
    }

    TCfreeInfo(array_pos);

    DBUG_RETURN(arg_node);
}

node* TCfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfuncall");
    if (FUNCALL_PARAMS(arg_node) != NULL) {
        fun_symbol_table_entry* entry = FUNCALL_FUNPOINTER(arg_node);
        type* needed = entry->paramtypes;
        size_t params = entry->paramamount;
        type given[params];

        info* new_info = TCmakeInfo();
        GIVEN(new_info) = given;

        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), new_info);

        for (size_t i = 0; i < params; i++) {
            if (needed[i] != GIVEN(new_info)[i]) {
                CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node),
                             "Wrong types given for function %s, argument %zu needs %s, but got, %s",
                             FUNCALL_FUNCTIONNAME(arg_node),
                             i + 1,
                             get_type_str(needed[i]),
                             get_type_str(GIVEN(new_info)[i]));
            }
        }
        TCfreeInfo(new_info);
    }

    fun_symbol_table_entry* entry = FUNCALL_FUNPOINTER(arg_node);

    DBUG_ASSERT((entry != NULL), "Entry is null!");
    TYPE(arg_info) = entry->returntype;

    DBUG_RETURN(arg_node);
}

node* TCmonop(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCmonop");

    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), arg_info);

    switch (MONOP_TYPE(arg_node)) {
        case MO_neg:
            if (TYPE(arg_info) == TP_bool) {
                CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Monop type \"-\" can only be float or int.");
            }
            break;
        case MO_not:
            if (TYPE(arg_info) != TP_bool) {
                CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Monop type \"!\" can only be bool.");
            }
            break;
        default:
            break;
    }

    DBUG_RETURN(arg_node);
}

node *TCbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCbinop");

    info* left_info = TCmakeInfo();
    info *right_info = TCmakeInfo();

    TRAVdo(BINOP_LEFT(arg_node), left_info);
    TRAVdo(BINOP_RIGHT(arg_node), right_info);

    if (TYPE(left_info) == TYPE(right_info)) {
        switch(BINOP_BINOPERATORS(arg_node)) {
            case BO_mod:
                if (TYPE(left_info) != TP_int) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \"\%\" requires two integers.");
                }
                TYPE(arg_info) = TYPE(left_info);
                break;
            case BO_lt:
                if (TYPE(left_info) != TP_int && TYPE(left_info) != TP_float) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \"<\" requires two integers or two floats.");
                }
                TYPE(arg_info) = TP_bool;
                break;
            case BO_le:
                if (TYPE(left_info) != TP_int && TYPE(left_info) != TP_float) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \"<=\" requires two integers or two floats.");
                }
                TYPE(arg_info) = TP_bool;
                break;
            case BO_eq:
                if (array_type_to_type(TYPE(left_info)) != TYPE(left_info)) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \"==\" requires a non-array type.");
                }
                TYPE(arg_info) = TP_bool;
                break;
            case BO_ne:
                if (array_type_to_type(TYPE(left_info)) != TYPE(left_info)) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \"!=\" requires a non-array type.");
                }
                TYPE(arg_info) = TP_bool;
                break;
            case BO_ge:
                if (TYPE(left_info) != TP_int && TYPE(left_info) != TP_float) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \">=\" requires two integers or two floats.");
                }
                TYPE(arg_info) = TP_bool;
                break;
            case BO_gt:
                if (TYPE(left_info) != TP_int && TYPE(left_info) != TP_float) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \">\" requires two integers or two floats.");
                }
                TYPE(arg_info) = TP_bool;
                break;
            case BO_and:
                if (TYPE(left_info) != TP_bool) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \"&&\" requires two bools.");
                }
                TYPE(arg_info) = TYPE(left_info);
                break;
            case BO_or:
                if (TYPE(left_info) != TP_bool) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Binop \"||\" requires two bools.");
                }
                TYPE(arg_info) = TYPE(left_info);
                break;
            default:
                TYPE(arg_info) = TYPE(left_info);
        }
    } else {
        CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Unequal types for binop \"%s\"!", get_binop_str(BINOP_BINOPERATORS(arg_node)));
    }

    TCfreeInfo(left_info);
    TCfreeInfo(right_info);

    DBUG_RETURN(arg_node);
}

node* TCintconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCintconst");

    TYPE(arg_info) = TP_int;

    DBUG_RETURN(arg_node);
}

node *TCfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfloatconst");

    TYPE(arg_info) = TP_float;

    DBUG_RETURN(arg_node);
}

node *TCboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCboolconst");

    TYPE(arg_info) = TP_bool;

    DBUG_RETURN(arg_node);
}

node *TCcast(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCcast");

    info* new_info = TCmakeInfo();

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), new_info);

    TCfreeInfo(new_info);

    TYPE(arg_info) = CAST_CASTTO(arg_node);

    DBUG_RETURN(arg_node);
}

node *TCdo(node *syntaxtree) {
    DBUG_ENTER("TCdo");

    TRAVpush(TR_tc);
    info *arg_info = TCmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
