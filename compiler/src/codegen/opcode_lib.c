#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"

#include "opcode_lib.h"

FILE* ofile = NULL;

static void prt_lbl(const char* label) {
    DBUG_ENTER("prt_lbl");
    DBUG_ASSERT((label != NULL), "Label was NULL!");

    if(fprintf(ofile, "%s:\n", label) < 0) {
        // Error occured during writing. This means a
        // label could not be printed which
        // completely invalidates the code.

        CTIabort("Could not write assembly to output: %s", strerror(errno));
    }

    DBUG_VOID_RETURN;
}

static void prt_inst(const char* instruction) {
    DBUG_ENTER("prt_inst");
    DBUG_ASSERT((instruction != NULL), "Instruction was NULL!");

    if(fprintf(ofile, "    %s\n", instruction) < 0) {
        // Error occured during writing. This means an
        // instruction could not be printed which
        // completely invalidates the code.

        CTIabort("Could not write assembly to output: %s", strerror(errno));
    }

    DBUG_VOID_RETURN;
}

/**
 * Prints the given label to assembly.
 * @param name: The label name WITHOUT the trailing :
 */
void label(const char* name) {
    DBUG_ENTER("label");
    DBUG_ASSERT((name != NULL), "Name was NULL!");
    DBUG_ASSERT((STRlen(name) > 0), "Can't print label with length 0!");

    prt_lbl(name);

    DBUG_VOID_RETURN;
}

void iadd(void) {
    DBUG_ENTER("iadd");
    prt_inst("iadd");
    DBUG_VOID_RETURN;
}

void fadd(void) {
    DBUG_ENTER("fadd");
    prt_inst("fadd");
    DBUG_VOID_RETURN;
}

void isub(void) {
    DBUG_ENTER("isub");
    prt_inst("isub");
    DBUG_VOID_RETURN;
}

void fsub(void) {
    DBUG_ENTER("fsub");
    prt_inst("fsub");
    DBUG_VOID_RETURN;
}

void imul(void) {
    DBUG_ENTER("imul");
    prt_inst("imul");
    DBUG_VOID_RETURN;
}

void fmul(void) {
    DBUG_ENTER("fmul");
    prt_inst("fmul");
    DBUG_VOID_RETURN;
}

void idiv(void) {
    DBUG_ENTER("idiv");
    prt_inst("idiv");
    DBUG_VOID_RETURN;
}

void fdiv(void) {
    DBUG_ENTER("fdiv");
    prt_inst("fdiv");
    DBUG_VOID_RETURN;
}

void irem(void) {
    DBUG_ENTER("irem");
    prt_inst("irem");
    DBUG_VOID_RETURN;
}

void ineg(void) {
    DBUG_ENTER("ineg");
    prt_inst("ineg");
    DBUG_VOID_RETURN;
}

void fneg(void) {
    DBUG_ENTER("fneg");
    prt_inst("fneg");
    DBUG_VOID_RETURN;
}

void bnot(void) {
    DBUG_ENTER("bnot");
    prt_inst("bnot");
    DBUG_VOID_RETURN;
}

void iinc(uint8_t local, const_t constant) {
    DBUG_ENTER("iinc");

    if(constant == 1) {
        DBUG_PRINT("GBC", ("Picking iinc_1"));
        char instruction[15];
        sprintf(instruction, "iinc_1 %" PRIu8, local);
        prt_inst(instruction);
    } else {
        DBUG_PRINT("GBC", ("Picking iinc"));
        char instruction[15];
        sprintf(instruction, "iinc %" PRIu8 " " PRIcon, local, constant);
        prt_inst(instruction);
    }

    DBUG_VOID_RETURN;
}

void idec(uint8_t local, const_t constant) {
    DBUG_ENTER("idec");

    if(constant == 1) {
        DBUG_PRINT("GBC", ("Picking idec_1"));
        char instruction[15];
        sprintf(instruction, "idec_1 %" PRIu8, local);
        prt_inst(instruction);
    } else {
        DBUG_PRINT("GBC", ("Picking idec"));
        char instruction[15];
        sprintf(instruction, "idec %" PRIu8 " " PRIcon, local, constant);
        prt_inst(instruction);
    }

    DBUG_VOID_RETURN;
}

void badd(void) {
    DBUG_ENTER("badd");
    prt_inst("badd");
    DBUG_VOID_RETURN;
}

void bmul(void) {
    DBUG_ENTER("bmul");
    prt_inst("bmul");
    DBUG_VOID_RETURN;
}

void ine(void) {
    DBUG_ENTER("ine");
    prt_inst("ine");
    DBUG_VOID_RETURN;
}

