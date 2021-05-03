#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void myfunc();
void func(int signo);

int main(){

	static struct sigaction act;

	act.sa_handler=func;
    sigfillset(&(act.sa_mask));

	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGINT, &act, NULL);

    int pid = fork();

    sleep(2);

    if (pid != 0) kill(pid,SIGUSR1);

    myfunc();

}

void func(int signo) {

    printf("exiting\n");
    exit(0);
}

void myfunc() {

    sleep(20);
}