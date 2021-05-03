#ifndef MONITOR
#define MONITOR

void read_from_pipe(unsigned int, unsigned int, int, void*);
void write_to_pipe(unsigned int , unsigned int , int , void* );
int getRecordArguments(char* , char *, FILE*);
int insertRecordToSystem(char , FILE*) ;
unsigned int wrongFormat_record(FILE*) ;
void sendBloomThroughPipe(void *, void *);
void getSubDirName(char* , char* ) ;
int updateSystem();
void sendAllBlooms();
void changeStatus_newRecords(int);
void writeLog(int );
int printCountry(void *, void *, void *, void *);
#endif