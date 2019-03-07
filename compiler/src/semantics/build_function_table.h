#ifndef __BUILD_FUNCTION_TABLE_H__
#define __BUILD_FUNCTION_TABLE_H__
#include "types.h"

extern node *FTBfunction(node *arg_node, info *arg_info);
extern node *FTBfunctiondefinition(node *arg_node, info *arg_info);
extern node *FTBbasicparameter(node *arg_node, info *arg_info);
extern node *FTBarrayparameter(node *arg_node, info *arg_info);

extern node *FTBdo(node *syntaxtree);
#endif
