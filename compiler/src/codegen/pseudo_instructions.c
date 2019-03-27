#include "pseudo_instructions.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"

// Shorthand
#define ps_in pseudo_instructions

ps_in* PSI_make(instr_type type) {
    DBUG_ENTER("PSI_make");
    ps_in* toreturn = NULL;

    toreturn = MEMmalloc(sizeof(ps_in));
    toreturn->itemcount = 0;
    toreturn->pseudo_instruction_type = type;
    toreturn->entries = NULL;

    DBUG_RETURN(toreturn);
}

void PSI_free(ps_in* tofree) {
    DBUG_ENTER("PSI_free");
    DBUG_ASSERT((tofree != NULL), "tofree was NULL!");

    for(size_t i = 0; i < tofree->itemcount; i++) {
        instruction* current_item = tofree->entries[i];
        switch(tofree->pseudo_instruction_type) {
            case INSTR_exportfun:
                current_item->exportfun->name = MEMfree(current_item->exportfun->name);
                current_item->exportfun->fun_label = MEMfree(current_item->exportfun->fun_label);
                current_item->exportfun = MEMfree(current_item->exportfun);
                break;
            case INSTR_importfun:
                current_item->importfun->name = MEMfree(current_item->importfun->name);
                current_item->importfun = MEMfree(current_item->importfun);
                break;
            case INSTR_exportvar:
                current_item->exportvar->name = MEMfree(current_item->exportvar->name);
                current_item->exportvar = MEMfree(current_item->exportvar);
                break;
            case INSTR_importvar:
                current_item->importvar->name = MEMfree(current_item->importvar->name);
                current_item->importvar = MEMfree(current_item->importvar);
                break;
            case INSTR_const:
                current_item->constant = MEMfree(current_item->constant);
                break;
            case INSTR_global:
                current_item->global = MEMfree(current_item->global);
                break;
        }

        current_item = MEMfree(current_item);
    }

    MEMfree(tofree->entries);
    MEMfree(tofree);

    DBUG_VOID_RETURN;
}

void PSI_add(ps_in* table, instruction* to_add) {
    DBUG_ENTER("PSI_add");

    DBUG_ASSERT((table != NULL), "Table was NULL!");
    DBUG_ASSERT((to_add != NULL), "to_add was NULL!");
    DBUG_ASSERT((table->pseudo_instruction_type == to_add->type), "Non-matching PSI types!");

    table->itemcount++;
    if(table->entries != NULL) {
        table->entries = MEMrealloc(table->entries, table->itemcount * sizeof(instruction*));
    } else {
         table->entries = MEMmalloc(table->itemcount * sizeof(instruction*));
    }
    table->entries[table->itemcount - 1] = to_add;
    to_add->index = table->itemcount - 1;

    DBUG_VOID_RETURN;
}

instruction* PSI_make_exportfun(char* name, type ret_type, size_t paramcount, type* paramtypes, char* fun_label) {
    DBUG_ENTER("PSI_make_exportfun");
    DBUG_ASSERT((name != NULL), "Name was NULL!");
    DBUG_ASSERT((fun_label != NULL), "fun_label was NULL!");
    DBUG_ASSERT((ret_type != TP_unknown), "Invalid ret_type!");
    DBUG_ASSERT((paramcount == 0 && paramtypes == NULL) ||
                (paramcount > 0 && paramtypes != NULL),
                "paramtypes problem!");

    instruction* toreturn = MEMmalloc(sizeof(instruction));
    toreturn->type = INSTR_exportfun;
    toreturn->index = INDEX_NOT_SET;

    toreturn->exportfun = MEMmalloc(sizeof(exportfun_t));
    toreturn->exportfun->name = STRcpy(name);
    toreturn->exportfun->ret_type = ret_type;
    toreturn->exportfun->paramcount = paramcount;
    toreturn->exportfun->paramtypes = paramtypes;
    toreturn->exportfun->fun_label = STRcpy(fun_label);

    DBUG_RETURN(toreturn);
}

