#include "finalise.h"

/**
 * Cleans all remaining memory and buffers
 */
void FINdoFinalise(void) {
    // Cleans lexer memory
    yylex_destroy();
}
