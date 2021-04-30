#include <string.h>

/*Check for correct chars in given string. Only letters allowed*/
int argumentsCheck_letters(char * string) {

    for (int i=strlen(string)-1 ; i>=0 ; i--) 
        if (!(string[i]>96 && string[i]<123) && !(string[i]>64 && string[i]<91)) 
            return 1;
    return 0;
}

/*Check for correct chars in given string. Only letters, numbers and dash allowed*/
int argumentsCheck_lettersNumbersDash(char *string) {

    for (int i=strlen(string)-1 ; i>=0 ; i--) 
        if (!(string[i]>96 && string[i]<123) && !(string[i]>64 && string[i]<91) &&\
             !(string[i]>47 && string[i]<58) && !(string[i]==45)) 
            return 1;
    return 0;
}

/*capitalize of input*/
void capitalize(char* string) {

    for(int i=strlen(string) ; i>=0 ; i--)
        if (string[i]>96 && string[i]<123)
            string[i] -= 32;
    return;
}