#ifndef CLIENTNANNY_H 
#define CLIENTNANNY_H

#include "childNanny.h"
#include "clerkNanny.h"


#include <unistd.h>
#include <signal.h>

#include "memwatch.h"
#include "vector.h"
#include "hashmap.h"

void clientNannyFlow(void);
void clientNannyReceiveData(char *s);
void clientNannyTeardown(void);
void clientNannyCheckForProcesses(int signum);
void clientNannyLoop(void);

#endif 
