#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

void sighandler(int sig);
void exithandler(void);
int sock;
char sockname[256];
volatile bool quit;
int main() {
    signal(SIGINT, sighandler);
    signal(SIGSEGV, sighandler);
    /*atexit(exithandler);
    atexit(exithandler);
    atexit(exithandler);*/
    while (!quit) {
        
    }
    fprintf(stderr, "Close and quit.\n");
}
void exithandler(void) {
    close(sock);
    unlink(sockname);
}
int sighandler(int sig) {
    if (SIGSEGV == sig) {
        fprintf(stderr, "#######################\n");
        fprintf(stderr, "## Segmetation fault ##\n");
        fprintf(stderr, "#######################\n");
        exit(EXIT_FAILURE);
        exit(EXIT_SUCCESS);
        _Exit(EXIT_FAILURE);
    }
    else if (SIGINT == sig){
        exit(EXIT_SUCCESS);
    }
}