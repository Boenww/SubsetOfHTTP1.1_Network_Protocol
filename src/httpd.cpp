#include <iostream>
#include "httpd.hpp"
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <stdlib.h>     /* for atoi() and exit() */
//#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

using namespace std;

#define MAXPENDING 5

static const unsigned int TIMEOUT_SECS = 5;

int SetupTCPServerSocket(unsigned short port) {
    
    int servSock;
    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    
    // Construct the server address structure
    struct sockaddr_in servAddr; // Criteria for address match
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;            /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(port);              /* Local port */
    
    /* Bind to the local address */
    if (::bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");
    
    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");
    
     return servSock;
}

int AcceptTCPConnection(int servSock) {
    int clntSock;
    struct sockaddr_in clntAddr; // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);
    
    // Wait for a client to connect
    if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen)) < 0)
        DieWithError("accept() failed");
    
    // clntSock is connected to a client!
    printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));
    return clntSock;
}

void *ThreadMain(void *threadArgs) {
    pthread_detach(pthread_self()); //guarantees that thread resources are deallocated upon return
    int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    string doc_root = ((struct ThreadArgs *) threadArgs)->doc_root;
    free(threadArgs); //deallocate memory for argument
    
    HandleTCPClient(clntSock, doc_root);
    return (NULL);
}

void start_httpd(unsigned short port, string doc_root)
{
    cerr << "Starting server (port: " << port <<
    ", doc_root: " << doc_root << ")" << endl;
    
    int servSock = SetupTCPServerSocket(port);                    /* Socket descriptor for server */
    if (servSock < 0)
        DieWithError("SetupTCPServerSocket() failed");
    
    /* set socket timeout */
    struct timeval timeout;
    timeout.tv_sec  = TIMEOUT_SECS;
    timeout.tv_usec = 0;
    if (setsockopt(servSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        DieWithError("setsockopt() failed");
    if (setsockopt(servSock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        DieWithError("setsockopt() failed");
    
    for (;;) /* Run forever */
    {
        int clntSock = AcceptTCPConnection(servSock);
        
        struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs)); //create memory for client argument
        if (threadArgs == NULL) {
            DieWithError("malloc() failed");
        }
        threadArgs->clntSock = clntSock;
        threadArgs->doc_root = doc_root;
        pthread_t threadID; //create client thread
        int returnValue = pthread_create(&threadID, NULL, ThreadMain, threadArgs);
        if (returnValue != 0) {
            DieWithError("pthread_create() failed");
            fputc(returnValue, stderr);
        }
        printf("with thread %lu\n", (unsigned long int) threadID);
        
    }
    /* NOT REACHED */
}

