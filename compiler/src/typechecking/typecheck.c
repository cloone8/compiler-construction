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
};


/*
 * INFO macros
 */

#define TYPE(__arg_info_struct__) ((__arg_info_struct__)->final_type)
#define DEPTH(__arg_info_struct__) ((__arg_info_struct__)->paramdepth)
#define GIVEN(__arg_info_struct__) ((__arg_info_struct__)->given_params)

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

node* TCvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCvarassign");

    symbol_table_entry* entry = VARASSIGN_VARPOINTER(arg_node);
    TYPE(arg_info) = entry->vartype;

    info* val_info = TCmakeInfo();

    if (VARASSIGN_VALUE(arg_node) != NULL) {
        TRAVdo(VARASSIGN_VALUE(arg_node), val_info);
    }

    if (TYPE(val_info) != TYPE(arg_info)) {
        CTIerror("Wrong type for %s, expected %s but got %s", entry->name, print_type(TYPE(arg_info)), print_type(TYPE(val_info)));
    }

    DBUG_RETURN(arg_node);
}

node* TCfuncallparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfuncallparameter");

    type needed = TYPE(arg_info);

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

node* TCfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCfuncall");

    if (FUNCALL_PARAMS(arg_node) != NULL) {

        fun_symbol_table_entry* entry = FUNCALL_FUNPOINTER(arg_node);
        type* needed = entry->paramtypes;
        size_t params = entry->paramamount;
        type* given[params];
        GIVEN(arg_info) = given;

        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);

        for (size_t i = 0; i < params; i++) {
            if (needed[i] != GIVEN(arg_info)[i]) {
                CTIerror("Wrong types given for function %s, argument %zu needs %s, but got, %s", FUNCALL_FUNCTIONNAME(arg_node),
                                                                                      i,
                                                                                      print_type(needed[i]), 
                                                                                      print_type(GIVEN(arg_info)[i]));
            }
        }
    }

    fun_symbol_table_entry* entry = FUNCALL_FUNPOINTER(arg_node);
    TYPE(arg_info) = entry->returntype;

    DBUG_RETURN(arg_node);
}

node *TCbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCBINOP");

    info* left_info = TCmakeInfo();
    info *right_info = TCmakeInfo();

    TRAVdo(BINOP_LEFT(arg_node), left_info);
    TRAVdo(BINOP_RIGHT(arg_node), right_info);

    if (TYPE(left_info) == TYPE(right_info)) {
        TYPE(arg_info) = TYPE(left_info);
    } else {
        CTIerror("Wrong types for binop");
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

node *TCdo(node *syntaxtree) {
    DBUG_ENTER("TCdo");

    TRAVpush(TR_tc);
    info *arg_info = TCmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
