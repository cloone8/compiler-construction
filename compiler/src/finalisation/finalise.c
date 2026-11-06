#include "finalise.h"
#include "strlist.h"
#include "memory.h"
#include "types.h"
#include "globals.h"
#include "util.h"

/**
 * Cleans all remaining memory and buffers
 */
node *FINdoFinalise(node *syntax_tree) {
    UNUSED_VAR(syntax_tree);

    // Cleans lexer memory
    yylex_destroy();

    // Clean dangling strings from other phases
    strlist_node* head = myglobal.dangling_strings;
    strlist_node* current = head;

    while(current != NULL) {
        if(current->content != NULL) {
            MEMfree(current->content);
        }

        current = current->next;
    }

    if(head != NULL) {
        strlistFree(head);
    }

    // Close input file handler
    if(myglobal.input_file_handler != NULL) {
        fclose(myglobal.input_file_handler);
    }

    // The tree was already freed by the previous phase
    return NULL;
}
