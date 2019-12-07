#ifndef UCONFIGKEYVALUE_H
#define UCONFIGKEYVALUE_H

#include "uconfigio.h"


class UconfigKeyValue : public UconfigIO
{
public:
    enum EntryType
    {
        UnknownEntry = 0,
        NormalEntry = 1
    };

    enum SubentryType
    {
        Raw = 0,
        KeyVal = 1,
        Comment = 2
    };

    static bool readUconfig(const char* filename,
                            UconfigFile* config);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config);
};

#endif // UCONFIGKEYVALUE_H
