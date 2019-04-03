#include "constantfolding.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "free_node.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"
#include "copy.h"

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

static node* binop_int(binop operator, node* left, node* right) {
    DBUG_ENTER("binop_int");
    node* toreturn = NULL;
    int new_intval = -1;
    bool new_boolval = FALSE;

    switch(operator) {
        case BO_add:
            new_intval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) +
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_div:
            new_intval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) /
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_eq:
            new_boolval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) ==
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_ge:
            new_boolval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) >=
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_gt:
            new_boolval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) >
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_le:
            new_boolval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) <=
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_lt:
            new_boolval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) <
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_mod:
            new_intval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) %
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_mul:
            new_intval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) *
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_ne:
            new_boolval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) !=
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_sub:
            new_intval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) -
                         INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid binop operator type!");
            break;
    }

    TRAVpush(TR_free);
    FREEexpr(left, NULL);
    FREEexpr(right, NULL);
    TRAVpop();

    DBUG_RETURN(toreturn);
}

static node* binop_float(binop operator, node* left, node* right) {
    DBUG_ENTER("binop_float");
    node* toreturn = NULL;
    float new_fltval = -1.0;
    bool new_boolval = FALSE;

    switch(operator) {
        case BO_add:
            new_fltval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) +
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        case BO_div:
            new_fltval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) /
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        case BO_eq:
            new_boolval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) ==
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_ge:
            new_boolval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) >=
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_gt:
            new_boolval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) >
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_le:
            new_boolval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) <=
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_lt:
            new_boolval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) <
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_mul:
            new_fltval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) *
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        case BO_ne:
            new_boolval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) !=
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_sub:
            new_fltval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) -
                         FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid binop operator type!");
            break;
    }

    TRAVpush(TR_free);
    FREEexpr(left, NULL);
    FREEexpr(right, NULL);
    TRAVpop();

    DBUG_RETURN(toreturn);
}

static node* binop_bool(binop operator, node* left, node* right) {
    DBUG_ENTER("binop_bool");
    node* toreturn = NULL;
    bool new_boolval = FALSE;

    switch(operator) {
        case BO_add:
            new_boolval = BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) ||
                         BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_mul:
            new_boolval = BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) &&
                         BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_eq:
            new_boolval = BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) ==
                         BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_ne:
            new_boolval = BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(left))) !=
                         BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(right)));
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid binop operator type!");
            break;
    }

    TRAVpush(TR_free);
    FREEexpr(left, NULL);
    FREEexpr(right, NULL);
    TRAVpop();

    DBUG_RETURN(toreturn);
}

node* CFObinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFObinop");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    node* toreturn = arg_node;

    if(NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant &&
       NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant) {

        switch(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node))))) {
            case N_intconst:
                toreturn = binop_int(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                BINOP_LEFT(arg_node) = NULL;
                BINOP_RIGHT(arg_node) = NULL;
                FREEbinop(arg_node, NULL);
                break;
            case N_floatconst:
                toreturn = binop_float(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                BINOP_LEFT(arg_node) = NULL;
                BINOP_RIGHT(arg_node) = NULL;
                FREEbinop(arg_node, NULL);
                break;
            case N_boolconst:
                toreturn = binop_bool(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                BINOP_LEFT(arg_node) = NULL;
                BINOP_RIGHT(arg_node) = NULL;
                FREEbinop(arg_node, NULL);
                break;
            default:
                DBUG_ASSERT((FALSE), "Invalid constant type");
                break;
        }
    }

    DBUG_RETURN(toreturn);
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
                        toreturn = COPYdoCopy(EXPR_CURRENT(MONOP_TARGET(arg_node)));
                        INTCONST_VALUE(CONSTANT_CURRENT(toreturn)) *= -1;

                        TRAVpush(TR_free);
                        FREEmonop(arg_node, NULL);
                        TRAVpop();
                        break;
                    case N_floatconst:
                        toreturn = COPYdoCopy(EXPR_CURRENT(MONOP_TARGET(arg_node)));
                        FLOATCONST_VALUE(CONSTANT_CURRENT(toreturn)) *= -1;

                        TRAVpush(TR_free);
                        FREEmonop(arg_node, NULL);
                        TRAVpop();
                        break;
                    default:
                        DBUG_ASSERT((FALSE), "Invalid monop constant type");
                        break;
                }
                break;
            case MO_not:
                toreturn = COPYdoCopy(EXPR_CURRENT(MONOP_TARGET(arg_node)));
                BOOLCONST_VALUE(CONSTANT_CURRENT(toreturn)) = !BOOLCONST_VALUE(CONSTANT_CURRENT(toreturn));

                TRAVpush(TR_free);
                FREEmonop(arg_node, NULL);
                TRAVpop();
                break;
            case MO_unknown:
                DBUG_ASSERT((FALSE), "Invalid monop type");
                break;
        }
    }

    DBUG_RETURN(toreturn);
}

node* CFOconditionalexpr(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOconditionalexpr");

    CONDITIONALEXPR_CONDITION(arg_node) = TRAVdo(CONDITIONALEXPR_CONDITION(arg_node), arg_info);
    CONDITIONALEXPR_IFBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_IFBRANCH(arg_node), arg_info);
    CONDITIONALEXPR_ELSEBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_ELSEBRANCH(arg_node), arg_info);

    if(NODE_TYPE(EXPR_CURRENT(CONDITIONALEXPR_CONDITION(arg_node))) == N_constant) {
        node* toreturn;

        if(BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(CONDITIONALEXPR_CONDITION(arg_node)))) == TRUE) {
            toreturn = COPYdoCopy(CONDITIONALEXPR_IFBRANCH(arg_node));
        } else {
            toreturn = COPYdoCopy(CONDITIONALEXPR_ELSEBRANCH(arg_node));
        }

        TRAVpush(TR_free);
        FREEconditionalexpr(arg_node, NULL);
        TRAVpop();

        DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(arg_node);
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
