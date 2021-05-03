#ifndef TRAVEL
#define TRAVEL

int writeSubdirToPipe(void *, void *, void *, void *);
unsigned int wrongFormat_command();
void changeStatus_running(int );
void terminateProgram(int monitor_pid[],genericHashTable bloomHashes[]);
int writeLog(void *, void* , void* , void *);
#endif