void ieq(void) {
    DBUG_ENTER("ieq");
    prt_inst("ieq");
    DBUG_VOID_RETURN;
}

void ilt(void) {
    DBUG_ENTER("ilt");
    prt_inst("ilt");
    DBUG_VOID_RETURN;
}

void ile(void) {
    DBUG_ENTER("ile");
    prt_inst("ile");
    DBUG_VOID_RETURN;
}

void igt(void) {
    DBUG_ENTER("igt");
    prt_inst("igt");
    DBUG_VOID_RETURN;
}

void ige(void) {
    DBUG_ENTER("ige");
    prt_inst("ige");
    DBUG_VOID_RETURN;
}

void fne(void) {
    DBUG_ENTER("fne");
    prt_inst("fne");
    DBUG_VOID_RETURN;
}

void feq(void) {
    DBUG_ENTER("feq");
    prt_inst("feq");
    DBUG_VOID_RETURN;
}

void flt(void) {
    DBUG_ENTER("flt");
    prt_inst("flt");
    DBUG_VOID_RETURN;
}

void fle(void) {
    DBUG_ENTER("fle");
    prt_inst("fle");
    DBUG_VOID_RETURN;
}

void fgt(void) {
    DBUG_ENTER("fgt");
    prt_inst("fgt");
    DBUG_VOID_RETURN;
}

void fge(void) {
    DBUG_ENTER("fge");
    prt_inst("fge");
    DBUG_VOID_RETURN;
}

void bne(void) {
    DBUG_ENTER("bne");
    prt_inst("bne");
    DBUG_VOID_RETURN;
}

void beq(void) {
    DBUG_ENTER("beq");
    prt_inst("beq");
    DBUG_VOID_RETURN;
}

void isr(void) {
    DBUG_ENTER("isr");
    prt_inst("isr");
    DBUG_VOID_RETURN;
}

