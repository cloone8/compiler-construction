
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
#include "util.h"
#include "node_basic.h"

/*
 * INFO structure
 */
struct INFO {
    bool firsterror;
    int indentlevel;
};

#define INFO_FIRSTERROR(n) ((n)->firsterror)
#define INFO_INDENTLEVEL(n) ((n)->indentlevel)

#ifdef _MSC_VER

__forceinline
static inline void print_indents(info *arg_info) {
    for(register int i = 0; i < arg_info->indentlevel * 4; i++)
        fputc(' ', stderr);
}

#else

__attribute__((always_inline))
static inline void print_indents(info *arg_info) {
    for(register int i = 0; i < arg_info->indentlevel * 4; i++)
        fputc(' ', stderr);
}

#endif


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

    fputc('(', stderr);
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);

    char* binop_str = get_binop_str(BINOP_BINOPERATORS(arg_node));
    fprintf(stderr, " %s ", binop_str);
    MEMfree(binop_str);

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    fputc(')', stderr);

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
    print_indents(arg_info);

    fprintf(stderr, "%s[", ARRAYVARASSIGN_VARNAME(arg_node));

    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    fprintf(stderr, "] = ");
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

    fprintf(stderr, "%s[", ARRAYVARREAD_ARRAYVARNAME(arg_node));

    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    fprintf(stderr, "]");

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

    fprintf(stderr, "%s = ", VARASSIGN_VARNAME(arg_node));

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
    UNUSED_VAR(arg_info);
    fprintf(stderr, "%s", VARREAD_VARNAME(arg_node));

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
            fprintf(stderr, "(bool) ");
            break;
        case TP_float:
            fprintf(stderr, "(float) ");
            break;
        case TP_int:
            fprintf(stderr, "(int) ");
            break;
        case TP_void:
            fprintf(stderr, "(void) ");
            break;
        default:
            fprintf(stderr, "(UNKNOWN_TYPE_CAST) ");
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
    UNUSED_VAR(arg_info);
    switch (BOOLCONST_VALUE(arg_node))
    {
        case TRUE:
            fprintf(stderr, "TRUE");
            break;
        case FALSE:
            fprintf(stderr, "FALSE");
            break;
        default:
            fprintf(stderr, "INVALID_BOOL_CONST_VALUE");
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
    UNUSED_VAR(arg_info);
    fprintf(stderr, "%d", INTCONST_VALUE(arg_node));

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
    UNUSED_VAR(arg_info);
    fprintf(stderr, "%.10f", FLOATCONST_VALUE(arg_node));

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
    print_indents(arg_info);

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


    fprintf(stderr, "%s%s %s(", prefix, functiontype, FUNCTION_NAME(arg_node));

    if(FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);

    fputc(')', stderr);

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        fprintf(stderr, " {\n");
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
        print_indents(arg_info);
        fprintf(stderr, "}\n");
    } else {
        fprintf(stderr, ";\n");
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

    fprintf(stderr, "%s %s", type, BASICPARAMETER_NAME(arg_node));

    if(BASICPARAMETER_NEXT(arg_node) != NULL) {
        fprintf(stderr, ", ");
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
    DBUG_ENTER ("PRTarrayparameter");

    char* type = "";

    switch(ARRAYPARAMETER_TYPE(arg_node)) {
        case TP_bool:
        case TP_bool_array:
            type = "bool";
            break;
        case TP_float:
        case TP_float_array:
            type = "float";
            break;
        case TP_int:
        case TP_int_array:
            type = "int";
            break;
        default:
            type = "INVALID_PARAMETER_TYPE";
    }

    fprintf(stderr, "%s%s", type, "[");

    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), arg_info);

    fputc(']', stderr);
    fprintf(stderr, " %s", ARRAYPARAMETER_NAME(arg_node));

    if(ARRAYPARAMETER_NEXT(arg_node) != NULL) {
        fprintf(stderr, ", ");
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
    fprintf(stderr, "%s", ARRAYDIMENSIONVARS_NAME(arg_node));

    if(ARRAYDIMENSIONVARS_NEXT(arg_node) != NULL) {
        fprintf(stderr, ", ");
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

    if (FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node) != NULL) {
        print_fun_symbol_table(FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node));
        fputc('\n', stderr);
    }

    if (FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node) != NULL) {
        print_symbol_table(FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node));
        fputc('\n', stderr);
    }

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
            print_indents(arg_info);
            break;
        default:
            break;
    }

    STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

    if (NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_for &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_branch &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_while &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_dowhile) {
        fprintf(stderr, ";\n");
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
    print_indents(arg_info);

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

    fprintf(stderr, "%s %s", type, BASICVARDECLARATION_VARNAME(arg_node));

    if(BASICVARDECLARATION_INITVAL(arg_node) != NULL) {

        fprintf(stderr, " = ");
        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);
    }

    fprintf(stderr, ";\n");

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
    print_indents(arg_info);

    char* type = "";

    switch(ARRAYVARDECLARATION_TYPE(arg_node)) {
        case TP_bool:
        case TP_bool_array:
            type = "bool";
            break;
        case TP_float:
        case TP_float_array:
            type = "float";
            break;
        case TP_int:
        case TP_int_array:
            type = "int";
            break;
        default:
            type = "INVALID_ARRAYVARDEC_TYPE";
    }

    fprintf(stderr, "%s [", type);
    if(ARRAYVARDECLARATION_DIMENSIONS(arg_node) != NULL) {
        ARRAYVARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(ARRAYVARDECLARATION_DIMENSIONS(arg_node), arg_info);
    }
    fprintf(stderr, "] %s", ARRAYVARDECLARATION_VARNAME(arg_node));


    if(ARRAYVARDECLARATION_INITVAL(arg_node) != NULL) {
        fprintf(stderr, " = ");
        ARRAYVARDECLARATION_INITVAL(arg_node) = TRAVdo(ARRAYVARDECLARATION_INITVAL(arg_node), arg_info);
    }

    fprintf(stderr, ";\n");

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
        fputc('[', stderr);
        EXPRARRAY_CURRENT(arg_node) = TRAVdo(EXPRARRAY_CURRENT(arg_node), arg_info);
        fputc(']', stderr);
    } else {
        EXPRARRAY_CURRENT(arg_node) = TRAVdo(EXPRARRAY_CURRENT(arg_node), arg_info);
    }

    if(EXPRARRAY_NEXT(arg_node) != NULL) {
        fprintf(stderr, ", ");
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

    print_indents(arg_info);
    if(FOR_COUNTER(arg_node) != NULL) {
        fprintf(stderr, "for(int %s = ", BASICVARDECLARATION_VARNAME(FOR_COUNTER(arg_node)));
        BASICVARDECLARATION_INITVAL(FOR_COUNTER(arg_node)) =
            TRAVdo(BASICVARDECLARATION_INITVAL(FOR_COUNTER(arg_node)), arg_info);

        fprintf(stderr, ", ");
    } else {
        fprintf(stderr, "for(");
    }


    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL) {
          fprintf(stderr, ", ");
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
    }

    fprintf(stderr, ") {\n");
    if (FOR_VARSYMBOLTABLE(arg_node) != NULL) {
        print_symbol_table(FOR_VARSYMBOLTABLE(arg_node));
        fputc('\n', stderr);
    }
    INFO_INDENTLEVEL(arg_info) += 1;
    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    print_indents(arg_info);
    fprintf(stderr, "}\n");

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

    print_indents(arg_info);
    fprintf(stderr, "while(");

    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);

    fprintf(stderr, ") {\n");

    INFO_INDENTLEVEL(arg_info) += 1;
    WHILE_CODE(arg_node) = TRAVdo(WHILE_CODE(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    print_indents(arg_info);
    fprintf(stderr, "}\n");

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

    print_indents(arg_info);
    fprintf(stderr, "do {\n");

    INFO_INDENTLEVEL(arg_info) += 1;
    DOWHILE_CODE(arg_node) = TRAVdo(DOWHILE_CODE(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    print_indents(arg_info);
    fprintf(stderr, "} while(");
    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);
    fprintf(stderr, ");\n");

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
        case TP_int_array:
            type = "int";
            break;
        case TP_float:
        case TP_float_array:
            type = "float";
            break;
        case TP_bool:
        case TP_bool_array:
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

    fprintf(stderr, "%s%s %s", exportextern, type, GLOBAL_NAME(arg_node));

    if(GLOBAL_DIMENSIONS(arg_node) != NULL) {
        fputc('[', stderr);
        GLOBAL_DIMENSIONS(arg_node) = TRAVdo(GLOBAL_DIMENSIONS(arg_node), arg_info);
        fputc(']', stderr);
    }

    if(GLOBAL_DEFINITION(arg_node) != NULL) {
        GLOBAL_DEFINITION(arg_node) = TRAVdo(GLOBAL_DEFINITION(arg_node), arg_info);
    }

    fprintf(stderr, ";\n");
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
        fprintf(stderr, " = ");
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

    fprintf(stderr, "%s(", FUNCALL_FUNCTIONNAME(arg_node));

    if(FUNCALL_PARAMS(arg_node) != NULL) {
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);
    }

    fputc(')', stderr);

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
        fprintf(stderr, ", ");
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
    print_indents(arg_info);
    fprintf(stderr, "return");

    if(RETURN_VALUE(arg_node) != NULL) {
        fputc(' ', stderr);
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
    print_indents(arg_info);

    fprintf(stderr, "if(");


    BRANCH_CONDITION(arg_node) = TRAVdo(BRANCH_CONDITION(arg_node), arg_info);


    fprintf(stderr, ") {\n");
    INFO_INDENTLEVEL(arg_info) += 1;
    BRANCH_THENBRANCH(arg_node) = TRAVdo(BRANCH_THENBRANCH(arg_node), arg_info);
    INFO_INDENTLEVEL(arg_info) -= 1;

    if (BRANCH_ELSEBRANCH(arg_node) != NULL) {

        print_indents(arg_info);
        fprintf(stderr, "} else {\n");

        INFO_INDENTLEVEL(arg_info) += 1;
        BRANCH_ELSEBRANCH(arg_node) = TRAVdo(BRANCH_ELSEBRANCH(arg_node), arg_info);
        INFO_INDENTLEVEL(arg_info) -= 1;
    }

    print_indents(arg_info);
    fprintf(stderr, "}\n");

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

    fputc('(', stderr);

    switch (MONOP_TYPE(arg_node)) {
        case MO_not:
            fputc('!', stderr);
            break;
        case MO_neg:
            fputc('-', stderr);
            break;
        default:
            fprintf(stderr, "INVALID_MONOP_TYPE");
    }

    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), arg_info);

    fputc(')', stderr);
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
    print_indents(arg_info);

    fprintf(stderr, "__LOCAL FUNCTION__\n");

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

    if (PROGRAM_FUNSYMBOLTABLE(arg_node) != NULL) {
        print_fun_symbol_table(PROGRAM_FUNSYMBOLTABLE(arg_node));
        fputc('\n', stderr);
    }

    if (PROGRAM_VARSYMBOLTABLE(arg_node) != NULL) {
        print_symbol_table(PROGRAM_VARSYMBOLTABLE(arg_node));
        fputc('\n', stderr);
    }

    PROGRAM_FIRST(arg_node) = TRAVdo(PROGRAM_FIRST(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node* PRTconditionalexpr(node* arg_node, info* arg_info) {
    DBUG_ENTER("PRTconditionalexpr");

    CONDITIONALEXPR_CONDITION(arg_node) = TRAVdo(CONDITIONALEXPR_CONDITION(arg_node), arg_info);
    fprintf(stderr, " ? ");
    CONDITIONALEXPR_IFBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_IFBRANCH(arg_node), arg_info);
    fprintf(stderr, " : ");
    CONDITIONALEXPR_ELSEBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_ELSEBRANCH(arg_node), arg_info);


    DBUG_RETURN(arg_node);
}

node *PRTarrayinit (node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTarrayinit");
    symbol_table_entry* entry = ARRAYINIT_VARPOINTER(arg_node);

    print_indents(arg_info);
    fprintf(stderr, "%s = __MALLOC__(", entry->name);
    ARRAYINIT_SIZE(arg_node) = TRAVdo(ARRAYINIT_SIZE(arg_node), arg_info);
    fputc(')', stderr);

    DBUG_RETURN(arg_node);
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

    fprintf(stderr, "\n\n ------------------- PROGRAM START ------------------- \n\n");

    info = MakeInfo();

    TRAVpush(TR_prt);

    syntaxtree = TRAVdo(syntaxtree, info);

    TRAVpop();

    info = FreeInfo(info);

    fprintf(stderr, "\n\n ------------------- PROGRAM END --------------------- \n\n");

    DBUG_RETURN(syntaxtree);
}

/**
 * @}
 */
