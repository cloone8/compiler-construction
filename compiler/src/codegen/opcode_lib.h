#ifndef __OPCODE_LIB_H__
#define __OPCODE_LIB_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef INT64_MAX
// Platform support 64 bit ints

/**
 * Type to be used for all constants given to the opcode lib
 */
typedef int64_t const_t;
#define PRIcon "%" PRId64
#else
// Platform only supports 32 bit ints

/**
 * Type to be used for all constants given to the opcode lib
 */
typedef int32_t const_t;
#define PRIcon "%" PRId32
#endif

/**
 * The file struct to write all the code to.
 */
extern FILE* ofile;

/**
 * Prints the given label to assembly.
 * @param name: The label name WITHOUT the trailing :
 */
extern void label(const char* name);

extern void iadd(void);
extern void fadd(void);

extern void isub(void);
extern void fsub(void);

extern void imul(void);
extern void fmul(void);

extern void idiv(void);
extern void fdiv(void);

extern void irem(void);

extern void ineg(void);
extern void fneg(void);
extern void bnot(void);

extern void iinc(uint8_t local, const_t constant);
extern void idec(uint8_t local, const_t constant);

extern void badd(void);
extern void bmul(void);

extern void ine(void);
extern void ieq(void);
extern void ilt(void);
extern void ile(void);
extern void igt(void);
extern void ige(void);

extern void fne(void);
extern void feq(void);
extern void flt(void);
extern void fle(void);
extern void fgt(void);
extern void fge(void);

extern void bne(void);
extern void beq(void);

extern void isr(void);
extern void isrn(uint8_t nesting_levels_higher);

extern void isrl(void);
extern void isrg(void);

extern void jsr(uint8_t num_arguments, char* jump_to);
extern void jsre(uint8_t import_index);
extern void esr(uint8_t var_count);

/**
 * NOTE: Instruction is called "return" but that is a C keyword
 */
extern void ret(void);

/**
 * NOTE: Instruction is called "ireturn" but that is a C keyword
 */
extern void iret(void);

/**
 * NOTE: Instruction is called "freturn" but that is a C keyword
 */
extern void fret(void);

/**
 * NOTE: Instruction is called "breturn" but that is a C keyword
 */
extern void bret(void);

extern void jump(char* jump_to);

extern void branch_t(char* branch_to);
extern void branch_f(char* branch_to);

extern void iload(uint8_t to_load);
extern void fload(uint8_t to_load);
extern void bload(uint8_t to_load);
extern void aload(uint8_t to_load);

extern void iloadn(uint8_t scopes_above, uint8_t to_load);
extern void floadn(uint8_t scopes_above, uint8_t to_load);
extern void bloadn(uint8_t scopes_above, uint8_t to_load);
extern void aloadn(uint8_t scopes_above, uint8_t to_load);

extern void iloadg(uint16_t global_index);
extern void floadg(uint16_t global_index);
extern void bloadg(uint16_t global_index);
extern void aloadg(uint16_t global_index);

extern void iloade(uint16_t import_table_index);
extern void floade(uint16_t import_table_index);
extern void bloade(uint16_t import_table_index);
extern void aloade(uint16_t import_table_index);


#endif
