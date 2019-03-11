#ifndef __PARSEHELPER_H__
#define __PARSEHELPER_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "types.h"
#include "tree_basic.h"
#include "str.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"

size_t countNewlines(char* string, size_t length);

node* paramsGetNextNode(node* current);
void paramsSetNextNode(node* current, node* newNext);
node* funcallExprsToParams(node *exprarray);

int getIntFromText(char* text, int line, int col);
int getFloatFromText(char* text, int line, int col);
#endif
