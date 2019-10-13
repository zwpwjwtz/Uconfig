#ifndef UCONFIGFILE_P_H
#define UCONFIGFILE_P_H

#include "uconfigfile.h"


class UconfigFilePrivate
{
protected:
    UconfigFile* p_ptr;

public:
    UconfigEntry* rootEntry;

    const char* fileName;
    int fileType;

    UconfigFilePrivate(UconfigFile* parent);
    ~UconfigFilePrivate();

    bool copyKey(UconfigKey* dest, const UconfigKey* src);
    void deleteKey(UconfigKey* key);

    bool copyEntry(UconfigEntry* dest,
                   const UconfigEntry* src,
                   bool recursive = false);
    void deleteEntry(UconfigEntry* entry);

    UconfigEntry* searchEntryByName(UconfigEntry* parent,
                                    const char* name);
};

#endif
