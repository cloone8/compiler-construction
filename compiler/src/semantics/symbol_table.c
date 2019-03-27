#include <stdlib.h>
#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "globals.h"

#include "symbol_table.h"


symbol_table* symbol_table_make(symbol_table* parent, size_t table_depth) {
    DBUG_ENTER("symbol_table_make");
    DBUG_PRINT("symbol_table", ("Making new symbol table\n"));

    symbol_table* toreturn = MEMmalloc(sizeof(symbol_table));

    toreturn->id = myglobal.varsymtable_num++;
    toreturn->__entry_array_size = 10;
    toreturn->__next_id = 0;
    toreturn->current_entries = 0;
    toreturn->upper = parent;
    toreturn->table_depth = table_depth;
    toreturn->entries = MEMmalloc(sizeof(symbol_table_entry*) * 10);

    DBUG_RETURN(toreturn);
}

int symbol_table_free(symbol_table* tofree) {
    DBUG_ENTER("symbol_table_free");
    DBUG_ASSERT((tofree != NULL), "Table was NULL!");

    for(size_t i = 0; i < tofree->current_entries; i++) {
        symbol_table_entry* current = tofree->entries[i];
        current->name = MEMfree(current->name);
        MEMfree(current);
    }

    MEMfree(tofree->entries);
    MEMfree(tofree);

    DBUG_RETURN(EXIT_SUCCESS);
}

int symbol_table_free_no_entries(symbol_table* tofree) {
    DBUG_ENTER("symbol_table_free_no_entries");
    DBUG_ASSERT((tofree != NULL), "Table was NULL!");

    MEMfree(tofree->entries);
    MEMfree(tofree);

    DBUG_RETURN(EXIT_SUCCESS);
}

void symbol_table_remove(symbol_table* table, size_t idx_to_delete) {
    DBUG_ENTER("symbol_table_remove");
    DBUG_ASSERT((table != NULL), "Table was NULL!");
    DBUG_ASSERT((idx_to_delete < table->__next_id), "idx too high!");

    symbol_table_entry* entry = symbol_table_get_idx(table, idx_to_delete);

    table->current_entries--;
    table->__next_id--;
    entry->name = MEMfree(entry->name);

    if(entry->idx != table->current_entries) {
        // Take entry from the top and add it in the middle
        table->entries[entry->idx] = table->entries[table->current_entries];
        table->entries[entry->idx]->idx = entry->idx;
        table->entries[table->current_entries] = NULL;
    }

    MEMfree(entry);
    DBUG_VOID_RETURN;
}

static void increase_table_size(symbol_table* toincrease) {
    DBUG_ENTER("increase_table_size");

    size_t extra_size;

    if(toincrease->__entry_array_size * 0.5 > 10) {
        extra_size = 10;
    } else if(toincrease->__entry_array_size * 0.5 < 1) {
        extra_size = 1;
    } else {
        extra_size = toincrease->__entry_array_size * 0.5;
    }

    toincrease->__entry_array_size += extra_size;

    toincrease->entries = MEMrealloc(toincrease->entries, toincrease->__entry_array_size * sizeof(symbol_table_entry*));

    DBUG_VOID_RETURN;
}

symbol_table_entry* symbol_table_add(symbol_table* table, char* name, type var_type, bool isimport) {
    DBUG_ENTER("symbol_table_add");
    DBUG_ASSERT((table != NULL), "table was NULL!");
    DBUG_ASSERT((name != NULL), "name is NULL!");

    if (symbol_table_get_name(table, name) != NULL) {
        CTIerror("Var %s already in scope", name);
        DBUG_RETURN(NULL);
    }

    symbol_table_entry* new_entry = MEMmalloc(sizeof(symbol_table_entry));
    new_entry->idx = table->__next_id++;
    new_entry->name = STRcpy(name);
    new_entry->vartype = var_type;
    new_entry->table = table;
    new_entry->isimport = isimport;

    table->current_entries++;

    if(table->current_entries > table->__entry_array_size) {
        increase_table_size(table);
    }


    table->entries[table->current_entries - 1] = new_entry;

    DBUG_ASSERT((table->entries[table->current_entries - 1] != NULL), "Couldn't add new entry!");
    DBUG_PRINT("symbol_table", ("Added %zu %s %d\n", new_entry->idx, new_entry->name, new_entry->vartype));
    DBUG_RETURN(new_entry);
}

