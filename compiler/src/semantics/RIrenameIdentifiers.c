#include "RIrenameIdentifiers.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

node *RIglobal(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIglobaldefinition(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIfunction(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIfunctiondefinition(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIbasicparameter(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIarrayparameter(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIfunctionvardeclaration(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIfor(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIvarassign(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIarrayvarassign(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIbasicvardeclaration(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIarrayvardeclaration(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIfuncall(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIfuncallparameter(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIvarread(node *arg_node, info *arg_info) {
    return arg_node;
}

node *RIarrayvarread(node *arg_node, info *arg_info) {
    return arg_node;
}


node *RIrenameIdentifiers(node *syntaxtree) {
    DBUG_ENTER("RIrenameIdentifiers");

    TRAVpush(TR_ri);

    syntaxtree = TRAVdo(syntaxtree, NULL);

    TRAVpop();

    DBUG_RETURN(syntaxtree);
}