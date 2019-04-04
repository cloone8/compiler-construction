/**
 * Implementation of the keyMap. For further documentation, see count_map.h
 */

#include "key_map.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"

#include <stdlib.h>

/**
 * Allocates a new keyMap to memory, and returns a pointer to it.
 */
keyMap* KMcreate() {
    keyMap* toReturn = (keyMap*) MEMmalloc(sizeof(keyMap));
    toReturn->itemCount = 0;
    toReturn->items = NULL;
    return toReturn;
}

/**
 * Adds a new item to the given map, with the given name. This function allocates
 * more memory for the keyMap as needed
 */
keyMapItem* KMaddItem(keyMap* map, void* key) {
    keyMapItem* item = (keyMapItem*) MEMmalloc(sizeof(keyMapItem));

    item->key = key;
    item->intval = 0;
    item->was_init = FALSE;
    item->can_read = FALSE;

    map->itemCount++;
    map->items = realloc(map->items, sizeof(keyMapItem*) * map->itemCount);
    map->items[map->itemCount - 1] = item;

    return item;
}

/**
 * Returns a pointer to the item with the given name if it exists in the
 * given map. Otherwise returns NULL
 */
keyMapItem* KMgetItem(keyMap* map, void* key) {
    for(size_t i = 0; i < map->itemCount; i++) {
        keyMapItem* item = map->items[i];
        if(item->key == key) {
            return item;
        }
    }

    return NULL;
}

/**
 * Frees the given map from memory. Automatically also frees any items within the
 * map.
 */
void KMfree(keyMap* map) {
    for(size_t i = 0; i < map->itemCount; i++) {
        keyMapItem* item = map->items[i];
        MEMfree(item);
    }
        if(map->items != NULL) {
            MEMfree(map->items);
        }
        MEMfree(map);
}
