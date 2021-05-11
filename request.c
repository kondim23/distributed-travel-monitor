#include <stdio.h>
#include <string.h>
#include "request.h"

/*Comparing two requests based on virus name*/
int request_compare(void *request1, void *request2) {

    return strcmp(((Request*)request1)->virusName,((Request*)request2)->virusName);
}

void statistics_compute(void *vrequest, void *vstat) {

    Request *request = (Request*) vrequest;
    ReqCompare *stat = (ReqCompare*) vstat;

    /*Accepting empty or proper country*/
    if (strcmp(stat->countryName,"") && strcmp(stat->countryName,request->countryName)) return;

    /*Checking virus and vaccination date*/
    if (!strcmp(stat->virusName,request->virusName) && request->dateOfRequest>=stat->date1 &&\
    request->dateOfRequest<=stat->date2) {

        if (request->boolReq==0) stat->statistics.acceptedReq++;
        else stat->statistics.rejectedReq++;
    }

    return;
}