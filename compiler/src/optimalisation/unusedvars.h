#ifndef __UNUSEDVARS_H__
#define __UNUSEDVARS_H__
#include "types.h"

extern node* UVRbasicvardeclaration(node* arg_node, info* arg_info);
extern node* UVRvarread(node* arg_node, info* arg_info);
extern node* UVRvarassign(node* arg_node, info* arg_info);
extern node* UVRfunctionvardeclaration(node* arg_node, info* arg_info);
extern node* UVRglobal(node* arg_node, info* arg_info);
extern node* UVRdeclaration(node* arg_node, info* arg_info);

extern node* UVRdo(node* syntaxtree);

#endif
