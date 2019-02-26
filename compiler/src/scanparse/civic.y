%{


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "types.h"
#include "tree_basic.h"
#include "str.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"
#include "parsehelper.h"

static node *parseresult = NULL;
extern int yylex();
static int yyerror(char *errname);

%}

%union {
 char               *id;
 int                 cint;
 float               cflt;
 node               *node;
 type                type;
}

%token BRACKET_L BRACKET_R COMMA SEMICOLON
%token MINUS PLUS STAR SLASH PERCENT LE LT GE GT EQ NE OR AND
%token TRUEVAL FALSEVAL LET
%token INTTYPE FLOATTYPE BOOLTYPE VOIDTYPE
%token EXTERN EXPORT
%token SQR_BRACKET_L SQR_BRACKET_R
%token FOR DO WHILE
%token IF ELSE
%token RETURN
%token BRACE_L BRACE_R
%token NOT
%token CAST_INT CAST_BOOL CAST_FLOAT

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> declarations declaration
%type <node> fundec fundef globaldec globaldef
%type <node> funheader params param
%type <type> basictype
%type <node> funbody
%type <node> const boolconst
%type <node> exprs expr
%type <node> arrexpr
%type <node> vardec vardecs localfundefs
%type <node> localfundef
%type <node> statements statement block

%right LET
%left MINUS PLUS
%left STAR SLASH PERCENT
%left LE LT GE GT EQ NE
%left OR AND
%right CAST_BOOL CAST_INT CAST_FLOAT
%right NOT

%nonassoc IF_NO_ELSE
%nonassoc ELSE

%start program

%%

program: declarations
            {
                parseresult = $1;
            }
            ;

declarations: declarations declaration
                {
                    node* new_final_node = TBmakeDeclaration($2, NULL);

                    node* current_final_node = $1;

                    // Go to the end of the branch
                    while(DECLARATION_NEXT(current_final_node) != NULL) {
                        current_final_node = DECLARATION_NEXT(current_final_node);
                    }

                    // End found, link the new node to the end
                    DECLARATION_NEXT(current_final_node) = new_final_node;

                    $$ = $1;
                }
            | declaration
                {
                    $$ = TBmakeDeclaration($1, NULL);
                }
                ;

declaration: fundec
                {
                    $$ = $1;
                }
            | fundef
                {
                    $$ = $1;
                }
            | globaldec
                {
                    $$ = $1;
                }
            | globaldef
                {
                    $$ = $1;
                }
                ;

fundec: EXTERN funheader SEMICOLON
        {
            $$ = $2;
        }
        ;

fundef: EXPORT funheader BRACE_L funbody BRACE_R
        {
            // Make a new function definition node, and link the function
            // declaration from the header to the new definition
            node* function_definition = TBmakeFunctiondefinition(TRUE, $4);

            FUNCTION_DEFINITION($2) = function_definition;

            $$ = $2;
        }
    | funheader BRACE_L funbody BRACE_R
        {
            // Make a new function definition node, and link the function
            // declaration from the header to the new definition
            node* function_definition = TBmakeFunctiondefinition(FALSE, $3);

            FUNCTION_DEFINITION($1) = function_definition;

            $$ = $1;
        }
    | EXPORT funheader BRACE_L BRACE_R
        {
            // Make a new function definition node, and link the function
            // declaration from the header to the new definition
            node* function_definition = TBmakeFunctiondefinition(TRUE, NULL);

            FUNCTION_DEFINITION($2) = function_definition;

            $$ = $2;
        }
    | funheader BRACE_L BRACE_R
        {
            // Make a new function definition node, and link the function
            // declaration from the header to the new definition
            node* function_definition = TBmakeFunctiondefinition(FALSE, NULL);

            FUNCTION_DEFINITION($1) = function_definition;

            $$ = $1;
        }
        ;

funheader: basictype ID BRACKET_L params BRACKET_R
        {
            $$ = TBmakeFunction(STRcpy($2), $1, $4, NULL);
        }
        | basictype ID BRACKET_L BRACKET_R
        {
            $$ = TBmakeFunction(STRcpy($2), $1, NULL, NULL);
        }
        | VOIDTYPE ID BRACKET_L params BRACKET_R
        {
            $$ = TBmakeFunction(STRcpy($2), TP_void, $4, NULL);
        }
        | VOIDTYPE ID BRACKET_L BRACKET_R
        {
            $$ = TBmakeFunction(STRcpy($2), TP_void, NULL, NULL);
        }
        ;

params: params param
        {
            node* new_final_node = $2;

            node* current_final_node = $1;

            // Go to the end of the branch
            while(paramsGetNextNode(current_final_node) != NULL) {
                current_final_node = paramsGetNextNode(current_final_node);
            }

            // End found, link the new node to the end
            paramsSetNextNode(current_final_node, new_final_node);

            $$ = $1;
        }
        |
        param
        {
            $$ = $1;
        }
        ;

param: basictype ID
    {
        $$ = TBmakeBasicparameter($2, $1, NULL);
    }
    | basictype SQR_BRACKET_L ID SQR_BRACKET_R ID
    {
        node* dimensions_branch = TBmakeArrayparamdimensions($3, NULL);
        $$ = TBmakeArrayparameter($5, $1, NULL, dimensions_branch);
        // TODO: Multi-dimensional arrays
    }
    ;

