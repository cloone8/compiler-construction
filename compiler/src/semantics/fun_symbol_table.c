#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"

#include <stdlib.h>

#include "fun_symbol_table.h"
#include "ctinfo.h"

fun_symbol_table* fun_symbol_table_make(fun_symbol_table* parent, size_t table_depth) {
    DBUG_ENTER("fun_symbol_table_make");
    DBUG_PRINT("fun_symbol_table", ("Making new symbol table\n"));
    fun_symbol_table* toreturn = MEMmalloc(sizeof(fun_symbol_table));

    toreturn->__entry_array_size = 10;
    toreturn->next_id = 0;
    toreturn->current_entries = 0;
    toreturn->upper = parent;
    toreturn->table_depth = table_depth;
    toreturn->entries = MEMmalloc(sizeof(fun_symbol_table_entry*) * 10);

    DBUG_RETURN(toreturn);
}

int fun_symbol_table_free(fun_symbol_table* tofree) {
    DBUG_ENTER("fun_symbol_table_free");

    for(size_t i = 0; i < tofree->current_entries; i++) {
        fun_symbol_table_entry* current = tofree->entries[i];
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

    toincrease->entries = MEMrealloc(toincrease->entries, toincrease->__entry_array_size * sizeof(fun_symbol_table_entry*));

    DBUG_VOID_RETURN;
}

int fun_symbol_table_add(fun_symbol_table* table, char* name, type return_type, size_t param_amount, type* param_types) {
    DBUG_ENTER("fun_symbol_table_add");

    if (fun_symbol_table_get_name(table, name) != NULL) {
        CTIerror("Function %s already in scope", name);
        DBUG_RETURN(EXIT_FAILURE);
    }

    fun_symbol_table_entry* new_entry = MEMmalloc(sizeof(fun_symbol_table_entry));
    new_entry->idx = table->next_id++;
    new_entry->name = STRcpy(name);
    new_entry->returntype = return_type;
    new_entry->paramamount = param_amount;
    new_entry->paramtypes = param_types;

    table->current_entries++;

    if(table->current_entries > table->__entry_array_size) {
        increase_table_size(table);
    }


    table->entries[table->current_entries - 1] = new_entry;
    DBUG_PRINT("fun_symbol_table", ("Added %zu %s %d %zu\n", new_entry->idx, new_entry->name, new_entry->returntype, new_entry->paramamount));
    DBUG_RETURN(EXIT_SUCCESS);
}

fun_symbol_table_entry* fun_symbol_table_get_name(fun_symbol_table* table, char* name) {
    DBUG_ENTER("fun_symbol_table_get_name");

    for(size_t i = 0; i < table->current_entries; i++) {
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
    DBUG_PRINT("FUN_SYMBOL_TABLE_LOOKUP", ("FINDING IDX %d", idx));

    DBUG_RETURN(table->entries[idx]);
}

fun_symbol_table* fun_symbol_table_move_up(fun_symbol_table* table, size_t scopes) {
    DBUG_ENTER("fun_symbol_table_move_up");
    DBUG_PRINT("fun_symbol_table_move_up", ("scopes to traverse: %d\n", scopes));

    if(scopes == 0) {
        DBUG_RETURN(table);
    }

    if(scopes == 1) {
        DBUG_RETURN(table->upper);
    } else {
        DBUG_RETURN(fun_symbol_table_move_up(table->upper, scopes - 1));
    }
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

type* fun_symbol_table_get_param_types(fun_symbol_table* table, char* fun_name) {
    DBUG_ENTER("fun_symbol_table_get_param_types");

    fun_symbol_table_entry *result = fun_symbol_table_get_name(table, fun_name);

    DBUG_RETURN(result->paramtypes);
}

size_t fun_symbol_table_get_param_amount(fun_symbol_table *table, char *fun_name) {
    DBUG_ENTER("fun_symbol_table_get_param_amount");

    fun_symbol_table_entry* result = fun_symbol_table_get_name(table, fun_name);

    DBUG_RETURN(result->paramamount);
}

void print_fun_symbol_table(fun_symbol_table *table) {
    DBUG_ENTER("**** print_symbol_table ****");

    for (size_t i = 0; i < table->table_depth; i++) {
        printf("    ");
    }

    printf("**** Function symbol table with depth: %zu ****\n", table->table_depth);
    fun_symbol_table_entry *current;
    for (size_t i = 0; i < table->current_entries; i++) {
        current = table->entries[i];
        char *type;

        switch (current->returntype) {
        case TP_void:
            type = "VOID";
            break;
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
        printf("**** ID: %zu name: %s returntype: %s parameters: %zu ****\n", current->idx, current->name, type, current->paramamount);
    }

    DBUG_VOID_RETURN;
}

void print_fun_symbol_table_chain(fun_symbol_table* table) {
    DBUG_ENTER("print_fun_symbol_table_chain");
    print_fun_symbol_table(table);
    if(table->upper != NULL)
        print_fun_symbol_table_chain(table->upper);

    DBUG_VOID_RETURN
}
