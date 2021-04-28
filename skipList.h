#ifndef SKIP_LIST
#define SKIP_LIST

typedef struct skipListNode{

    struct skipListNode* next;
    struct skipListNode* lowerLayer;
    void* data;
}skipListNode;

typedef struct{

    unsigned int countOfEntries;
    unsigned int countOfLayers;
    skipListNode* firstLayer;
}skipListHead;


typedef skipListHead* skipList;

/*Initializes and return a skipList*/
skipList skipList_initializeSkipList();

/*Inserts the given value to given skipList*/
int skipList_insertValue(skipList, void*, unsigned int, int (*compare)(void*,void*));

/*Recursively inserts value through layers - used by skipList_insertValue()*/
skipListNode* skipList_insertInLayer(skipListNode*, void*, unsigned int, int (*compare)(void*,void*), char*);

/*Checks for new layer availability*/
char skipList_newLayerAvailability(skipList);

/*Adds a new layer in skipList*/
int skipList_addLayer(skipList);

/*search given value in given skipList*/
int skipList_searchValue(skipList , void* , int (*compare)(void*,void*));

/*remove given value from given skipList*/
int skipList_removeValue(skipList , void* , int (*compare)(void*,void*));

/*Recursively remove value from all layers - used by skipList_removeValue()*/
int skipList_removeValueFromLayer(skipListNode *, skipListNode *);

/*apply funcToApply to all elements of skipList*/
void skipList_applyToAll(skipList, void*, void*, void*, int (*funcToApply)(void*,void*,void*,void*));

/*free skipList*/
void skipList_destroy(skipList );

#endif