#include <stdio.h>
#include <stdlib.h>
#include "../include/bloomFilter.h"

static unsigned long (*hashFunctions[3]) (unsigned char*) = {&djb2 , &sdbm , &hash_i};

/*Initialize and return a bloomFilter*/
bloomFilter bloomFilter_create(unsigned int size) {

    /*Initialize size-counted chars*/
    bloomFilter currentBloomFilter = (bloomFilter) malloc(sizeof(char)*size);
    for (int i=0 ; i<size ; i++) currentBloomFilter[i]=0;
    return currentBloomFilter;
}

/*Insert given char* data in bloomFilter*/
void bloomFilter_insert(bloomFilter currentBloomFilter, int size, char* data) {

    unsigned long index;

    for (int i=0 ; i<3 ; i++) {

        /*Receive right index of bloomFilter via hashfunction*/
        index = (*hashFunctions[i])(data)%(size*8);
        bloomFilter_bitShift(currentBloomFilter , size , index );
    }
    return;
}

/*Change the right bit of given bloomFilter - used in bloomFilter_insert*/
void bloomFilter_bitShift(bloomFilter currentBloomFilter, int size, unsigned int index) {

    /*Computing the right byte in bloomFilter*/
    unsigned int targetChar=index/8;
    /*Computing the right bit of selected byte*/
    char targetBit=index%8;

    /*Right shift 1000000 correctly and perform bitwise OR*/
    char mask=128>>targetBit;
    currentBloomFilter[targetChar] |= mask;

    return;
}

/*Search char* data in bloomFilter*/
int bloomFilter_search(bloomFilter currentBloomFilter, int size, char* data) {

    unsigned long index;

    for (int i=0 ; i<3 ; i++) {

        /*Receive right index of bloomFilter via hashfunction*/
        index = (*hashFunctions[i])(data)%(size*8);
        
        if (bloomFilter_bitConfirm(currentBloomFilter , size , index )<0) {
            return 1;
        }
    }

    return 0;
}

/*Compute and check the right bit (0 or 1) - used in bloomFilter_search*/
int bloomFilter_bitConfirm(bloomFilter currentBloomFilter, int size, unsigned int index) {

    /*Computing the right byte in bloomFilter*/
    unsigned int targetChar=index/8;
    /*Computing the right bit of selected byte*/
    char targetBit=index%8;

    if (!(currentBloomFilter[targetChar]<<targetBit>>7)) return -1;

    return 0;
}

/*Free bloomFilter*/
void bloomFilter_delete(bloomFilter currentBloomFilter) {

    free(currentBloomFilter);
}

/*Provided hash functions*/
unsigned long djb2(unsigned char *str) {
	unsigned long hash = 5381;
	int c; 
	while (c = *str++) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}

unsigned long sdbm(unsigned char *str) {
	unsigned long hash = 0;
	int c;

	while (c = *str++) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

unsigned long hash_i(unsigned char *str) {
    unsigned int i=5;
	return djb2(str) + i*sdbm(str) + i*i;
}
