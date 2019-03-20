/* ---------------------------------------------------------------------------
 *
 * SAC Compiler Construction Framework
 *
 * ---------------------------------------------------------------------------
 *
 * SAC COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER
 *
 * (c) Copyright 1994 - 2011 by
 *
 *   SAC Development Team
 *   SAC Research Foundation
 *
 *   http://www.sac-home.org
 *   email:info@sac-home.org
 *
 *   All rights reserved
 *
 * ---------------------------------------------------------------------------
 *
 * The SAC compiler construction framework, all accompanying
 * software and documentation (in the following named this software)
 * is developed by the SAC Development Team (in the following named
 * the developer) which reserves all rights on this software.
 *
 * Permission to use this software is hereby granted free of charge
 * exclusively for the duration and purpose of the course
 *   "Compilers and Operating Systems"
 * of the MSc programme Grid Computing at the University of Amsterdam.
 * Redistribution of the software or any parts thereof as well as any
 * alteration  of the software or any parts thereof other than those
 * required to use the compiler construction framework for the purpose
 * of the above mentioned course are not permitted.
 *
 * The developer disclaims all warranties with regard to this software,
 * including all implied warranties of merchantability and fitness.  In no
 * event shall the developer be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data, or profits, whether in an action of contract, negligence, or
 * other tortuous action, arising out of or in connection with the use or
 * performance of this software. The entire risk as to the quality and
 * performance of this software is with you. Should this software prove
 * defective, you assume the cost of all servicing, repair, or correction.
 *
 * ---------------------------------------------------------------------------
 */



#include <stdio.h>

#include "scanparse.h"
#include "dbug.h"
#include "globals.h"
#include "ctinfo.h"
#include "str.h"
#include "memory.h"


/*
 * file handle for parsing
 */
FILE *yyin;

/*
 * external parser function from fun.y
 */
extern node *YYparseTree();

/**
 * Fills the initial raw syntax tree with the parsed nodes, gathered by bison.
 * @param syntax_tree: MUST be NULL
 * @return the filled in syntax tree.
 */
node* SPdoScanParse(node *syntax_tree) {
    DBUG_ENTER("SPdoScanParse");

    node *result = NULL;
    char *filename;

    // Something went wrong with phasing if the syntax tree is already initialised
    DBUG_ASSERT(syntax_tree == NULL,
                "SPdoScanParse() called with existing syntax tree.");

    if (global.cpp) {
        // Take the preprocessed file
        filename = STRcatn(3,
                            "./",
                            global.infile,
                            CVC_PROCESSED_EXTENSION);
    } else {
        // Take the raw file
        filename = STRcpy(global.infile);
    }

    yyin = fopen(filename, "r");

    if (yyin == NULL) {
        CTIabort("Cannot open file '%s'.", filename);
    }

    // For clean closing later
    myglobal.input_file_handler = yyin;

    // Do the actual parsing
    result = YYparseTree();

    // Delete the intermediate file to make sure we don't leave any trash
    char* rm_intermediate_file_str = STRcat(DELETE_COMMAND, filename);
    int err = system(rm_intermediate_file_str);
    rm_intermediate_file_str = MEMfree(rm_intermediate_file_str);

    if(err) {
        MEMfree(filename);
        CTIabort("Could not remove intermediate file, some junk files might have been left behind");
    }

    MEMfree(filename);

    DBUG_RETURN(result);
}

/**
 * Runs the GCC C preprocessor, putting the result in an intermediate file.
 * @param syntax_tree: Parameter required by the framework. UNUSED_VAR
 * @return NULL
 */
node* SPdoRunPreProcessor(node *syntax_tree) {
    UNUSED_VAR(syntax_tree);

    int  err;
    char *cppcallstr;
    char *processedfile;

    DBUG_ENTER("SPdoRunPreProcessor");

    // New name of pre-processed file
    processedfile = STRcat(global.infile, CVC_PROCESSED_EXTENSION);

    cppcallstr = STRcatn(5,
                        "gcc -E -x c ",
                        global.infile,
                        " > ./",
                        processedfile,
                        ERROR_TO_NULL);

    err = system(cppcallstr);

    if (err) {
        // Delete the intermediate file and free the strings.
        char* rm_intermediate_file_str = STRcat(DELETE_COMMAND, processedfile);
        err = system(rm_intermediate_file_str);

        rm_intermediate_file_str = MEMfree(rm_intermediate_file_str);
        cppcallstr = MEMfree(cppcallstr);
        processedfile = MEMfree(processedfile);

        CTIabort("Unable to run C preprocessor");
    }

    cppcallstr = MEMfree(cppcallstr);
    processedfile = MEMfree(processedfile);

    // Let the other phases know that the preprocessor has been used.
    global.cpp = TRUE;

    DBUG_RETURN(NULL);
}
