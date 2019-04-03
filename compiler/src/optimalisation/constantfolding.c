#include "constantfolding.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "free_node.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
};


/*
 * INFO macros
 */

#define TYPE(__arg_info_struct__) ((__arg_info_struct__)->final_type)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *CFOmakeInfo(void) {
    DBUG_ENTER("CFOmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *CFOfreeInfo(info *info) {
    DBUG_ENTER("CFOfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* CFObinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFObinop");

    DBUG_RETURN(arg_node);
}

node* CFOmonop(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOmonop");

    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), arg_info);

    node* toreturn = arg_node;

    if(NODE_TYPE(EXPR_CURRENT(MONOP_TARGET(arg_node))) == N_constant) {
        switch(MONOP_TYPE(arg_node)) {
            case MO_neg:
                switch(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(MONOP_TARGET(arg_node))))) {
                    case N_intconst:
                        toreturn = MONOP_TARGET(arg_node);
                        INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(toreturn))) *= -1;
                        MONOP_TARGET(arg_node) = NULL;
                        FREEmonop(arg_node, NULL);
                        break;
                    case N_floatconst:
                        toreturn = MONOP_TARGET(arg_node);
                        FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(toreturn))) *= -1;
                        MONOP_TARGET(arg_node) = NULL;
                        FREEmonop(arg_node, NULL);
                        break;
                    default:
                        DBUG_ASSERT((FALSE), "Invalid monop constant type");
                        break;
                }
                break;
            case MO_not:
                toreturn = MONOP_TARGET(arg_node);
                BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(toreturn))) = !BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(toreturn)));
                MONOP_TARGET(arg_node) = NULL;
                FREEmonop(arg_node, NULL);
                break;
            case MO_unknown:
                DBUG_ASSERT((FALSE), "Invalid monop type");
                break;
        }
    }

    DBUG_RETURN(toreturn);
}

node* CFOdo(node* syntaxtree) {
    DBUG_ENTER("CFOdo");

    TRAVpush(TR_cfo);
    info *arg_info = CFOmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    CFOfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