void isrn(uint8_t nesting_levels_higher) {
    DBUG_ENTER("isrn");
    char instruction[15];
    sprintf(instruction, "isrn %" PRIu8, nesting_levels_higher);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void isrl(void) {
    DBUG_ENTER("isrl");
    prt_inst("isrl");
    DBUG_VOID_RETURN;
}

void isrg(void) {
    DBUG_ENTER("isrg");
    prt_inst("isrg");
    DBUG_VOID_RETURN;
}

void jsr(uint8_t num_arguments, char* jump_to) {
    DBUG_ENTER("jsr");
    char instruction[20];
    sprintf(instruction, "jsr %" PRIu8 " %s", num_arguments, jump_to);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void jsre(uint8_t import_index) {
    DBUG_ENTER("jsr");
    char instruction[20];
    sprintf(instruction, "jsre %" PRIu8, import_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void esr(uint8_t var_count) {
    DBUG_ENTER("esr");
    char instruction[20];
    sprintf(instruction, "esr %" PRIu8, var_count);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

/**
 * NOTE: Instruction is called "return" but that is a C keyword
 */
void ret(void) {
    DBUG_ENTER("ret");
    prt_inst("return");
    DBUG_VOID_RETURN;
}

/**
 * NOTE: Instruction is called "ireturn" but that is a C keyword
 */
void iret(void) {
    DBUG_ENTER("iret");
    prt_inst("ireturn");
    DBUG_VOID_RETURN;
}

/**
 * NOTE: Instruction is called "freturn" but that is a C keyword
 */
void fret(void) {
    DBUG_ENTER("fret");
    prt_inst("freturn");
    DBUG_VOID_RETURN;
}
/**
 * NOTE: Instruction is called "breturn" but that is a C keyword
 */
void bret(void) {
    DBUG_ENTER("bret");
    prt_inst("breturn");
    DBUG_VOID_RETURN;
}

void jump(char* jump_to) {
    DBUG_ENTER("jump");
    char* instruction = MEMmalloc((6 + STRlen(jump_to)) * sizeof(char));
    sprintf(instruction, "jump %s", jump_to);
    prt_inst(instruction);
    MEMfree(instruction);
    DBUG_VOID_RETURN;
}

void branch_t(char* branch_to) {
    DBUG_ENTER("branch_t");
    char* instruction = MEMmalloc((10 + STRlen(branch_to)) * sizeof(char));
    sprintf(instruction, "branch_t %s", branch_to);
    prt_inst(instruction);
    MEMfree(instruction);
    DBUG_VOID_RETURN;
}

void branch_f(char* branch_to) {
    DBUG_ENTER("branch_f");
    char* instruction = MEMmalloc((10 + STRlen(branch_to)) * sizeof(char));
    sprintf(instruction, "branch_f %s", branch_to);
    prt_inst(instruction);
    MEMfree(instruction);
    DBUG_VOID_RETURN;
}

void iload(uint8_t to_load) {
    DBUG_ENTER("iload");
    char instruction[15];
    switch(to_load) {
        case 0:
            prt_inst("iload_0");
            break;
        case 1:
            prt_inst("iload_1");
            break;
        case 2:
            prt_inst("iload_2");
            break;
        case 3:
            prt_inst("iload_3");
            break;
        default:
            sprintf(instruction, "iload %" PRIu8, to_load);
            prt_inst(instruction);
            break;
    }
    DBUG_VOID_RETURN;
}

void fload(uint8_t to_load) {
    DBUG_ENTER("fload");
    char instruction[15];
    switch(to_load) {
        case 0:
            prt_inst("fload_0");
            break;
        case 1:
            prt_inst("fload_1");
            break;
        case 2:
            prt_inst("fload_2");
            break;
        case 3:
            prt_inst("fload_3");
            break;
        default:
            sprintf(instruction, "fload %" PRIu8, to_load);
            prt_inst(instruction);
            break;
    }
    DBUG_VOID_RETURN;
}

void bload(uint8_t to_load) {
    DBUG_ENTER("bload");
    char instruction[15];
    switch(to_load) {
        case 0:
            prt_inst("bload_0");
            break;
        case 1:
            prt_inst("bload_1");
            break;
        case 2:
            prt_inst("bload_2");
            break;
        case 3:
            prt_inst("bload_3");
            break;
        default:
            sprintf(instruction, "bload %" PRIu8, to_load);
            prt_inst(instruction);
            break;
    }
    DBUG_VOID_RETURN;
}

void aload(uint8_t to_load) {
    DBUG_ENTER("aload");
    char instruction[15];
    switch(to_load) {
        case 0:
            prt_inst("aload_0");
            break;
        case 1:
            prt_inst("aload_1");
            break;
        case 2:
            prt_inst("aload_2");
            break;
        case 3:
            prt_inst("aload_3");
            break;
        default:
            sprintf(instruction, "aload %" PRIu8, to_load);
            prt_inst(instruction);
            break;
    }
    DBUG_VOID_RETURN;
}

void iloadn(uint8_t scopes_above, uint8_t to_load) {
    DBUG_ENTER("iloadn");
    char instruction[25];
    sprintf(instruction, "iloadn %" PRIu8 " %" PRIu8, scopes_above, to_load);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void floadn(uint8_t scopes_above, uint8_t to_load) {
    DBUG_ENTER("floadn");
    char instruction[25];
    sprintf(instruction, "floadn %" PRIu8 " %" PRIu8, scopes_above, to_load);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void bloadn(uint8_t scopes_above, uint8_t to_load) {
    DBUG_ENTER("bloadn");
    char instruction[25];
    sprintf(instruction, "bloadn %" PRIu8 " %" PRIu8, scopes_above, to_load);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void aloadn(uint8_t scopes_above, uint8_t to_load) {
    DBUG_ENTER("aloadn");
    char instruction[25];
    sprintf(instruction, "aloadn %" PRIu8 " %" PRIu8, scopes_above, to_load);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void iloadg(uint16_t global_index) {
    DBUG_ENTER("iloadg");
    char instruction[25];
    sprintf(instruction, "iloadg %" PRIu16, global_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void floadg(uint16_t global_index) {
    DBUG_ENTER("floadg");
    char instruction[25];
    sprintf(instruction, "floadg %" PRIu16, global_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void bloadg(uint16_t global_index) {
    DBUG_ENTER("bloadg");
    char instruction[25];
    sprintf(instruction, "bloadg %" PRIu16, global_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void aloadg(uint16_t global_index) {
    DBUG_ENTER("aloadg");
    char instruction[25];
    sprintf(instruction, "aloadg %" PRIu16, global_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void iloade(uint16_t import_table_index) {
    DBUG_ENTER("iloade");
    char instruction[25];
    sprintf(instruction, "iloade %" PRIu16, import_table_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void floade(uint16_t import_table_index) {
    DBUG_ENTER("floade");
    char instruction[25];
    sprintf(instruction, "floade %" PRIu16, import_table_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void bloade(uint16_t import_table_index) {
    DBUG_ENTER("bloade");
    char instruction[25];
    sprintf(instruction, "bloade %" PRIu16, import_table_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}

void aloade(uint16_t import_table_index) {
    DBUG_ENTER("aloade");
    char instruction[25];
    sprintf(instruction, "aloade %" PRIu16, import_table_index);
    prt_inst(instruction);
    DBUG_VOID_RETURN;
}
