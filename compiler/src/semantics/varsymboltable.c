#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"

#include <stdlib.h>

#include "varsymboltable.h"

var_symbol_table* var_symbol_table_make(void) {
    DBUG_ENTER("var_symbol_table_make");
    var_symbol_table* toreturn = MEMmalloc(sizeof(var_symbol_table));

    toreturn->__entry_array_size = 10;
    toreturn->next_id = 0;
    toreturn->current_entries = 0;
    toreturn->entries = MEMmalloc(sizeof(var_symbol_table_entry*) * 10);

    DBUG_RETURN(toreturn);
}

int var_symbol_table_free(var_symbol_table* tofree) {
    DBUG_ENTER("var_symbol_table_free");

    for(int i = 0; i < tofree->current_entries; i++) {
        var_symbol_table_entry* current = tofree->entries[i];
        current->name = MEMfree(current->name);
        MEMfree(current);
    }

    MEMfree(tofree->entries);
    MEMfree(tofree);

    DBUG_RETURN(EXIT_SUCCESS);
}

static void increase_table_size(var_symbol_table* toincrease) {
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

    toincrease->entries = MEMrealloc(toincrease->entries, toincrease->__entry_array_size);

    DBUG_VOID_RETURN;
}

int var_symbol_table_add(var_symbol_table* table, char* name, type var_type) {
    DBUG_ENTER("var_symbol_table_add");

    var_symbol_table_entry* new_entry = MEMmalloc(sizeof(var_symbol_table_entry));
    new_entry->idx = table->next_id++;
    new_entry->name = STRcpy(name);
    new_entry->vartype = var_type;

    table->current_entries++;

    if(table->current_entries > table->__entry_array_size) {
        increase_table_size(table);
    }

    table->entries[table->current_entries - 1] = new_entry;

    DBUG_RETURN(EXIT_SUCCESS);
}

var_symbol_table_entry* var_symbol_table_get_name(var_symbol_table* table, char* name) {
    DBUG_ENTER("var_symbol_table_get_name");

    for(int i = 0; i < table->current_entries; i++) {
        var_symbol_table_entry* current = table->entries[i];
        if(STReq(current->name, name)) {
            DBUG_PRINT("VAR_SYMBOL_TABLE_LOOKUP", ("Found name %s\n", name));
            DBUG_RETURN(current);
        }
    }

    DBUG_PRINT("VAR_SYMBOL_TABLE_LOOKUP", ("Did not find name %s\n", name));
    DBUG_RETURN(NULL);
}

var_symbol_table_entry* var_symbol_table_get_idx(var_symbol_table* table, size_t idx) {
    DBUG_ENTER("var_symbol_table_get_idx");

    for(int i = 0; i < table->current_entries; i++) {
        var_symbol_table_entry* current = table->entries[i];
        if(current->idx == idx) {
            DBUG_PRINT("VAR_SYMBOL_TABLE_LOOKUP", ("Found idx %d\n", idx));
            DBUG_RETURN(current);
        }
    }

    DBUG_PRINT("VAR_SYMBOL_TABLE_LOOKUP", ("Did not find idx %d\n", idx));
    DBUG_RETURN(NULL);
}

int var_symbol_table_update(var_symbol_table* table, size_t idx_to_update, char* new_name) {
    DBUG_ENTER("var_symbol_table_update");

    var_symbol_table_entry* tochange = var_symbol_table_get_idx(table, idx_to_update);

    if(tochange == NULL) {
        DBUG_RETURN(EXIT_FAILURE);
    }

    tochange->name = MEMfree(tochange->name);
    tochange->name = STRcpy(new_name);

    DBUG_RETURN(EXIT_SUCCESS);
}
