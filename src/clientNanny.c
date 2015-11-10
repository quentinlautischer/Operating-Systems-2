#include "clientNanny.h"

static void clientNannySendDataToClerk(char* s, int lt);
static void clientNannySendDataToChild(void);

char *msgData;
char *psLine;
char username[256];
int killedProcesses = 0;
VectorArray boredChildren;

FILE* fpin;

struct ConfigExtractedData {
	char name[1024];
	char secs[1024];
	struct ConfigExtractedData *next;
};

typedef struct ConfigExtractedData item;

item * head;
item * curr;

void clientNannyFlow(void){
	msgData = (char*)malloc(1024 * sizeof(char));
	psLine = (char*)malloc(1024 * sizeof(char));
	vector_init(&boredChildren);

	sprintf(msgData,"Info: Parent process is PID %d", getpid());
	clientNannySendDataToClerk(msgData, LOGFILE);

	//GET CURRENT USER NAME
	// sprintf(psLine, "id -u -n");
	// if(!(fpin = popen(psLine, "r"))){
	// 	printf("Error: Failed on popen of %s", psLine);
	// } else{
	// 	while(fgets(psLine, 1024, fpin)){
	// 		strtok(psLine, "\n");
	// 		sprintf(username, psLine);
	// 		clientNannySendDataToClerk(username, DEBUG);
	// 	}
	// 	fclose(fpin);	
	// }

	clerkNannyParseConfigFile(SIGHUP);//Should trigger its inital Check of processes
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

	curr = head;
	while(curr){
		sprintf(psLine, "pgrep -u %s %s", "root", curr->name);
		clientNannySendDataToClerk(psLine, DEBUG);

		FILE* fpin;
		extern FILE *popen();


		if(!(fpin = popen(psLine, "r"))){
			sprintf(msgData,"Error: Failed on popen of %s", psLine);
			clientNannySendDataToClerk(msgData, DEBUG);
		} else{
			if(fpin!=NULL){
				while(fgets(psLine, 1024, fpin)!=NULL){
				
					strtok(psLine, "\n");
					sprintf(msgData, "Info: Initializing monitoring of process '%s' (PID %s).", curr->name, psLine);
					clientNannySendDataToClerk(msgData, DEBUG);					
					// pid_t proc_pid = (pid_t) strtol(psLine, NULL, 10);
				

					// forkProcMon(proc_pid, line);	
					clientNannySendDataToChild();
				}
				
			} else {
				sprintf(msgData, "Info: No '%s' processes found.", curr->name);
				clientNannySendDataToClerk(msgData, DEBUG);		
			} 
				
			
		}
		curr = curr->next;
		}
	fclose(fpin);	
}


