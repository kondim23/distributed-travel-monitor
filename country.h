#ifndef COUNTRY_H
#define COUNTRY_H

typedef struct{

    char name[11];
}Country;

/*Return name of country*/
char* country_getName(Country *);

/*Compare two countries based on their name*/
int country_compare(void* , void* );

#endif