#ifndef __BOOLCOMP_H__
#define __BOOLCOMP_H__
#include "types.h"


extern node* BDCboolconst(node* arg_node, info* arg_info);
extern node* BDCvarread(node* arg_node, info* arg_info);
extern node *BDCbinop(node* arg_node, info* arg_info);

extern node *BDCdo(node * syntaxtree);

#endif
