#ifndef UCONFIGJSON_P_H
#define UCONFIGJSON_P_H

#include "uconfigjson.h"


typedef struct _IO_FILE FILE;

class UconfigJSONPrivate
{
public:
    static UconfigValueType getValueType(const char* value, int length);

    static int freadEntry(FILE* file,
                          UconfigEntryObject& entry);
    static bool fwriteEntry(FILE* file,
                            UconfigEntryObject& entry,
                            int level = 0);
};

#endif // UCONFIGJSON_P_H
