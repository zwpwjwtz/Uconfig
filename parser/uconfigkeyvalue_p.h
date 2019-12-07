#ifndef UCONFIGKEYVALUE_P_H
#define UCONFIGKEYVALUE_P_H

#include "uconfigentryobject.h"


typedef struct _IO_FILE FILE;

class UconfigKeyValuePrivate
{
public:
    static int parseExpKeyValue(const char* expression,
                                UconfigKeyObject& key,
                                int expressionLength = 0,
                                const char* delimiter = NULL);
    static int parseLineComment(const char* expression,
                                UconfigKeyObject& key,
                                int expressionLength = 0,
                                const char* delimiter = NULL);

    static bool fwriteEntry(FILE* file,
                            UconfigEntryObject& entry,
                            const char* lineDelimiter = NULL,
                            const char* keyValueDelimiter = NULL,
                            const char* commentDelimiter = NULL);
};

#endif // UCONFIGKEYVALUE_P_H
