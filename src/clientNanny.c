#include "clientNanny.h"

static void clientNannySendDataToClerk(char* s, int lt);
static void clientNannySendDataToChild(void);

char *msgData;
char *psLine;
char *childPID;
char username[256];
int killedProcesses = 0;
char pidVal[150];

char line[MAXLINE];
int fd = 0;
char *myfifo;

FILE* fpin;

struct ConfigExtractedData {
	char name[1024];
	char secs[1024];
	struct ConfigExtractedData *next;
};

typedef struct ConfigExtractedData item;

item * head;
item * curr;

BoredChild *boredChild_head;
MonChild *monChild_head;


void clientNannyFlow(void){
	msgData = (char*)malloc(1024 * sizeof(char));
	psLine = (char*)malloc(1024 * sizeof(char));
	childPID = (char*)malloc(1024 * sizeof(char));
	boredChild_head = NULL;
	monChild_head = NULL;

	myfifo = (char*)malloc(1024 * sizeof(char));


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
			// clientNannySendDataToClerk(username, DEBUG);
		}
		fclose(fpin);	
	}

	clerkNannyParseConfigFile(SIGHUP);//Should trigger its inital Check of processes
}

void clientNannyLoop(void){}

void clientNannyTeardown(void){
	monLL_clear(&monChild_head);
	stack_clear(&boredChild_head);//CLEAN BORED CHILDREN
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
	int whileFixFlag = 0;

	curr = head;
	while(curr){
		sprintf(psLine, "pgrep -u %s %s", username, curr->name); //REMEMBER TO REPLACE TO USERNAME
		if(!(fpin = popen(psLine, "r"))){
			sprintf(msgData,"Error: Failed on popen of %s", psLine);
		} else {
			sprintf(pidVal, "%s", "0");
			whileFixFlag = 0;	
			while(fgets(pidVal, sizeof(pidVal), fpin)){
				whileFixFlag = 1;
				if(atoi(pidVal)){
					strtok(pidVal, "\n");
			
					clientNannySendDataToChild();
					//IF PID NOT IN TABLE THEN START MONITORING
					if(monLL_elem(&monChild_head, (pid_t) atoi(pidVal)) == 0){
						// clientNannySendDataToClerk("DEBUG: PID not in table.", DEBUG);
						// sprintf(msgData, "%d", stack_len(boredChild_head));
						sprintf(msgData, "Info: Initializing monitoring of process '%s' (PID %s).", curr->name, pidVal);
						clientNannySendDataToClerk(msgData, DEBUG);		
						if(stack_len(boredChild_head) == 0 ){
							//THEN FORK A NEW CHILD
							clientNannySendDataToClerk("FORKING A NEW CHILD", DEBUG);
							clientNannyForkProcMon(); //LALA BIG FORK
						} else {
							clientNannySendDataToClerk("USING AN EXISTING CHILD", DEBUG);
							//USE EXISTING CHILD 
							int child_pid = stack_pop(&boredChild_head);
							
							// char line[MAXLINE];
							// int fd;
						 //    char * myfifo;

							sprintf(myfifo, "%d", child_pid);
							mkfifo(myfifo, 0666);

							fd = open(myfifo, O_WRONLY);
							sprintf(msgData, "%s %s %s %d\n", curr->name, curr->secs, pidVal, child_pid);
							write(fd, msgData, (strlen(msgData)+1));
							close(fd);
							// TELL CHILDPID TO MON pidVal
						}
					} //ALREADY BEING MONITORED. DO NOTHING.
				}
			}
			if (!whileFixFlag){
				//clientNannySendDataToClerk(psLine, DEBUG);
				sprintf(msgData, "Info: No '%s' processes found.", curr->name);
				clientNannySendDataToClerk(msgData, DEBUG);		
			} 
			fclose(fpin);	
			curr = curr->next;
		}
	}
	MonChild * iterCurr = malloc(sizeof(MonChild));
	MonChild * tmp = malloc(sizeof(MonChild));
	iterCurr = monChild_head;
	while(iterCurr){
		tmp = iterCurr;
		iterCurr = iterCurr->next;
		sprintf(msgData, "ITERATING %d",tmp->childPID);
		clientNannySendDataToClerk(msgData, DEBUG);

		// if (tmp->monPID > 0){
		// 	sprintf(psLine, "ps -u %s | grep %d", username, tmp->monPID); //REMEMBER TO REPLACE TO USERNAME

		// 	if(!(fpin = popen(psLine, "r"))){
		// 		sprintf(msgData,"Error: Failed on popen of %s", psLine);
		// 		clientNannySendDataToClerk(msgData, DEBUG);
		// 	} else {
		// 		if(fgets(msgData, sizeof(msgData), fpin)){
		// 			clientNannySendDataToClerk("ITS STILL RUNNING", DEBUG);
		// 		} else {
		// 			clientNannySendDataToClerk("ITS NOT RUNNING", DEBUG);
		// 			stack_push(&boredChild_head, tmp->childPID);
		// 			monChild_head =  monLL_remove(monChild_head, tmp->childPID);
		// 			killedProcesses++;
		// 		}
		// 	}
		// }

		// pipe(&tmp->c2p_pipe);

		sprintf(myfifo, "%d", tmp->childPID);

		/* open, read, and display the message from the FIFO */
		fd = open(myfifo, O_RDONLY);
		read(fd, line, MAXLINE);
		strtok(line, "\n");
		sprintf(msgData, "PARENT READS DATA: %s", line);
		clientNannySendDataToClerk(msgData, DEBUG);
		if (atoi(line) <=0 ){
			// Did not kill but is gone.
		} else {
			// did kill
			killedProcesses++;
		}
		stack_push(&boredChild_head, tmp->childPID);
		monChild_head = monLL_remove(monChild_head, tmp->childPID);
		close(fd);
	}
	free(tmp);
	free(iterCurr);

}

