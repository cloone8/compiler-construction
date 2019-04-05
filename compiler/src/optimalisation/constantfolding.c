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

#define BINOP_INTCONST_VAL(x) INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(x)))
#define BINOP_FLOATCONST_VAL(x) FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(x)))
#define BINOP_BOOLCONST_VAL(x) BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(x)))

/**
 * Binop folding function for a binop which acts on integers. This function folds
 * binops when one of the two sides is equal to 0.
 *
 * @param arg_node: The binop node
 * @param operator: The binop operator
 * @param other: The non-zero side of the binop
 * @param noremove: Whether the other side is something that can't be removed (like a function call with side effects)
 *
 * @return The folded binop
 */
static node* binop_zero_int(node* arg_node, binop operator, node* other, bool noremove) {
    DBUG_ENTER("binop_zero_int");
    node* toreturn = arg_node;

    switch(operator) {
        case BO_add:
        case BO_sub:
            // (x + 0) = x and (x - 0) = x
            toreturn = COPYdoCopy(EXPR_CURRENT(other));
            break;
        case BO_mul:
        case BO_div:
        case BO_mod:
            // (x * 0) = 0, (0 / x) = 0, (0 % x) = 0
            if(noremove) {
                // Binop contains a function call which can have side effects.
                DBUG_RETURN(toreturn);
            } else {
                toreturn = TBmakeConstant(TBmakeIntconst(0));
                break;
            }
        default:
            DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(toreturn);
}

/**
 * Binop folding function for a binop which acts on floats. This function folds
 * binops when one of the two sides is equal to 0.0
 *
 * @param arg_node: The binop node
 * @param operator: The binop operator
 * @param other: The non-zero side of the binop
 * @param noremove: Whether the other side is something that can't be removed (like a function call with side effects)
 *
 * @return The folded binop
 */
static node* binop_zero_float(node* arg_node, binop operator, node* other, bool noremove) {
    DBUG_ENTER("binop_zero_float");
    node* toreturn = arg_node;

    switch(operator) {
        case BO_add:
        case BO_sub:
            // (x + 0.0) = x and (x - 0.0) = x
            toreturn = COPYdoCopy(EXPR_CURRENT(other));
            break;
        case BO_mul:
        case BO_div:
            if(noremove) {
                // Binop contains a function call which can have side effects
                DBUG_RETURN(toreturn);
            } else {
                toreturn = TBmakeConstant(TBmakeFloatconst(0.0));
                break;
            }
        default:
            DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(toreturn);
}

/**
 * Binop folding function for a binop which acts on booleans. This function folds
 * binops when one of the two sides is equal to FALSE.
 *
 * @param arg_node: The binop node
 * @param operator: The binop operator
 * @param other: The non-zero side of the binop
 * @param noremove: Whether the other side is something that can't be removed (like a function call with side effects)
 *
 * @return The folded binop
 */
static node* binop_zero_bool(node* arg_node, binop operator, node* other, bool noremove) {
    DBUG_ENTER("binop_zero_bool");
    node* toreturn = arg_node;

    switch(operator) {
        case BO_add:
            // (x OR false) = x
            toreturn = COPYdoCopy(EXPR_CURRENT(other));
            break;
        case BO_mul:
            // (x AND false) = false
            if(noremove) {
                // Binop contains a function call which can have side effects
                DBUG_RETURN(toreturn);
            }
            toreturn = TBmakeConstant(TBmakeBoolconst(FALSE));
            break;
        default:
            DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(toreturn);
}

/**
 * Binop folding function for a binop which acts on integers.
 *
 * @param operator: The binop operator
 * @param left: The left side of the binop
 * @param right: The right side of the binop
 *
 * @return The folded binop
 */
static node* binop_int(binop operator, node* left, node* right) {
    DBUG_ENTER("binop_int");
    node* toreturn = NULL;
    int new_intval = -1;
    bool new_boolval = FALSE;

    switch(operator) {
        case BO_add:
            new_intval = BINOP_INTCONST_VAL(left) + BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_div:
            new_intval = BINOP_INTCONST_VAL(left) / BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_eq:
            new_boolval = BINOP_INTCONST_VAL(left) == BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_ge:
            new_boolval = BINOP_INTCONST_VAL(left) >= BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_gt:
            new_boolval = BINOP_INTCONST_VAL(left) > BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_le:
            new_boolval = BINOP_INTCONST_VAL(left) <= BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_lt:
            new_boolval = BINOP_INTCONST_VAL(left) < BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_mod:
            new_intval = BINOP_INTCONST_VAL(left) % BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_mul:
            new_intval = BINOP_INTCONST_VAL(left) * BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        case BO_ne:
            new_boolval = BINOP_INTCONST_VAL(left) != BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_sub:
            new_intval = BINOP_INTCONST_VAL(left) - BINOP_INTCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeIntconst(new_intval));
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid binop operator type!");
            break;
    }

    DBUG_RETURN(toreturn);
}

/**
 * Binop folding function for a binop which acts on floats.
 *
 * @param operator: The binop operator
 * @param left: The left side of the binop
 * @param right: The right side of the binop
 *
 * @return The folded binop
 */
static node* binop_float(binop operator, node* left, node* right) {
    DBUG_ENTER("binop_float");
    node* toreturn = NULL;
    float new_fltval = -1.0;
    bool new_boolval = FALSE;

    switch(operator) {
        case BO_add:
            new_fltval = BINOP_FLOATCONST_VAL(left) + BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        case BO_div:
            new_fltval = BINOP_FLOATCONST_VAL(left) / BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        case BO_eq:
            new_boolval = BINOP_FLOATCONST_VAL(left) == BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_ge:
            new_boolval = BINOP_FLOATCONST_VAL(left) >= BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_gt:
            new_boolval = BINOP_FLOATCONST_VAL(left) > BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_le:
            new_boolval = BINOP_FLOATCONST_VAL(left) <= BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_lt:
            new_boolval = BINOP_FLOATCONST_VAL(left) < BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_mul:
            new_fltval = BINOP_FLOATCONST_VAL(left) * BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        case BO_ne:
            new_boolval = BINOP_FLOATCONST_VAL(left) != BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_sub:
            new_fltval = BINOP_FLOATCONST_VAL(left) - BINOP_FLOATCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeFloatconst(new_fltval));
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid binop operator type!");
            break;
    }

    DBUG_RETURN(toreturn);
}

/**
 * Binop folding function for a binop which acts on booleans.
 *
 * @param operator: The binop operator
 * @param left: The left side of the binop
 * @param right: The right side of the binop
 *
 * @return The folded binop
 */
static node* binop_bool(binop operator, node* left, node* right) {
    DBUG_ENTER("binop_bool");
    node* toreturn = NULL;
    bool new_boolval = FALSE;

    switch(operator) {
        case BO_add:
            new_boolval = BINOP_BOOLCONST_VAL(left) || BINOP_BOOLCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_mul:
            new_boolval = BINOP_BOOLCONST_VAL(left) && BINOP_BOOLCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_eq:
            new_boolval = BINOP_BOOLCONST_VAL(left) == BINOP_BOOLCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        case BO_ne:
            new_boolval = BINOP_BOOLCONST_VAL(left) != BINOP_BOOLCONST_VAL(right);
            toreturn = TBmakeConstant(TBmakeBoolconst(new_boolval));
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid binop operator type!");
            break;
    }

    DBUG_RETURN(toreturn);
}

/**
 * Checks for any special cases when the binop is a modulo binop
 *
 * @param arg_node: The binop node
 * @return The folded binop
 */
static node* special_case_mod(node* arg_node) {
    DBUG_ENTER("special_case_mod");
    node* toreturn = arg_node;

    bool left_constant = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant;
    bool right_constant = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant;

    bool left_varread = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_varread;
    bool right_varread = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_varread;

    // x % x is always 0
    if(left_constant && right_constant) {
        if(BINOP_INTCONST_VAL(BINOP_LEFT(arg_node)) == BINOP_INTCONST_VAL(BINOP_RIGHT(arg_node))) {
                toreturn = TBmakeConstant(TBmakeIntconst(0));
        }
    } else if(left_varread && right_varread) {
        if(VARREAD_VARPOINTER(EXPR_CURRENT(BINOP_LEFT(arg_node))) ==
            VARREAD_VARPOINTER(EXPR_CURRENT(BINOP_RIGHT(arg_node)))) {
                toreturn = TBmakeConstant(TBmakeIntconst(0));
        }
    }

    DBUG_RETURN(toreturn);
}

/**
 * Checks for any special cases when the binop is an addition binop
 *
 * @param arg_node: The binop node
 * @return The folded binop
 */
static node* special_case_add(node* arg_node) {
    DBUG_ENTER("special_case_add");

    bool left_constant = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant;
    bool right_constant = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant;

    bool left_varread = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_varread;
    bool right_varread = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_varread;

    node* constant;
    node* variable;

    if(left_constant && right_varread) {
        if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node)))) == N_boolconst) {
            constant = BINOP_LEFT(arg_node);
            variable = BINOP_RIGHT(arg_node);
        } else {
            // This special case works only for booleans
            DBUG_RETURN(arg_node);
        }
    } else if(left_varread && right_constant) {
        if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_RIGHT(arg_node)))) == N_boolconst) {
            variable = BINOP_LEFT(arg_node);
            constant = BINOP_RIGHT(arg_node);
        } else {
            // This special case works only for booleans
            DBUG_RETURN(arg_node);
        }
    } else {
        // Binop not a variable and a constant so no special cases are
        // possible
        DBUG_RETURN(arg_node);
    }

    if(BINOP_BOOLCONST_VAL(constant) == TRUE) {
        // a OR TRUE is always TRUE
        DBUG_RETURN(TBmakeConstant(TBmakeBoolconst(TRUE)));
    } else {
        // a OR FALSE is always a
        DBUG_RETURN(COPYdoCopy(EXPR_CURRENT(variable)));
    }
}

