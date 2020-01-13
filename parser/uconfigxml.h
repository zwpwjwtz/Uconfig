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
        CDATAEntry = 3,
        XMLDeclEntry = 4,
        DoctypeEntry = 5
    };

    static bool readUconfig(const char* filename,
                            UconfigFile* config);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config);
};

#endif // UCONFIGXML_H
