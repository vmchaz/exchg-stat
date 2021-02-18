#include <iostream>
#include <csignal>
#include "roundbuffer.h"
#include "dataparse.h"
#include "datastore.h"
#include "network.h"



using namespace std;

RoundBufferRec r;
ConnectionRec * c;
DataStore * ds;
int save_interval = 1;


void signalHandler( int signum ) {
    if (signum == 2)
    {
        cout << "Interrupt signal received.\nExiting\n";

        deinitialize_data_store(ds);
        ssl_shutdown(c);

        exit(signum);
    }
}


int main(int argc, char const *argv[])
{
    char address[100] = "test.deribit.com";
    char filename[100] = "prices.log";


    cout << "Bids and asks from " << address << " are stored in " << filename << " each " << save_interval << " second(s)"<< std::endl << "Press Ctrl+C to exit" << std::endl;
    
    init_strings();
    ds = initialize_data_store(filename);
    if (open_file(ds) != 0)
    {
        cout << "Error opening file!";
        exit(1);
    }
        
    int lRes;
    size_t lbuffer_size;
    
    c = ssl_prepare_connection();
    if (ssl_connect(c, address) != 0)
    {
        cout << "Failed to connect" << std::endl;
        exit(1);
    }
    
    signal(SIGINT, signalHandler);
    
    char cb[10000];
    
        
    while(1)
    {
        if(ssl_send_request(c) != 0)
            exit(1);
            
        if (ssl_receive_response(c, &r) != 0)
            exit(1);
            
        char *timestamp_start, *bids_start, *asks_start;
        int timestamp_len, bids_len, asks_len;
        
        char * p2 = (char *)&r.buffer + r.resp_offset;
        
        parse_response(p2, &timestamp_start, &timestamp_len, &bids_start, &bids_len, &asks_start, &asks_len);
        prepare_buffer((char *)&cb, &lbuffer_size, (char *)&r.buffer, timestamp_start, timestamp_len, bids_start, bids_len, asks_start, asks_len, &r.ts);
        write_data(ds, &cb, lbuffer_size);
        
        
        sleep(save_interval);
    }
    
    return 0;
}
