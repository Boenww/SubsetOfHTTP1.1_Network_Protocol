#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <iostream>
#include <assert.h>
#include "httpd.h"
#include "framer.hpp"
#include "parser.hpp"
#include "framer.cpp"
#include "parser.cpp"
#include "HandleInst.cpp"

using namespace std;

static const int MAX_WIRE_SIZE = 1024;

/* Read up to bufSize bytes or until delimiter, copying into the given
 * buffer as we go.
 * Returns the number of bytes placed in buf (delimiter NOT transferred).
 * If stream ends, -1 is returned.
 */

int GetMsg(FILE *in, char *buf, int bufSize){
    int count = 0;
    char nextChar;
    while(count < bufSize)
    {
        nextChar = getc(in);
        if (nextChar == EOF)
        {
            buf[count] = '\0';
            return -1;
        }
        else if (nextChar == '\n')
        {
            buf[count] = '\n';
            buf[count+1] = '\0';
            count += 2;
            return count;
        } else {
            buf[count] = nextChar;
            count++;
        }
    }
    buf[count] = '\0';
    count++;
    return count;
}

void HandleTCPClient(int clntSocket, const string &doc_root)
{
    char InBuf[MAX_WIRE_SIZE];
    char OutBuf[MAX_WIRE_SIZE];
    
    Framer framer;
    Parser parser;
    
    FILE *InChannel  = fdopen(clntSocket, "r+"); //File streams can abstract network connections(TCP sockets)
    FILE *OutChannel = fdopen(clntSocket, "w");
    if (InChannel == NULL || OutChannel == NULL)
        DieWithError("fdopen() failed");
    
    int handlePro = 0;
    
    while(GetMsg(InChannel, InBuf, MAX_WIRE_SIZE) > 0 && handlePro == 0)
    {
        string data(InBuf);
        framer.append(data);
        
        while (framer.hasMessage())
        {
            string m = framer.topMessage();
            framer.popMessage();
            Instruction inst = parser.parse(m);
            
            handlePro = HandleInst(inst, clntSocket, doc_root, OutChannel, OutBuf);
            
        }
    }
    fclose(InChannel);
    fclose(OutChannel);
    close(clntSocket);
}


