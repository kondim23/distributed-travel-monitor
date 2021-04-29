#ifndef GENERIC_HASH
#define GENERIC_HASH

typedef struct hashnode{

    void* data;
    struct hashnode* nextblock;
}hashnode;

typedef hashnode** genericHashTable;

/*Return an allocated-initialized hash*/
genericHashTable hash_Initialize();

/*Search given data in hash. If not found and if unsigned int size>0 insert and return*/
void* hash_searchValue(genericHashTable, void*, void*, unsigned int, int (*compare)(void*,void*));

/*free given hash*/
void hash_destroy(genericHashTable);

/*Apply functionToExecute in all data nodes of hash*/
void hash_applyToAllNodes(genericHashTable, void *, void (*functionToExecute) (void*, void*));

#endif