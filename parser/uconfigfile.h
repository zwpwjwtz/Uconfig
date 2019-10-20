#ifndef UCONFIGFILE_H
#define UCONFIGFILE_H

/*
 * This class is a container class for parsed configuration.
 * When destructed, the class will try to free the resource
 * allocated to the entry list.
 */

#include "uconfigentry.h"


enum UconfigFileType
{
    Unknown = 0,
    Environment = 1,
    WinINI = 2
};


class UconfigFilePrivate;

class UconfigFile
{
protected:
    UconfigFilePrivate* d_ptr;

public:
    UconfigFile();
    ~UconfigFile();

    char* fileName();
    int configType();

    // Warning: When the newEntry is filled, existing resource
    // in the newEntry will NOT be free!
    bool getEntry(UconfigEntry* newEntry,
                  const char* entryName = NULL,
                  const char* parentName = NULL,
                  bool recursive = false);
    bool addEntry(const UconfigEntry* newEntry,
                  const char* parentName = NULL);
    bool deleteEntry(const char* entryName,
                     const char* parentName = NULL);
    bool modifyEntry(const UconfigEntry* newEntry,
                     const char* entryName,
                     const char* parentName = NULL);

    // Warning: When the newKey is filled, existing resource
    // in the newKey will NOT be free!
    bool getKey(UconfigKey* newKey,
                const char* keyName,
                const char* entryName,
                const char* parentName = NULL);
    bool addKey(UconfigKey* newKey,
                const char* entryName,
                const char* parentName = NULL);
    bool deleteKey(const char* keyName,
                   const char* entryName,
                   const char* parentName = NULL);
    bool modifyKey(UconfigKey* newKey,
                   const char* entryName,
                   const char* parentName = NULL);
};

#endif
