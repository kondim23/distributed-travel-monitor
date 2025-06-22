#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/genericHashTable.h"
#include "../include/bloomFilter.h"

int numBuckets = 100;
static unsigned long (*hash_function)(unsigned char*) = &djb2;

/*Return an allocated-initialized hash*/
genericHashTable hash_Initialize() {

    /*Allocate a hash of hashnode pointers*/
    genericHashTable Hash = (genericHashTable) malloc(sizeof(hashnode*)*numBuckets);
    for (int i=0 ; i<numBuckets ; i++) Hash[i]=NULL;
    return Hash;
}

/*Search given data in hash. If not found and if unsigned int size>0 insert and return*/
void* hash_searchValue(genericHashTable Hash, void* value, void* data, unsigned int size, int (*compare)(void*,void*)) {

    int key;
    hashnode *currentNode,*lastNode;
    
    /*Calculating right bucket via hash_function*/
    key=(*hash_function)((char*)value)%numBuckets;

    /*In case of empty bucket*/
    if (Hash[key]==NULL) {

        /*size==0 -> no insertion*/
        if (!size) return NULL;

        /*else allocate new node and insert*/
        Hash[key]=(hashnode*)malloc(sizeof(hashnode));

        Hash[key]->data = malloc(size);
        memcpy(Hash[key]->data, data, size);
        Hash[key]->nextblock=NULL;

        return Hash[key]->data;
    }

    /*Navigate in bucket*/
    currentNode=Hash[key];
    while (currentNode!=NULL) {

        /*Desired data found*/
        if (!(*compare)(data,currentNode->data)) return currentNode->data;

        lastNode=currentNode;
        currentNode=currentNode->nextblock;
    }

    /*size==0 -> no insertion*/
    if (!size) return NULL;

    /*Data not in bucket -> Allocate a new hashnode for page*/
    lastNode->nextblock=(hashnode*)malloc(sizeof(hashnode));

    lastNode->nextblock->data = malloc(size);
    memcpy(lastNode->nextblock->data, data, size);
    lastNode->nextblock->nextblock=NULL;

    return lastNode->nextblock->data;
}

void* hash_insertDupAllowed(genericHashTable Hash, void* value, void* data, unsigned int size) {

    int key;
    hashnode *currentNode,*lastNode;


    /*size==0 -> no insertion*/
    if (!size) return NULL;
    
    /*Calculating right bucket via hash_function*/
    key=(*hash_function)((char*)value)%numBuckets;

    /*In case of empty bucket*/
    if (Hash[key]==NULL) {

        /*else allocate new node and insert*/
        Hash[key]=(hashnode*)malloc(sizeof(hashnode));

        Hash[key]->data = malloc(size);
        memcpy(Hash[key]->data, data, size);
        Hash[key]->nextblock=NULL;

        return Hash[key]->data;
    }

    /*Navigate in bucket*/
    currentNode=Hash[key];
    while (currentNode!=NULL) {

        lastNode=currentNode;
        currentNode=currentNode->nextblock;
    }

    /*Data not in bucket -> Allocate a new hashnode for page*/
    lastNode->nextblock=(hashnode*)malloc(sizeof(hashnode));

    lastNode->nextblock->data = malloc(size);
    memcpy(lastNode->nextblock->data, data, size);
    lastNode->nextblock->nextblock=NULL;

    return lastNode->nextblock->data;
}

/*free given hash*/
void hash_destroy(genericHashTable Hash) {

    hashnode *temp,*current;

    for (int i=0 ; i<numBuckets ; i++) {

        current=Hash[i];
        /*free all hashnodes of bucket i*/
        while (current!=NULL) {

            temp=current;
            current=current->nextblock;
            free(temp->data);
            free(temp);
        }
    }
    /*free hash*/
    free(Hash);
    return;
}

/*Apply functionToExecute in all data nodes of hash*/
void hash_applyToAllNodes(genericHashTable currentHashTable, void *data, void (*functionToExecute) (void*, void*)) {

    hashnode *current;

    /*for every bucket*/
    for (int i=0 ; i<numBuckets ; i++) {

        current=currentHashTable[i];
        
        /*for every bucket of overflow list*/
        while (current!=NULL) {
            
            /*apply function*/
            (*functionToExecute) (current->data, data);
            current = current->nextblock;    
        }
    }
    return;
}

void hash_applyToAllNodesV2(genericHashTable currentHashTable, void *data, void *data2,\
 void *data3, void *data4, void (*functionToExecute) (void*, void*, void*, void*, void*)) {

    hashnode *current;

    /*for every bucket*/
    for (int i=0 ; i<numBuckets ; i++) {

        current=currentHashTable[i];
        
        /*for every bucket of overflow list*/
        while (current!=NULL) {
            
            /*apply function*/
            (*functionToExecute) (current->data, data, data2, data3, data4);
            current = current->nextblock;    
        }
    }
    return;
}