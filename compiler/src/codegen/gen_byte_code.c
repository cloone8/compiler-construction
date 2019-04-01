#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"
#include "ctinfo.h"

#include "symbol_table.h"
#include "fun_symbol_table.h"
#include "str.h"

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
    size_t branch_amount;
    size_t condexpr_amount;
    size_t loop_amount;
    size_t current_depth;
    bool is_expr;
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
#define BRANCH_AMOUNT(__arg_info_struct__) ((__arg_info_struct__)->branch_amount)
#define CONDEXPR_AMOUNT(__arg_info_struct__) ((__arg_info_struct__)->condexpr_amount)
#define LOOP_AMOUNT(__arg_info_struct__) ((__arg_info_struct__)->loop_amount)
#define DEPTH(__arg_info_struct__) ((__arg_info_struct__)->current_depth)
#define IS_EXPR(__arg_info_struct__) ((__arg_info_struct__)->is_expr)

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
    BRANCH_AMOUNT(result) = 0;
    CONDEXPR_AMOUNT(result) = 0;
    LOOP_AMOUNT(result) = 0;
    DEPTH(result) = 0;
    IS_EXPR(result) = FALSE;


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

static void set_globalvar_idx(symbol_table* table, info* arg_info) {
    DBUG_ENTER("set_globalvar_idx");
    size_t importcount = 0;
    size_t nonimportcount = 0;

    for(size_t i = 0; i < table->current_entries; i++) {
        symbol_table_entry* curr = table->entries[i];
        if(curr->isimport) {
            curr->idx = importcount++;

            if(importcount > UINT16_MAX) {
                CTIabort("Too many imported variables for the VM to handle!");
            }

            instruction* to_add = PSI_make_importvar(curr->name,
                                                    curr->vartype);
            PSI_add(IMPORTVARS(arg_info), to_add);
        } else {
            curr->idx = nonimportcount++;

            if(nonimportcount > UINT16_MAX) {
                CTIabort("Too many global definitions in this module for the VM to handle!");
            }

            instruction* to_add = PSI_make_global(curr->vartype);
            PSI_add(GLOBALS(arg_info), to_add);

            if(curr->isexport) {
                instruction* export_to_add = PSI_make_exportvar(curr->name, (uint16_t) to_add->index);
                PSI_add(EXPORTVARS(arg_info), export_to_add);
            }
        }
    }

    DBUG_VOID_RETURN;
}

