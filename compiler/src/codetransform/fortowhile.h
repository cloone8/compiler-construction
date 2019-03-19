#ifndef __FORTOWHILE_H__
#define __FORTOWHILE_H__

#include "types.h"

extern node* FTWfunctiondefinition(node* arg_node, info* arg_info);
extern node* FTWfor(node* arg_node, info* arg_info);

extern node* FTWdo(node* syntaxtree);

#endif
