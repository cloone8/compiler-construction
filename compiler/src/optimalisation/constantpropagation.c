#include "constantpropagation.h"
#include "types.h"
#include "tree_basic.h"
#include "node_basic.h"
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

/**
 * Basicvardeclaration traversal. Does something else depending on the phase.
 *
 * Phase 1 (Identify phase): Adds itself to the keymap to declare this variable to be
 *                           valid for eventual replacement
 * Phase 2 (Count phase): Does nothing
 *
 * Phase 3 (Replace phase): If the variable was only written to once, delete it from
 *                          its symbol table and return NULL to indicate that this
 *                          basicvardec is no longer needed
 *
 * @param arg_node: The basicvardeclaration node to process
 * @param arg_info: The info struct to process
 *
 * @return: Itself or NULL
 */
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

/**
 * Global traversal. Does something else depending on the phase.
 *
 * Phase 1 (Identify phase): Adds itself to the keymap to declare this variable to be
 *                           valid for eventual replacement, if it is not exported
 *                           or imported
 * Phase 2 (Count phase): Does nothing
 *
 * Phase 3 (Replace phase): If the variable was only written to once, delete it from
 *                          its symbol table and return NULL to indicate that this
 *                          basicvardec is no longer needed
 *
 * @param arg_node: The Global node to process
 * @param arg_info: The info struct to process
 *
 * @return: Itself or NULL
 */
node* CPRglobal(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRglobal");

    if(!REPLACE(arg_info) && !DOCOUNT(arg_info)) {
        if(GLOBAL_DEFINITION(arg_node) != NULL) {
            symbol_table_entry* entry = GLOBALDEFINITION_VARPOINTER(GLOBAL_DEFINITION(arg_node));
            if(!entry->isimport && ! entry->isexport) {
                keyMapItem* item = KMaddItem(MAP(arg_info), entry);
                item->vartype = entry->vartype;
                DBUG_PRINT("CPR", ("Adding %s to keymap", entry->name));
            }
        }
    } else if(REPLACE(arg_info)) {
        if(GLOBAL_DEFINITION(arg_node) != NULL) {
            symbol_table_entry* entry = GLOBALDEFINITION_VARPOINTER(GLOBAL_DEFINITION(arg_node));
            keyMapItem* item = KMgetItem(MAP(arg_info), entry);

            if(item != NULL && item->count == 1 && item->can_read == TRUE) {
                DBUG_PRINT("CPR",("Deleting %s", entry->name));
                symbol_table_remove(entry->table, entry->idx);
                GLOBALDEFINITION_VARPOINTER(GLOBAL_DEFINITION(arg_node)) = NULL;

                DBUG_RETURN(NULL);
            }
        }
    }

    DBUG_RETURN(arg_node);
}

/**
 * Varread traversal. Does something else depending on the phase.
 *
 * Phase 1 (Identify phase): Does nothing
 * Phase 2 (Count phase): Does nothing
 *
 * Phase 3 (Replace phase): If the varreads variable was only written to once, replace this
 *                          varread with the proper value as stored in the keymap
 *
 * @param arg_node: The varread node to process
 * @param arg_info: The info struct to process
 *
 * @return: Itself or a constant
 */
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

/**
 * Varassign traversal. Does something else depending on the phase.
 *
 * Phase 1 (Identify phase): Does nothing
 * Phase 2 (Count phase): Increments the write counter in the keymap for this variable,
 *                        if the variable is in the keypmap. Otherwise does nothing.
 *
 * Phase 3 (Replace phase): If the variable was only written to once and is valid for
 *                          constant substitution, replace this varassign with a NOOP
 *                          because all its varreads have been replaces and this
 *                          varassign is no longer nessecary.
 *
 * @param arg_node: The varassign node to process
 * @param arg_info: The info struct to process
 *
 * @return: Itself or NULL
 */
node* CPRvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRvarassign");

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    if(DOCOUNT(arg_info)) {
        keyMapItem* item = KMgetItem(MAP(arg_info), VARASSIGN_VARPOINTER(arg_node));

        if(item != NULL) {
            DBUG_PRINT("CPR", ("Found item for %s", VARASSIGN_VARNAME(arg_node)));

            item->count++;

            if(item->count == 1) {
                // Store the value for this varassign in the keymap, as it is the
                // first time this variable was written to (and thus the initial value)
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
            // Valid for substitution and thus all varreads have been substituted.
            // Remove this varassign as it's no longer needed
            TRAVpush(TR_free);
            FREEvarassign(arg_node, NULL);
            TRAVpop();

            DBUG_RETURN(TBmakeNoop());
        }
    }

    DBUG_RETURN(arg_node);
}

/**
 * Included to handle the deletion of basicvardecs. Removes itself when the
 * child basicvardec was removed.
 *
 * @param arg_node: The functionvardeclaration to process
 * @param arg_info: The info struct
 *
 * @return: Itself, or the next functionvardec in line if arg_node was deleted
 */
node* CPRfunctionvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRfunctionvardeclaration");

    node* result = TRAVdo(FUNCTIONVARDECLARATION_CURRENT(arg_node), arg_info);

    FUNCTIONVARDECLARATION_NEXT(arg_node) = TRAVopt(FUNCTIONVARDECLARATION_NEXT(arg_node), arg_info);

    if(result == NULL) {
        // This functionvardec was deleted.
        node* toreturn = FUNCTIONVARDECLARATION_NEXT(arg_node);
        FUNCTIONVARDECLARATION_NEXT(arg_node) = NULL;

        TRAVpush(TR_free);
        FREEfunctionvardeclaration(arg_node, NULL);
        TRAVpop();

        DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(arg_node);
}

/**
 * Included to handle the deletion of globals. Removes itself when the
 * child global was removed.
 *
 * @param arg_node: The declaration to process
 * @param arg_info: The info struct
 *
 * @return: Itself, or the next declaration in line if arg_node was deleted
 */
node* CPRdeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("CPRdeclaration");

    node* result = TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);

    DECLARATION_NEXT(arg_node) = TRAVopt(DECLARATION_NEXT(arg_node), arg_info);

    if(result == NULL) {
        // This functionvardec was deleted.
        node* toreturn = DECLARATION_NEXT(arg_node);
        DECLARATION_NEXT(arg_node) = NULL;

        TRAVpush(TR_free);
        FREEdeclaration(arg_node, NULL);
        TRAVpop();

        DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(arg_node);
}

/**
 * Starts the constant propagation traversal.
 * The propagation traversal is a three part traversal consisting of the following
 * steps:
 *
 * 1. Go through all basicvardecs to identify variables to fold. This excludes any
 *    imported, external or function parameter variables.
 * 2. Count the assignments to those variables. We do not fold variables with
 *    multiple assignments, as we have not transformed the AST into SSA form, and
 *    we consequently can't properly discern dependencies
 * 3. Replace any varreads of single-assignment known variables, and delete those
 *    variables from the symbol table to save stack space.
 *
 * @param syntaxtree: The tree to propagate
 * @return: The propagated tree
 */
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

    if(PROGRAM_FIRST(syntaxtree) == NULL) {
        CTIwarn("The output file is empty as no output has been generated.");
        CTIwarnContinued("This is the result of compiling a file which exports no functions.");
        if(global.outfile != NULL) {
            FILE* outfile = fopen(global.outfile, "w");
            fclose(outfile);
        }
        CTIterminateCompilation(syntaxtree);
    }

    DBUG_RETURN(syntaxtree);
}
