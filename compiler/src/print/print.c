
/**
 * @file print.c
 *
 * Functions needed by print traversal.
 *
 */

/**
 * @defgroup print Print Functions.
 *
 * Functions needed by print traversal.
 *
 * @{
 */


#include "print.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"
#include "ctinfo.h"
#include "str.h"


/*
 * INFO structure
 */
struct INFO {
    bool firsterror;
    int indentlevel;
};

#define INFO_FIRSTERROR(n) ((n)->firsterror)
#define INFO_INDENTLEVEL(n) ((n)->indentlevel)
#define PRINT_INDENTS(__indents_info_struct__) for(int __indents_counter__ = 0; __indents_counter__ < (__indents_info_struct__)->indentlevel * 4; __indents_counter__++) \
                                                    putchar(' ');

static info *MakeInfo() {
    info *result;

    result = MEMmalloc(sizeof(info));

    INFO_FIRSTERROR(result) = FALSE;
    INFO_INDENTLEVEL(result) = 0;

    return result;
}


static info *FreeInfo(info *info) {
    info = MEMfree(info);

    return info;
}

/** <!--******************************************************************-->
 *
 * @fn PRTbinop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTbinop (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTbinop");

    putchar('(');
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);

    switch (BINOP_BINOPERATORS(arg_node))
    {
    case BO_add:
        printf(" + ");
        break;
    case BO_sub:
        printf(" - ");
        break;
    case BO_mul:
        printf(" * ");
        break;
    case BO_div:
        printf(" / ");
        break;
    case BO_mod:
        printf(" %% ");
        break;
    case BO_lt:
        printf(" < ");
        break;
    case BO_le:
        printf(" <= ");
        break;
    case BO_ge:
        printf(" >= ");
        break;
    case BO_gt:
        printf(" > ");
        break;
    case BO_eq:
        printf(" == ");
        break;
    case BO_ne:
        printf(" != ");
        break;
    case BO_and:
        printf(" && ");
        break;
    case BO_or:
        printf(" || ");
        break;
    default:
        printf(" UNKNOWN_LOGIC_OPERATOR ");
    }

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    putchar(')');

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTdeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTdeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTdeclaration");

    DECLARATION_CURRENT(arg_node) = TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);

    if(DECLARATION_NEXT(arg_node) != NULL)
        DECLARATION_NEXT(arg_node) = TRAVdo(DECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTarrayvarassign
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTarrayvarassign (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTarrayvarassign");
    PRINT_INDENTS(arg_info);

    printf("%s[", ARRAYVARASSIGN_VARNAME(arg_node));

    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    printf("] = ");
    ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTarrayvarread
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTarrayvarread (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTarrayvarread");

    printf("%s[", ARRAYVARREAD_ARRAYVARNAME(arg_node));

    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    printf("]");

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTvarassign
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTvarassign (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTvarassign");

    printf("%s = ", VARASSIGN_VARNAME(arg_node));

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTvarread
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTvarread (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTvarread");

    printf("%s", VARREAD_VARNAME(arg_node));

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTcast
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTcast (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTcast");

    switch(CAST_CASTTO(arg_node)) {
        case TP_bool:
            printf("(bool) ");
            break;
        case TP_float:
            printf("(float) ");
            break;
        case TP_int:
            printf("(int) ");
            break;
        case TP_void:
            printf("(void) ");
            break;
        default:
            printf("(UNKNOWN_TYPE_CAST) ");
    }

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTboolconst
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTboolconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTboolconst");

    switch (BOOLCONST_VALUE(arg_node))
    {
        case TRUE:
            printf("TRUE");
            break;
        case FALSE:
            printf("FALSE");
            break;
        default:
            printf("INVALID_BOOL_CONST_VALUE");
    }

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTintconst
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTintconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTintconst");

    printf("%d", INTCONST_VALUE(arg_node));

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTfloatconst
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTfloatconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTfloatconst");

    printf("%.10f", FLOATCONST_VALUE(arg_node));

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTconstant
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* PRTconstant (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTconstant");

    CONSTANT_CURRENT(arg_node) = TRAVdo(CONSTANT_CURRENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTerror
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTerror (node * arg_node, info * arg_info) {
    bool first_error;

    DBUG_ENTER ("PRTerror");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    first_error = INFO_FIRSTERROR(arg_info);

    if((global.outfile != NULL)
            && (ERROR_ANYPHASE(arg_node) == global.compiler_anyphase)) {

        if (first_error) {
            printf ("\n/******* BEGIN TREE CORRUPTION ********\n");
            INFO_FIRSTERROR(arg_info) = FALSE;
        }

        printf ("%s\n", ERROR_MESSAGE(arg_node));

        if (ERROR_NEXT (arg_node) != NULL) {
            TRAVopt (ERROR_NEXT (arg_node), arg_info);
        }

        if (first_error) {
            printf ("********  END TREE CORRUPTION  *******/\n");
            INFO_FIRSTERROR(arg_info) = TRUE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfunction
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTfunction (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTfunction");
    PRINT_INDENTS(arg_info);

    // Only used when the definition is actually given. Otherwise remains empty
    char* prefix = "";

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        if(FUNCTIONDEFINITION_ISEXPORT(FUNCTION_DEFINITION(arg_node))) {
            prefix = "export ";
        }
    } else {
        prefix = "extern ";
    }

    char* functiontype = NULL;

    switch(FUNCTION_TYPE(arg_node)) {
        case TP_bool:
            functiontype = "bool";
            break;
        case TP_float:
            functiontype = "float";
            break;
        case TP_void:
            functiontype = "void";
            break;
        case TP_int:
            functiontype = "int";
            break;
        default:
            functiontype = "UNKNOWN_FUNCTION_TYPE";
    }


    printf("%s%s %s(", prefix, functiontype, FUNCTION_NAME(arg_node));

    if(FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);

    putchar(')');

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        printf(" {\n");
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
        PRINT_INDENTS(arg_info);
        printf("}\n");
    } else {
        printf(";\n");
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTbasicparameter
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTbasicparameter (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTbasicparameter");
    char* type = "";

    switch(BASICPARAMETER_TYPE(arg_node)) {
        case TP_bool:
            type = "bool";
            break;
        case TP_float:
            type = "float";
            break;
        case TP_int:
            type = "int";
            break;
        default:
            type = "INVALID_PARAMETER_TYPE";
    }

    printf("%s %s", type, BASICPARAMETER_NAME(arg_node));

    if(BASICPARAMETER_NEXT(arg_node) != NULL) {
        printf(", ");
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTarrayparameter
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTarrayparameter (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTbasicparameter");

    char* type = "";

    switch(ARRAYPARAMETER_TYPE(arg_node)) {
        case TP_bool:
            type = "bool";
            break;
        case TP_float:
            type = "float";
            break;
        case TP_int:
            type = "int";
            break;
        default:
            type = "INVALID_PARAMETER_TYPE";
    }

    printf("%s%s", type, "[");

    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), arg_info);

    putchar(']');
    printf(" %s", ARRAYPARAMETER_NAME(arg_node));

    if(ARRAYPARAMETER_NEXT(arg_node) != NULL) {
        printf(", ");
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTarraydimensionvars
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTarraydimensionvars (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTarraydimensionvars");
    printf("%s", ARRAYDIMENSIONVARS_NAME(arg_node));

    if(ARRAYDIMENSIONVARS_NEXT(arg_node) != NULL) {
        printf(", ");
        ARRAYDIMENSIONVARS_NEXT(arg_node) = TRAVdo(ARRAYDIMENSIONVARS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfunctiondefinition
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTfunctiondefinition (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTfunctiondefinition");

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfunctionbody
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTfunctionbody (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTfunctionbody");
    INFO_INDENTLEVEL(arg_info) += 1;

    if(FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL) {
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_LOCALFUNCTIONS(arg_node) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
        FUNCTIONBODY_STATEMENTS(arg_node) = TRAVdo(FUNCTIONBODY_STATEMENTS(arg_node), arg_info);
    }

    INFO_INDENTLEVEL(arg_info) -= 1;
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTstatements
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTstatements (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTstatements");

    // These can also be expressions, so we can't just print the indents there
    switch(NODE_TYPE(STATEMENTS_CURRENT(arg_node))) {
        case N_funcall:
        case N_varassign:
            PRINT_INDENTS(arg_info);
            break;
        default:
            break;
    }

    STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

    if (NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_for &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_branch &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_while &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_dowhile) {
        printf(";\n");
    }

    if(STATEMENTS_NEXT(arg_node) != NULL) {
        STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTbasicvardeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTbasicvardeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTbasicvardeclaration");
    PRINT_INDENTS(arg_info);

    char* type = "";

    switch(BASICVARDECLARATION_TYPE(arg_node)) {
        case TP_bool:
            type = "bool";
            break;
        case TP_float:
            type = "float";
            break;
        case TP_int:
            type = "int";
            break;
        default:
            type = "INVALID_BASICVARDEC_TYPE";
    }

    printf("%s %s", type, BASICVARDECLARATION_VARNAME(arg_node));

    if(BASICVARDECLARATION_INITVAL(arg_node) != NULL) {

        printf(" = ");
        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);
    }

    printf(";\n");

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTarrayvardeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTarrayvardeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTarrayvardeclaration");
    PRINT_INDENTS(arg_info);

    printf("%s[", ARRAYVARDECLARATION_VARNAME(arg_node));

    ARRAYVARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(ARRAYVARDECLARATION_DIMENSIONS(arg_node), arg_info);

    putchar(']');


    if(ARRAYVARDECLARATION_INITVAL(arg_node) != NULL) {
        printf(" = ");
        ARRAYVARDECLARATION_INITVAL(arg_node) = TRAVdo(ARRAYVARDECLARATION_INITVAL(arg_node), arg_info);
    }

    printf(";\n");

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTexprarray
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTexprarray (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTexprarray");

    if(NODE_TYPE(EXPRARRAY_CURRENT(arg_node)) == N_exprarray) {
        putchar('[');
        EXPRARRAY_CURRENT(arg_node) = TRAVdo(EXPRARRAY_CURRENT(arg_node), arg_info);
        putchar(']');
    } else {
        EXPRARRAY_CURRENT(arg_node) = TRAVdo(EXPRARRAY_CURRENT(arg_node), arg_info);
    }

    if(EXPRARRAY_NEXT(arg_node) != NULL) {
        printf(", ");
        EXPRARRAY_NEXT(arg_node) = TRAVdo(EXPRARRAY_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfor
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTfor (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTfor");

    PRINT_INDENTS(arg_info);
    printf("for(int %s = ", BASICVARDECLARATION_VARNAME(FOR_COUNTER(arg_node)));

    BASICVARDECLARATION_INITVAL(FOR_COUNTER(arg_node)) =
        TRAVdo(BASICVARDECLARATION_INITVAL(FOR_COUNTER(arg_node)), arg_info);

    printf(", ");
    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL) {
          printf(", ");
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
    }

    printf(") {\n");
    INFO_INDENTLEVEL(arg_info) += 1;
    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    PRINT_INDENTS(arg_info);
    printf("}\n");

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTwhile
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTwhile (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTwhile");

    PRINT_INDENTS(arg_info);
    printf("while(");

    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);

    printf(") {\n");

    INFO_INDENTLEVEL(arg_info) += 1;
    WHILE_CODE(arg_node) = TRAVdo(WHILE_CODE(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    PRINT_INDENTS(arg_info);
    printf("}\n");

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTdowhile
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTdowhile(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTdowhile");

    PRINT_INDENTS(arg_info);
    printf("do {\n");

    INFO_INDENTLEVEL(arg_info) += 1;
    DOWHILE_CODE(arg_node) = TRAVdo(DOWHILE_CODE(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    PRINT_INDENTS(arg_info);
    printf("} while(");
    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);
    printf(");\n");

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTglobal
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTglobal(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTglobal");
    char* exportextern;
    char* type;

    switch (GLOBAL_TYPE(arg_node)) {
        case TP_int:
            type = "int";
            break;
        case TP_float:
            type = "float";
            break;
        case TP_bool:
            type = "bool";
            break;
        default:
            type = "INVALID_GLOBAL_TYPE";
            break;
    }

    if (GLOBAL_DEFINITION(arg_node) != NULL) {
        if(GLOBALDEFINITION_ISEXPORT(GLOBAL_DEFINITION(arg_node))) {
            exportextern = "export ";
        } else {
            exportextern = "";
        }
    } else {
        exportextern = "extern ";
    }

    printf("%s%s %s", exportextern, type, GLOBAL_NAME(arg_node));

    if(GLOBAL_DIMENSIONS(arg_node) != NULL) {
        putchar('[');
        GLOBAL_DIMENSIONS(arg_node) = TRAVdo(GLOBAL_DIMENSIONS(arg_node), arg_info);
        putchar(']');
    }

    if(GLOBAL_DEFINITION(arg_node) != NULL) {
        GLOBAL_DEFINITION(arg_node) = TRAVdo(GLOBAL_DEFINITION(arg_node), arg_info);
    }

    printf(";\n");
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTglobaldefinition
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTglobaldefinition (node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTglobaldefinition");

    if(GLOBALDEFINITION_VALUES(arg_node) != NULL) {
        printf(" = ");
        GLOBALDEFINITION_VALUES(arg_node) = TRAVdo(GLOBALDEFINITION_VALUES(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfuncall
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTfuncall(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTfuncall");

    printf("%s(", FUNCALL_FUNCTIONNAME(arg_node));

    if(FUNCALL_PARAMS(arg_node) != NULL) {
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);
    }

    putchar(')');

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfuncallparameter
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTfuncallparameter(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTfuncallparameter");

    FUNCALLPARAMETER_VALUE(arg_node) = TRAVdo(FUNCALLPARAMETER_VALUE(arg_node), arg_info);

    if (FUNCALLPARAMETER_NEXT(arg_node) != NULL) {
        printf(", ");
        FUNCALLPARAMETER_NEXT(arg_node) = TRAVdo(FUNCALLPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTreturn
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTreturn(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTreturn");
    PRINT_INDENTS(arg_info);
    printf("return");

    if(RETURN_VALUE(arg_node) != NULL) {
        putchar(' ');
        RETURN_VALUE(arg_node) = TRAVdo(RETURN_VALUE(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTbranch
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTbranch(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTbranch");
    PRINT_INDENTS(arg_info);

    printf("if(");


    BRANCH_CONDITION(arg_node) = TRAVdo(BRANCH_CONDITION(arg_node), arg_info);


    printf(") {\n");
    INFO_INDENTLEVEL(arg_info) += 1;
    BRANCH_THENBRANCH(arg_node) = TRAVdo(BRANCH_THENBRANCH(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    if (BRANCH_ELSEBRANCH(arg_node) != NULL) {

        PRINT_INDENTS(arg_info);
        printf("} else {\n");

        INFO_INDENTLEVEL(arg_info) += 1;
        BRANCH_ELSEBRANCH(arg_node) = TRAVdo(BRANCH_ELSEBRANCH(arg_node), arg_info);
        INFO_INDENTLEVEL(arg_info) -= 1;
    }

    PRINT_INDENTS(arg_info);
    printf("}\n");

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTexpr
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTexpr(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTexpr");

    EXPR_CURRENT(arg_node) = TRAVdo(EXPR_CURRENT(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTmonop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTmonop(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTmonop");

    putchar('(');

    switch (MONOP_TYPE(arg_node)) {
        case MO_not:
            putchar('!');
            break;
        case MO_neg:
            putchar('-');
            break;
        default:
            printf("INVALID_MONOP_TYPE");
    }

    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), arg_info);

    putchar(')');
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfunctionvardeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTfunctionvardeclaration(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTfunctionvardeclaration");

    FUNCTIONVARDECLARATION_CURRENT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_CURRENT(arg_node), arg_info);

    if (FUNCTIONVARDECLARATION_NEXT(arg_node) != NULL) {
        FUNCTIONVARDECLARATION_NEXT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTlocalfunction
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTlocalfunction(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTlocalfunction");
    PRINT_INDENTS(arg_info);

    printf("__LOCAL FUNCTION__\n");

    LOCALFUNCTION_CURRENT(arg_node) = TRAVdo(LOCALFUNCTION_CURRENT(arg_node), arg_info);

    if (LOCALFUNCTION_NEXT(arg_node) != NULL) {
        LOCALFUNCTION_NEXT(arg_node) = TRAVdo(LOCALFUNCTION_NEXT(arg_node), arg_info);
    }


    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTprogram
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* PRTprogram(node * arg_node, info * arg_info) {
    DBUG_ENTER ("PRTlocalfunction");

    printf("Printing program:\n");

    PROGRAM_FIRST(arg_node) = TRAVdo(PROGRAM_FIRST(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Prints the given syntaxtree
 *
 * @param syntaxtree a node structure
 *
 * @return the unchanged nodestructure
 ******************************************************************************/

node* PRTdoPrint(node *syntaxtree) {
    info *info;

    DBUG_ENTER("PRTdoPrint");

    DBUG_ASSERT((syntaxtree!= NULL), "PRTdoPrint called with empty syntaxtree");
    DBUG_ASSERT((NODE_TYPE(syntaxtree) == N_program), "First node of syntaxtree not a 'program' node");

    printf("\n\n------------------------------\n\n");

    info = MakeInfo();

    TRAVpush(TR_prt);

    syntaxtree = TRAVdo(syntaxtree, info);

    TRAVpop();

    info = FreeInfo(info);

    printf("\n------------------------------\n\n");

    DBUG_RETURN(syntaxtree);
}

/**
 * @}
 */
