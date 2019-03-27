#ifndef __ARRAYLOWER_H__
#define __ARRAYLOWER_H__

#include "types.h"

extern node* LADarrayvarread(node* arg_node, info* arg_info);
extern node* LADarrayvarassign(node* arg_node, info* arg_info);
extern node* LADarrayinit(node* arg_node, info* arg_info);
extern node* LADarrayvardeclaration(node* arg_node, info* arg_info);
extern node* LADdo(node* syntaxtree);

#endif
