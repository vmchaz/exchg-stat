#ifndef _DATAPARSE_H_
#define _DATAPARSE_H_
void parse_response(char * response, char ** timestamp_start, int * timestamp_len, char ** bids_start, int * bids_len, char ** asks_start, int * asks_len);
void prepare_buffer(char * buffer, size_t * buffer_size, char * response, char * timestamp_start, int timestamp_len, char * bids_start, int bids_len, char * asks_start, int asks_len, timespec * receive_time);
#endif
