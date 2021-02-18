#ifndef _DATASTORE_H_
#define _DATASTORE_H_


typedef struct {
    FILE * pFile;
    char filename[1000];
} DataStore;


DataStore * initialize_data_store(const char * filename);
int open_file(DataStore * ds);
int write_data(DataStore * ds, void * data, size_t datasize);
void deinitialize_data_store(DataStore * ds);

#endif
