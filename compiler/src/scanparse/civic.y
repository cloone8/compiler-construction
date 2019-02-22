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
%token NEG

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> declarations declaration
%type <node> fundec fundef globaldec globaldef
%type <node> funheader params param
%type <type> basictype

%right LET
%left MINUS PLUS
%left STAR SLASH PERCENT
%left LE LT GE GT EQ NE
%left OR AND
%right NEG

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
                    $$ = TBmakeDeclaration($2, $1);
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

        }
    | funheader BRACE_L funbody BRACE_R
        {

        }
    | EXPORT funheader BRACE_L BRACE_R
        {

        }
    | funheader BRACE_L BRACE_R
        {

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

params: param params
        {
            ARRAYPARAMETER_NEXT($1) = $2;
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

        }
        | EXTERN basictype SQR_BRACKET_L ID SQR_BRACKET_R ID SEMICOLON
        {

        }
        ;

globaldef: EXPORT basictype ID LET expr SEMICOLON
            {

            }
        | basictype ID LET expr SEMICOLON
            {

            }
        | EXPORT basictype ID SEMICOLON
            {

            }
        | basictype ID SEMICOLON
            {

            }
        | EXPORT basictype SQR_BRACKET_L expr SQR_BRACKET_R ID LET arrexpr SEMICOLON
            {

            }
        | basictype SQR_BRACKET_L expr SQR_BRACKET_R ID LET arrexpr SEMICOLON
            {

            }
        | EXPORT basictype SQR_BRACKET_L expr SQR_BRACKET_R ID SEMICOLON
            {

            }
        | basictype SQR_BRACKET_L expr SQR_BRACKET_R ID SEMICOLON
            {

            }
            ;

funbody: vardecs localfundefs statements
        {

        }
        | vardecs
        {

        }
        | statements
        {

        }
        | localfundefs
        {

        }
        | vardecs statements
        {

        }
        | vardecs localfundefs
        {

        }
        | localfundefs statements
        {

        }
        ;

localfundefs: localfundef
        {

        }
        | localfundef localfundefs
        {

        }
        ;

localfundef: funheader BRACE_L funbody BRACE_R
        {

        }
        ;

vardecs: vardec
        {

        }
    | vardecs vardec
        {

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

        }
    | basictype ID LET expr SEMICOLON
        {

        }
    | basictype SQR_BRACKET_L expr SQR_BRACKET_R ID LET arrexpr SEMICOLON
        {

        }
    | basictype SQR_BRACKET_L expr SQR_BRACKET_R ID SEMICOLON
        {

        }
        ;

statement: ID LET expr SEMICOLON
        {

        }
        | ID BRACKET_L BRACKET_R SEMICOLON
        {

        }
        | ID BRACKET_L exprs BRACKET_R SEMICOLON
        {

        }
        | IF BRACKET_L expr BRACKET_R block %prec IF_NO_ELSE
        {

        }
        | IF BRACKET_L expr BRACKET_R block ELSE block
        {

        }
        | WHILE BRACKET_L expr BRACKET_R block
        {

        }
        | DO block WHILE BRACKET_L expr BRACKET_R SEMICOLON
        {

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

        }
    | expr
        {

        }
        ;

block: BRACE_L BRACE_R
        {

        }
    | BRACE_L statements BRACE_R
        {

        }
    | statement
        {

        }
        ;

exprs: exprs COMMA expr
    {

    }
    | expr
    {

    }
    ;

expr: BRACKET_L expr BRACKET_R
        {

        }
    | expr PLUS expr
        {

        }
    | expr MINUS expr
        {

        }
    | expr STAR expr
        {

        }
    | expr SLASH expr
        {

        }
    | expr PERCENT expr
        {

        }
    | expr EQ expr
        {

        }
    | expr NE expr
        {

        }
    | expr LT expr
        {

        }
    | expr LE expr
        {

        }
    | expr GT expr
        {

        }
    | expr GE expr
        {

        }
    | expr AND expr
        {

        }
    | expr OR expr
        {

        }
    | MINUS expr %prec MINUS
        {

        }
    | NEG expr
        {

        }
    | BRACKET_L basictype BRACKET_R expr
        {

        }
    | ID BRACKET_L BRACKET_R
        {

        }
    | ID BRACKET_L exprs BRACKET_R
        {

        }
    | ID
        {

        }
    | const
        {

        }
    | ID SQR_BRACKET_L expr SQR_BRACKET_R
        {

        }
        ;

const: boolconst
        {

        }
    | NUM
        {

        }
    | FLOAT
        {

        }
        ;

boolconst: TRUEVAL
            {

            }
        | FALSEVAL
            {

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
