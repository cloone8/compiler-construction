#ifndef __FORSYMTABLEMIGRATE_H__
#define __FORSYMTABLEMIGRATE_H__

#include "types.h"

extern node* FSMfunctiondefinition(node* arg_node, info* arg_info);
extern node* FSMfor(node* arg_node, info* arg_info);

extern node* FSMdo(node* syntaxtree);

#endif
