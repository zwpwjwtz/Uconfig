#ifndef UCONFIGXML_P_H
#define UCONFIGXML_P_H

#include "uconfigentryobject.h"


typedef struct _IO_FILE FILE;

class UconfigXMLPrivate
{
public:
    static UconfigValueType getValueType(const char* value, int length);

    static int freadEntry(FILE* file,
                          UconfigEntryObject& entry,
                          bool inTag = false);
    static bool fwriteEntry(FILE* file,
                            UconfigEntryObject& entry,
                            int level = 0);
    static bool fwriteEntryKeys(FILE* file,
                                UconfigEntryObject& entry,
                                bool valueOnly = false);

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
