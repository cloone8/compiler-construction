%{

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "node_basic.h"
#include "str.h"
#include "free.h"
#include "civic.tab.h"
#include "globals.h"
#include "ctinfo.h"
#include "parsehelper.h"

#define FILTER(token) \
    global.col += yyleng;  \
    return(token);

#define LINEBUF_SIZE 256
#define YY_NO_UNPUT
#define YY_NO_INPUT

%}

%p 6000 /* set positions to 6000 */
%e 2000 /* set nodes to 1100 */
%option noyywrap
%option nounput


%%

"("                        { FILTER(BRACKET_L); }
")"                        { FILTER(BRACKET_R); }
"{"                        { FILTER(BRACE_L); }
"}"                        { FILTER(BRACE_R); }
","                        { FILTER(COMMA); }
";"                        { FILTER(SEMICOLON); }

"-"                        { FILTER(MINUS); }
"+"                        { FILTER(PLUS);  }
"*"                        { FILTER(STAR);  }
"/"                        { FILTER(SLASH);  }
"%"                        { FILTER(PERCENT);  }
"!"                        { FILTER(NEG); }

"<="                       { FILTER(LE);    }
"<"                        { FILTER(LT);    }
">="                       { FILTER(GE);    }
">"                        { FILTER(GT);    }
"=="                       { FILTER(EQ);    }
"!="                       { FILTER(NE);    }
"&&"                       { FILTER(AND);    }
"||"                       { FILTER(OR);    }

"="                        { FILTER(LET);    }

"true"                     { FILTER(TRUEVAL); }
"false"                    { FILTER(FALSEVAL); }

"int"                      { FILTER(INTTYPE); }
"float"                    { FILTER(FLOATTYPE); }
"bool"                     { FILTER(BOOLTYPE); }
"void"                     { FILTER(VOIDTYPE); }

"extern"                   { FILTER(EXTERN); }
"export"                   { FILTER(EXPORT); }
"static"                   { FILTER(STATIC); }

"["                        { FILTER(SQR_BRACKET_L); }
"]"                        { FILTER(SQR_BRACKET_R); }

"for"                      { FILTER(FOR); }
"do"                       { FILTER(DO); }
"while"                    { FILTER(WHILE); }
"return"                   { FILTER(RETURN); }

"if"                       { FILTER(IF); }
"else"                     { FILTER(ELSE); }


[A-Za-z][A-Za-z0-9_]*      { yylval.id = STRcpy(yytext);
                             FILTER(ID);
                           }

[0-9]+                     { yylval.cint=atoi(yytext);
                             FILTER(NUM);
                           }

[0-9]*\.[0-9]+             { yylval.cflt=atof(yytext);
                             FILTER(FLOAT);
                           }

\n.*                       { global.line += 1;
                             global.col = 0;
                             yyless(1);
                           }

[ \t]                      { global.col += yyleng;
                           }

\/\/[^\n]*(\r\n|\r|\n)     { global.col = 0;
                             global.line += 1;
                           }

\/\*([^*]|\*+[^\/])*\*+\/  { global.col += yyleng;
                             global.line += countNewlines(yytext, yyleng);
                           }
