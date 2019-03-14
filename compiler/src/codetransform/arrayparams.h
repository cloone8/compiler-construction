#ifndef __ARRAYPARAMS_H__
#define __ARRAYPARAMS_H__

#include "types.h"

extern node* APPfunction(node* arg_node, info* arg_info);
extern node* APParrayparameter(node* arg_node, info* arg_info);
extern node* APParraydimensionvars(node* arg_node, info* arg_info);
extern node* APPfunctiondefinition(node* arg_node, info* arg_info);
extern node* APPfuncall(node* arg_node, info* arg_info);
extern node *APPdo(node* syntaxtree);

#endif
