#ifndef __INSTRUCTION_LIB_H__
#define __INSTRUCTION_LIB_H__

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
 * @param name: The label name WITHOUT the trailing ":"
 */
extern void label(const char* name);

/**
 * Pops two values from the top of the stack, adds them and pushes
 * the result onto the stack
 */

extern void iadd(void);
extern void fadd(void);

/**
 * Pops two values from the top of the stack, subtracts the first popped
 * value from the second popped value and pushes the result onto the stack
 */

extern void isub(void);
extern void fsub(void);

/**
 * Pops two values from the top of the stack, multiplies them and
 * pushes the result onto the stack
 */

extern void imul(void);
extern void fmul(void);

/**
 * Pops two values from the top of the stack, divides the second popped value
 * by the first popped value and pushes the result onto the stack
 */

extern void idiv(void);
extern void fdiv(void);

/**
 * Pops two values from the top of the stack, divides the second popped value
 * by the first popped value (i.e., the values are popped right-to-left) and
 * pushes the remainder onto the stack
 */
extern void irem(void);

/**
 * Pops a value from the top of the stack, negates it and pushes the
 * result onto the stack
 */

extern void ineg(void);
extern void fneg(void);
extern void bnot(void);

/**
 * Increments or decrements the local variable identified by local with the value
 * loaded from the constant table at index C
 */

extern void iinc(uint8_t local, const_t constant);
extern void idec(uint8_t local, const_t constant);

/**
 * Increments or decrements the local variable identified by local with 1
 */

extern void iinc_1(uint8_t local);
extern void idec_1(uint8_t local);

/**
 * Pops two values from the top of the stack, calculates disjunction or
 * conjunction and pushes the result onto the stack. badd calculates
 * disjunction, bmul calculates conjunction.
 */

extern void badd(void);
extern void bmul(void);

/**
 * These instructions pop two values off the top of the stack, compare them,
 * and push the boolean result onto the stack
 */

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

/**
 * Initiate Subroutine call
 */

extern void isr(void);
extern void isrn(uint8_t nesting_levels_higher);
extern void isrl(void);
extern void isrg(void);

/**
 * Jump to Subroutine
 */

extern void jsr(uint8_t num_arguments, char* jump_to);
extern void jsre(uint8_t import_index);

/**
 * Enter Subroutine
 */
extern void esr(uint8_t var_count);


/**
 * Restores the stack top to the current subroutine’s activation record,
 * set the current program counter to the return address in the activation
 * record and pops the activation record. ireturn, freturn and breturn
 * additionally pop an integer, floating-point or boolean value off the top of
 * the stack at the beginning, and push that value onto the stack at the end.
 */

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

/**
 * These instructions either (unconditionally) jump or conditionally branch to
 * the given label
 */
extern void jump(char* jump_to);

extern void branch_t(char* branch_to);
extern void branch_f(char* branch_to);

/**
 * Reads a local variable and pushes its value onto the stack
 */

extern void iload(uint8_t to_load);
extern void fload(uint8_t to_load);
extern void bload(uint8_t to_load);
extern void aload(uint8_t to_load);

/**
 * Reads a local variable from a higher frame and pushes its value
 * onto the stack
 */

extern void iloadn(uint8_t scopes_above, uint8_t to_load);
extern void floadn(uint8_t scopes_above, uint8_t to_load);
extern void bloadn(uint8_t scopes_above, uint8_t to_load);
extern void aloadn(uint8_t scopes_above, uint8_t to_load);

/**
 * Loads a value from the module’s global table at the given index
 * and pushes it onto the stack
 */

extern void iloadg(uint16_t global_index);
extern void floadg(uint16_t global_index);
extern void bloadg(uint16_t global_index);
extern void aloadg(uint16_t global_index);

/**
 * Loads a value from the module’s variable import table at the given index
 * and pushes it onto the stack
 */

extern void iloade(uint16_t import_table_index);
extern void floade(uint16_t import_table_index);
extern void bloade(uint16_t import_table_index);
extern void aloade(uint16_t import_table_index);

/**
 * Push a constant value onto the stack.
 * iloadc, floadc and bloadc load the constant value from the constant table,
 * at the given index. The other instructions push the following values as
 * indicated in their description.
 */

extern void iloadc(uint16_t constant_index);
extern void floadc(uint16_t constant_index);
extern void bloadc(uint16_t constant_index);

extern void iloadc_0(void);
extern void iloadc_1(void);
extern void iloadc_m1(void);

extern void floadc_0(void);
extern void floadc_1(void);

extern void bloadc_t(void);
extern void bloadc_f(void);

/**
 * Pops a value from the stack and stores it into a local variable
 */

extern void istore(uint8_t to_store);
extern void fstore(uint8_t to_store);
extern void bstore(uint8_t to_store);
extern void astore(uint8_t to_store);

/**
 * Pops a value from a higher scope from the stack and stores it
 * into a local variable
 */

extern void istoren(uint8_t scopes_above, uint8_t to_load);
extern void fstoren(uint8_t scopes_above, uint8_t to_load);
extern void bstoren(uint8_t scopes_above, uint8_t to_load);
// No astoren???

/**
 * Pops a value from the stack and stores it into the
 * module’s global table at the given index
 */

extern void istoreg(uint16_t global_index);
extern void fstoreg(uint16_t global_index);
extern void bstoreg(uint16_t global_index);
extern void astoreg(uint16_t global_index);

/**
 * Pops a value from the stack and stores it into the variable at
 * the given index of the module’s variable import table
 * (in the global table of another module)
 */

extern void istoree(uint16_t import_table_index);
extern void fstoree(uint16_t import_table_index);
extern void bstoree(uint16_t import_table_index);
extern void astoree(uint16_t import_table_index);

/**
 * inewa, fnewa and bnewa create a new (one-dimensional) array of integer,
 * floating-point or boolean values, respectively. These instructions pop an
 * integer value off the top of the stack (the array size to allocate), and
 * push a reference to the created array onto the stack.
 */

extern void inewa(void);
extern void fnewa(void);
extern void bnewa(void);

/**
 * Pops an array reference off the top of the stack. Then pops an integer
 * off the top of the stack, which is used to index the array. Then, iloada,
 * floada, bloada take the indexed array integer, floating-point or boolean
 * element, respectively, and push it on top of the stack.
 */

extern void iloada(void);
extern void floada(void);
extern void bloada(void);

/**
 * Pops an array reference off the top of the stack. Then pops an integer
 * off the top of the stack, which is used to index the array in order of
 *  popping them off the stack. Then, istorea, fstorea, bstorea pop an integer,
 * floating-point or boolean value off the top of the stack and store that
 * in the indexed array element.
 */

extern void istorea(void);
extern void fstorea(void);
extern void bstorea(void);

/**
 * Type conversion instructions
 */

extern void i2f(void);
extern void f2i(void);

/**
 * ipop, fpop and bpop pop an integer number, floating-point number
 * and boolean value off the top of the stack, respectively. The
 * popped value is discarded
 */

extern void ipop(void);
extern void fpop(void);
extern void bpop(void);

#endif
