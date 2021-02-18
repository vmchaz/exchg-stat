#ifndef _ROUNDBUFFER_H_
#define _ROUNDBUFFER_H_
#include <sys/time.h>

typedef struct {
    char * line_start;
    int line_length;
} HeaderLineIdx;

typedef struct {
    char buffer[10000];
    HeaderLineIdx header_lines[32];    
    int header_line_count;
    int data_len;
    int resp_offset;
    timespec ts;
} RoundBufferRec;

typedef struct {
    RoundBufferRec buffer[16];
    int size;
    int pread;
    int pwrite;
} RoundBuffer;

#endif
