void read_from_pipe(unsigned int, unsigned int, int, void*);
int getRecordArguments(char* , char *, FILE*);
int insertRecordToSystem(char , FILE*) ;
unsigned int wrongFormat_record(FILE*) ;
int argumentsCheck_letters(char * ) ;
int argumentsCheck_lettersNumbersDash(char *) ;
void capitalize(char* ) ;
void sendBloomThroughPipe(void *, void *);
void getSubDirName(char* , char* ) ;