#include <stdio.h>
#include <cstring>

#include "datastore.h"

DataStore * initialize_data_store(const char * filename)
{
    DataStore * ds = new DataStore;
    strcpy((char *)&ds->filename, filename);
    return ds;
}

int open_file(DataStore * ds)
{
    ds->pFile = fopen (ds->filename , "w");
    if(ferror(ds->pFile))
        return 1;
        
    return 0;    
    //ds->filestream.open(filename);
}

int write_data(DataStore * ds, void * data, size_t datasize)
{
    fwrite(data, 1, datasize, ds->pFile);
    if(ferror(ds->pFile))
        return 1;
        
    return 0;
    //ds->filestream << data;
}

void deinitialize_data_store(DataStore * ds)
{
    fclose(ds->pFile);
    //df->filestream.close();
    delete ds;
}