/**
 * Checks for any special cases when the binop is a multiplication binop
 *
 * @param arg_node: The binop node
 * @return The folded binop
 */
static node* special_case_mul(node* arg_node) {
    DBUG_ENTER("special_case_mul");

    bool left_constant = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant;
    bool right_constant = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant;

    bool left_varread = NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_varread;
    bool right_varread = NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_varread;

    node* constant;
    node* variable;

    if(left_constant && right_varread) {
        if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node)))) == N_boolconst) {
            constant = BINOP_LEFT(arg_node);
            variable = BINOP_RIGHT(arg_node);
        } else {
            // This special case works only for booleans
            DBUG_RETURN(arg_node);
        }
    } else if(left_varread && right_constant) {
        if(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_RIGHT(arg_node)))) == N_boolconst) {
            variable = BINOP_LEFT(arg_node);
            constant = BINOP_RIGHT(arg_node);
        } else {
            // This special case works only for booleans
            DBUG_RETURN(arg_node);
        }
    } else {
        // Binop not a variable and a constant so no special cases are
        // possible
        DBUG_RETURN(arg_node);
    }

    if(BINOP_BOOLCONST_VAL(constant) == TRUE) {
        // a AND TRUE is always a
        DBUG_RETURN(COPYdoCopy(EXPR_CURRENT(variable)));
    } else {
        // a AND FALSE is always FALSE
        DBUG_RETURN(TBmakeConstant(TBmakeBoolconst(FALSE)));
    }
}

