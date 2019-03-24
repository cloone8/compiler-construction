#ifndef __COUNT_MAP_H__
#define __COUNT_MAP_H__

#include <stdlib.h>

typedef struct {
    char* name;
    size_t count;
} countMapItem;

typedef struct {
    size_t itemCount;
    countMapItem** items;
} countMap;

#define CMgetItemCount(map) ((map)->itemCount)

countMap* CMcreate();

countMapItem* CMgetItem(countMap* map, char* name);

void CMaddOccurence(countMap* map, char* name);

void CMfree(countMap* map);


#endif
