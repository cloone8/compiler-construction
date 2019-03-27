#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"
#include "ctinfo.h"

#include "instruction_lib.h"
#include "pseudo_instructions.h"
#include "util.h"

/*
 * INFO structure
 */
struct INFO {
    type final_type;
    type returntype;
    pseudo_instructions* exportfuns;
    pseudo_instructions* exportvars;
    pseudo_instructions* importfuns;
    pseudo_instructions* importvars;
    pseudo_instructions* constants;
    pseudo_instructions* globals;
};

/*
 * INFO macros
 */
#define TYPE(__arg_info_struct__) ((__arg_info_struct__)->final_type)
#define RETURNTYPE(__arg_info_struct__) ((__arg_info_struct__)->returntype)
#define EXPORTFUNS(__arg_info_struct__) ((__arg_info_struct__)->exportfuns)
#define EXPORTVARS(__arg_info_struct__) ((__arg_info_struct__)->exportvars)
#define IMPORTFUNS(__arg_info_struct__) ((__arg_info_struct__)->importfuns)
#define IMPORTVARS(__arg_info_struct__) ((__arg_info_struct__)->importvars)
#define CONSTANTS(__arg_info_struct__) ((__arg_info_struct__)->constants)
#define GLOBALS(__arg_info_struct__) ((__arg_info_struct__)->globals)
/**
 * Creates a new info struct and sets the initial values
 */
