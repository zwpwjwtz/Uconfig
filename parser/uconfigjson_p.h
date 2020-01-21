#ifndef UCONFIGJSON_P_H
#define UCONFIGJSON_P_H

#include "uconfigio.h"


typedef struct _IO_FILE FILE;

class UconfigJSONKey : public UconfigKeyObject, public UconfigIO
{
public:
    bool parseValue(const char* expression, int length = 0);

    int fwriteValue(FILE* file);
};

class UconfigJSONPrivate
{
public:
    static int freadEntry(FILE* file,
                          UconfigEntryObject& entry);
    static bool fwriteEntry(FILE* file,
                            UconfigEntryObject& entry,
                            int level = 0);
};

#endif // UCONFIGJSON_P_H
