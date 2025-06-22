#ifndef VIRUS
#define VIRUS

typedef struct{

    char name[11];
    void *bloomFilter;
    void *vaccinated_persons;
    void *not_vaccinated_persons;
}Virus;


/*Compare two given viruses based on their name*/
int virus_compare(void* , void* );

/*Search given vaccination Data in virus data and print the result*/
/*Used in vaccineStatus with given virus*/
void virus_searchRecordInVaccinatedType1(void *, void *);

/*Search given vaccination Data in virus data and print the result*/
/*Used in vaccineStatus without given virus*/
void virus_searchRecordInVaccinatedType2(void *, void *);

/*Initialize virus based on name given*/
void virus_initialize(Virus* , char* );

/*Free all contents of virus*/
void virus_destroy(void*, void*);

#endif