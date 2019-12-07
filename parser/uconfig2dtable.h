#ifndef UCONFIG2DTABLE_H
#define UCONFIG2DTABLE_H

#include "uconfigio.h"


class Uconfig2DTable : public UconfigIO
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
        Row = 1
    };

    static bool readUconfig(const char* filename, UconfigFile* config);
    static bool writeUconfig(const char* filename, UconfigFile* config);

    static bool readUconfig(const char* filename,
                            UconfigFile* config,
                            const char* rowDelimiter,
                            const char* columnDelimiter,
                            bool skipEmptyRow = true,
                            bool skipEmptyColumn = true);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config,
                             const char* rowDelimiter,
                             const char* columnDelimiter);

    static int parseValues(const char* expression,
                           UconfigEntryObject& entry,
                           int expressionLength = 0,
                           const char* delimiter = NULL,
                           bool skipEmptyValue = true);
};

#endif // UCONFIG2DTABLE_H
