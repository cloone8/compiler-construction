#ifndef __CHECK_FUN_SCOPE
#define __CHECK_FUN_SCOPE
#include "types.h"

extern node* CFSfunctiondefinition(node* arg_node, info* arg_info);
extern node* CFSfunctioncall(node* arg_node, info* arg_info);

extern node *CFSdo(node *syntaxtree);
#endif