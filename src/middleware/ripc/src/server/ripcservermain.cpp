#include <stdio.h>
#include <string.h>
#include "RIPCServer.h"

int main(int argc, char* argv[])
{ 
    int debug = RIPCServer::DEFAULT_DEBUG_LEVEL;
    bool isDaemon = false;
    int port = 0;
    for (int i = 1; i < argc; i++) { 
	if (*argv[i] == '-') { 
	    if (strcmp(argv[i] + 1, "d") == 0) { 
		isDaemon = true;
	    } else if (strcmp(argv[i] + 1, "debug") == 0 && i+1 < argc) { 
		if (sscanf(argv[i+1], "%d", &debug) != 1) { 
		    printf("Integer debug level is expected\n");
		    return 1;
		}
		i += 1;
	    } else { 
		printf("Unknown option %s\n", argv[i]);
		return 1;
	    }
	} else if (port != 0) {
	    printf("Syntax: RIPCServer [-d] [-debug N] PORT\n");
	    return 1;
	} else { 
	    if (sscanf(argv[i], "%d", &port) != 1 || port <= 0) { 
		printf("Invalid port value: %s\n", argv[i]);
		return 1;
	    }
	}	
    }
    if (port == 0) { 
	printf("Syntax: RIPCServer [-d] [-debug N] PORT\n");
	return 1;
    }
    RIPCServer* server = new RIPCServer(port, debug);
    if (isDaemon) { 
	server->run();
    } else {	
	server->start();
	server->dialog();
    }
    return 0;
}

