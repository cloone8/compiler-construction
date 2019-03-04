#ifndef _RI_RENAME_IDENTIFIERS_H_
#define _RI_RENAME_IDENTIFIERS_H_
#include "types.h"

extern node *RIglobal(node *arg_node, info *arg_info);
extern node *RIglobaldefinition(node *arg_node, info *arg_info);
extern node *RIfunction(node *arg_node, info *arg_info);
extern node *RIfunctiondefinition(node *arg_node, info *arg_info);
extern node *RIbasicparameter(node *arg_node, info *arg_info);
extern node *RIarrayparameter(node *arg_node, info *arg_info);
extern node *RIfunctionvardeclaration(node *arg_node, info *arg_info);
extern node *RIfor(node *arg_node, info *arg_info);
extern node *RIvarassign(node *arg_node, info *arg_info);
extern node *RIarrayvarassign(node *arg_node, info *arg_info);
extern node *RIbasicvardeclaration(node *arg_node, info *arg_info);
extern node *RIarrayvardeclaration(node *arg_node, info *arg_info);
extern node *RIfuncall(node *arg_node, info *arg_info);
extern node *RIfuncallparameter(node *arg_node, info *arg_info);
extern node *RIvarread(node *arg_node, info *arg_info);
extern node *RIarrayvarread(node *arg_node, info *arg_info);

extern node *RIrenameIdentifiers(node *syntaxtree);

#endif