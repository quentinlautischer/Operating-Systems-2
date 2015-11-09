#include "clientNanny.h"

#include <unistd.h>
#include "vector.h"

static void clientNannySendDataToClerk(char* s, int lt);
static void clientNannySendDataToChild(void);

char *msgData;
char *psLine;
FILE* fpin;
extern FILE *popen();
int killedProcesses = 0;
VectorArray boredChildren;

void clientNannyFlow(void){
	msgData = (char*)malloc(1024 * sizeof(char));
	psLine = (char*)malloc(1024 * sizeof(char));
	vector_init(&boredChildren);


	sprintf(msgData,"Info: Parent process is PID %d", getpid());
	clientNannySendDataToClerk(msgData, LOGFILE);

	clerkNannyParseConfigFile(SIGHUP);
	clientNannyCheckForProcesses(SIGALRM);


	//Do work
	sprintf(psLine, "ps -C %s -o pid=", "");

	if(!(fpin = popen(psLine, "r"))){
		printf("Error: Failed on popen of %s", psLine);
	} else{
		// Do work
		fclose(fpin);	
	}
}

void clientNannyLoop(void){}

void clientNannyTeardown(void){
	vector_free(&boredChildren);
	free(msgData);
	free(psLine);
	fclose(fpin);
}

static void clientNannySendDataToClerk(char* s, int lt){
	clerkNannyReceiveData(s, lt);
}

static void clientNannySendDataToChild(void){
	return;
}

void clientNannyReceiveData(char* s){
	return;
}


void clientNannyCheckForProcesses(int signum){
	alarm(5);
	sprintf(msgData,"Checking Processes");
	clientNannySendDataToClerk(msgData, DEBUG);

	clientNannySendDataToChild();
}


