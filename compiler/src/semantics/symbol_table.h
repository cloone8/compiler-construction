#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include "types.h"
#include <stdlib.h>

struct __symbol_table_entry;
typedef struct __symbol_table_entry symbol_table_entry;
struct __symbol_table;
typedef struct __symbol_table symbol_table;

struct __symbol_table_entry {
    size_t idx;
    char* name;
    type vartype;
    symbol_table* table;
};

struct __symbol_table {
    size_t id;
    size_t __next_id;
    size_t table_depth;
    size_t current_entries;
    size_t __entry_array_size;
    symbol_table* upper;
    symbol_table_entry** entries;
};

symbol_table* symbol_table_make(symbol_table* parent, size_t table_depth);
int symbol_table_free(symbol_table* tofree);
int symbol_table_free_no_entries(symbol_table* tofree);
symbol_table_entry* symbol_table_add(symbol_table* table, char* name, type var_type);
void symbol_table_insert(symbol_table* table, symbol_table_entry* to_insert);
symbol_table_entry* symbol_table_get_name(symbol_table* table, char* name);
symbol_table_entry* symbol_table_get_idx(symbol_table* table, size_t idx);
int symbol_table_update(symbol_table* table, size_t idx_to_update, char* new_name);
symbol_table* symbol_table_move_up(symbol_table* table, size_t scopes);
void print_symbol_table(symbol_table *table);
void print_symbol_table_chain(symbol_table *table);

#endif
