#ifndef __VARSPLIT_INFO_H__
#define __VARSPLIT_INFO_H__

#include "types.h"

/*
 * INFO structure
 */
struct INFO {
    node* initbody;
    node* currentfunbody;
    symbol_table* currentsymtable;
    node* newassignblock;
    node* newdeclblock;
    node* programnode;
};

/*
 * INFO macros
 */

#define INITBODY(__arg_info_struct__) ((__arg_info_struct__)->initbody)
#define FUNBODY(__arg_info_struct__) ((__arg_info_struct__)->currentfunbody)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentsymtable)
#define ASSIGNS(__arg_info_struct__) ((__arg_info_struct__)->newassignblock)
#define DECLS(__arg_info_struct__) ((__arg_info_struct__)->newdeclblock)
#define PROGRAM_NODE(__arg_info_struct__) ((__arg_info_struct__)->programnode)

/*
 * INFO functions
 */

#endif
