#include "main_args.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"
#include "usage.h"
#include "ctinfo.h"
#include "phase_options.h"
#include "myglobals.h"
#include "strlist.h"

static char* copy_arg(const char* source) {
    char* copy = STRcpy(source);

    if(myglobal.dangling_strings == NULL) {
        myglobal.dangling_strings = strlistMake(copy);
    } else {
        strlistAdd(myglobal.dangling_strings, copy);
    }

    return copy;
}

void OPTcheckOptions(int argc, char **argv) {
    DBUG_ENTER("OPTcheckOptions");

    ARGS_BEGIN(argc, argv);

    ARGS_OPTION("b", PHOinterpretBreakOption(ARG))

    ARGS_FLAG("h", USGprintUsage(); exit(0););

    ARGS_OPTION("p", ARG_RANGE(myglobal.printtree, 0, 3));

    ARGS_OPTION("o", global.outfile = copy_arg(ARG));

    ARGS_OPTION("v", ARG_RANGE(global.verbosity, 0, 3));

    ARGS_FLAG("tc", global.treecheck = TRUE);

    ARGS_OPTION("#", DBUG_PUSH(copy_arg(ARG)));

    ARGS_ARGUMENT(global.infile = copy_arg(ARG); );

    ARGS_UNKNOWN(ARGS_ERROR("Invalid command line entry"));

    ARGS_END();

    if (global.infile == NULL) {
        CTIabort("No input file given.");
    }

    DBUG_VOID_RETURN;
}