basictype: BOOLTYPE
        {
            $$ = TP_bool;
        }
        | INTTYPE
        {
            $$ = TP_int;
        }
        | FLOATTYPE
        {
            $$ = TP_float;
        }
        ;

globaldec: EXTERN basictype ID SEMICOLON
        {
            $$ = TBmakeGlobal(STRcpy($3), $2, NULL, NULL);
        }
        | EXTERN basictype SQR_BRACKET_L ID SQR_BRACKET_R ID SEMICOLON
        {
            node* dimensions = TBmakeArrayparamdimensions(STRcpy($4), NULL);
            $$ = TBmakeGlobal(STRcpy($6), $2, NULL, dimensions);
            // TODO: Multi-dimensional arrays
        }
        ;

globaldef: EXPORT basictype ID LET expr SEMICOLON
            {
                $$ = TBmakeGlobaldefinition(STRcpy($3), TRUE, $2, TBmakeExprarray(NULL, $5), NULL);
            }
        | basictype ID LET expr SEMICOLON
            {
                $$ = TBmakeGlobaldefinition(STRcpy($2), FALSE, $1, TBmakeExprarray(NULL, $4), NULL);
            }
        | EXPORT basictype ID SEMICOLON
            {
                $$ = TBmakeGlobaldefinition(STRcpy($3), TRUE, $2, NULL, NULL);
            }
        | basictype ID SEMICOLON
            {
                $$ = TBmakeGlobaldefinition(STRcpy($2), FALSE, $1, NULL, NULL);
            }
        | EXPORT basictype SQR_BRACKET_L exprs SQR_BRACKET_R ID LET arrexpr SEMICOLON
            {
                $$ = TBmakeGlobaldefinition(STRcpy($6), TRUE, $2, $8, $4);
            }
        | basictype SQR_BRACKET_L exprs SQR_BRACKET_R ID LET arrexpr SEMICOLON
            {
                 $$ = TBmakeGlobaldefinition(STRcpy($5), FALSE, $1, $7, $3);
            }
        | EXPORT basictype SQR_BRACKET_L exprs SQR_BRACKET_R ID SEMICOLON
            {
                $$ = TBmakeGlobaldefinition(STRcpy($6), TRUE, $2, NULL, $4);
            }
        | basictype SQR_BRACKET_L exprs SQR_BRACKET_R ID SEMICOLON
            {
                $$ = TBmakeGlobaldefinition(STRcpy($5), FALSE, $1, NULL, $3);
            }
            ;

funbody: vardecs localfundefs statements
        {
            $$ = TBmakeFunctionbody($1, $2, $3);
        }
        | vardecs
        {
            $$ = TBmakeFunctionbody($1, NULL, NULL);
        }
        | statements
        {
            $$ = TBmakeFunctionbody(NULL, NULL, $1);
        }
        | localfundefs
        {
            $$ = TBmakeFunctionbody(NULL, $1, NULL);
        }
        | vardecs statements
        {
            $$ = TBmakeFunctionbody($1, NULL, $2);
        }
        | vardecs localfundefs
        {
            $$ = TBmakeFunctionbody($1, $2, NULL);
        }
        | localfundefs statements
        {
            $$ = TBmakeFunctionbody(NULL, $1, $2);
        }
        ;

localfundefs: localfundef
        {
            // TODO: Multiple local function definitions
        }
        | localfundefs localfundef
        {

        }
        ;

localfundef: funheader BRACE_L funbody BRACE_R
        {
            // Make a new function definition node, and link the function
            // declaration from the header to the new definition
            node* function_definition = TBmakeFunctiondefinition(FALSE, $3);

            FUNCTION_DEFINITION($1) = function_definition;

            $$ = $1;
        }
        ;

vardecs: vardec
        {
            $$ = $1;
        }
    | vardecs vardec
        {
            //TODO: Multiple vardecs
        }
        ;

statements: statement
        {

        }
    | statements statement
        {

        }
        ;

vardec: basictype ID SEMICOLON
        {
            $$ = TBmakeBasicvardeclaration(STRcpy($2), $1, NULL);
        }
    | basictype ID LET expr SEMICOLON
        {
            $$ = TBmakeBasicvardeclaration(STRcpy($2), $1, $4);
        }
    | basictype SQR_BRACKET_L expr SQR_BRACKET_R ID LET arrexpr SEMICOLON
        {
            $$ = TBmakeArrayvardeclaration(STRcpy($5), $1, $7, $3);
        }
    | basictype SQR_BRACKET_L expr SQR_BRACKET_R ID SEMICOLON
        {
            $$ = TBmakeArrayvardeclaration(STRcpy($5), $1, NULL, $3);
        }
        ;

