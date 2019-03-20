#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "util.h"


/** <!--******************************************************************-->
 *
 * @fn GBCbinop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCbinop (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCbinop");

    fputc('(', stderr);
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);


    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    fputc(')', stderr);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCdeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCdeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCdeclaration");

    DECLARATION_CURRENT(arg_node) = TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);

    if(DECLARATION_NEXT(arg_node) != NULL)
        DECLARATION_NEXT(arg_node) = TRAVdo(DECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarrayvarassign
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCarrayvarassign (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCarrayvarassign");



    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarrayvarread
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCarrayvarread (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCarrayvarread");


    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);


    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCvarassign
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCvarassign (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCvarassign");


    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCvarread
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCvarread (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCvarread");
    UNUSED(arg_info);

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCcast
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCcast (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCcast");

    switch(CAST_CASTTO(arg_node)) {
        case TP_bool:
            break;
        case TP_float:
            break;
        case TP_int:
            break;
        case TP_void:
            break;
        default:
    }

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCboolconst
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCboolconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCboolconst");
    UNUSED(arg_info);
    switch (BOOLCONST_VALUE(arg_node))
    {
        case TRUE:
            break;
        case FALSE:
            break;
        default:
    }

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCintconst
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCintconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCintconst");
    UNUSED(arg_info);

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCfloatconst
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCfloatconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCfloatconst");
    UNUSED(arg_info);

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCconstant
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCconstant (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCconstant");

    CONSTANT_CURRENT(arg_node) = TRAVdo(CONSTANT_CURRENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCerror
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCerror (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCerror");
    UNUSED(arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfunction
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfunction (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfunction");


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



    if(FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);

    fputc(')', stderr);

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);

    } else {
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCbasicparameter
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCbasicparameter (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCbasicparameter");
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


    if(BASICPARAMETER_NEXT(arg_node) != NULL) {
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarrayparameter
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCarrayparameter (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCarrayparameter");

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


    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), arg_info);

    fputc(']', stderr);

    if(ARRAYPARAMETER_NEXT(arg_node) != NULL) {
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarraydimensionvars
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCarraydimensionvars (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCarraydimensionvars");

    if(ARRAYDIMENSIONVARS_NEXT(arg_node) != NULL) {
        ARRAYDIMENSIONVARS_NEXT(arg_node) = TRAVdo(ARRAYDIMENSIONVARS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfunctiondefinition
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfunctiondefinition (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfunctiondefinition");

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
 * @fn GBCfunctionbody
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfunctionbody (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfunctionbody");

    if(FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL) {
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_LOCALFUNCTIONS(arg_node) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
        FUNCTIONBODY_STATEMENTS(arg_node) = TRAVdo(FUNCTIONBODY_STATEMENTS(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCstatements
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCstatements (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCstatements");

    // These can also be expressions, so we can't just print the indents there
    switch(NODE_TYPE(STATEMENTS_CURRENT(arg_node))) {
        case N_funcall:
        case N_varassign:

            break;
        default:
            break;
    }

    STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

    if (NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_for &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_branch &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_while &&
        NODE_TYPE(STATEMENTS_CURRENT(arg_node)) != N_dowhile) {
    }

    if(STATEMENTS_NEXT(arg_node) != NULL) {
        STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCbasicvardeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCbasicvardeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCbasicvardeclaration");


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


    if(BASICVARDECLARATION_INITVAL(arg_node) != NULL) {

        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);
    }


    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarrayvardeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCarrayvardeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCarrayvardeclaration");


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


    ARRAYVARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(ARRAYVARDECLARATION_DIMENSIONS(arg_node), arg_info);



    if(ARRAYVARDECLARATION_INITVAL(arg_node) != NULL) {
        ARRAYVARDECLARATION_INITVAL(arg_node) = TRAVdo(ARRAYVARDECLARATION_INITVAL(arg_node), arg_info);
    }


    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCexprarray
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCexprarray (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCexprarray");

    if(NODE_TYPE(EXPRARRAY_CURRENT(arg_node)) == N_exprarray) {
        fputc('[', stderr);
        EXPRARRAY_CURRENT(arg_node) = TRAVdo(EXPRARRAY_CURRENT(arg_node), arg_info);
        fputc(']', stderr);
    } else {
        EXPRARRAY_CURRENT(arg_node) = TRAVdo(EXPRARRAY_CURRENT(arg_node), arg_info);
    }

    if(EXPRARRAY_NEXT(arg_node) != NULL) {
        EXPRARRAY_NEXT(arg_node) = TRAVdo(EXPRARRAY_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfor
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfor (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfor");


    if(FOR_COUNTER(arg_node) != NULL) {
        BASICVARDECLARATION_INITVAL(FOR_COUNTER(arg_node)) =
            TRAVdo(BASICVARDECLARATION_INITVAL(FOR_COUNTER(arg_node)), arg_info);

    } else {
    }


    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL) {
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
    }

    if (FOR_VARSYMBOLTABLE(arg_node) != NULL) {
        print_symbol_table(FOR_VARSYMBOLTABLE(arg_node));
        fputc('\n', stderr);
    }
    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);



    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCwhile
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCwhile (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCwhile");



    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);


    WHILE_CODE(arg_node) = TRAVdo(WHILE_CODE(arg_node), arg_info);



    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCdowhile
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCdowhile(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCdowhile");



    DOWHILE_CODE(arg_node) = TRAVdo(DOWHILE_CODE(arg_node), arg_info);


    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCglobal
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCglobal(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCglobal");
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


    if(GLOBAL_DIMENSIONS(arg_node) != NULL) {
        fputc('[', stderr);
        GLOBAL_DIMENSIONS(arg_node) = TRAVdo(GLOBAL_DIMENSIONS(arg_node), arg_info);
        fputc(']', stderr);
    }

    if(GLOBAL_DEFINITION(arg_node) != NULL) {
        GLOBAL_DEFINITION(arg_node) = TRAVdo(GLOBAL_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCglobaldefinition
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCglobaldefinition (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCglobaldefinition");

    if(GLOBALDEFINITION_VALUES(arg_node) != NULL) {
        GLOBALDEFINITION_VALUES(arg_node) = TRAVdo(GLOBALDEFINITION_VALUES(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfuncall
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfuncall(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfuncall");


    if(FUNCALL_PARAMS(arg_node) != NULL) {
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);
    }

    fputc(')', stderr);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfuncallparameter
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfuncallparameter(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfuncallparameter");

    FUNCALLPARAMETER_VALUE(arg_node) = TRAVdo(FUNCALLPARAMETER_VALUE(arg_node), arg_info);

    if (FUNCALLPARAMETER_NEXT(arg_node) != NULL) {
        FUNCALLPARAMETER_NEXT(arg_node) = TRAVdo(FUNCALLPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCreturn
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCreturn(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCreturn");


    if(RETURN_VALUE(arg_node) != NULL) {
        fputc(' ', stderr);
        RETURN_VALUE(arg_node) = TRAVdo(RETURN_VALUE(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCbranch
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCbranch(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCbranch");




    BRANCH_CONDITION(arg_node) = TRAVdo(BRANCH_CONDITION(arg_node), arg_info);


    BRANCH_THENBRANCH(arg_node) = TRAVdo(BRANCH_THENBRANCH(arg_node), arg_info);

    if (BRANCH_ELSEBRANCH(arg_node) != NULL) {




        BRANCH_ELSEBRANCH(arg_node) = TRAVdo(BRANCH_ELSEBRANCH(arg_node), arg_info);

    }



    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCexpr
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCexpr(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCexpr");

    EXPR_CURRENT(arg_node) = TRAVdo(EXPR_CURRENT(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCmonop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCmonop(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCmonop");

    fputc('(', stderr);

    switch (MONOP_TYPE(arg_node)) {
        case MO_not:
            fputc('!', stderr);
            break;
        case MO_neg:
            fputc('-', stderr);
            break;
        default:
    }

    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), arg_info);

    fputc(')', stderr);
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfunctionvardeclaration
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfunctionvardeclaration(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfunctionvardeclaration");

    FUNCTIONVARDECLARATION_CURRENT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_CURRENT(arg_node), arg_info);

    if (FUNCTIONVARDECLARATION_NEXT(arg_node) != NULL) {
        FUNCTIONVARDECLARATION_NEXT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBClocalfunction
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBClocalfunction(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBClocalfunction");



    LOCALFUNCTION_CURRENT(arg_node) = TRAVdo(LOCALFUNCTION_CURRENT(arg_node), arg_info);

    if (LOCALFUNCTION_NEXT(arg_node) != NULL) {
        LOCALFUNCTION_NEXT(arg_node) = TRAVdo(LOCALFUNCTION_NEXT(arg_node), arg_info);
    }


    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCprogram
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCprogram(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBClocalfunction");

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

node* GBCconditionalexpr(node* arg_node, info* arg_info) {
    DBUG_ENTER("GBCconditionalexpr");

    CONDITIONALEXPR_CONDITION(arg_node) = TRAVdo(CONDITIONALEXPR_CONDITION(arg_node), arg_info);
    CONDITIONALEXPR_IFBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_IFBRANCH(arg_node), arg_info);
    CONDITIONALEXPR_ELSEBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_ELSEBRANCH(arg_node), arg_info);


    DBUG_RETURN(arg_node);
}


/*
 * Traversal start function
 */

node *GBCdoGenByteCode(node *syntaxtree) {
  DBUG_ENTER("GBCdoGenByteCode");

  /*
   * Your work here....
   */

  DBUG_RETURN(syntaxtree);
}
