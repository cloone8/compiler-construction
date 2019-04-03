#ifndef __FORSPLIT_H__
#define __FORSPLIT_H__

#include "types.h"

extern node* FSPfunctiondefinition(node * arg_node, info * arg_info);
extern node* FSPfunctionbody(node * arg_node, info * arg_info);
extern node* FSPfor(node * arg_node, info * arg_info);
extern node* FSPstatements(node * arg_node, info * arg_info);
extern node* FSPfuncall(node * arg_node, info * arg_info);
extern node *FSPdo(node * syntaxtree);

#endif
