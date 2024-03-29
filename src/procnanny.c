#ifndef MAIN_H 
#define MAIN_H

#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>


#include "childNanny.h"
#include "clerkNanny.h"
#include "clientNanny.h"

#include "memwatch.h"
#include "vector.h"

#endif

void teardown(void);
void signalCallbackHandler(int signum);

char *configFileName;
int killedProcesses;

char *msgData;
pid_t parent_pid;

int main(int argc, char* argv[]){

	signal(SIGINT, signalCallbackHandler);
	signal(SIGHUP, clerkNannyParseConfigFile);
	signal(SIGALRM, clientNannyCheckForProcesses);

	if ( argc == 2 ){ /* Ensure only one argument provided */
		configFileName = malloc(strlen(argv[1]) + 1);
		strcpy(configFileName, argv[1]);
	
		clerkNannySetup();
	

		clientNannyFlow(); //Main Loop

		while(1){};

	
	}

	return EXIT_SUCCESS;
}

void teardown(void){
	clerkNannyTeardown();
	clientNannyTeardown();
}

void signalCallbackHandler(int signum){
	if(getpid() == parent_pid){
		sprintf(msgData, "Info: Caught SIGINT. Exiting cleanly. %d process(es) killed.", killedProcesses);
		clerkNannyReceiveData(msgData, BOTH);	
	}
	teardown();
	exit(signum);
}




