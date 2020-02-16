#ifndef UCONFIGENTRY_H
#define UCONFIGENTRY_H

// Use C-style coding, avoiding class definitions
// Look after well your pointers XD


struct UconfigKey
{
    char* name;
    int nameSize;  // Number of bytes required by the name

    int valueType; // Used by specific parser
    int valueSize; // Number of bytes required by the value
    char* value;
};

struct UconfigEntry
{
    char* name;
    int nameSize;

    int type; // Used by specific parser

    int keyCount;   // Number of keys
    int subentryCount; // Number of subentries;
    UconfigKey** keys;
    UconfigEntry** subentries;

    UconfigEntry* parentEntry;
};

#endif
