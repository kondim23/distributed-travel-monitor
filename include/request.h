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

    unsigned int rejectedReq;
    unsigned int acceptedReq;
}RequestStat;

typedef struct{

    char virusName[20];
    char countryName[20];
    time_t date1;
    time_t date2;
    RequestStat statistics;
}ReqCompare;

int request_compare(void *, void *);
void statistics_compute(void *, void*);
#endif