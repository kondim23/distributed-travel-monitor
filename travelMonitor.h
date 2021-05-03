#ifndef TRAVEL
#define TRAVEL

int writeSubdirToPipe(void *, void *, void *, void *);
unsigned int wrongFormat_command();
void changeStatus_running(int );
void terminateProgram(int monitor_pid[],genericHashTable bloomHashes[]);
int writeLog(void *, void* , void* , void *);
void recreateChild(int );
int assignCountryToNewChild(void *, void *, void *, void *);
void create_pipes_and_monitors(int );
void receive_bloom_filters(int );
#endif