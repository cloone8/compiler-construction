#ifndef __CONSTANTFOLDING_H__
#define __CONSTANTFOLDING_H__
#include "types.h"

extern node* CFObinop(node* arg_node, info* arg_info);
extern node* CFOmonop(node* arg_node, info* arg_info);

extern node* CFOdo(node* syntaxtree);

#endif