statement: ID LET expr SEMICOLON
        {
            $$ = TBmakeVarassign(STRcpy($1), $3);
        }
        | ID BRACKET_L BRACKET_R SEMICOLON
        {

        }
        | ID BRACKET_L exprs BRACKET_R SEMICOLON
        {
            $$ = TBmakeExpr($3);
        }
        | IF BRACKET_L expr BRACKET_R block %prec IF_NO_ELSE
        {
            $$ = TBmakeBranch($3, $5, NULL);
        }
        | IF BRACKET_L expr BRACKET_R block ELSE block
        {
            $$ = TBmakeBranch($3, NULL, $7);
        }
        | WHILE BRACKET_L expr BRACKET_R block
        {
            $$ = TBmakeWhile($3, $5);
        }
        | DO block WHILE BRACKET_L expr BRACKET_R SEMICOLON
        {
            $$ = TBmakeDowhile($5, $2);
        }
        | FOR BRACKET_L INTTYPE ID LET expr COMMA expr BRACKET_R block
        {

        }
        | FOR BRACKET_L INTTYPE ID LET expr COMMA expr COMMA expr BRACKET_R block
        {

        }
        | RETURN SEMICOLON
        {

        }
        | RETURN expr SEMICOLON
        {

        }
        | ID SQR_BRACKET_L expr SQR_BRACKET_R LET expr SEMICOLON
        {

        }
        ;

arrexpr: SQR_BRACKET_L exprs SQR_BRACKET_R
        {
            $$ = TBmakeExprarray(NULL, $2);
        }
    | expr
        {
            $$ = TBmakeExprarray(NULL, $1);
        }
        ;

block: BRACE_L BRACE_R
        {
        }
    | BRACE_L statements BRACE_R
        {
            $$ = TBmakeStatements($2, NULL);
        }
    | statement
        {
            $$ = TBmakeStatements($1, NULL);
        }
        ;

exprs: exprs COMMA expr
    {
        $$ = TBmakeExprarray($1, $3);
        // TODO: Fix order
    }
    | expr
    {
        $$ = TBmakeExpr($1);
    }
    ;

expr: BRACKET_L expr BRACKET_R
        {
            $$ = $2;
        }
    | expr PLUS expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeArithop(AO_add, $1, $3)));
        }
    | expr MINUS expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeArithop(AO_sub, $1, $3)));
        }
    | expr STAR expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeArithop(AO_mul, $1, $3)));
        }
    | expr SLASH expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeArithop(AO_div, $1, $3)));
        }
    | expr PERCENT expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeArithop(AO_mod, $1, $3)));
        }
    | expr EQ expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeRelop(RO_eq, $1, $3)));
        }
    | expr NE expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeRelop(RO_ne, $1, $3)));
        }
    | expr LT expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeRelop(RO_lt, $1, $3)));
        }
    | expr LE expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeRelop(RO_le, $1, $3)));
        }
    | expr GT expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeRelop(RO_gt, $1, $3)));
        }
    | expr GE expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeRelop(RO_ge, $1, $3)));
        }
    | expr AND expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeLogicop(LO_and, $1, $3)));
        }
    | expr OR expr
        {
            $$ = TBmakeExpr(TBmakeBinop(TBmakeLogicop(LO_or, $1, $3)));
        }
    | MINUS expr %prec MINUS
        {
            $$ = TBmakeExpr(TBmakeMonop(MO_neg, $2));
        }
    | NOT expr
        {
            $$ = TBmakeExpr(TBmakeMonop(MO_not, $2));
        }
    | CAST_BOOL expr
        {
            $$ = TBmakeExpr(TBmakeCast(TP_bool, $2));
        }
    | CAST_INT expr
        {
            $$ = TBmakeExpr(TBmakeCast(TP_int, $2));
        }
    | CAST_FLOAT expr
        {
            $$ = TBmakeExpr(TBmakeCast(TP_float, $2));
        }
    | ID BRACKET_L BRACKET_R
        {
            $$ = TBmakeExpr(TBmakeFuncall(STRcpy($1), NULL));
        }
    | ID BRACKET_L exprs BRACKET_R
        {
            $$ = TBmakeExpr(TBmakeFuncall(STRcpy($1), $3));
        }
    | ID
        {
            $$ = TBmakeExpr(TBmakeVarread(STRcpy($1)));
        }
    | const
        {
            $$ = TBmakeExpr(TBmakeConstant($1));
        }
    | ID SQR_BRACKET_L expr SQR_BRACKET_R
        {
            $$ = TBmakeArrayvarread(STRcpy($1), $3);
        }
        ;

const: boolconst
        {
            $$ = $1;
        }
    | NUM
        {
            $$ = TBmakeIntconst($1);
        }
    | FLOAT
        {
            $$ = TBmakeFloatconst($1);
        }
        ;

boolconst: TRUEVAL
            {
                $$ = TBmakeBoolconst(TRUE);
            }
        | FALSEVAL
            {
                $$ = TBmakeBoolconst(FALSE);
            }
            ;

%%

static int yyerror(char *error) {
    CTIabort("line %d, col %d\nError parsing source code: %s\n",
            global.line + 1, global.col + 1, error);

    return(0);
}

node *YYparseTree(void) {
    DBUG_ENTER("YYparseTree");

    yyparse();

    DBUG_RETURN(parseresult);
}
