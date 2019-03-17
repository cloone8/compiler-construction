#ifndef __NODE_APPENDS_H__
#define __NODE_APPENDS_H__
#include "types.h"

void append_statement_to_tail(node* block_to_append_to, node* statement_to_append);
void append_declaration_to_tail(node *block_to_append_to, node *declaration_to_append);
void append_functionvardeclaration_to_tail(node *block_to_append_to, node *declaration_to_append);
void append_exprarray_to_tail(node *block_to_append_to, node *exprarray_to_append);
void append_functionparameter_to_tail(node *block_to_append_to, node *funparam_to_append);
void append_funcallparam_to_tail(node *block_to_append_to, node *funcallparam_to_append);

#endif