void symbol_table_insert(symbol_table* table, symbol_table_entry* to_insert) {
    DBUG_ENTER("symbol_table_insert");
    DBUG_ASSERT((table != NULL), "table was NULL!");
    DBUG_ASSERT((to_insert != NULL), "to_insert is NULL!");

    if (symbol_table_get_name(table, to_insert->name) != NULL) {
        CTIerror("Var %s already in scope", to_insert->name);
        DBUG_VOID_RETURN;
    }

    table->current_entries++;

    if(table->current_entries > table->__entry_array_size) {
        increase_table_size(table);
    }


    table->entries[table->current_entries - 1] = to_insert;
    to_insert->idx = table->current_entries - 1;
    to_insert->table = table;

    DBUG_ASSERT((table->entries[table->current_entries - 1] != NULL), "Couldn't add new entry!");
    DBUG_PRINT("symbol_table", ("Inserted %zu %s %d\n", to_insert->idx, to_insert->name, to_insert->vartype));
    DBUG_VOID_RETURN;
}

symbol_table_entry* symbol_table_get_name(symbol_table* table, char* name) {
    DBUG_ENTER("symbol_table_get_name");
    DBUG_ASSERT((table != NULL), "Table was NULL!");
    DBUG_PRINT("symbol_table_get_name", ("Finding name %s", name));

    for(size_t i = 0; i < table->current_entries; i++) {
        symbol_table_entry* current = table->entries[i];
        if(STReq(current->name, name)) {
            DBUG_PRINT("SYMBOL_TABLE_LOOKUP", ("Found name %s", name));
            DBUG_RETURN(current);
        }
    }

    DBUG_PRINT("SYMBOL_TABLE_LOOKUP", ("Did not find name %s", name));
    DBUG_RETURN(NULL);
}

symbol_table_entry* symbol_table_get_idx(symbol_table* table, size_t idx) {
    DBUG_ENTER("symbol_table_get_idx");
    DBUG_ASSERT((table != NULL), "Table was NULL!");
    DBUG_PRINT("SYMBOL_TABLE_LOOKUP", ("FINDING IDX %d", idx));

    DBUG_ASSERT((idx < table->current_entries), "IDX too high!");

    DBUG_RETURN(table->entries[idx]);
}

symbol_table* symbol_table_move_up(symbol_table* table, size_t scopes) {
    DBUG_ENTER("symbol_table_move_up");
    DBUG_PRINT("symbol_table_move_up", ("scopes to traverse: %d\n", scopes));

    if(scopes == 0) {
        DBUG_RETURN(table);
    }

    if(scopes == 1) {
        DBUG_RETURN(table->upper);
    } else {
        DBUG_RETURN(symbol_table_move_up(table->upper, scopes - 1));
    }
}

int symbol_table_update(symbol_table* table, size_t idx_to_update, char* new_name) {
    DBUG_ENTER("symbol_table_update");
    DBUG_ASSERT((table != NULL), "Table was NULL!");

    symbol_table_entry* tochange = symbol_table_get_idx(table, idx_to_update);

    if(tochange == NULL) {
        DBUG_RETURN(EXIT_FAILURE);
    }

    tochange->name = MEMfree(tochange->name);
    tochange->name = STRcpy(new_name);

    DBUG_RETURN(EXIT_SUCCESS);
}

void print_symbol_table(symbol_table* table) {
    DBUG_ENTER("**** print_symbol_table ****");
    DBUG_ASSERT((table != NULL), "Table was NULL!");

    for (size_t i = 0; i < table->table_depth; i++) {
        fprintf(stderr, "    ");
    }
    fprintf(stderr, "**** Variable symbol table with depth: %zu ****\n", table->table_depth);
    symbol_table_entry *current;

    for (size_t i = 0; i < table->current_entries; i++) {
        current = table->entries[i];
        char* type = get_type_str(current->vartype);

        for (size_t i = 0; i < table->table_depth; i++) {
            fprintf(stderr, "    ");
        }
        fprintf(stderr, "**** ID: %zu name: %s type: %s ****\n", current->idx, current->name, type);

        MEMfree(type);
    }

    DBUG_VOID_RETURN;
}

void print_symbol_table_chain(symbol_table* table) {
    DBUG_ENTER("print_symbol_table_chain");
    DBUG_ASSERT((table != NULL), "Table was NULL!");
    print_symbol_table(table);
    if(table->upper != NULL)
        print_symbol_table_chain(table->upper);

    DBUG_VOID_RETURN;
}
