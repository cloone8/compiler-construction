#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"

#include <stdlib.h>

#include "symbol_table.h"
#include "ctinfo.h"

symbol_table* symbol_table_make(symbol_table* parent, size_t table_depth) {
    DBUG_ENTER("symbol_table_make");
    DBUG_PRINT("symbol_table", ("Making new symbol table\n"));

    symbol_table* toreturn = MEMmalloc(sizeof(symbol_table));

    toreturn->__entry_array_size = 10;
    toreturn->next_id = 0;
    toreturn->current_entries = 0;
    toreturn->upper = parent;
    toreturn->table_depth = table_depth;
    toreturn->entries = MEMmalloc(sizeof(symbol_table_entry*) * 10);

    DBUG_RETURN(toreturn);
}

int symbol_table_free(symbol_table* tofree) {
    DBUG_ENTER("symbol_table_free");

    for(size_t i = 0; i < tofree->current_entries; i++) {
        symbol_table_entry* current = tofree->entries[i];
        current->name = MEMfree(current->name);
        MEMfree(current);
    }

    MEMfree(tofree->entries);
    MEMfree(tofree);

    DBUG_RETURN(EXIT_SUCCESS);
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

symbol_table_entry* symbol_table_add(symbol_table* table, char* name, type var_type) {
    DBUG_ENTER("symbol_table_add");

    if (symbol_table_get_name(table, name) != NULL) {
        CTIerror("Var %s already in scope", name);
        DBUG_RETURN(NULL);
    }

    symbol_table_entry* new_entry = MEMmalloc(sizeof(symbol_table_entry));
    new_entry->idx = table->next_id++;
    new_entry->name = STRcpy(name);
    new_entry->vartype = var_type;

    table->current_entries++;

    if(table->current_entries > table->__entry_array_size) {
        increase_table_size(table);
    }


    table->entries[table->current_entries - 1] = new_entry;

    DBUG_PRINT("symbol_table", ("Added %zu %s %d\n", new_entry->idx, new_entry->name, new_entry->vartype));
    DBUG_RETURN(new_entry);
}

symbol_table_entry* symbol_table_get_name(symbol_table* table, char* name) {
    DBUG_ENTER("symbol_table_get_name");
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
    DBUG_PRINT("SYMBOL_TABLE_LOOKUP", ("FINDING IDX %d", idx));

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

    for (size_t i = 0; i < table->table_depth; i++) {
        printf("    ");
    }
    printf("**** Variable symbol table with depth: %zu ****\n", table->table_depth);
    symbol_table_entry *current;

    for (size_t i = 0; i < table->current_entries; i++) {
        current = table->entries[i];
        char* type;

        switch (current->vartype) {
            case TP_bool:
                type = "BOOL";
                break;
            case TP_bool_array:
                type = "BOOL_ARRAY";
                break;
            case TP_int:
                type = "INT";
                break;
            case TP_int_array:
                type = "INT_ARRAY";
                break;
            case TP_float:
                type = "FLOAT";
                break;
            case TP_float_array:
                type = "FLOAT_ARRAY";
                break;
            default:
                type = "UNKNOWN TYPE";
        }

        for (size_t i = 0; i < table->table_depth; i++) {
            printf("    ");
        }
        printf("**** ID: %zu name: %s type: %s ****\n", current->idx, current->name, type);
    }

    DBUG_VOID_RETURN;
}
