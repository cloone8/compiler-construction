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
#include "globals.h"
#include "util.h"

/*
 * INFO structure
 */
struct INFO {
};


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

static node* binop_zero_int(node* arg_node, binop operator, node* other, bool noremove) {
    DBUG_ENTER("binop_zero_int");
    node* toreturn = arg_node;

    switch(operator) {
        case BO_add:
        case BO_sub:
            toreturn = COPYdoCopy(EXPR_CURRENT(other));
            break;
        case BO_mul:
        case BO_div:
        case BO_mod:
            if(noremove) {
                DBUG_RETURN(toreturn);
            }
            toreturn = TBmakeConstant(TBmakeIntconst(0));
            break;
        default:
            DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(toreturn);
}

static node* binop_zero_float(node* arg_node, binop operator, node* other, bool noremove) {
    DBUG_ENTER("binop_zero_float");
    node* toreturn = arg_node;

    switch(operator) {
        case BO_add:
        case BO_sub:
            toreturn = COPYdoCopy(EXPR_CURRENT(other));
            break;
        case BO_mul:
        case BO_div:
            if(noremove) {
                DBUG_RETURN(toreturn);
            }
            toreturn = TBmakeConstant(TBmakeFloatconst(0.0));
            break;
        default:
            DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(toreturn);
}

static node* binop_zero_bool(node* arg_node, binop operator, node* other, bool noremove) {
    DBUG_ENTER("binop_zero_bool");
    node* toreturn = arg_node;

    switch(operator) {
        case BO_add:
            toreturn = COPYdoCopy(EXPR_CURRENT(other));
            break;
        case BO_mul:
            if(noremove) {
                DBUG_RETURN(toreturn);
            }
            toreturn = TBmakeConstant(TBmakeBoolconst(FALSE));
            break;
        default:
            DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(toreturn);
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

    DBUG_RETURN(toreturn);
}

static node* check_special_cases(node* arg_node) {
    DBUG_ENTER("check_special_cases");
    node* toreturn = arg_node;

    bool left_constant = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant;
    bool right_constant = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant;

    bool left_varread = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_varread;
    bool right_varread = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_varread;

    node* constant = NULL;
    node* variable = NULL;

    switch(BINOP_BINOPERATORS(arg_node)) {
        case BO_mod:
            // x % x is always 0
            if(left_constant && right_constant) {
                if(INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node)))) ==
                   INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_RIGHT(arg_node))))) {
                       toreturn = TBmakeConstant(TBmakeIntconst(0));
                   }
            } else if(left_varread && right_varread) {
                if(VARREAD_VARPOINTER(EXPR_CURRENT(BINOP_LEFT(arg_node))) ==
                   VARREAD_VARPOINTER(EXPR_CURRENT(BINOP_RIGHT(arg_node)))) {
                       toreturn = TBmakeConstant(TBmakeIntconst(0));
                   }
            }
            break;
        case BO_add:
            if(left_constant && right_varread) {
                if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node)))) == N_boolconst) {
                    constant = BINOP_LEFT(arg_node);
                    variable = BINOP_RIGHT(arg_node);
                } else {
                    break;
                }
            } else if(left_varread && right_constant) {
                if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_RIGHT(arg_node)))) == N_boolconst) {
                    variable = BINOP_LEFT(arg_node);
                    constant = BINOP_RIGHT(arg_node);
                } else {
                    break;
                }
            } else {
                break;
            }

            if(constant == NULL) {
                break;
            }

            if(BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(constant))) == TRUE) {
                // a OR TRUE is always TRUE
                toreturn = TBmakeConstant(TBmakeBoolconst(TRUE));
            } else {
                // a OR FALSE is always a
                toreturn = COPYdoCopy(EXPR_CURRENT(variable));
            }
            break;
        case BO_mul:
            if(left_constant && right_varread) {
                if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node)))) == N_boolconst) {
                    constant = BINOP_LEFT(arg_node);
                    variable = BINOP_RIGHT(arg_node);
                } else {
                    break;
                }
            } else if(left_varread && right_constant) {
                if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_RIGHT(arg_node)))) == N_boolconst) {
                    variable = BINOP_LEFT(arg_node);
                    constant = BINOP_RIGHT(arg_node);
                } else {
                    break;
                }
            } else {
                break;
            }

            if(constant == NULL) {
                break;
            }

            if(BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(constant))) == TRUE) {
                // a AND TRUE is always a
                toreturn = COPYdoCopy(EXPR_CURRENT(variable));
            } else {
                // a AND FALSE is always FALSE
                toreturn = TBmakeConstant(TBmakeBoolconst(FALSE));
            }
            break;
        default:
            break;
    }

    DBUG_RETURN(toreturn);
}

node* CFObinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFObinop");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    node* toreturn = arg_node;

    if((NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant &&
       NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant)) {
        switch(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node))))) {
            case N_intconst:
                toreturn = binop_int(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                TRAVpush(TR_free);
                FREEbinop(arg_node, NULL);
                TRAVpop();
                break;
            case N_floatconst:
                toreturn = binop_float(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                TRAVpush(TR_free);
                FREEbinop(arg_node, NULL);
                TRAVpop();
                break;
            case N_boolconst:
                toreturn = binop_bool(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                TRAVpush(TR_free);
                FREEbinop(arg_node, NULL);
                TRAVpop();
                break;
            default:
                DBUG_ASSERT((FALSE), "Invalid constant type");
                break;
        }
    } else if (NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant ||
               NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant) {
        node* other;
        node* constant;
        bool nodiv = FALSE;
        bool noremove = FALSE;

        if(NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant) {
            constant = BINOP_LEFT(arg_node);
            other = BINOP_RIGHT(arg_node);
            if(NODE_TYPE(EXPR_CURRENT(other)) == N_funcall) {
                noremove = TRUE;
            }
        } else {
            constant = BINOP_RIGHT(arg_node);
            other = BINOP_LEFT(arg_node);
            if(NODE_TYPE(EXPR_CURRENT(other)) == N_funcall) {
                noremove = TRUE;
            }
            if(BINOP_BINOPERATORS(arg_node) == BO_div || BINOP_BINOPERATORS(arg_node) == BO_mod) {
                nodiv = TRUE;
            }
        }

        switch(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(constant)))) {
            case N_intconst:
                if(INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(constant))) != 0) {
                    break;
                }
                if(nodiv) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Expression folds to division or modulo by zero!");
                }
                toreturn = binop_zero_int(arg_node, BINOP_BINOPERATORS(arg_node), other, noremove);
                if(toreturn != arg_node) {
                    TRAVpush(TR_free);
                    FREEbinop(arg_node, NULL);
                    TRAVpop();
                }
                break;
            case N_floatconst:
                if(FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(constant))) != 0.0) {
                    break;
                }
                if(nodiv) {
                    CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Expression folds to division or modulo by zero!");
                }
                toreturn = binop_zero_float(arg_node, BINOP_BINOPERATORS(arg_node), other, noremove);
                if(toreturn != arg_node) {
                    TRAVpush(TR_free);
                    FREEbinop(arg_node, NULL);
                    TRAVpop();
                }
                break;
            case N_boolconst:
                if(BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(constant))) != FALSE) {
                    toreturn = check_special_cases(arg_node);
                    if(toreturn != arg_node) {
                        TRAVpush(TR_free);
                        FREEbinop(arg_node, NULL);
                        TRAVpop();
                    }
                    break;
                }
                toreturn = binop_zero_bool(arg_node, BINOP_BINOPERATORS(arg_node), other, noremove);
                if(toreturn != arg_node) {
                    TRAVpush(TR_free);
                    FREEbinop(arg_node, NULL);
                    TRAVpop();
                }
                break;
            default:
                DBUG_ASSERT((FALSE), "Invalid constant type");
                break;
        }
    } else {
        toreturn = check_special_cases(arg_node);
        if(toreturn != arg_node) {
            TRAVpush(TR_free);
            FREEbinop(arg_node, NULL);
            TRAVpop();
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
            toreturn = COPYdoCopy(EXPR_CURRENT(CONDITIONALEXPR_IFBRANCH(arg_node)));
        } else {
            toreturn = COPYdoCopy(EXPR_CURRENT(CONDITIONALEXPR_ELSEBRANCH(arg_node)));
        }

        TRAVpush(TR_free);
        FREEconditionalexpr(arg_node, NULL);
        TRAVpop();

        DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(arg_node);
}

