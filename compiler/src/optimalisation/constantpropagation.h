#ifndef __CONSTANTPROPAGATION_H__
#define __CONSTANTPROPAGATION_H__
#include "types.h"

extern node* CPRbasicvardeclaration(node* arg_node, info* arg_info);
extern node* CPRvarread(node* arg_node, info* arg_info);
extern node* CPRvarassign(node* arg_node, info* arg_info);
extern node* CPRfunctionvardeclaration(node* arg_node, info* arg_info);

extern node* CPRdo(node* syntaxtree);

#endif
