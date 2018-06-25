#ifndef HTTPD_H
#define HTTPD_H

#include <stdio.h>
#include <string>
#include <pthread.h>
#include "parser.hpp"

using namespace std;

void DieWithError(const char *errorMessage);
void *ThreadMain(void *arg);
int SetupTCPServerSocket(unsigned short port);
int AcceptTCPConnection(int servSock);
int HandleInst(Instruction inst, int clntSocket,const string &doc_root, FILE *out, char *buf);
void HandleTCPClient(int clntSocket, const string &doc_root);
void start_httpd(unsigned short port, string doc_root);

struct ThreadArgs
{
    string doc_root;
    int clntSock;
};

#endif // HTTPD_H