node* CFObranch(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFObranch");

    BRANCH_CONDITION(arg_node) = TRAVdo(BRANCH_CONDITION(arg_node), arg_info);

    BRANCH_THENBRANCH(arg_node) = TRAVopt(BRANCH_THENBRANCH(arg_node), arg_info);
    BRANCH_ELSEBRANCH(arg_node) = TRAVopt(BRANCH_ELSEBRANCH(arg_node), arg_info);

    if(NODE_TYPE(EXPR_CURRENT(BRANCH_CONDITION(arg_node))) == N_constant) {
        if(BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(BRANCH_CONDITION(arg_node)))) == TRUE) {
            if(BRANCH_THENBRANCH(arg_node) != NULL) {
                node* toreturn = COPYdoCopy(BRANCH_THENBRANCH(arg_node));

                TRAVpush(TR_free);
                FREEbranch(arg_node, NULL);
                TRAVpop();

                DBUG_RETURN(toreturn);
            } else {
                TRAVpush(TR_free);
                FREEbranch(arg_node, NULL);
                TRAVpop();

                DBUG_RETURN(TBmakeNoop());
            }
        } else {
            if(BRANCH_ELSEBRANCH(arg_node) != NULL) {
                node* toreturn = COPYdoCopy(BRANCH_ELSEBRANCH(arg_node));

                TRAVpush(TR_free);
                FREEbranch(arg_node, NULL);
                TRAVpop();

                DBUG_RETURN(toreturn);
            }  else {
                TRAVpush(TR_free);
                FREEbranch(arg_node, NULL);
                TRAVpop();

                DBUG_RETURN(TBmakeNoop());
            }
        }
    }

    DBUG_RETURN(arg_node);
}

node* CFOstatements(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOstatements");

    STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

    node* toreturn = arg_node;

    if(NODE_TYPE(STATEMENTS_CURRENT(arg_node)) == N_statements) {
        if(STATEMENTS_NEXT(arg_node) != NULL) {
            STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), arg_info);
            append_statement_to_tail(STATEMENTS_CURRENT(arg_node), STATEMENTS_NEXT(arg_node));
        }

        toreturn = STATEMENTS_CURRENT(arg_node);
        STATEMENTS_CURRENT(arg_node) = NULL;
        STATEMENTS_NEXT(arg_node) = NULL;
        FREEstatements(arg_node, NULL);
        TRAVdo(toreturn, arg_info);
    } else {
        STATEMENTS_NEXT(arg_node) = TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);
    }

    if(NODE_TYPE(STATEMENTS_CURRENT(toreturn)) == N_return) {
        if(STATEMENTS_NEXT(toreturn) != NULL) {
            TRAVpush(TR_free);
            FREEstatements(STATEMENTS_NEXT(toreturn), NULL);
            TRAVpop();

            STATEMENTS_NEXT(toreturn) = NULL;
        }
    }
    DBUG_RETURN(toreturn);
}

node* CFOvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOvarassign");

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    if(NODE_TYPE(EXPR_CURRENT(VARASSIGN_VALUE(arg_node))) == N_varread) {
        if(VARASSIGN_VARPOINTER(arg_node) == VARREAD_VARPOINTER(EXPR_CURRENT(VARASSIGN_VALUE(arg_node)))) {

            TRAVpush(TR_free);
            FREEvarassign(arg_node, NULL);
            TRAVpop();

            DBUG_RETURN(TBmakeNoop());
        }
    }

    DBUG_RETURN(arg_node);
}

node* CFOcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOcast");

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);

    if(NODE_TYPE(EXPR_CURRENT(CAST_TARGET(arg_node))) == N_constant) {
        node* constant = EXPR_CURRENT(CAST_TARGET(arg_node));
        node* toreturn = arg_node;

        switch(CAST_CASTTO(arg_node)) {
            case TP_int:
                toreturn = TBmakeConstant(TBmakeIntconst((int) FLOATCONST_VALUE(CONSTANT_CURRENT(constant))));
                break;
            case TP_float:
                toreturn = TBmakeConstant(TBmakeFloatconst((float) INTCONST_VALUE(CONSTANT_CURRENT(constant))));
                break;
            default:
                DBUG_ASSERT((FALSE), "Invalid cast type!");
                DBUG_RETURN(toreturn);
                break;
        }

        TRAVpush(TR_free);
        FREEcast(arg_node, NULL);
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
