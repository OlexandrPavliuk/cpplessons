#include <iostream>
#include <cstring>
#include <cstdlib>
#include <getopt.h>
#include "httpserver.h"
#include <unistd.h>

int main(int argc, char **argv) {
  
    int opt;
    
    char ip[255] = "\0";
    int port;
    char directory[255] = "\0";
    
    
    while ((opt = getopt(argc, argv, "h:p:d:")) != -1) {
	switch (opt) {
	case 'h':	    
	    strcpy(ip, optarg);
	    break;
	case 'p':	    	    
	    port = atoi(optarg);	    
	    break;
	case 'd':	    
	    strcpy(directory, optarg);
	    break;
	default:  
	    return 1;
	}
    }
    
    daemon(1, 1);
    
    HttpServer server(ip, (unsigned short)port, directory);
    server.Start();
    
  
    return 0;
}
