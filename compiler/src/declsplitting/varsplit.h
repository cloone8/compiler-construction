#ifndef __VARSPLIT_H__
#define __VARSPLIT_H__

#include "types.h"

extern node* VSPprogram(node *arg_node, info *arg_info);
extern node* VSPglobal(node *arg_node, info *arg_info);
extern node* VSPbasicvardeclaration(node * arg_node, info * arg_info);
extern node* VSParrayvardeclaration(node * arg_node, info * arg_info);
extern node* VSPfunctiondefinition(node * arg_node, info * arg_info);
extern node* VSPfunctionbody(node * arg_node, info * arg_info);
extern node* VSPdeclaration(node * arg_node, info * arg_info);
extern node* VSPfor(node * arg_node, info * arg_info);
extern node *VSPdo(node * syntaxtree);

#endif
