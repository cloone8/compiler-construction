#ifndef __TYPECHECK_H__
#define __TYPECHECK_H__
#include "types.h"

extern node *TCvarassign(node *arg_node, info *arg_info);
extern node *TCfuncall(node * arg_node, info * arg_info);

extern node *TCdo(node * syntaxtree);

#endif
