#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include "types.h"
#include <stdlib.h>

struct __symbol_table_entry {
    size_t idx;
    char* name;
    type vartype;
};

typedef struct __symbol_table_entry symbol_table_entry;

struct __symbol_table {
    size_t next_id;
    size_t current_entries;
    size_t __entry_array_size;
    symbol_table_entry** entries;
};

typedef struct __symbol_table symbol_table;

symbol_table* symbol_table_make(void);
int symbol_table_free(symbol_table* tofree);
int symbol_table_add(symbol_table* table, char* name, type var_type);
symbol_table_entry* symbol_table_get_name(symbol_table* table, char* name);
symbol_table_entry* symbol_table_get_idx(symbol_table* table, size_t idx);
int symbol_table_update(symbol_table* table, size_t idx_to_update, char* new_name);

#endif
