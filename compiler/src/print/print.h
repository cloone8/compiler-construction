
/**
 * @file print.h
 *
 * Functions to print node structures
 *
 */

#ifndef _SAC_PRT_NODE_H_
#define _SAC_PRT_NODE_H_

#include "types.h"

extern node *PRTbinop (node * arg_node, info * arg_info);
extern node* PRTconditionalexpr(node* arg_node, info* arg_info);
extern node *PRTerror (node * arg_node, info * arg_info);
extern node *PRTdeclaration (node * arg_node, info * arg_info);
extern node *PRTfunction (node * arg_node, info * arg_info);
extern node *PRTbasicparameter (node * arg_node, info * arg_info);
extern node *PRTarrayparameter (node * arg_node, info * arg_info);
extern node *PRTarraydimensionvars (node * arg_node, info * arg_info);
extern node *PRTfunctiondefinition (node * arg_node, info * arg_info);
extern node *PRTfunctionbody (node * arg_node, info * arg_info);
extern node *PRTstatements (node * arg_node, info * arg_info);
extern node *PRTbasicvardeclaration (node * arg_node, info * arg_info);
extern node *PRTarrayvardeclaration (node * arg_node, info * arg_info);
extern node *PRTexprarray (node * arg_node, info * arg_info);
extern node *PRTglobaldefinition (node * arg_node, info * arg_info);
extern node *PRTfor (node * arg_node, info * arg_info);
extern node *PRTwhile (node * arg_node, info * arg_info);
extern node *PRTdowhile (node * arg_node, info * arg_info);
extern node *PRTglobal (node * arg_node, info * arg_info);
extern node *PRTfuncall (node * arg_node, info * arg_info);
extern node *PRTfuncallparameter (node * arg_node, info * arg_info);
extern node *PRTreturn (node * arg_node, info * arg_info);
extern node *PRTbranch (node * arg_node, info * arg_info);
extern node *PRTexpr (node * arg_node, info * arg_info);
extern node *PRTmonop (node * arg_node, info * arg_info);
extern node *PRTlogicop (node * arg_node, info * arg_info);
extern node *PRTrelop (node * arg_node, info * arg_info);
extern node *PRTarithop (node * arg_node, info * arg_info);
extern node *PRTarrayvarassign (node * arg_node, info * arg_info);
extern node *PRTarrayvarread (node * arg_node, info * arg_info);
extern node *PRTvarassign (node * arg_node, info * arg_info);
extern node *PRTvarread (node * arg_node, info * arg_info);
extern node *PRTcast (node * arg_node, info * arg_info);
extern node *PRTboolconst (node * arg_node, info * arg_info);
extern node *PRTintconst (node * arg_node, info * arg_info);
extern node *PRTfloatconst (node * arg_node, info * arg_info);
extern node *PRTconstant (node * arg_node, info * arg_info);
extern node *PRTfunctionvardeclaration (node * arg_node, info * arg_info);
extern node *PRTlocalfunction (node * arg_node, info * arg_info);
extern node *PRTprogram (node * arg_node, info * arg_info);



extern node *PRTdoPrint(node *syntaxtree);

#endif /* _SAC_PRT_NODE_H_ */
