#ifndef UCONFIGINI_H
#define UCONFIGINI_H

#include "uconfigio.h"


class UconfigINI : public UconfigIO
{
public:
    enum EntryType
    {
        UnknownEntry = 0,
        NormalEntry = 1,
        CommentEntry = 2
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

    static bool parseExpKeyValue(const char* expression,
                                 UconfigKeyObject& key,
                                 int length = 0);
    static bool parseLineComment(const char* expression,
                                 UconfigKeyObject& key,
                                 int length = 0);
};

#endif // UCONFIGINI_H
