#include <iostream>
#include <ctype.h>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <string>
#include <time.h>

#include <chrono>
#include "roundbuffer.h"

using namespace std;
    
uint64_t cKW[3] = {0x2262696473223a, 0x2261736b73223a, 0x657374616d70223a};
uint64_t cKWMask[3] = {0xffffffffffffff, 0xffffffffffffff, 0xffffffffffffffff};


void read_array(char * buffer, char ** ppbuf, int * start, int * length)
{
    int lv = 0;
    char * p = *ppbuf;
    char c = *p;
    
    
    while (c != 0)
    {
        if (c == '[')
        {
            if(lv == 0)
            {
                *start = p - buffer;                
            }
            ++lv;
        } else 
        if (c == ']')
        {
            --lv;
            if(lv==0)
            {
                *length = p - buffer - *start;
                *ppbuf = p;
                return;
            }
        }

        ++p;
        c = *p;        
    }
}

void fast_itoa_zfill(int v, int d_count, char * buffer)
{
    int d;
    char * p = buffer + d_count - 1;
    for(int i = 0; i < d_count; i++)
    {
        d = v % 10;
        v = v / 10;        
        *p = 0x30+d;
        p--;
    }
}

void timestamp_to_buffer(char * buffer, timespec * ts)
{
    buffer[10] = '.';
    fast_itoa_zfill(ts->tv_sec, 10, buffer);
    fast_itoa_zfill(ts->tv_nsec / 1000, 6, buffer+11);
}

void read_int(char * buffer, char ** ppbuf, int * start, int * length)
{
    char *p = *ppbuf;
    char c = *p;
    while ((c != 0) && ((c < 0x30) || (c > 0x39)))
    {
        ++p;
        c = *p;
    }
    *start = p - buffer;
    while ((c >= 0x30) && (c <= 0x39))
    {
        ++p;
        c = *p;
    }
    *length = p - buffer - *start;
    *ppbuf = p;
}

void parse_response(char * response, char ** timestamp_start, int * timestamp_len, char ** bids_start, int * bids_len, char ** asks_start, int * asks_len)
{
    char * p = response;    
    char c = *p;
    uint64_t w = 0;
    int l_start = 0;
    int l_end = 0;
    int l_len = 0;
    while(c != 0)
    {
        w = ((w << 8) | c) & 0xffffffffffffff;
        if (w == 0x2262696473223a)
        {
            read_array(response, &p, &l_start, &l_len);
            *bids_start = response + l_start;
            *bids_len = l_len;
        } else 
        if (w == 0x2261736b73223a)
        {
            read_array(response, &p, &l_start, &l_len);
            *asks_start = response + l_start;
            *asks_len = l_len;
        }
        else if (w == 0x7374616d70223a)
        {            
            read_int(response, &p, &l_start, &l_len);
            *timestamp_start = response + l_start;
            *timestamp_len = l_len;
        }
        ++p;
        c = *p;
    }
}

char cBids[10] = " bids:";
char cAsks[10] = " asks:";
char cTimestamp[20] = "timestamp:";
char cTimestamp1[20] = " timestamp1:";
char cTimestamp2[20] = " timestamp2:";


void prepare_buffer(char * buffer, size_t * buffer_len, char * response, char * timestamp_start, int timestamp_len, char * bids_start, int bids_len, char * asks_start, int asks_len, timespec * receive_time)
{
    char * p = buffer;
    
    memcpy(p, cTimestamp, 10);
    p += 10;
    memcpy(p, timestamp_start, timestamp_len);
    p += timestamp_len;
    
    memcpy(p, cBids, 6);
    p += 6;
    memcpy(p, bids_start, bids_len);
    
    p += bids_len;
    memcpy(p, cAsks, 6);
    p += 6;
    memcpy(p, asks_start, asks_len);
    p += asks_len;
    
    memcpy(p, cTimestamp1, 12);
    p += 12;
    timestamp_to_buffer(p, receive_time);
    p+= 17;
    
    timespec ts2;
    clock_gettime(CLOCK_REALTIME, &ts2);
    memcpy(p, cTimestamp2, 12);
    p += 12;
    timestamp_to_buffer(p, &ts2);
    p+= 17;
    
    *(uint16_t *)p = 0x000a;
    p+=1;
    *buffer_len = p - buffer;
}

