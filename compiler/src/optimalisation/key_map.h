#ifndef __KEY_MAP_H__
#define __KEY_MAP_H__

#include <stdlib.h>
#include "types.h"

typedef struct {
    void* key;
    bool was_init;
    union {
        int intval;
        float floatval;
        bool boolval;
    };
} keyMapItem;

typedef struct {
    size_t itemCount;
    keyMapItem** items;
} keyMap;

#define KMgetItemCount(map) ((map)->itemCount)

keyMap* KMcreate();

keyMapItem* KMaddItem(keyMap* map, void* key);

keyMapItem* KMgetItem(keyMap* map, void* key);

void KMfree(keyMap* map);


#endif
