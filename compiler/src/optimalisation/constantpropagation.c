#include "constantpropagation.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "free_node.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"
#include "copy.h"
#include "globals.h"
#include "util.h"
#include "key_map.h"
#include "symbol_table.h"

/*
 * INFO structure
 */
struct INFO {
    keyMap* map;
    bool replacephase;
    bool countphase;
};


/*
 * INFO macros
 */

#define MAP(__arg_info_struct__) ((__arg_info_struct__)->map)
#define REPLACE(__arg_info_struct__) ((__arg_info_struct__)->replacephase)
#define DOCOUNT(__arg_info_struct__) ((__arg_info_struct__)->countphase)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info* CPRmakeInfo(void) {
    DBUG_ENTER("CPRmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    result->map = KMcreate();
    result->replacephase = FALSE;
    result->countphase = FALSE;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info* CPRfreeInfo(info *info) {
    DBUG_ENTER("CPRfreeInfo");

    KMfree(info->map);
    info = MEMfree(info);

    DBUG_RETURN(info);
}


node* CPRbasicvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRbasicvardeclaration");

    if(!REPLACE(arg_info) && !DOCOUNT(arg_info)) {
        keyMapItem* item = KMaddItem(MAP(arg_info), BASICVARDECLARATION_VARPOINTER(arg_node));
        item->vartype = BASICVARDECLARATION_VARPOINTER(arg_node)->vartype;
        DBUG_PRINT("CPR", ("Adding %s to keymap", BASICVARDECLARATION_VARNAME(arg_node)));
    } else if(REPLACE(arg_info)) {
        symbol_table_entry* entry = BASICVARDECLARATION_VARPOINTER(arg_node);
        keyMapItem* item = KMgetItem(MAP(arg_info), entry);

        if(item != NULL && item->count == 1 && item->can_read == TRUE) {
            DBUG_PRINT("CPR",("Deleting %s", BASICVARDECLARATION_VARNAME(arg_node)));
            symbol_table_remove(entry->table, entry->idx);
            BASICVARDECLARATION_VARPOINTER(arg_node) = NULL;

            DBUG_RETURN(NULL);
        }
    }

    DBUG_RETURN(arg_node);
}

node* CPRvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRvarread");

    if(REPLACE(arg_info)) {
        keyMapItem* item = KMgetItem(MAP(arg_info), VARREAD_VARPOINTER(arg_node));

        if(item != NULL && item->count == 1 && item->can_read == TRUE) {
            DBUG_PRINT("CPR", ("Can replace %s", VARREAD_VARNAME(arg_node)));
            node* toreturn;
            switch(item->vartype) {
                case TP_int:
                    toreturn = TBmakeConstant(TBmakeIntconst(item->intval));
                    break;
                case TP_float:
                    toreturn = TBmakeConstant(TBmakeFloatconst(item->floatval));
                    break;
                case TP_bool:
                    toreturn = TBmakeConstant(TBmakeBoolconst(item->boolval));
                    break;
                default:
                    DBUG_ASSERT((FALSE), "Invalid type!");
                    DBUG_RETURN(arg_node);
            }
            TRAVpush(TR_free);
            FREEvarread(arg_node, NULL);
            TRAVpop();

            DBUG_RETURN(toreturn);
        }
    }

    DBUG_RETURN(arg_node);
}

node* CPRvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRvarassign");

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    if(DOCOUNT(arg_info)) {
        keyMapItem* item = KMgetItem(MAP(arg_info), VARASSIGN_VARPOINTER(arg_node));

        if(item != NULL) {
            DBUG_PRINT("CPR", ("Found item for %s", VARASSIGN_VARNAME(arg_node)));

            item->count++;

            if(item->count == 1) {
                if(NODE_TYPE(EXPR_CURRENT(VARASSIGN_VALUE(arg_node))) == N_constant) {
                    item->can_read = TRUE;
                    DBUG_PRINT("CPR", ("Setting initval for %s", VARASSIGN_VARNAME(arg_node)));
                    switch(item->vartype) {
                        case TP_int:
                            item->intval = INTCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(VARASSIGN_VALUE(arg_node))));
                            break;
                        case TP_float:
                            item->floatval = FLOATCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(VARASSIGN_VALUE(arg_node))));
                            break;
                        case TP_bool:
                            item->boolval = BOOLCONST_VALUE(CONSTANT_CURRENT(EXPR_CURRENT(VARASSIGN_VALUE(arg_node))));
                            break;
                        default:
                            DBUG_ASSERT((FALSE), "Invalid type!");
                            break;
                    }
                }
            }

            DBUG_PRINT("CPR", ("Count for %s: %zu", VARASSIGN_VARNAME(arg_node), item->count));
        } else {
            DBUG_PRINT("CPR", ("Didn't find item for %s", VARASSIGN_VARNAME(arg_node)));
        }
    }  else if(REPLACE(arg_info)) {
        keyMapItem* item = KMgetItem(MAP(arg_info), VARASSIGN_VARPOINTER(arg_node));

        if(item != NULL && item->count == 1 && item->can_read == TRUE) {

            TRAVpush(TR_free);
            FREEvarassign(arg_node, NULL);
            TRAVpop();

            DBUG_RETURN(TBmakeNoop());
        }
    }

    DBUG_RETURN(arg_node);
}

node* CPRfunctionvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRfunctionvardeclaration");

    node* result = TRAVdo(FUNCTIONVARDECLARATION_CURRENT(arg_node), arg_info);

    FUNCTIONVARDECLARATION_NEXT(arg_node) = TRAVopt(FUNCTIONVARDECLARATION_NEXT(arg_node), arg_info);

    if(result == NULL) {
        node* toreturn = FUNCTIONVARDECLARATION_NEXT(arg_node);
        FUNCTIONVARDECLARATION_NEXT(arg_node) = NULL;

        TRAVpush(TR_free);
        FREEfunctionvardeclaration(arg_node, NULL);
        TRAVpop();

        DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(arg_node);
}

node* CPRdo(node* syntaxtree) {
    DBUG_ENTER("CPRdo");

    TRAVpush(TR_cpr);
    info *arg_info = CPRmakeInfo();

    DBUG_PRINT("CPR", ("Starting var identify phase"));
    syntaxtree = TRAVdo(syntaxtree, arg_info);

    DOCOUNT(arg_info) = TRUE;
    DBUG_PRINT("CPR", ("Starting var count phase"));
    syntaxtree = TRAVdo(syntaxtree, arg_info);

    DOCOUNT(arg_info) = FALSE;
    REPLACE(arg_info) = TRUE;
    DBUG_PRINT("CPR", ("Starting var replace phase"));
    syntaxtree = TRAVdo(syntaxtree, arg_info);

    CPRfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
