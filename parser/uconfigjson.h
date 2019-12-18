#ifndef UCONFIGJSON_H
#define UCONFIGJSON_H

#include "uconfigio.h"


class UconfigJSON : public UconfigIO
{
public:
    enum EntryType
    {
        UnknownEntry = 0,
        ObjectEntry = 1,
        ArrayEntry = 16
    };

    static bool readUconfig(const char* filename,
                            UconfigFile* config);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config);
};

#endif // UCONFIGJSON_H
