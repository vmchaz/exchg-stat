#include <sys/time.h>
#include "roundbuffer.h"


RoundBufferRec * roundbuffer_get_new_write_ptr(RoundBuffer * roundbuffer)
{
    int i = (roundbuffer->pwrite + 1) % roundbuffer->size;
    return &roundbuffer->buffer[i];
}

RoundBufferRec * roundbuffer_get_read_ptr(RoundBuffer * roundbuffer)
{
    return &roundbuffer->buffer[roundbuffer->pread];
}

void roundbuffer_increment_write_ptr(RoundBuffer * roundbuffer)
{
    roundbuffer->pwrite = (roundbuffer->pwrite + 1) % roundbuffer->size;
}

void roundbuffer_increment_read_ptr(RoundBuffer * roundbuffer)
{
    roundbuffer->pwrite = (roundbuffer->pwrite + 1) % roundbuffer->size;
}

void initialize_roundbuffer(RounBuffer * roundbuffer)
{
    memset(*roundbuffer, 0, sizeof(RoundBuffer));
    roundbuffer->size = 16;        
}
