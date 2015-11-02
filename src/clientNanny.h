#ifndef CLIENTNANNY_H 
#define CLIENTNANNY_H

#include "childNanny.h"
#include "clerkNanny.h"

#include "memwatch.h"

void clientNannyFlow(void);
void clientNannyReceiveData(char *s);
void clientNannyTeardown(void);
void clientNannyCheckForProcesses(int signum);

#endif 
