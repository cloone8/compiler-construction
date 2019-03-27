#ifndef __PSEUDO_INSTRUCTIONS_H__
#define __PSEUDO_INSTRUCTIONS_H__

#include "types.h"
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>

#define INDEX_NOT_SET SIZE_MAX

typedef struct {
    char* name;
    type ret_type;
    size_t paramcount;
    type* paramtypes;
    char* fun_label;
} exportfun_t;

typedef struct {
    char* name;
    type ret_type;
    size_t paramcount;
    type* paramtypes;
} importfun_t;

typedef struct {
    char* name;
    uint64_t global_index;
} exportvar_t;

typedef struct {
    char* name;
    type vartype;
} importvar_t;

typedef union {
    int intval;
    float fltval;
    bool boolval;
} constval_t;

typedef struct {
    type consttype;
    constval_t val;
} const_t;

typedef struct {
    type globaltype;
} global_t;

typedef enum {
    INSTR_exportfun,
    INSTR_importfun,
    INSTR_exportvar,
    INSTR_importvar,
    INSTR_const,
    INSTR_global,
} instr_type;

typedef struct {
    size_t index;
    instr_type type;
    union {
        exportfun_t* exportfun;
        importfun_t* importfun;
        exportvar_t* exportvar;
        importvar_t* importvar;
        const_t* constant;
        global_t* global;
    };
} instruction;

typedef struct {
    size_t itemcount;
    instr_type pseudo_instruction_type;
    instruction** entries;
} pseudo_instructions;


extern pseudo_instructions* PSI_make(instr_type type);

extern void PSI_free(pseudo_instructions* tofree);
extern void PSI_add(pseudo_instructions* table, instruction* to_add);

extern instruction* PSI_make_exportfun(char* name, type ret_type, size_t paramcount, type* paramtypes, char* fun_label);
extern instruction* PSI_make_importfun(char* name, type ret_type, size_t paramcount, type* paramtypes);
extern instruction* PSI_make_exportvar(char* name, uint16_t global_index);
extern instruction* PSI_make_importvar(char* name, type vartype);
extern instruction* PSI_make_const(type consttype, constval_t value);
extern instruction* PSI_make_global(type globaltype);
extern instruction* PSI_find_constant(pseudo_instructions* table, type consttype, constval_t value);

#endif
