#ifndef __VARRENAME_H__
#define __VARRENAME_H__

#include "types.h"

extern node* VRNbasicvardeclaration(node* arg_node, info* arg_info);
extern node* VRNarrayvardeclaration(node* arg_node, info* arg_info);
extern node* VRNarrayvarread(node* arg_node, info* arg_info);
extern node* VRNvarread(node* arg_node, info* arg_info);
extern node* VRNvarassign(node* arg_node, info* arg_info);
extern node* VRNarrayvarassign(node* arg_node, info* arg_info);
extern node* VRNbasicparameter(node* arg_node, info* arg_info);
extern node* VRNarrayparameter(node* arg_node, info* arg_info);
extern node* VRNarraydimensionvars(node* arg_node, info* arg_info);
extern node* VRNfunction(node* arg_node, info* arg_info);
extern node* VRNdo(node* syntaxtree);

#endif