/**
 * Binop folding function for special cases.
 *
 * @param arg_node: The binop node
 * @return The folded binop
 */
static node* check_special_cases(node* arg_node) {
    DBUG_ENTER("check_special_cases");
    node* toreturn = arg_node;

    switch(BINOP_BINOPERATORS(arg_node)) {
        case BO_mod:
            toreturn = special_case_mod(toreturn);
            break;
        case BO_add:
            toreturn = special_case_add(toreturn);
            break;
        case BO_mul:
            toreturn = special_case_mul(toreturn);
            break;
        default:
            break;
    }

    DBUG_RETURN(toreturn);
}

/**
 * Folds the given binop node when only one side of the binop is
 * a constant type node
 *
 * @param arg_node: The binop node
 * @param arg_info: The info struct
 * @return The folded binop
 */
static node* binop_single_constant(node* arg_node) {
    DBUG_ENTER("binop_single_constant");
    node* toreturn = arg_node;
    node* other;
    node* constant;
    bool nozero = FALSE;
    bool noremove = FALSE;

    // Identify which of the two sides is the constant
    if(NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant) {
        constant = BINOP_LEFT(arg_node);
        other = BINOP_RIGHT(arg_node);
        if(NODE_TYPE(EXPR_CURRENT(other)) == N_funcall) {
            // Function calls may have side effects and can't be removed
            noremove = TRUE;
        }
    } else {
        constant = BINOP_RIGHT(arg_node);
        other = BINOP_LEFT(arg_node);
        if(NODE_TYPE(EXPR_CURRENT(other)) == N_funcall) {
            // Function calls may have side effects and can't be removed
            noremove = TRUE;
        }
        if(BINOP_BINOPERATORS(arg_node) == BO_div || BINOP_BINOPERATORS(arg_node) == BO_mod) {
            // Dividing by zero or modulo by zero is undefined
            nozero = TRUE;
        }
    }

    switch(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(constant)))) {
        case N_intconst:
            if(BINOP_INTCONST_VAL(constant) != 0) {
                // Can't do any folding when the constant isn't zero
                break;
            }
            if(nozero) {
                CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Expression folds to division or modulo by zero!");
            }
            toreturn = binop_zero_int(arg_node, BINOP_BINOPERATORS(arg_node), other, noremove);
            break;
        case N_floatconst:
            if(BINOP_FLOATCONST_VAL(constant) != 0.0) {
                // Can't do any folding when the constant isn't zero
                break;
            }
            if(nozero) {
                CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Expression folds to division or modulo by zero!");
            }
            toreturn = binop_zero_float(arg_node, BINOP_BINOPERATORS(arg_node), other, noremove);
            break;
        case N_boolconst:
            if(BINOP_BOOLCONST_VAL(constant) != FALSE) {
                toreturn = check_special_cases(arg_node);
                break;
            }
            toreturn = binop_zero_bool(arg_node, BINOP_BINOPERATORS(arg_node), other, noremove);
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid constant type");
            break;
    }

    DBUG_RETURN(toreturn);
}

