#ifndef __VAR_SYMBOL_TABLE_H__
#define __VAR_SYMBOL_TABLE_H__

#include "types.h"
#include <stdlib.h>

struct __var_symbol_table_entry {
    size_t idx;
    char* name;
    type vartype;
};

typedef struct __var_symbol_table_entry var_symbol_table_entry;

struct __var_symbol_table {
    size_t next_id;
    size_t current_entries;
    size_t __entry_array_size;
    var_symbol_table_entry** entries;
};

typedef struct __var_symbol_table var_symbol_table;

var_symbol_table* var_symbol_table_make(void);
int var_symbol_table_free(var_symbol_table* tofree);
int var_symbol_table_add(var_symbol_table* table, char* name, type var_type);
var_symbol_table_entry* var_symbol_table_get_name(var_symbol_table* table, char* name);
var_symbol_table_entry* var_symbol_table_get_idx(var_symbol_table* table, size_t idx);
int var_symbol_table_update(var_symbol_table* table, size_t idx_to_update, char* new_name);

#endif
