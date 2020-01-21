#ifndef UCONFIGJSON_P_H
#define UCONFIGJSON_P_H

#include "uconfigio.h"


typedef struct _IO_FILE FILE;

class UconfigJSONKey : public UconfigKeyObject, public UconfigIO
{
public:
    bool parseName(const char* expression, int length = 0);
    bool parseValue(const char* expression, int length = 0);

    int fwriteName(FILE* file);
    int fwriteValue(FILE* file);
};

class UconfigJSONEntry : public UconfigEntryObject
{
public:
    bool parseName(const char* expression, int length = 0);

    int fwriteName(FILE* file);
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
