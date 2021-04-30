#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "skipList.h"

static float probability = 0.5;

/*Initializes and return a skipList*/
skipList skipList_initializeSkipList() {

    skipList currentSkipList = (skipList) malloc(sizeof(skipListHead));
    currentSkipList->countOfEntries=0;
    currentSkipList->countOfLayers=0;
    currentSkipList->firstLayer=NULL;

    return currentSkipList;
}

/*Inserts the given value to given skipList*/
int skipList_insertValue(skipList currentSkipList, void* value, unsigned int size, int (*compare)(void*,void*)) {

    skipListNode *temp_skipListNode, *promotedNode;
    char valueAlreadyExists=0;

    if (currentSkipList == NULL) return -1;

    /*If value is the first element entering currentSkipList*/
    if (currentSkipList->firstLayer == NULL) {

        /*Allocating and initializing header node of layer (having NULL value)*/
        temp_skipListNode = (skipListNode*) malloc(sizeof(skipListNode));

        currentSkipList->firstLayer = temp_skipListNode;
        
        temp_skipListNode->next = NULL;
        temp_skipListNode->lowerLayer = NULL;
        temp_skipListNode->data = NULL;

        currentSkipList->countOfLayers++;

        /*Inserting value to first layer*/
        temp_skipListNode = (skipListNode*) malloc(sizeof(skipListNode));

        currentSkipList->firstLayer->next = temp_skipListNode;

        temp_skipListNode->next = NULL;
        temp_skipListNode->lowerLayer = NULL;
        temp_skipListNode->data = malloc(size);
        memcpy(temp_skipListNode->data,value,size);

        currentSkipList->countOfEntries++;
        
        return 0;
    }

    /*If elements exist in skipList insert value recursively*/
    if ((promotedNode = skipList_insertInLayer(currentSkipList->firstLayer, value, size, compare, &valueAlreadyExists)) != NULL) {

        currentSkipList->countOfEntries++;
        /*If value inserted to all layers until top, check for new layer availability*/
        if (!skipList_newLayerAvailability(currentSkipList)) {

            /*If yes insert new layer*/
            if (skipList_addLayer(currentSkipList)<0) {

                printf("Error: Could not create new layer\n");
                return -1;
            }
        }
    }
    else if (valueAlreadyExists) {
        
        valueAlreadyExists=0;
        return 1;
    }
    else currentSkipList->countOfEntries++;
    
    return 0;
}

/*Recursively inserts value through layers - used by skipList_insertValue()*/
skipListNode* skipList_insertInLayer(skipListNode* currentSkipListNode, void* value, unsigned int size,  int (*compare)(void*,void*), char* valueAlreadyExists) {

    skipListNode *temp_skipListNodePtr, *nodeToPromote;

    /*Reach the right position in current layer*/
    while (currentSkipListNode->next != NULL && compare(currentSkipListNode->next->data,value) < 0) 
        currentSkipListNode = currentSkipListNode->next;

    /*If the value allready exists*/
    if (currentSkipListNode->next != NULL && compare(currentSkipListNode->next->data,value) == 0) {

        *valueAlreadyExists = 1;
        return NULL;
    }

    /*If we are on lowest layer connect the value to layer-list*/
    if (currentSkipListNode->lowerLayer == NULL) {

        temp_skipListNodePtr = (skipListNode*) malloc(sizeof(skipListNode));

        temp_skipListNodePtr->lowerLayer = NULL;
        temp_skipListNodePtr->data = malloc(size);
        memcpy(temp_skipListNodePtr->data,value,size);

        temp_skipListNodePtr->next = currentSkipListNode->next;
        currentSkipListNode->next = temp_skipListNodePtr;

        return temp_skipListNodePtr;
    }

    /*Get to the lowest layer and when returning check probability for insertion to this layer*/
    if ((nodeToPromote = skipList_insertInLayer(currentSkipListNode->lowerLayer, value, size, compare, valueAlreadyExists)) != NULL &&\
        rand()%10 <= probability*10) {

        /*We insert value to this layer*/
        temp_skipListNodePtr = (skipListNode*) malloc(sizeof(skipListNode));

        temp_skipListNodePtr->lowerLayer = nodeToPromote;
        temp_skipListNodePtr->data = nodeToPromote->data;

        temp_skipListNodePtr->next = currentSkipListNode->next;
        currentSkipListNode->next = temp_skipListNodePtr;

        return temp_skipListNodePtr;
    }
    else return NULL;
}

