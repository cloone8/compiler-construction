#include "types.h"
#include "dbug.h"

#include "util.h"

type type_to_array_type(type current) {
    DBUG_ENTER("type_to_array_type");
    DBUG_ASSERT((current != TP_void && current != TP_unknown), "Invalid type given");

    switch(current) {
        case TP_bool:
            DBUG_RETURN(TP_bool_array);
        case TP_int:
            DBUG_RETURN(TP_int_array);
        case TP_float:
            DBUG_RETURN(TP_float_array);
        default:
            DBUG_RETURN(current);
    }
}
