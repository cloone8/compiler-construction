#ifndef _BUILD_SYMBOL_TABLE_H_
#define _BUILD_SYMBOL_TABLE_H_
#include "types.h"

extern node *STBglobal(node *arg_node, info *arg_info);
extern node *STBglobaldefinition(node *arg_node, info *arg_info);
extern node *STBfunction(node *arg_node, info *arg_info);
extern node *STBfunctiondefinition(node *arg_node, info *arg_info);
extern node *STBbasicparameter(node *arg_node, info *arg_info);
extern node *STBarrayparameter(node *arg_node, info *arg_info);
extern node *STBfunctionvardeclaration(node *arg_node, info *arg_info);
extern node *STBfor(node *arg_node, info *arg_info);
extern node *STBvarassign(node *arg_node, info *arg_info);
extern node *STBarrayvarassign(node *arg_node, info *arg_info);
extern node *STBbasicvardeclaration(node *arg_node, info *arg_info);
extern node *STBarrayvardeclaration(node *arg_node, info *arg_info);
extern node *STBfuncallparameter(node *arg_node, info *arg_info);
extern node *STBvarread(node *arg_node, info *arg_info);
extern node *STBarrayvarread(node *arg_node, info *arg_info);
extern node *STBarraydimensionvars(node *arg_node, info *arg_info);
extern node *STBdeclaration(node *arg_node, info *arg_info);

extern node *STBdo(node *syntaxtree);

#endif