/*Checks for new layer availability*/
char skipList_newLayerAvailability(skipList currentSkipList) {

    /*base of logarithm*/
    unsigned int base = 1.0/probability, entries = currentSkipList->countOfEntries, layersAllowed = 0;

    /*Compute log*/
    while (entries != 0) {

        entries /= base;
        layersAllowed++;
    }

    /*If num layers < log(elements)*/
    if (layersAllowed <= currentSkipList->countOfLayers) return 1;
    else return 0;
}

/*Adds a new layer in skipList*/
int skipList_addLayer(skipList currentSkipList) {

    skipListNode *providedSkipListPtr, *newLayerPtr, *lastNodeOfNewLayer;
    char firstNodeOfList = 1;

    if (currentSkipList == NULL || currentSkipList->firstLayer == NULL) {

        return -1;
    }

    providedSkipListPtr = currentSkipList->firstLayer;

    /*for all elements of current skipList upper layer*/
    while (providedSkipListPtr->next!=NULL) {

        /*Insert head node*/
        if (firstNodeOfList) {

            newLayerPtr = (skipListNode*) malloc(sizeof(skipListNode));
            newLayerPtr->data = providedSkipListPtr->data;
            newLayerPtr->lowerLayer = providedSkipListPtr;
            newLayerPtr->next = NULL;

            currentSkipList->firstLayer = newLayerPtr;
            lastNodeOfNewLayer = newLayerPtr;
            firstNodeOfList = 0;
        }
        else if(rand()%10 <= probability*10) {

            /*If lucky insert this element*/
            newLayerPtr = (skipListNode*) malloc(sizeof(skipListNode));
            newLayerPtr->data = providedSkipListPtr->data;
            newLayerPtr->lowerLayer = providedSkipListPtr;
            newLayerPtr->next = NULL;

            lastNodeOfNewLayer->next = newLayerPtr;
            lastNodeOfNewLayer = lastNodeOfNewLayer->next;
        }

        providedSkipListPtr = providedSkipListPtr->next;
    }

    currentSkipList->countOfLayers++;

    return 0;
}

/*search given value in given skipList*/
int skipList_searchValue(skipList currentSkipList, void* value, int (*compare)(void*,void*)) {

    skipListNode* currentSkipListNodePtr;

    if (currentSkipList == NULL) {

        return -1;
    }

    currentSkipListNodePtr = currentSkipList->firstLayer;

    while (currentSkipListNodePtr != NULL) {

        /*Reach the right position of this layer*/
        while (currentSkipListNodePtr->next != NULL && compare(currentSkipListNodePtr->next->data,value) < 0)
            currentSkipListNodePtr = currentSkipListNodePtr->next;

        /*Values match*/
        if (currentSkipListNodePtr->next != NULL && compare(currentSkipListNodePtr->next->data,value) == 0) {

            return 0;
        }
        /*Head to lower layer*/
        currentSkipListNodePtr = currentSkipListNodePtr->lowerLayer;
    }

    return 1;
}

/*search given value in given skipList*/
void* skipList_searchReturnValue(skipList currentSkipList, void* value, int (*compare)(void*,void*)) {

    skipListNode* currentSkipListNodePtr;

    if (currentSkipList == NULL) {

        return NULL;
    }

    currentSkipListNodePtr = currentSkipList->firstLayer;

    while (currentSkipListNodePtr != NULL) {

        /*Reach the right position of this layer*/
        while (currentSkipListNodePtr->next != NULL && compare(currentSkipListNodePtr->next->data,value) < 0)
            currentSkipListNodePtr = currentSkipListNodePtr->next;

        /*Values match*/
        if (currentSkipListNodePtr->next != NULL && compare(currentSkipListNodePtr->next->data,value) == 0) {

            return currentSkipListNodePtr->next->data;
        }
        /*Head to lower layer*/
        currentSkipListNodePtr = currentSkipListNodePtr->lowerLayer;
    }

    return NULL;
}

