#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <iostream>
#include <ctype.h>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <string>
#include <time.h>

#include <openssl/ssl.h>
#include "roundbuffer.h"
#include "dataparse.h"
#include "datastore.h"
#include <csignal>


typedef struct 
{
    SSL * conn;
    SSL_CTX * ssl_ctx;
    int sock;
    struct sockaddr_in client;
} ConnectionRec;

ConnectionRec * ssl_prepare_connection();
int ssl_connect(ConnectionRec * cr, char * address);
int ssl_send_request(ConnectionRec * cr);
int ssl_receive_response(ConnectionRec * cr, RoundBufferRec * rb);
void ssl_shutdown(ConnectionRec * cr);


#endif
