#ifndef BLOOM_FILTER
#define BLOOM_FILTER

/*Pointer to allocated chars forming the bloomFilter*/
typedef char* bloomFilter;

/*Initialize and return a bloomFilter*/
bloomFilter bloomFilter_create(unsigned int);

/*Insert given char* data in bloomFilter*/
void bloomFilter_insert(bloomFilter,int,char*);

/*Change the right bit of given bloomFilter - used in bloomFilter_insert*/
void bloomFilter_bitShift(bloomFilter,int,unsigned int);

/*Search char* data in bloomFilter*/
int bloomFilter_search(bloomFilter,int,char*);

/*Compute and check the right bit (0 or 1) - used in bloomFilter_search*/
int bloomFilter_bitConfirm(bloomFilter,int,unsigned int);

/*Free bloomFilter*/
void bloomFilter_delete(bloomFilter);

/*Provided hash functions*/
unsigned long djb2(unsigned char *);
unsigned long sdbm(unsigned char *);
unsigned long hash_i(unsigned char *);

#endif