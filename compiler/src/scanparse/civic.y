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
 nodetype            nodetype;
 char               *id;
 int                 cint;
 float               cflt;
 binop               cbinop;
 node               *node;
}

%token BRACKET_L BRACKET_R COMMA SEMICOLON
%token MINUS PLUS STAR SLASH PERCENT LE LT GE GT EQ NE OR AND
%token TRUEVAL FALSEVAL LET
%token INTTYPE FLOATTYPE BOOLTYPE VOIDTYPE
%token EXTERN EXPORT STATIC
%token SQR_BRACKET_L SQR_BRACKET_R
%token FOR DO WHILE
%token IF ELSE
%token RETURN
%token BRACE_L BRACE_R
%token NEG

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> intval floatval boolval constant expr
%type <node> stmts stmt assign varlet program
%type <cbinop> binop

%start program

%%

program: declarations
            {
                parseresult = $1
            }
            ;

declarations: declaration declarations
                {

                }
            | declaration
                {

                }
                ;

declaration: fundec
                {

                }
            | fundef
                {

                }
            | globaldec
                {

                }
            | globaldef
                {

                }
                ;

fundec: EXTERN funheader SEMICOLON
        {

        }
        ;

fundef: EXPORT funheader BRACE_L funbody BRACE_R
        {

        }
    | funheader BRACE_L funbody BRACE_R
        {

        }
        ;

funheader: rettype ID BRACKET_L params BRACKET_R
        {

        }
        | rettype ID BRACKET_L BRACKET_R
        {

        }
        ;

params: param params
        {

        }
        |
        param
        {

        }
        ;

param: type ID
    {

    }
    ;

rettype: VOIDTYPE
        {

        }
        | basictype
        {

        };

basictype: BOOLTYPE
        {

        }
        | INTTYPE
        {

        }
        | FLOATTYPE
        {

        }
        ;

globaldec: EXTERN basictype ID SEMICOLON
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
            ;

funbody: vardecs statements
        {

        }
        | vardecs
        {

        }
        | statements
        {

        }
        |
        {

        }
        ;

vardecs: vardec
        {

        }
    | vardec vardecs
        {

        }
        ;

statements: statement
        {

        }
    | statement statements
       {

       }
       ;

vardec: basictype ID SEMICOLON
        {

        }
    | basictype ID LET expr SEMICOLON
        {

        }

statement: ID LET expr SEMICOLON
        {

        }
        | ID BRACKET_L BRACKET_R SEMICOLON
        {

        }
        | ID BRACKET_L exprs BRACKET_R SEMICOLON
        {

        }
        | IF BRACKET_L expr BRACKET_R block
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

exprs: expr exprs
    {

    }
    | expr
    {

    }
    ;

expr: BRACKET_L expr BRACKET_R
        {

        }
    | expr binop expr
        {

        }
    | monop expr
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
        ;

binop: arithop
        {

        }
    | relop
        {

        }
    | logicop
        {

        }
        ;

arithop: PLUS
        {

        }
        | MINUS
        {

        }
        | STAR
        {

        }
        | SLASH
        {

        }
        | PERCENT
        {

        }
        ;

relop: EQ
        {

        }
    | NE
        {

        }
    | LT
        {

        }
    | LE
        {

        }
    | GT
        {

        }
    | GE
        {

        }
        ;

logicop: AND
        {

        }
        | OR
        {

        }
        ;

monop: MINUS
        {

        }
    | NEG
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

boolconst: TRUE
            {

            }
        | FALSE
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
