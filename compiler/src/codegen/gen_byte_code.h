#ifndef _GEN_BYTE_CODE_H_
#define _GEN_BYTE_CODE_H_
#include "types.h"

extern node *GBCbinop(node *arg_node, info *arg_info);
extern node *GBCconditionalexpr(node *arg_node, info *arg_info);
extern node *GBCerror(node *arg_node, info *arg_info);
extern node *GBCdeclaration(node *arg_node, info *arg_info);
extern node *GBCfunction(node *arg_node, info *arg_info);
extern node *GBCbasicparameter(node *arg_node, info *arg_info);
extern node *GBCarrayparameter(node *arg_node, info *arg_info);
extern node *GBCarraydimensionvars(node *arg_node, info *arg_info);
extern node *GBCfunctiondefinition(node *arg_node, info *arg_info);
extern node *GBCfunctionbody(node *arg_node, info *arg_info);
extern node *GBCstatements(node *arg_node, info *arg_info);
extern node *GBCbasicvardeclaration(node *arg_node, info *arg_info);
extern node *GBCarrayvardeclaration(node *arg_node, info *arg_info);
extern node *GBCexprarray(node *arg_node, info *arg_info);
extern node *GBCglobaldefinition(node *arg_node, info *arg_info);
extern node *GBCfor(node *arg_node, info *arg_info);
extern node *GBCwhile(node *arg_node, info *arg_info);
extern node *GBCdowhile(node *arg_node, info *arg_info);
extern node *GBCglobal(node *arg_node, info *arg_info);
extern node *GBCfuncall(node *arg_node, info *arg_info);
extern node *GBCfuncallparameter(node *arg_node, info *arg_info);
extern node *GBCreturn(node *arg_node, info *arg_info);
extern node *GBCbranch(node *arg_node, info *arg_info);
extern node *GBCexpr(node *arg_node, info *arg_info);
extern node *GBCmonop(node *arg_node, info *arg_info);
extern node *GBClogicop(node *arg_node, info *arg_info);
extern node *GBCrelop(node *arg_node, info *arg_info);
extern node *GBCarithop(node *arg_node, info *arg_info);
extern node *GBCarrayvarassign(node *arg_node, info *arg_info);
extern node *GBCarrayvarread(node *arg_node, info *arg_info);
extern node *GBCvarassign(node *arg_node, info *arg_info);
extern node *GBCvarread(node *arg_node, info *arg_info);
extern node *GBCcast(node *arg_node, info *arg_info);
extern node *GBCboolconst(node *arg_node, info *arg_info);
extern node *GBCintconst(node *arg_node, info *arg_info);
extern node *GBCfloatconst(node *arg_node, info *arg_info);
extern node *GBCconstant(node *arg_node, info *arg_info);
extern node *GBCfunctionvardeclaration(node *arg_node, info *arg_info);
extern node *GBClocalfunction(node *arg_node, info *arg_info);
extern node *GBCarrayinit(node *arg_node, info *arg_info);
extern node *GBCprogram(node *arg_node, info *arg_info);
extern node *GBCnoop(node *arg_node, info *arg_info);

extern node *GBCdoGenByteCode(node *syntaxtree);

#endif
