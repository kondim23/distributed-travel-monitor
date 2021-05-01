#ifndef REQUEST
#define REQUEST

#include <time.h>

typedef struct{

    char virusName[20];
    time_t dateOfRequest;
    char countryName[20];
    char boolReq;
}Request;

typedef struct{

    char countryName[20];
    unsigned int rejectedReq;
    unsigned int acceptedReq;
}RequestStat;

int request_compare(void *, void *);

#endif