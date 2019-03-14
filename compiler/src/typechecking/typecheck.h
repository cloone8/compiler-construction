#ifndef __TYPECHECK_H__
#define __TYPECHECK_H__
#include "types.h"

extern node* TCfuncallparameter(node* arg_node, info* arg_info);
extern node* TCvarread(node* arg_node, info* arg_info);
extern node* TCfuncall(node* arg_node, info* arg_info);
extern node* TCfunctiondefinition(node* arg_node, info* arg_info);
extern node* TCvarassign(node* arg_node, info* arg_info);
extern node* TCintconst(node* arg_node, info* arg_info);
extern node *TCfloatconst(node *arg_node, info *arg_info);
extern node *TCboolconst(node *arg_node, info *arg_info);
extern node *TCbinop(node *arg_node, info *arg_info);

extern node *TCdo(node * syntaxtree);

#endif
