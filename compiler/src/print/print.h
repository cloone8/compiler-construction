
/**
 * @file print.h
 *
 * Functions to print node structures
 *
 */

#ifndef _SAC_PRT_NODE_H_
#define _SAC_PRT_NODE_H_

#include "types.h"

extern node *PRTstmts (node * arg_node, info * arg_info);
extern node *PRTassign (node * arg_node, info * arg_info);
extern node *PRTvar (node * arg_node, info * arg_info);
extern node *PRTvarlet (node * arg_node, info * arg_info);
extern node *PRTbinop (node * arg_node, info * arg_info);
extern node *PRTfloat (node * arg_node, info * arg_info);
extern node *PRTnum (node * arg_node, info * arg_info);
extern node *PRTbool (node * arg_node, info * arg_info);
extern node *PRTsymboltableentry (node * arg_node, info * arg_info);
extern node *PRTerror (node * arg_node, info * arg_info);
extern node *PRTmodule (node * arg_node, info * arg_info);
extern node *PRTdeclaration (node * arg_node, info * arg_info);
extern node *PRTlogicop (node * arg_node, info * arg_info);
extern node * PRTrelop (node * arg_node, info * arg_info);
extern node * PRTarithop (node * arg_node, info * arg_info);
extern node * PRTarrayvarassign (node * arg_node, info * arg_info);
extern node * PRTarrayvarread (node * arg_node, info * arg_info);
extern node * PRTvarassign (node * arg_node, info * arg_info);
extern node * PRTvarread (node * arg_node, info * arg_info);
extern node * PRTcast (node * arg_node, info * arg_info);
extern node * PRTboolconst (node * arg_node, info * arg_info);
extern node * PRTintconst (node * arg_node, info * arg_info);
extern node * PRTfloatconst (node * arg_node, info * arg_info);
extern node * PRTconstant (node * arg_node, info * arg_info);     



extern node *PRTdoPrint(node *syntaxtree);

#endif /* _SAC_PRT_NODE_H_ */
