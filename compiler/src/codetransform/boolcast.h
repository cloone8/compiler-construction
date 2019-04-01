#ifndef __BOOLCAST_H__
#define __BOOLCAST_H__
#include "types.h"

extern node* CBCcast(node* arg_node, info* arg_info);
extern node* CBCfloatconst(node* arg_node, info* arg_info);
extern node *CBCboolconst(node *arg_node, info *arg_info);
extern node *CBCintconst(node *arg_node, info *arg_info);
extern node* CBCvarread(node* arg_node, info* arg_info);
extern node* CBCbinop(node* arg_node, info* arg_info);

extern node* CBCdo(node* syntaxtree);

#endif
