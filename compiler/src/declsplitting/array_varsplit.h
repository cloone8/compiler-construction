#ifndef __ARRAY_VARSPLIT_H__
#define __ARRAY_VARSPLIT_H__

#include "types.h"

extern node* ASPdo(node *syntaxtree);
extern node* ASParrayvardeclaration(node* arg_node, info* arg_info);
extern node* ASPprogram(node* arg_node, info* arg_info);
extern node* ASPfunctiondefinition(node* arg_node, info* arg_info);
extern node* ASPfunctionbody(node* arg_node, info* arg_info);
extern node* ASPglobal(node* arg_node, info* arg_info);
extern node* ASPdeclaration(node* arg_node, info* arg_info);

#endif
