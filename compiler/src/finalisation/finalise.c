#include "finalise.h"
#include "strlist.h"
#include "myglobals.h"
#include "memory.h"

/**
 * Cleans all remaining memory and buffers
 */
void FINdoFinalise(void) {
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
    fclose(myglobal.input_file_handler);
}
