#ifndef UCONFIGENTRY_H
#define UCONFIGENTRY_H

// Use C-style coding, avoiding class definitions
// Look after well your pointers XD


enum UconfigValueType
{
    Raw = 0, // Array of bytes
    Bool = 1,
    Chars = 2, // Array of ASCII characters terminated by 0x00
    Integer = 3,
    Float = 4,
    Double = 5,
    List = 128
};

struct UconfigKey
{
    char* name;
    int nameSize;  // Number of bytes required by the name

    int valueType; // Using enum UconfigValueType
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
