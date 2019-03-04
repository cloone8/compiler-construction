#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"

#include <stdlib.h>

#include "funsymboltable.h"

fun_symbol_table* fun_symbol_table_make(void) {
    DBUG_ENTER("fun_symbol_table_make");

    fun_symbol_table* toreturn = MEMmalloc(sizeof(fun_symbol_table));

    toreturn->current_entries = 0;
    toreturn->next_id = 0;
    toreturn->__entry_array_size = 10;
    toreturn->entries = MEMmalloc(sizeof(var_symbol_table_entry*) * 10);

    DBUG_RETURN(toreturn);
}

fun_symbol_table_param* fun_symbol_table_make_param(type paramtype, char* name) {
    DBUG_ENTER("fun_symbol_table_make_param");

    DBUG_ASSERT((name == NULL), "Tried to make fun_symbol_table parameter with a name of NULL!");
    DBUG_ASSERT((paramtype != TP_unknown), "Tried to make fun_symbol_table parameter with a type of TP_unknown!");

    fun_symbol_table_param* toreturn = MEMmalloc(sizeof(fun_symbol_table_param));

    toreturn->paramtype = paramtype;
    toreturn->name = STRcpy(name);

    DBUG_RETURN(toreturn);
}

int fun_symbol_table_free(fun_symbol_table* tofree) {
    DBUG_ENTER("fun_symbol_table_free");

    for(int i = 0; i < tofree->current_entries; i++) {
        fun_symbol_table_entry* current = tofree->entries[i];

        for(int j = 0; j < current->paramcount; j++) {
            fun_symbol_table_param* current_param = current->params[j];
            current_param->name = MEMfree(current_param->name);
            current_param = MEMfree(current_param);
        }

        current->name = MEMfree(current->name);
        MEMfree(current);
    }

    MEMfree(tofree->entries);
    MEMfree(tofree);

    DBUG_RETURN(EXIT_SUCCESS);
}

static void increase_table_size(fun_symbol_table* toincrease) {
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

int fun_symbol_table_add(fun_symbol_table* table, char* name, type fun_type, size_t paramcount, fun_symbol_table_param** params) {
    DBUG_ENTER("fun_symbol_table_add");

    fun_symbol_table_entry* new_entry = MEMmalloc(sizeof(fun_symbol_table_entry));

    new_entry->idx = table->next_id++;
    new_entry->name = STRcpy(name);
    new_entry->returntype = fun_type;
    new_entry->paramcount = paramcount;
    new_entry->params =  params;

    table->current_entries++;

    if(table->current_entries > table->__entry_array_size) {
        increase_table_size(table);
    }

    table->entries[table->current_entries - 1] = new_entry;

    DBUG_RETURN(EXIT_SUCCESS);
}

fun_symbol_table_entry* fun_symbol_table_get_name(fun_symbol_table* table, char* name) {
    DBUG_ENTER("fun_symbol_table_get_name");

    for(int i = 0; i < table->current_entries; i++) {
        fun_symbol_table_entry* current = table->entries[i];
        if(STReq(current->name, name)) {
            DBUG_PRINT("FUN_SYMBOL_TABLE_LOOKUP", ("Found name %s\n", name));
            DBUG_RETURN(current);
        }
    }

    DBUG_PRINT("FUN_SYMBOL_TABLE_LOOKUP", ("Did not find name %s\n", name));
    DBUG_RETURN(NULL);
}

fun_symbol_table_entry* fun_symbol_table_get_idx(fun_symbol_table* table, size_t idx) {
    DBUG_ENTER("fun_symbol_table_get_idx");

    for(int i = 0; i < table->current_entries; i++) {
        fun_symbol_table_entry* current = table->entries[i];
        if(current->idx == idx) {
            DBUG_PRINT("FUN_SYMBOL_TABLE_LOOKUP", ("Found idx %d\n", idx));
            DBUG_RETURN(current);
        }
    }

    DBUG_PRINT("FUN_SYMBOL_TABLE_LOOKUP", ("Did not find idx %d\n", idx));
    DBUG_RETURN(NULL);
}

int fun_symbol_table_update(fun_symbol_table* table, size_t idx_to_update, char* new_name) {
    DBUG_ENTER("fun_symbol_table_update");
    fun_symbol_table_entry* tochange = fun_symbol_table_get_idx(table, idx_to_update);

    if(tochange == NULL) {
        DBUG_RETURN(EXIT_FAILURE);
    }

    tochange->name = MEMfree(tochange->name);
    tochange->name = STRcpy(new_name);

    DBUG_RETURN(EXIT_SUCCESS);
}
