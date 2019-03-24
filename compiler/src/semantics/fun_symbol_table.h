#ifndef __FUN_SYMBOL_TABLE_H__
#define __FUN_SYMBOL_TABLE_H__

#include "types.h"
#include <stdlib.h>

struct __fun_symbol_table;
typedef struct __fun_symbol_table fun_symbol_table;
struct __fun_symbol_table_entry;
typedef struct __fun_symbol_table_entry fun_symbol_table_entry;

struct __fun_symbol_table_entry {
    size_t idx;
    char* name;
    type returntype;
    size_t paramamount;
    type* paramtypes;
    fun_symbol_table* table;
};

struct __fun_symbol_table {
    size_t id;
    size_t __next_id;
    size_t current_entries;
    size_t table_depth;
    size_t __entry_array_size;
    fun_symbol_table* upper;
    fun_symbol_table_entry** entries;
};

fun_symbol_table* fun_symbol_table_make(fun_symbol_table* parent, size_t table_depth);
int fun_symbol_table_free(fun_symbol_table* tofree);
void fun_symbol_table_remove(fun_symbol_table* table, size_t idx_to_delete);
fun_symbol_table_entry* fun_symbol_table_add(fun_symbol_table *table, char *name, type return_type, size_t param_amount, type* param_types);
fun_symbol_table_entry* fun_symbol_table_get_name(fun_symbol_table* table, char* name);
fun_symbol_table_entry* fun_symbol_table_get_idx(fun_symbol_table* table, size_t idx);
int fun_symbol_table_update(fun_symbol_table* table, size_t idx_to_update, char* new_name);
fun_symbol_table* fun_symbol_table_move_up(fun_symbol_table* table, size_t scopes);
type *fun_symbol_table_get_param_types(fun_symbol_table *table, char *fun_name);
size_t fun_symbol_table_get_param_amount(fun_symbol_table * table, char *fun_name);
void print_fun_symbol_table(fun_symbol_table *table);
void print_fun_symbol_table_chain(fun_symbol_table *table);

#endif