/**
 * Folds the given binop node and all its children, if possible
 *
 * @param arg_node: The binop node
 * @param arg_info: The info struct
 * @return The folded binop
 */
node* CFObinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFObinop");
    UNUSED_VAR(arg_info);

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), NULL);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), NULL);

    node* toreturn = arg_node;

    if((NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant &&
       NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant)) {
        // Both sides of the binop are constants and can thus always be folded
        switch(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(BINOP_LEFT(arg_node))))) {
            case N_intconst:
                toreturn = binop_int(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                break;
            case N_floatconst:
                toreturn = binop_float(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                break;
            case N_boolconst:
                toreturn = binop_bool(BINOP_BINOPERATORS(arg_node), BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node));
                break;
            default:
                DBUG_ASSERT((FALSE), "Invalid constant type");
                break;
        }
    } else if (NODE_TYPE(EXPR_CURRENT(BINOP_LEFT(arg_node))) == N_constant ||
               NODE_TYPE(EXPR_CURRENT(BINOP_RIGHT(arg_node))) == N_constant) {
        // One side of the binop is a constant and the binop may be able to be
        // folded
        toreturn = binop_single_constant(arg_node);
    } else {
        toreturn = check_special_cases(arg_node);
    }

    if(toreturn != arg_node) {
        TRAVpush(TR_free);
        FREEbinop(arg_node, NULL);
        TRAVpop();
    }

    DBUG_RETURN(toreturn);
}

/**
 * Folds the given monop and all its children.
 *
 * @param arg_node: The monop node to fold
 * @param arg_info: Should be NULL
 *
 * @return: The folded monop node
 */
node* CFOmonop(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOmonop");
    UNUSED_VAR(arg_info);

    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), NULL);

    node* toreturn = arg_node;

    if(NODE_TYPE(EXPR_CURRENT(MONOP_TARGET(arg_node))) == N_constant) {
        // Can only fold monops which act on constants
        switch(MONOP_TYPE(arg_node)) {
            case MO_neg:
                switch(NODE_TYPE(CONSTANT_CURRENT(EXPR_CURRENT(MONOP_TARGET(arg_node))))) {
                    case N_intconst:
                        toreturn = COPYdoCopy(EXPR_CURRENT(MONOP_TARGET(arg_node)));
                        INTCONST_VALUE(CONSTANT_CURRENT(toreturn)) *= -1;
                        break;
                    case N_floatconst:
                        toreturn = COPYdoCopy(EXPR_CURRENT(MONOP_TARGET(arg_node)));
                        FLOATCONST_VALUE(CONSTANT_CURRENT(toreturn)) *= -1;
                        break;
                    default:
                        DBUG_ASSERT((FALSE), "Invalid monop constant type");
                        break;
                }
                break;
            case MO_not:
                toreturn = COPYdoCopy(EXPR_CURRENT(MONOP_TARGET(arg_node)));
                BOOLCONST_VALUE(CONSTANT_CURRENT(toreturn)) = !BOOLCONST_VALUE(CONSTANT_CURRENT(toreturn));
                break;
            case MO_unknown:
                DBUG_ASSERT((FALSE), "Invalid monop type");
                break;
        }

        TRAVpush(TR_free);
        FREEmonop(arg_node, NULL);
        TRAVpop();
    }

    DBUG_RETURN(toreturn);
}

