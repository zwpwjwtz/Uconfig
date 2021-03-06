#ifndef UCONFIGINI_H
#define UCONFIGINI_H

#include "uconfigkeyvalue.h"


class UconfigINI : public UconfigKeyValue
{
public:
    enum EntryType
    {
        UnknownEntry = 0,
        NormalEntry = 1,
        CommentEntry = 2
    };

    static bool readUconfig(const char* filename,
                            UconfigFile* config);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config);
};

#endif // UCONFIGINI_H
