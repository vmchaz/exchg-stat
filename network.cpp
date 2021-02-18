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


// link = test.deribit.com/api/v2/public/get_order_book?instrument_name=BTC-PERPETUAL

using namespace std;


int PORT = 443;

char cCL[100] = "Content-Length:";
uint64_t cCL1 = 0;
uint32_t cCL2 = 0;


char cRequest[1000] = "GET /api/v2/public/get_order_book?instrument_name=BTC-PERPETUAL HTTP/1.1\r\n"
    "Host: test.deribit.com\r\n"
    "Accept: application/json\r\n"
    "\r\n\r\n";
int cRequestLen = 0;


void parse_line(char * line, int len, uint32_t & content_length)
{
    uint64_t * cl_p1 = (uint64_t *)line;
    uint32_t * cl_p2 = (uint32_t *)(line+8);
    
    if ((*cl_p1 == 0x2d746e65746e6f43) && (*cl_p2 == 0x676e654c))
    {
        char * p2 = line + 15;
        while (((*p2 < '0') || (*p2 > '9')) && (*p2 != 0))
        {
            p2++;
        }
        int l = 0;
        while((*p2 >= 0x30) && (*p2 <= 0x39))
        {
            l = l*10+(*p2 - 0x30);
            p2++;
        }
        if (l > 0)
            content_length = l;
    }
}

int read_header(SSL * conn, RoundBufferRec * rb, uint32_t & content_length)
{
    char cc;
    int header_buf_ptr = 0;
    int lbn = 0;
    int cn = 0;
    uint32_t local_content_length = 0;
    cc = 0;
    char * buffer = (char *)&rb->buffer;
    
    int nRecvd = SSL_read(conn, &cc, 1);    
    if (nRecvd <= 0)
        return -1;
    uint32_t w = cc;
    buffer[header_buf_ptr++] = cc;
    
    //char * line_ptr = (char *)&buffer;
    int line_start = 0;
    int line_length = 0;
    int line_ct = 0;

    while ((w != 0x0d0a0d0a) && (nRecvd > 0) )
    {
        nRecvd = SSL_read(conn, &cc, 1);
        if (nRecvd <= 0)
            return -1;
            
        w = (w << 8) | cc;
        buffer[header_buf_ptr++] = cc;
        if (cc == 0x0A)
        {
            line_length = header_buf_ptr - line_start;
            char * line_ptr = (char *)&rb->buffer + line_start;
            parse_line(line_ptr, line_length, local_content_length);
            
            rb->header_lines[line_ct].line_start = (char *)&rb->buffer[line_start];
            rb->header_lines[line_ct].line_length = line_length;
            ++line_ct;
            line_start = header_buf_ptr;
        }
    }
    rb->header_line_count = line_ct;
    rb->resp_offset = header_buf_ptr;

    
    content_length = local_content_length;
    return 0;
}

int read_body(SSL * conn, RoundBufferRec * rb, int content_length)
{
    char * recv_buf_ptr = (char *)&rb->buffer;
    recv_buf_ptr += rb->resp_offset;
    char * recv_buf_copy = recv_buf_ptr;
    
    int nRem = content_length;
    int nToRecv = min(100, nRem);
    
    int nRecvd = SSL_read(conn, recv_buf_ptr, nToRecv);
    if (nRecvd <= 0)
        return -1;
        
    nRem -= nRecvd;
    recv_buf_ptr += nRecvd;
    
    while (nRem > 0)
    {        
        nToRecv = min(100, nRem);
                        
        nRecvd = SSL_read(conn, recv_buf_ptr, nToRecv);
        if (nRecvd <= 0)
            return -1;
                
        nRem = nRem - nRecvd;
        recv_buf_ptr += nRecvd;
    }
    
    *recv_buf_ptr = 0;
    ++recv_buf_ptr;
    
    rb->data_len = recv_buf_ptr - recv_buf_copy;
    clock_gettime(CLOCK_REALTIME, &rb->ts);
    
    return 0;
}

typedef struct 
{
    SSL * conn;
    SSL_CTX * ssl_ctx;
    int sock;
    struct sockaddr_in client;
} ConnectionRec;

ConnectionRec * ssl_prepare_connection()
{
    ConnectionRec * cr = new ConnectionRec;
    cr->conn = NULL;
    cr->sock = 0;
    memset(&cr->client, 0, sizeof(cr->client));
    return cr;
}

int ssl_connect(ConnectionRec * cr, char * address)
{
    struct hostent * host = gethostbyname(address);

    if ( (host == NULL) || (host->h_addr == NULL) ) {
        cout << "Error retrieving DNS information." << endl;
        return 1;
    }


    cr->client.sin_family = AF_INET;
    cr->client.sin_port = htons( PORT );
    memcpy(&cr->client.sin_addr, host->h_addr, host->h_length);

    cr->sock = socket(AF_INET, SOCK_STREAM, 0);

    if (cr->sock < 0) {
        cout << "Error creating socket." << endl;
        return 1;
    }
    
    SSL_load_error_strings();
    SSL_library_init();
    cr->ssl_ctx = SSL_CTX_new(SSLv23_client_method());

    if ( connect(cr->sock, (struct sockaddr *)&cr->client, sizeof(cr->client)) < 0 ) {
        close(cr->sock);
        cout << "Could not connect" << endl;
        return 1;
    }
    
    cr->conn = SSL_new(cr->ssl_ctx);
    SSL_set_fd(cr->conn, cr->sock);
    
    SSL_set_mode(cr->conn, SSL_MODE_AUTO_RETRY);
    
    if (SSL_connect(cr->conn) != 1)
        return 1;
        
    return 0;
}

int ssl_send_request(ConnectionRec * cr)
{
    if (SSL_write(cr->conn, /*request.c_str()*/ cRequest, cRequestLen) != cRequestLen) {
        cout << "Error sending request." << endl;
        return 1;
    }
    
    return 0;
}

int ssl_receive_response(ConnectionRec * cr, RoundBufferRec * rb)
{
    uint32_t content_length;
    
    if (read_header(cr->conn, rb, content_length) != 0)
        return 1;
        
    if (read_body(cr->conn, rb, content_length) != 0)
        return 1;
        
    return 0;
}

void ssl_shutdown(ConnectionRec * cr)
{
    SSL_shutdown(cr->conn);
    delete cr;
}

void init_strings()
{
    cRequestLen = strlen((char *)cRequest);
    cCL1 = *(uint64_t *)&cCL1;
    cCL2 = *(uint32_t *)&cCL1;
}
