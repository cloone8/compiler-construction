#ifndef __BASIC_VARSPLIT_H__
#define __BASIC_VARSPLIT_H__

#include "types.h"

extern node* BSPprogram(node *arg_node, info *arg_info);
extern node* BSPglobal(node *arg_node, info *arg_info);
extern node* BSPbasicvardeclaration(node * arg_node, info * arg_info);
extern node* BSPfunctionbody(node * arg_node, info * arg_info);
extern node *BSPdo(node * syntaxtree);

#endif
