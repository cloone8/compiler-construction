/**
 * Implementation of the countMap. For further documentation, see count_map.h
 */

#include "count_map.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"

#include <stdlib.h>

/**
 * Allocates a new countMap to memory, and returns a pointer to it.
 */
countMap* CMcreate() {
    countMap* toReturn = (countMap*) MEMmalloc(sizeof(countMap));
    toReturn->itemCount = 0;
    toReturn->items = NULL;
    return toReturn;
}

/**
 * Adds a new item to the given map, with the given name. This function allocates
 * more memory for the countmap as needed
 */
static countMapItem* CMaddItem(countMap* map, char* name) {
    char* heapString = STRcpy(name);

    DBUG_ASSERT(heapString != NULL, "Memory allocation in CMAddItem failed");

    countMapItem* item = (countMapItem*) MEMmalloc(sizeof(countMapItem));

    item->name = heapString;
    item->count = 0;

    map->itemCount++;
    map->items = realloc(map->items, sizeof(countMapItem*) * map->itemCount);
    map->items[map->itemCount - 1] = item;

    return item;
}

/**
 * Returns a pointer to the item with the given name if it exists in the
 * given map. Otherwise returns NULL
 */
countMapItem* CMgetItem(countMap* map, char* name) {
    for(int i = 0; i < map->itemCount; i++) {
        countMapItem* item = map->items[i];
        if(STReq(item->name, name)) {
            return item;
        }
    }

    return NULL;
}

/**
 * Adds an occurence of the given variable to the map. Automatically adds new items
 * to the map if required.
 */
void CMaddOccurence(countMap* map, char* name) {
    countMapItem* item = CMgetItem(map, name);
    if(item == NULL) {
        item = CMaddItem(map, name);
    }

    item->count++;
}

/**
 * Frees the given map from memory. Automatically also frees any items within the
 * map.
 */
void CMfree(countMap* map) {
    for(int i = 0; i < map->itemCount; i++) {
        countMapItem* item = map->items[i];
        MEMfree(item->name);
        MEMfree(item);
    }
        if(map->items != NULL) {
            MEMfree(map->items);
        }
        MEMfree(map);
}
