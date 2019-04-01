#ifndef __RETURNCHECK_H__
#define __RETURNCHECK_H__
#include "types.h"

extern node *RCreturn(node *arg_node, info *arg_info);
extern node *RCfunctionbody(node *arg_node, info *arg_info);
extern node *RCfunction(node *arg_node, info *arg_info);
extern node *RCstatements(node *arg_node, info *arg_info);
extern node *RCbranch(node *arg_node, info *arg_info);

extern node *RCdo(node * syntaxtree);

#endif