/**
 * Folds the given conditional expression when the condition is a constant
 *
 * @param arg_node: The conditional expression to fold
 * @param arg_info: Should be NULL
 *
 * @return: The folded conditional expression
 */
node* CFOconditionalexpr(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOconditionalexpr");
    UNUSED_VAR(arg_info);

    CONDITIONALEXPR_CONDITION(arg_node) = TRAVdo(CONDITIONALEXPR_CONDITION(arg_node), NULL);
    CONDITIONALEXPR_IFBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_IFBRANCH(arg_node), NULL);
    CONDITIONALEXPR_ELSEBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_ELSEBRANCH(arg_node), NULL);

    if(NODE_TYPE(EXPR_CURRENT(CONDITIONALEXPR_CONDITION(arg_node))) == N_constant) {
        node* toreturn;

        if(BINOP_BOOLCONST_VAL(CONDITIONALEXPR_CONDITION(arg_node)) == TRUE) {
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

/**
 * Folds the given branch when the condition is a constant
 *
 * @param arg_node: The branch to fold
 * @param arg_info: Should be NULL
 *
 * @return: The folded branch
 */
node* CFObranch(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFObranch");
    UNUSED_VAR(arg_info);

    BRANCH_CONDITION(arg_node) = TRAVdo(BRANCH_CONDITION(arg_node), NULL);

    BRANCH_THENBRANCH(arg_node) = TRAVopt(BRANCH_THENBRANCH(arg_node), NULL);
    BRANCH_ELSEBRANCH(arg_node) = TRAVopt(BRANCH_ELSEBRANCH(arg_node), NULL);

    if(NODE_TYPE(EXPR_CURRENT(BRANCH_CONDITION(arg_node))) == N_constant) {
        // Although this returns an N_statement type node, this is handled
        // in the statement traversal, where the returned statements
        // are appended into the main list of statements
        if(BINOP_BOOLCONST_VAL(BRANCH_CONDITION(arg_node)) == TRUE) {
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

/**
 * Included to handle folded branches. This checks if the STATEMENTS_CURRENT of
 * this statement node is equal to another block of statements (which normally
 * can't happen), and if it is, replaces itself with that block of statements.
 *
 * @param arg_node: The statements node to check
 * @param arg_info: Should be NULL
 *
 * @return: The updated statements node
 */
node* CFOstatements(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOstatements");
    UNUSED_VAR(arg_info);

    STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), NULL);

    node* toreturn = arg_node;

    if(NODE_TYPE(STATEMENTS_CURRENT(arg_node)) == N_statements) {
        if(STATEMENTS_NEXT(arg_node) != NULL) {
            STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), NULL);
            append_statement_to_tail(STATEMENTS_CURRENT(arg_node), STATEMENTS_NEXT(arg_node));
        }

        toreturn = STATEMENTS_CURRENT(arg_node);
        STATEMENTS_CURRENT(arg_node) = NULL;
        STATEMENTS_NEXT(arg_node) = NULL;
        FREEstatements(arg_node, NULL);
        TRAVdo(toreturn, NULL);
    } else {
        STATEMENTS_NEXT(arg_node) = TRAVopt(STATEMENTS_NEXT(arg_node), NULL);
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

/**
 * Folds the given varassign statement when a variable is assigned to its own value
 *
 * @param arg_node: The varassign to fold
 * @param arg_info: Should be NULL
 *
 * @return: itself, or a NOOP statement when it is a self-assign
 */
node* CFOvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOvarassign");
    UNUSED_VAR(arg_info);

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), NULL);

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

/**
 * Folds the given cast when the target is a constant
 *
 * @param arg_node: The cast to fold
 * @param arg_info: Should be NULL
 *
 * @return: The cast constant
 */
node* CFOcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("CFOcast");
    UNUSED_VAR(arg_info);

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), NULL);

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
                // Bool casts should have already been handled in an earlier phase
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

/**
 * Starts the folding traversal
 *
 * @param syntaxtree: The syntaxtree to fold
 *
 * @return: The folded syntaxtree
 */
node* CFOdo(node* syntaxtree) {
    DBUG_ENTER("CFOdo");

    TRAVpush(TR_cfo);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
