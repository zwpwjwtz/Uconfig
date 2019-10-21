#ifndef UCONFIGFILE_H
#define UCONFIGFILE_H

/*
 * This class is a container class for parsed configuration.
 * When destructed, the class will try to free the resource
 * allocated to the entry list.
 */

#include "uconfigentryobject.h"


class UconfigFile
{
public:
    UconfigFile();
    UconfigFile(const UconfigFile& file);
    ~UconfigFile();

    UconfigEntryObject metadata;
    UconfigEntryObject rootEntry;
};

#endif
