#include "clientNanny.h"

static void clientNannySendDataToClerk(char* s, int lt);
static void clientNannySendDataToChild(void);

char *msgData;
char *psLine;
char *childPID;
char username[256];
int killedProcesses = 0;
VectorArray boredChildren;
map_t monitoredPids; //MAP OF PID -> CHILD MON PID

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
	childPID = (char*)malloc(1024 * sizeof(char));
	vector_init(&boredChildren);
	monitoredPids = hashmap_new();
	extern FILE *popen();

	sprintf(msgData,"Info: Parent process is PID %d", getpid());
	clientNannySendDataToClerk(msgData, LOGFILE);

	//GET CURRENT USER NAME
	sprintf(psLine, "id -u -n");
	if(!(fpin = popen(psLine, "r"))){
		printf("Error: Failed on popen of %s", psLine);
	} else{
		while(fgets(psLine, 1024, fpin)){
			strtok(psLine, "\n");
			sprintf(username, psLine);
			clientNannySendDataToClerk(username, DEBUG);
		}
		fclose(fpin);	
	}

	clerkNannyParseConfigFile(SIGHUP);//Should trigger its inital Check of processes
}

void clientNannyLoop(void){}

void clientNannyTeardown(void){
	hashmap_free(monitoredPids);
	vector_free(&boredChildren);
	free(childPID);
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
		sprintf(psLine, "pgrep -u %s %s", username, curr->name); //REMEMBER TO REPLACE TO USERNAME
		clientNannySendDataToClerk(psLine, DEBUG);

		if(!(fpin = popen(psLine, "r"))){
			sprintf(msgData,"Error: Failed on popen of %s", psLine);
			clientNannySendDataToClerk(msgData, DEBUG);
		} else {
			char pidVal[150];
			fgets(pidVal, sizeof(psLine), fpin);
			clientNannySendDataToClerk(psLine, DEBUG);
			if(pidVal!=NULL){
				strtok(psLine, "\n");
				//IF PID NOT IN TABLE THEN START MONITORING
				if(hashmap_get(monitoredPids, psLine, (void**)childPID) == -3){
					clientNannySendDataToClerk("not in table", DEBUG);
				}
					//IF EMPTYCHILD AVAILABLE USE
					//ELSE CREATE A CHILD
				//ELSE ALREADY BEING MONITORED.
				sprintf(msgData, "Info: Initializing monitoring of process '%s' (PID %s).", curr->name, psLine);
				clientNannySendDataToClerk(msgData, DEBUG);					
				clientNannySendDataToChild();
		
			} else {
				clientNannySendDataToClerk(psLine, DEBUG);
				sprintf(msgData, "Info: No '%s' processes found.", curr->name);
				clientNannySendDataToClerk(msgData, DEBUG);		
			} 
			fclose(fpin);	
		}
		curr = curr->next;
	}
}


