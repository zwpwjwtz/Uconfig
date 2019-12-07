#ifndef UCONFIGINI_P_H
#define UCONFIGINI_P_H

#include "uconfigkeyvalue_p.h"


class UconfigINIPrivate : public UconfigKeyValuePrivate
{
public:
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

#endif // UCONFIGINI_P_H