static info *GBCmakeInfo(void) {
    DBUG_ENTER("GBCmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    TYPE(result) = TP_unknown;
    RETURNTYPE(result) = TP_unknown;

    EXPORTFUNS(result) = PSI_make(INSTR_exportfun);
    EXPORTVARS(result) = PSI_make(INSTR_exportvar);
    IMPORTFUNS(result) = PSI_make(INSTR_importfun);
    IMPORTVARS(result) = PSI_make(INSTR_importvar);
    CONSTANTS(result) = PSI_make(INSTR_const);
    GLOBALS(result) = PSI_make(INSTR_global);

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *GBCfreeInfo(info* arg_info) {
    DBUG_ENTER("GBCfreeInfo");

    PSI_free(EXPORTFUNS(arg_info));
    PSI_free(EXPORTVARS(arg_info));
    PSI_free(IMPORTFUNS(arg_info));
    PSI_free(IMPORTVARS(arg_info));
    PSI_free(CONSTANTS(arg_info));
    PSI_free(GLOBALS(arg_info));

    arg_info = MEMfree(arg_info);

    DBUG_RETURN(arg_info);
}

static void print_pseudo_instructions(info* arg_info) {
    DBUG_ENTER("print_pseudo_instructions");

    // Print constants
    for(size_t i = 0; i < CONSTANTS(arg_info)->itemcount; i++) {
        instruction* item = CONSTANTS(arg_info)->entries[i];
        char* const_type_str = get_type_str(item->constant->consttype);
        fprintf(ofile, ".const %s ", const_type_str);
        MEMfree(const_type_str);

        switch(item->constant->consttype) {
            case TP_bool:
                if(item->constant->val.boolval == TRUE) {
                    fprintf(ofile, "true\n");
                } else {
                    fprintf(ofile, "false\n");
                }
                break;
            case TP_int:
                fprintf(ofile, "%d\n", item->constant->val.intval);
                break;
            case TP_float:
                fprintf(ofile, "%.5f\n", item->constant->val.fltval);
                break;
            default:
                DBUG_ASSERT((FALSE), "Invalid type!");
        }
    }

    // Print globals
    for(size_t i = 0; i < GLOBALS(arg_info)->itemcount; i++) {
        instruction* item = GLOBALS(arg_info)->entries[i];
        char* global_type_str = get_type_str(item->global->globaltype);
        fprintf(ofile, ".global %s\n", global_type_str);
        MEMfree(global_type_str);
    }

    // Print imported functions
    for(size_t i = 0; i < IMPORTFUNS(arg_info)->itemcount; i++) {
        instruction* item = IMPORTFUNS(arg_info)->entries[i];
        char* ret_type_str = get_type_str(item->importfun->ret_type);
        fprintf(ofile, ".importfun \"%s\" %s", item->importfun->name, ret_type_str);
        MEMfree(ret_type_str);

        // Print parameters
        for(size_t j = 0; j < item->importfun->paramcount; j++) {
            char* param_type_str = get_type_str(item->importfun->paramtypes[j]);
            fprintf(ofile, " %s", param_type_str);
            MEMfree(param_type_str);
        }

        fputc('\n', ofile);
    }

    // Print imported variables
    for(size_t i = 0; i < IMPORTVARS(arg_info)->itemcount; i++) {
        instruction* item = IMPORTVARS(arg_info)->entries[i];
        char* var_type_str = get_type_str(item->importvar->vartype);

        fprintf(ofile, ".importvar \"%s\" %s\n", item->importvar->name, var_type_str);
        MEMfree(var_type_str);
    }

    // Print exported functions
    for(size_t i = 0; i < EXPORTFUNS(arg_info)->itemcount; i++) {
        instruction* item = EXPORTFUNS(arg_info)->entries[i];
        char* ret_type_str = get_type_str(item->exportfun->ret_type);
        fprintf(ofile, ".exportfun \"%s\" %s", item->exportfun->name, ret_type_str);
        MEMfree(ret_type_str);

        // Print parameters
        for(size_t j = 0; j < item->exportfun->paramcount; j++) {
            char* param_type_str = get_type_str(item->exportfun->paramtypes[j]);
            fprintf(ofile, " %s", param_type_str);
            MEMfree(param_type_str);
        }

        // Print label
        fprintf(ofile, " %s\n", item->exportfun->fun_label);
    }

    // Print exported variables. Must come after constants
    for(size_t i = 0; i < EXPORTVARS(arg_info)->itemcount; i++) {
        instruction* item = EXPORTVARS(arg_info)->entries[i];

        fprintf(ofile, ".exportvar \"%s\" %" PRIu16 "\n", item->exportvar->name, item->exportvar->global_index);
    }

    DBUG_VOID_RETURN;
}

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

    info* left = GBCmakeInfo();
    info* right = GBCmakeInfo();

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), left);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), right);

    binop operator = BINOP_BINOPERATORS(arg_node);
    type to_process = TYPE(left);

    switch (operator) {
        case BO_add:
            switch (to_process) {
                case TP_bool:
                    badd();
                    break;
                case TP_int:
                    iadd();
                    break;
                case TP_float:
                    fadd();
                    break;
                default:
                    break;
            }
            break;
        case BO_sub:
            switch (to_process) {
                case TP_int:
                    isub();
                    break;
                case TP_float:
                    fsub();
                    break;
                default:
                    break;
            }
            break;
        case BO_mul:
            switch (to_process) {
                case TP_bool:
                    bmul();
                    break;
                case TP_int:
                    imul();
                    break;
                case TP_float:
                    fmul();
                    break;
                default:
                    break;
            }
            break;
        case BO_div:
            switch (to_process) {
                case TP_int:
                    idiv();
                    break;
                case TP_float:
                    fdiv();
                    break;
                default:
                    break;
            }
            break;
        case BO_mod:
            switch (to_process) {
                case TP_int:
                    irem();
                    break;
                default:
                    break;
            }
            break;
        case BO_eq:
            switch (to_process) {
                case TP_int:
                    ieq();
                    break;
                case TP_float:
                    feq();
                    break;
                case TP_bool:
                    beq();
                    break;
                default:
                    break;
            }
            break;
        case BO_ne:
            switch (to_process) {
                case TP_int:
                    ine();
                    break;
                case TP_float:
                    fne();
                    break;
                case TP_bool:
                    bne();
                    break;
                default:
                    break;
            }
            break;
        case BO_ge:
            switch (to_process) {
                case TP_int:
                    ige();
                    break;
                case TP_float:
                    fge();
                    break;
                default:
                    break;
            }
            break;
        case BO_gt:
            switch (to_process) {
                case TP_int:
                    igt();
                    break;
                case TP_float:
                    fgt();
                    break;
                default:
                    break;
            }
            break;
        case BO_le:
            switch (to_process) {
                case TP_int:
                    ile();
                    break;
                case TP_float:
                    fle();
                    break;
                default:
                    break;
            }
            break;
        case BO_lt:
            switch (to_process) {
                case TP_int:
                    ilt();
                    break;
                case TP_float:
                    flt();
                    break;
                default:
                    break;
            }
            break;
        case BO_and:
            CTIerror("CiviCVM cannot handle \'&&\'");
            break;
        case BO_or:
            CTIerror("CiviCVM cannot handle \'||\'");
            break;
        default:
            break;
    }

    GBCfreeInfo(left);
    GBCfreeInfo(right);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCdeclaration
 *
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
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCvarread (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCvarread");
    UNUSED_VAR(arg_info);

    TYPE(arg_info) = VARREAD_VARPOINTER(arg_node)->vartype;

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCcast
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCcast (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCcast");
    printf("%s", get_type_str(CAST_CASTTO(arg_node)));
    printf("kasjhdf");

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);
    switch (CAST_CASTTO(arg_node)) {
        case TP_int:
            i2f();
            break;
        case TP_float:
            f2i();
            break;
        default:
            break;
    }

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCboolconst
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCboolconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCboolconst");
    UNUSED_VAR(arg_info);

    TYPE(arg_info) = TP_bool;

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCintconst
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCintconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCintconst");

    TYPE(arg_info) = TP_int;

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCfloatconst
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node* GBCfloatconst (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCfloatconst");
    UNUSED_VAR(arg_info);

    TYPE(arg_info) = TP_float;

    DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn GBCconstant
 *
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
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCerror (node * arg_node, info * arg_info) {
    DBUG_ENTER("GBCerror");
    UNUSED_VAR(arg_info);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfunction
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCfunction (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCfunction");

    if (FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);


    if (FUNCTION_DEFINITION(arg_node) != NULL) {
        info* new = GBCmakeInfo();
        RETURNTYPE(new) = FUNCTION_TYPE(arg_node);
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), new);
        GBCfreeInfo(new);
    }
    else {
    }


    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCbasicparameter
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCbasicparameter (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCbasicparameter");

    if(BASICPARAMETER_NEXT(arg_node) != NULL) {
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarrayparameter
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCarrayparameter (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCarrayparameter");

    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), arg_info);


    if(ARRAYPARAMETER_NEXT(arg_node) != NULL) {
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarraydimensionvars
 *
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
    }

    if (FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node) != NULL) {
    }

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfunctionbody
 *
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
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCstatements (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCstatements");

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
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCbasicvardeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCbasicvardeclaration");

    if(BASICVARDECLARATION_INITVAL(arg_node) != NULL) {

        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);
    }


    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCarrayvardeclaration
 *
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCarrayvardeclaration (node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCarrayvardeclaration");

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
        EXPRARRAY_CURRENT(arg_node) = TRAVdo(EXPRARRAY_CURRENT(arg_node), arg_info);
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
    }
    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);



    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCwhile
 *
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
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCglobal(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCglobal");

    if(GLOBAL_DIMENSIONS(arg_node) != NULL) {
        GLOBAL_DIMENSIONS(arg_node) = TRAVdo(GLOBAL_DIMENSIONS(arg_node), arg_info);
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


    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfuncallparameter
 *
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
        RETURN_VALUE(arg_node) = TRAVdo(RETURN_VALUE(arg_node), arg_info);
    }

    type to_return = RETURNTYPE(arg_info);

    switch (to_return) {
        case TP_void:
            ret();
            break;
        case TP_bool:
            bret();
            break;
        case TP_int:
            iret();
            break;
        case TP_float:
            fret();
            break;
        default:
            break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCbranch
 *
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
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node* GBCmonop(node * arg_node, info * arg_info) {
    DBUG_ENTER ("GBCmonop");

    info* new = GBCmakeInfo();
    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), new);
    switch(MONOP_TYPE(arg_node)) {
        case MO_neg:
            switch (TYPE(new)) {
                case TP_int:
                    ineg();
                    break;
                case TP_float:
                    fneg();
                    break;
                default:
                    break;
            }
            break;
        case MO_not:
            bnot();
            break;
        default:
            break;

    }

    GBCfreeInfo(new);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn GBCfunctionvardeclaration
 *
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
    }

    if (PROGRAM_VARSYMBOLTABLE(arg_node) != NULL) {
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

    info* arg_info = GBCmakeInfo();

    TRAVpush(TR_gbc);

    // If there is no output file supplied, just use stdout
    DBUG_PRINT("GBC", ("Opening output stream"));
    if(global.outfile == NULL) {
        DBUG_PRINT("GBC", ("No output file supplied, using STDOUT"));
        ofile = stdout;
    } else {
        DBUG_PRINT("GBC", ("Opening file %s", global.outfile));
        ofile = fopen(global.outfile, "w");
    }


    if(ofile == NULL) {
        CTIabort("Error writing to output file \"%s\": %s", global.outfile, strerror(errno));
    }

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    print_pseudo_instructions(arg_info);

    // Only close non-stdout output streams
    if(global.outfile != NULL) {
        DBUG_PRINT("GBC", ("Closing output stream"));
        fclose(ofile);
    } else {
        DBUG_PRINT("GBC", ("STDOUT detected as output stream, not closing"));
    }

    TRAVpop();

    arg_info = GBCfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
