#ifndef UCONFIGINI_H
#define UCONFIGINI_H

#include "uconfigio.h"


class UconfigINI : public UconfigIO
{
public:
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
