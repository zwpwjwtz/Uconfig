#ifndef UCONFIGXML_H
#define UCONFIGXML_H

#include "uconfigio.h"


class UconfigXML : public UconfigIO
{
public:
    enum EntryType
    {
        UnknownEntry = 0,
        NormalEntry = 1,
        CommentEntry = 2,
        TextEntry = 3,
        CDATAEntry = 4,
        XMLDeclEntry = 5,
        DoctypeEntry = 6
    };

    static bool readUconfig(const char* filename,
                            UconfigFile* config);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config);

    static bool readUconfig(const char* filename,
                            UconfigFile* config,
                            bool skipBlankTextNode);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config,
                             bool forceQuotingValue);
};

#endif // UCONFIGXML_H