instruction* PSI_make_importfun(char* name, type ret_type, size_t paramcount, type* paramtypes) {
    DBUG_ENTER("PSI_make_importfun");
    DBUG_ASSERT((name != NULL), "Name was NULL!");
    DBUG_ASSERT((ret_type != TP_unknown), "Invalid ret_type!");
    DBUG_ASSERT((paramcount == 0 && paramtypes == NULL) ||
                (paramcount > 0 && paramtypes != NULL),
                "paramtypes problem!");

    instruction* toreturn = MEMmalloc(sizeof(instruction));
    toreturn->type = INSTR_importfun;
    toreturn->index = INDEX_NOT_SET;

    toreturn->importfun = MEMmalloc(sizeof(importfun_t));
    toreturn->importfun->name = STRcpy(name);
    toreturn->importfun->ret_type = ret_type;
    toreturn->importfun->paramcount = paramcount;
    toreturn->importfun->paramtypes = paramtypes;

    DBUG_RETURN(toreturn);
}

instruction* PSI_make_exportvar(char* name, uint16_t global_index) {
    DBUG_ENTER("PSI_make_exportvar");
    DBUG_ASSERT((name != NULL), "Name was NULL!");

    instruction* toreturn = MEMmalloc(sizeof(instruction));
    toreturn->type = INSTR_exportvar;
    toreturn->index = INDEX_NOT_SET;

    toreturn->exportvar = MEMmalloc(sizeof(exportvar_t));
    toreturn->exportvar->name = STRcpy(name);
    toreturn->exportvar->global_index = global_index;

    DBUG_RETURN(toreturn);
}

instruction* PSI_make_importvar(char* name, type vartype) {
    DBUG_ENTER("PSI_make_importvar");
    DBUG_ASSERT((vartype != TP_unknown && vartype != TP_void), "Incorrect vartype!");
    DBUG_ASSERT((name != NULL), "Name was NULL!");

    instruction* toreturn = MEMmalloc(sizeof(instruction));
    toreturn->type = INSTR_importvar;
    toreturn->index = INDEX_NOT_SET;

    toreturn->importvar = MEMmalloc(sizeof(importvar_t));
    toreturn->importvar->name = STRcpy(name);
    toreturn->importvar->vartype = vartype;

    DBUG_RETURN(toreturn);
}

instruction* PSI_make_const(type consttype, constval_t value) {
    DBUG_ENTER("PSI_make_const");
    DBUG_ASSERT((consttype == TP_bool || consttype == TP_float || consttype == TP_int), "Incorrect constant type!");

    instruction* toreturn = MEMmalloc(sizeof(instruction));
    toreturn->type = INSTR_const;
    toreturn->index = INDEX_NOT_SET;

    toreturn->constant = MEMmalloc(sizeof(const_t));
    toreturn->constant->consttype = consttype;

    switch(consttype) {
        case TP_bool:
            toreturn->constant->val.boolval = value.boolval;
            break;
        case TP_int:
            toreturn->constant->val.intval = value.intval;
            break;
        case TP_float:
            toreturn->constant->val.fltval = value.fltval;
            break;
        default:
            DBUG_ASSERT((FALSE), "Wrong consttype!");
            break;
    }

    DBUG_RETURN(toreturn);
}

instruction* PSI_make_global(type globaltype) {
    DBUG_ENTER("PSI_make_global");
    DBUG_ASSERT((globaltype != TP_unknown && globaltype != TP_void), "Incorrect globaltype!");

    instruction* toreturn = MEMmalloc(sizeof(instruction));
    toreturn->type = INSTR_global;
    toreturn->index = INDEX_NOT_SET;

    toreturn->global = MEMmalloc(sizeof(global_t));
    toreturn->global->globaltype = globaltype;

    DBUG_RETURN(toreturn);
}

instruction* PSI_find_constant(ps_in* table, type consttype, constval_t value) {
    DBUG_ENTER("PSI_find_constant");
    DBUG_ASSERT((table != NULL), "Table was NULL!");
    DBUG_ASSERT((table->pseudo_instruction_type == INSTR_const), "Wront table type!");

    for(size_t i = 0; i < table->itemcount; i++) {
        instruction* current_entry = table->entries[i];
        if(current_entry->constant->consttype == consttype) {
            switch(consttype) {
                case TP_bool:
                    if(current_entry->constant->val.boolval == value.boolval) {
                        DBUG_RETURN(current_entry);
                    }
                    break;
                case TP_int:
                    if(current_entry->constant->val.intval == value.intval) {
                        DBUG_RETURN(current_entry);
                    }
                    break;
                case TP_float:
                    if(current_entry->constant->val.fltval == value.fltval) {
                        DBUG_RETURN(current_entry);
                    }
                    break;
                default:
                    DBUG_ASSERT((FALSE), "Incorrect constant type!");
                    break;
            }
        }
    }


    DBUG_RETURN(NULL);
}
