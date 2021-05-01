#include <stdio.h>
#include "request.h"

int request_compare(void *request1, void *request2) {

    return strcmp(((Request*)request1)->virusName,((Request*)request2)->virusName);
}