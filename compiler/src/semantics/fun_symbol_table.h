#ifndef __FUN_SYMBOL_TABLE_H__
#define __FUN_SYMBOL_TABLE_H__

#include "types.h"
#include <stdlib.h>

struct __fun_symbol_table_entry {
    size_t idx;
    char* name;
    type vartype;
    type* paramtypes;
};

typedef struct __fun_symbol_table_entry fun_symbol_table_entry;

struct __fun_symbol_table {
    size_t next_id;
    size_t current_entries;
    size_t __entry_array_size;
    struct __symbol_table* upper;
    fun_symbol_table_entry** entries;
};

typedef struct __fun_symbol_table fun_symbol_table;

fun_symbol_table* fun_symbol_table_make(fun_symbol_table* parent);
int fun_symbol_table_free(fun_symbol_table* tofree);
int fun_symbol_table_add(fun_symbol_table* table, char* name, type var_type);
fun_symbol_table_entry* fun_symbol_table_get_name(fun_symbol_table* table, char* name);
fun_symbol_table_entry* fun_symbol_table_get_idx(fun_symbol_table* table, size_t idx);
int fun_symbol_table_update(fun_symbol_table* table, size_t idx_to_update, char* new_name);
fun_symbol_table* fun_symbol_table_move_up(fun_symbol_table* table, size_t scopes);

#endif
