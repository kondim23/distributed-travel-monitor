#ifndef VACCINATION_DATA
#define VACCINATION_DATA
#include <time.h>
#include "record.h"

typedef struct {

    Record* record;
    time_t dateVaccinated;
}VaccData;

/*Compare two vaccData based on citizenID of indicating citizen*/
int vaccData_compare(void* , void* );

#endif