void clientNannyForkProcMon(void){
	pid_t child_pid;
	// char line[MAXLINE];
	// int fd;
 //    char * myfifo;

	child_pid = fork();
	if (child_pid >= 0){ /* fork success */
		if (child_pid == 0) { /* Child */
			//pid_t proc_pid = (pid_t) strtol(pidVal, NULL, 10);
			pid_t proc_pid;
			char* pid_name;
			char* pid_secs;
			char* myPid;
			sprintf(msgData, "I AM NEW CHILD %d", child_pid);
			clientNannySendDataToClerk(msgData, DEBUG);

			// close(myPipes[1]);
			while(1){
				clientNannySendDataToClerk("GOT HERE", DEBUG);
				sprintf(myfifo, "%d", getpid());
				clientNannySendDataToClerk("GOT HERE2", DEBUG);
    			/* open, read, and display the message from the FIFO */
    			fd = open(myfifo, O_RDONLY);
    			read(fd, line, MAXLINE);
    			strtok(line, "\n");
    			sprintf(msgData, "CHILD READS DATA: %s", line);
				clientNannySendDataToClerk(msgData, DEBUG);	
    			close(fd);

				char seps[] = " ";
				char *token = strtok( line, seps );

			    if( token != NULL ){
			    	pid_name = token;
			    }
			    token = strtok( NULL, seps );
			    if( token != NULL ){
			    	pid_secs = token;
			    }
			    token = strtok( NULL, seps );
			    if( token != NULL ){
			    	proc_pid = (pid_t) strtol(token, NULL, 10);
			    }
			    token = strtok( NULL, seps );
			    if( token != NULL ){
			    	myPid = token;
			    }

				sprintf(msgData, "NOW KNOWS: %s %s %d %s", pid_name, pid_secs, proc_pid, myPid);
				clientNannySendDataToClerk(msgData, DEBUG);	

				if(proc_pid == -1){
					exit(0);
				}

				sleep(atoi(pid_secs));
				int killStatus = kill(proc_pid, SIGKILL);

				if (killStatus == 0) {
					clientNannySendDataToClerk("KILLED PROCESS", DEBUG);
					sprintf(msgData, "%s\n", pidVal);
				} else {
					clientNannySendDataToClerk("ERROR KILLING PROCESS", DEBUG);
					sprintf(msgData, "%s\n", "-1");
				}
				sprintf(myfifo, "%s", myPid);
				mkfifo(myfifo, 0666);
				fd = open(myfifo, O_WRONLY);
				write(fd, msgData, (strlen(msgData)+1));
				close(fd);
			}

			clerkNannyTeardown();
			clientNannyTeardown();
			exit(1);

		} else { /* Parent */
			sprintf(myfifo, "%d", child_pid);
			mkfifo(myfifo, 0666);

			fd = open(myfifo, O_WRONLY);
			sprintf(msgData, "%s %s %s %d\n", curr->name, curr->secs, pidVal, child_pid);
			write(fd, msgData, (strlen(msgData)+1));
			close(fd);
			
			monLL_push(&monChild_head, (pid_t) atoi(pidVal), child_pid);
			// TELL CHILDPID TO MON pidVal
		}
	} else { /* Failure */
		perror("Error: Fork Error");
		exit(1);
	}
}


