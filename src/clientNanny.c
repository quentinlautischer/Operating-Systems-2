#include "clientNanny.h"

static void clientNannySendDataToClerk(char* s, int lt);
static void clientNannySendDataToChild(void);

char *msgData;
char *psLine;
char *childPID;
char username[256];
int killedProcesses = 0;


int fd[2];

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
			sprintf(username, "%s", psLine);
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
	//	clientNannySendDataToClerk(psLine, DEBUG);

		if(!(fpin = popen(psLine, "r"))){
			sprintf(msgData,"Error: Failed on popen of %s", psLine);
			//clientNannySendDataToClerk(msgData, DEBUG);
		} else {
			char pidVal[150];
			sprintf(pidVal, "%s", "0");	
			fgets(pidVal, sizeof(pidVal), fpin);
	//		clientNannySendDataToClerk(pidVal, DEBUG);
			if(atoi(pidVal)){
				strtok(psLine, "\n");
				//IF PID NOT IN TABLE THEN START MONITORING
				if(hashmap_get(monitoredPids, psLine, (void**)childPID) == MAP_MISSING){
					clientNannySendDataToClerk("not in table", DEBUG);
					sprintf(msgData, "%d", vector_size(&boredChildren));
					clientNannySendDataToClerk(msgData, DEBUG);
					if(vector_size(&boredChildren) == 0 ){
						//THEN FORK A NEW CHILD
						pid_t proc_pid = (pid_t) strtol(pidVal, NULL, 10);
						clientNannyForkProcMon();
					} else {
						//USE EXISTING CHILD 
						sprintf(childPID, "%d", vector_get(&vector, 0));
						// TELL CHILDPID TO MON pidVal
							
					}
				sprintf(msgData, "Info: Initializing monitoring of process '%s' (PID %s).", curr->name, psLine);
				clientNannySendDataToClerk(msgData, DEBUG);					
				clientNannySendDataToChild();
			} //ALREADY BEING MONITORED.
		
			} else {
			//	clientNannySendDataToClerk(psLine, DEBUG);
				sprintf(msgData, "Info: No '%s' processes found.", curr->name);
				clientNannySendDataToClerk(msgData, DEBUG);		
			} 
			fclose(fpin);	
		}
		curr = curr->next;
	}
}

void clientNannyForkProcMon(void){
	pid_t child_pid;
	pipe(fd);
	child_pid = fork();
	if (child_pid >= 0){ /* fork success */
		if (child_pid == 0) { /* Child */
			pid_t proc_pid;
			char* pid_name;
			char* pid_secs;

			close(fd[1]);
			// n = read(fd[0], line, MAXLINE);
			// write(STDOUT_FILENO, line, n);
			while(1){


				sleep(atoi(pid_secs));
				int killStatus = kill(proc_pid, SIGKILL);
				if (killStatus == 0) {
					//ALERT PARENT
				} else {
					//ERROR HANDLE HERE
				}
			}
		} else { /* Parent */
		}
	} else { /* Failure */
		perror("Error: Fork Error");
		exit(1);
	}
}


