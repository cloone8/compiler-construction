#ifndef __FINALISE_H__
#define __FINALISE_H__

#include "types.h"

extern int yylex_destroy();
/* Phase function: must match phase_fun_t, node *(*)(node *) */
node *FINdoFinalise(node *syntax_tree);

#endif