/*remove given value from given skipList*/
int skipList_removeValue(skipList currentSkipList, void* value, int (*compare)(void*,void*)) {

    skipListNode* currentSkipListNodePtr;

    if (currentSkipList == NULL) {

        return -1;
    }

    currentSkipListNodePtr = currentSkipList->firstLayer;

    while (currentSkipListNodePtr != NULL) {

        /*Reach the right position of this layer*/
        while (currentSkipListNodePtr->next != NULL && compare(currentSkipListNodePtr->next->data,value) < 0)
            currentSkipListNodePtr = currentSkipListNodePtr->next;

        /*Values match*/
        if (currentSkipListNodePtr->next != NULL && compare(currentSkipListNodePtr->next->data,value) == 0) {

            /*Remove value*/
            if (skipList_removeValueFromLayer(currentSkipListNodePtr, currentSkipListNodePtr->next)<0) {

                printf("Error: removal from layer\n");
                return -1;
            }
            currentSkipList->countOfEntries--;
            
            /*If the upper level is empty free it*/
            if (currentSkipListNodePtr == currentSkipList->firstLayer && currentSkipListNodePtr->next == NULL) {

                currentSkipList->firstLayer = currentSkipListNodePtr->lowerLayer;
                free (currentSkipListNodePtr);
                currentSkipList->countOfLayers--;
            }

            return 0;
        }
        currentSkipListNodePtr = currentSkipListNodePtr->lowerLayer;
    }

    return 1;
}

/*Recursively remove value from all layers - used by skipList_removeValue()*/
int skipList_removeValueFromLayer(skipListNode *nodePrecedingTarget, skipListNode *nodeTodelete) {

    if (nodePrecedingTarget == NULL || nodeTodelete == NULL) return 0;

    /*Reach the right position of this layer*/
    while (nodePrecedingTarget->next != nodeTodelete) {
        
        nodePrecedingTarget = nodePrecedingTarget->next;
        if (nodePrecedingTarget == NULL) return -1;    
    }

    /*Reach lower level*/
    if (skipList_removeValueFromLayer(nodePrecedingTarget->lowerLayer, nodeTodelete->lowerLayer)<0)
        return -1;

    /*Exiting recursion after deleting element from this layer*/
    nodePrecedingTarget->next = nodeTodelete->next;
    if (nodeTodelete->lowerLayer == NULL) free(nodeTodelete->data);
    free(nodeTodelete);

    return 0;
}

/*apply funcToApply to all elements of skipList*/
void skipList_applyToAll(skipList currentSkipList, void* data1, void* data2, void* data3, int (*funcToApply)(void*,void*,void*,void*)) {

    skipListNode* currentSkipListNodePtr;

    if (currentSkipList == NULL) return;

    currentSkipListNodePtr = currentSkipList->firstLayer;

    /*Head to lowest layer*/
    while (currentSkipListNodePtr->lowerLayer != NULL) currentSkipListNodePtr = currentSkipListNodePtr->lowerLayer;
    currentSkipListNodePtr = currentSkipListNodePtr->next;

    /*funcToApply to all Nodes*/
    while (currentSkipListNodePtr != NULL) {

        funcToApply(currentSkipListNodePtr->data, data1, data2, data3);
        currentSkipListNodePtr = currentSkipListNodePtr->next;
    }
    return;
}

/*free skipList*/
void skipList_destroy(skipList currentSkipList) {

    skipListNode* currentSkipListNodePtr, *temp, *temp2;

    if (currentSkipList == NULL) return;

    currentSkipListNodePtr = currentSkipList->firstLayer;

    /*For all lists*/
    while (currentSkipListNodePtr != NULL) {

        temp = currentSkipListNodePtr;
        currentSkipListNodePtr = currentSkipListNodePtr->lowerLayer;

        /*free every element of list*/
        while (temp != NULL) {

            temp2 = temp;
            temp = temp->next;
            if (temp2->lowerLayer == NULL) free(temp2->data);
            free(temp2);
        }
    }

    free(currentSkipList);
    return;
}