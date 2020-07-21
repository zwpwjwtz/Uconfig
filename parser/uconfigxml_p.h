#ifndef UCONFIGXML_P_H
#define UCONFIGXML_P_H

#include "uconfigio.h"

typedef UconfigIO::ValueType ValueType;

typedef struct _IO_FILE FILE;

class UconfigXMLKey : public UconfigKeyObject, public UconfigIO
{
public:
    bool parseValue(const char* expression, int length = 0);

    int fwriteValue(FILE* file, bool forceWrappingQuotes = true);
};

class UconfigXMLPrivate
{
public:
    static ValueType getValueType(const char* value, int length);

    static int freadEntry(FILE* file,
                          UconfigEntryObject& entry,
                          bool inTag = false,
                          bool skipBlankTextNode = true);
    static bool fwriteEntry(FILE* file,
                            UconfigEntryObject& entry,
                            int level = 0,
                            bool forceQuotingValue = false);
    static bool fwriteEntryKeys(FILE* file,
                                UconfigEntryObject& entry,
                                bool valueOnly = false,
                                bool forceQuotingValue = false);

    static int parseTagAttribute(const char* expression,
                                 UconfigKeyObject& key,
                                 int expressionLength = 0,
                                 bool complete = true);
    static int parseComment(FILE *file,
                            UconfigEntryObject& entry,
                            int maxLength = 0);
    static int parseCDATA(FILE* file,
                          UconfigEntryObject& entry,
                          int maxLength = 0);
    static int parseXMLDelcaration(FILE* file,
                                   UconfigEntryObject& entry,
                                   int maxLength = 0);
    static int parseDoctype(FILE* file,
                            UconfigEntryObject& entry,
                            int maxLength = 0);
};
#endif // UCONFIGXML_P_H
