#ifndef __VARIABLEPROPAGATION_H__
#define __VARIABLEPROPAGATION_H__
#include "types.h"

extern node* VPRbasicvardeclaration(node* arg_node, info* arg_info);
extern node* VPRvarread(node* arg_node, info* arg_info);
extern node* VPRvarassign(node* arg_node, info* arg_info);
extern node* VPRfunctionvardeclaration(node* arg_node, info* arg_info);
extern node* VPRglobal(node* arg_node, info* arg_info);
extern node* VPRdeclaration(node* arg_node, info* arg_info);

extern node* VPRdo(node* syntaxtree);

#endif