static void set_globalfun_idx(fun_symbol_table* table, info* arg_info) {
    DBUG_ENTER("set_globalfun_idx");
    size_t importcount = 0;

    for(size_t i = 0; i < table->current_entries; i++) {
        fun_symbol_table_entry* curr = table->entries[i];
        if(curr->isimport) {
            curr->idx = importcount++;

            if(importcount > UINT16_MAX) {
                CTIabort("Too many imported variables for the VM to handle!");
            }

            instruction* to_add = PSI_make_importfun(curr->name,
                                                     curr->returntype,
                                                     curr->paramamount,
                                                     curr->paramtypes);
            PSI_add(IMPORTFUNS(arg_info), to_add);

        } else if(curr->isexport) {
            char* fun_label = get_fun_label(curr);
            instruction* to_add = PSI_make_exportfun(curr->name,
                                                     curr->returntype,
                                                     curr->paramamount,
                                                     curr->paramtypes,
                                                     fun_label);
            MEMfree(fun_label);
            PSI_add(EXPORTFUNS(arg_info), to_add);
        }
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

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    binop operator = BINOP_BINOPERATORS(arg_node);
    type to_process = TYPE(arg_info);

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
            TYPE(arg_info) = TP_bool;
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
            TYPE(arg_info) = TP_bool;
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
            TYPE(arg_info) = TP_bool;
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
            TYPE(arg_info) = TP_bool;
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
            TYPE(arg_info) = TP_bool;
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
            TYPE(arg_info) = TP_bool;
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

    symbol_table_entry* entry = ARRAYVARASSIGN_VARPOINTER(arg_node);

    ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);

    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    if(entry->table->table_depth == 0) {
        if(entry->isimport) {
            aloade((uint16_t) entry->idx);
        } else {
            aloadg((uint16_t) entry->idx);
        }
    } else if(entry->table->table_depth == DEPTH(arg_info)) {
        aload((uint8_t) entry->idx);
    } else {
        aloadn(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
    }

    switch(entry->vartype) {
        case TP_int_array:
            istorea();
            break;
        case TP_float_array:
            fstorea();
            break;
        case TP_bool_array:
            bstorea();
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid array type!");
            CTIabort("Internal compiler error!");
            break;
    }

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
    symbol_table_entry* entry = ARRAYVARREAD_VARPOINTER(arg_node);

    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    if(entry->table->table_depth == 0) {
        if(entry->isimport) {
            aloade((uint16_t) entry->idx);
        } else {
            aloadg((uint16_t) entry->idx);
        }
    } else if(entry->table->table_depth == DEPTH(arg_info)) {
        aload((uint8_t) entry->idx);
    } else {
        aloadn(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
    }

    switch(entry->vartype) {
        case TP_int_array:
            iloada();
            break;
        case TP_float_array:
            floada();
            break;
        case TP_bool_array:
            bloada();
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid array type!");
            CTIabort("Internal compiler error!");
            break;
    }

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

    symbol_table_entry* entry = VARASSIGN_VARPOINTER(arg_node);

    switch(entry->vartype) {
        case TP_int:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    istoree((uint16_t) entry->idx);
                } else {
                    istoreg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                istore((uint8_t) entry->idx);
            } else {
                istoren(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
            }
            break;
        case TP_float:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    fstoree((uint16_t) entry->idx);
                } else {
                    fstoreg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                fstore((uint8_t) entry->idx);
            } else {
                fstoren(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
            }
            break;
        case TP_bool:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    bstoree((uint16_t) entry->idx);
                } else {
                    bstoreg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                bstore((uint8_t) entry->idx);
            } else {
                bstoren(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
            }
            break;
        case TP_int_array:
        case TP_float_array:
        case TP_bool_array:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    astoree((uint16_t) entry->idx);
                } else {
                    astoreg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                astore((uint8_t) entry->idx);
            } else {
                DBUG_ASSERT((FALSE), "Can't read array reference from higher scope!");
                CTIabort("Internal compiler error!");
            }

            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid varassign type");
            break;
    }

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

    symbol_table_entry* entry = VARREAD_VARPOINTER(arg_node);

    TYPE(arg_info) = entry->vartype;

    switch(entry->vartype) {
        case TP_int:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    iloade((uint16_t) entry->idx);
                } else {
                    iloadg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                iload((uint8_t) entry->idx);
            } else {
                iloadn(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
            }
            break;
        case TP_float:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    floade((uint16_t) entry->idx);
                } else {
                    floadg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                fload((uint8_t) entry->idx);
            } else {
                floadn(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
            }
            break;
        case TP_bool:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    bloade((uint16_t) entry->idx);
                } else {
                    bloadg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                bload((uint8_t) entry->idx);
            } else {
                bloadn(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
            }
            break;
        case TP_int_array:
        case TP_float_array:
        case TP_bool_array:
            if(entry->table->table_depth == 0) {
                if(entry->isimport) {
                    aloade((uint16_t) entry->idx);
                } else {
                    aloadg((uint16_t) entry->idx);
                }
            } else if(entry->table->table_depth == DEPTH(arg_info)) {
                aload((uint8_t) entry->idx);
            } else {
                aloadn(DEPTH(arg_info) - entry->table->table_depth, entry->idx);
            }
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid varread type");
            break;
    }

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

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);
    switch (CAST_CASTTO(arg_node)) {
        case TP_int:
            f2i();
            TYPE(arg_info) = TP_int;
            break;
        case TP_float:
            i2f();
            TYPE(arg_info) = TP_float;
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

    if(BOOLCONST_VALUE(arg_node) == TRUE) {
        bloadc_t();
    } else {
        bloadc_f();
    }

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
    constval_t val;
    instruction* const_entry = NULL;

    switch(INTCONST_VALUE(arg_node)) {
        case 1:
            iloadc_1();
            break;
        case 0:
            iloadc_0();
            break;
        case -1:
            iloadc_m1();
            break;
        default:
            val.intval = INTCONST_VALUE(arg_node);
            const_entry = PSI_find_constant(CONSTANTS(arg_info), TP_int, val);
            if(const_entry == NULL) {
                const_entry = PSI_make_const(TP_int, val);
                PSI_add(CONSTANTS(arg_info), const_entry);
            }
            iloadc((uint16_t) const_entry->index);
            break;
    }

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
    constval_t val;
    instruction* const_entry = NULL;

    if (FLOATCONST_VALUE(arg_node) == 1.0) {
        floadc_1();
    }
    else if (FLOATCONST_VALUE(arg_node) == 0.0) {
        floadc_0();
    }
    else {
        val.fltval = FLOATCONST_VALUE(arg_node);
        const_entry = PSI_find_constant(CONSTANTS(arg_info), TP_float, val);
        if (const_entry == NULL) {
            const_entry = PSI_make_const(TP_float, val);
            PSI_add(CONSTANTS(arg_info), const_entry);
        }
        floadc((uint16_t) const_entry->index);
    }

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
        fun_label(FUNCTION_FUNPOINTER(arg_node));
        type old_returntype = RETURNTYPE(arg_info);
        RETURNTYPE(arg_info) = FUNCTION_TYPE(arg_node);

        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
        RETURNTYPE(arg_info) = old_returntype;

        fputc('\n', ofile);
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
    DEPTH(arg_info)++;

    size_t varcount = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node)->current_entries;
    if(varcount > UINT8_MAX) {
        CTIabort("Too many variables in function!");
    }

    if (varcount != 0) {
        esr((uint8_t) varcount);
    }

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    DEPTH(arg_info)--;
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

    if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
        FUNCTIONBODY_STATEMENTS(arg_node) = TRAVdo(FUNCTIONBODY_STATEMENTS(arg_node), arg_info);
    }

    if(FUNCTIONBODY_LOCALFUNCTIONS(arg_node) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(arg_node), arg_info);
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

    STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);

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
    UNUSED_VAR(arg_info);

    DBUG_ASSERT((FALSE), "For loop should have been converted to do while loop!");
    CTIabort("Critical compiler error");

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
    UNUSED_VAR(arg_info);

    DBUG_ASSERT((FALSE), "While loop should have been converted to do while loop!");
    CTIabort("Critical compiler error");

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

    get_size_t_str(buf, LOOP_AMOUNT(arg_info)++);
    char* loop_start_label = STRcat("loop_", buf);

    label(loop_start_label);

    DOWHILE_CODE(arg_node) = TRAVdo(DOWHILE_CODE(arg_node), arg_info);

    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);
    branch_t(loop_start_label);

    MEMfree(loop_start_label);

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

    fun_symbol_table_entry* entry = FUNCALL_FUNPOINTER(arg_node);
    size_t curr_depth = entry->table->table_depth;
    if(entry->table->table_depth == 0) {
        isrg();
    } else if(entry->table->table_depth == DEPTH(arg_info)) {
        isrl();
    } else if((entry->table->table_depth + 1) == DEPTH(arg_info)) {
        isr();
    } else {
        isrn(DEPTH(arg_info) - curr_depth);
    }

    if(FUNCALL_PARAMS(arg_node) != NULL) {
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);
    }

    if(entry->isimport) {
        jsre(entry->idx);
    } else {
        char* fun_label = get_fun_label(entry);
        jsr(entry->paramamount, fun_label);
        MEMfree(fun_label);
    }

    if(entry->returntype == TP_void) {
        DBUG_RETURN(arg_node);
    }

    /* Pop top of stack if return value is not used by calee. */
    if (!IS_EXPR(arg_info)) {
        switch (entry->returntype) {
            case TP_bool:
                bpop();
                break;
            case TP_int:
                ipop();
                break;
            case TP_float:
                fpop();
                break;
            default:
                break;
        }
    } else {
        TYPE(arg_info) = entry->returntype;
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
            DBUG_ASSERT((FALSE), "Invalid return type");
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

    get_size_t_str(count, BRANCH_AMOUNT(arg_info)++);
    char* end_label = STRcatn(3, "branch_", count, "_end");

    if (BRANCH_ELSEBRANCH(arg_node) != NULL) {
        char* else_label = STRcatn(3, "branch_", count, "_else");
        branch_f(else_label);

        BRANCH_THENBRANCH(arg_node) = TRAVdo(BRANCH_THENBRANCH(arg_node), arg_info);

        jump(end_label);

        label(else_label);
        MEMfree(else_label);
        BRANCH_ELSEBRANCH(arg_node) = TRAVdo(BRANCH_ELSEBRANCH(arg_node), arg_info);
    } else {
        branch_f(end_label);

        BRANCH_THENBRANCH(arg_node) = TRAVdo(BRANCH_THENBRANCH(arg_node), arg_info);
    }

    label(end_label);
    MEMfree(end_label);

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

    /*This flag is used to check if the return value from a funcall needs to be stored or popped.*/
    bool prev_isexpr = IS_EXPR(arg_info);
    IS_EXPR(arg_info) = TRUE;
    EXPR_CURRENT(arg_node) = TRAVdo(EXPR_CURRENT(arg_node), arg_info);
    IS_EXPR(arg_info) = prev_isexpr;

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

    MONOP_TARGET(arg_node) = TRAVdo(MONOP_TARGET(arg_node), arg_info);

    switch(MONOP_TYPE(arg_node)) {
        case MO_neg:
            switch (TYPE(arg_info)) {
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

    PROGRAM_FIRST(arg_node) = TRAVdo(PROGRAM_FIRST(arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node* GBCconditionalexpr(node* arg_node, info* arg_info) {
    DBUG_ENTER("GBCconditionalexpr");

    CONDITIONALEXPR_CONDITION(arg_node) = TRAVdo(CONDITIONALEXPR_CONDITION(arg_node), arg_info);

    get_size_t_str(count, CONDEXPR_AMOUNT(arg_info)++);
    char* end_label = STRcatn(3, "condexpr_", count, "_end");

    char* else_label = STRcatn(3, "condexpr_", count, "_else");
    branch_f(else_label);

    CONDITIONALEXPR_IFBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_IFBRANCH(arg_node), arg_info);

    jump(end_label);

    label(else_label);
    MEMfree(else_label);

    CONDITIONALEXPR_ELSEBRANCH(arg_node) = TRAVdo(CONDITIONALEXPR_ELSEBRANCH(arg_node), arg_info);

    label(end_label);
    MEMfree(end_label);

    DBUG_RETURN(arg_node);
}

node *GBCarrayinit(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCarrayinit");
    symbol_table_entry* entry = ARRAYINIT_VARPOINTER(arg_node);

    ARRAYINIT_SIZE(arg_node) = TRAVdo(ARRAYINIT_SIZE(arg_node), arg_info);

    switch(entry->vartype) {
        case TP_int_array:
            inewa();
            break;
        case TP_float_array:
            fnewa();
            break;
        case TP_bool_array:
            bnewa();
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid array type");
            CTIabort("Internal compiler error!");
            break;
    }

    if(entry->table->table_depth == 0) {
        if(entry->isimport) {
            astoree((uint16_t) entry->idx);
        } else {
            astoreg((uint16_t) entry->idx);
        }
    } else if(entry->table->table_depth == DEPTH(arg_info)) {
        astore((uint8_t) entry->idx);
    } else {
        DBUG_ASSERT((FALSE), "Can't init an array from upper scope!");
        CTIabort("Internal compiler error!");
    }

    DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */

node *GBCdoGenByteCode(node *syntaxtree) {
    DBUG_ENTER("GBCdoGenByteCode");

    info* arg_info = GBCmakeInfo();

    TRAVpush(TR_gbc);
    set_globalvar_idx(PROGRAM_VARSYMBOLTABLE(syntaxtree), arg_info);
    set_globalfun_idx(PROGRAM_FUNSYMBOLTABLE(syntaxtree), arg_info);

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
