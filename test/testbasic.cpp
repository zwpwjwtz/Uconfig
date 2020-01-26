#include <cstring>
#include <cstdio>

#include "parser/uconfigio.h"


bool testEntry()
{
    const char* name1 = "TestEntry";
    const char* name2 = "Hello";
    const char bigdata[] = "235\x00\x00735689067458\x00934890346";

    UconfigEntry e1;

    e1.name = new char[32];
    strcpy(e1.name, name1);
    e1.nameSize = strlen(name1) + 1;

    e1.keyCount = 2;
    e1.keys = new UconfigKey*[2];
    e1.keys[0] = new UconfigKey;
    e1.keys[0]->name = new char[16];
    strcpy(e1.keys[0]->name, name2);
    e1.keys[0]->nameSize = strlen(name2) + 1;
    e1.keys[1] = new UconfigKey;
    e1.keys[1]->valueSize = 32;
    e1.keys[1]->value = new char[e1.keys[1]->valueSize];
    memcpy(e1.keys[1]->value, bigdata, 32);
    e1.subentryCount = 0;

    UconfigEntryObject e(&e1);
    UconfigFile f;
    f.rootEntry.addSubentry(&e);

    UconfigEntryObject e2 = f.rootEntry.searchSubentry(name1);
    UconfigEntryObject e3(e2);

    bool success = true;
    success &= e1.name != e2.name();
    success &= strcmp(e1.name, e2.name()) == 0;
    UconfigKeyObject* newKeys = e2.keys();
    success &= e1.keys[0]->name != newKeys[0].name();
    success &= strcmp(e1.keys[0]->name, newKeys[0].name()) == 0;
    success &= e1.keys[1]->value != newKeys[1].value();
    success &= memcmp(e1.keys[1]->value, newKeys[1].value(), 32) == 0;
    UconfigKeyObject* newKeys2 = e3.keys();
    success &= newKeys2[1].value() != newKeys[1].value();
    success &= memcmp(newKeys2[1].value(), newKeys[1].value(), 32) == 0;

    return success;
}

bool testGuessValueType()
{
    const char* sampleChars = "\"339f,jF)9j4hhh.falseG;;1\"";
    const char* sampleInteger = "26978274";
    const char* sampleDouble = "269.78274";
    const char* sampleBool = "FaLsE";
    const char* sampleRaw1 = "\"633401'";
    const char* sampleRaw2 = "633.401E23";
    const char* sampleRaw3 = "633401False";

    bool success = true;
    success &= UconfigIO::guessValueType(sampleChars, strlen(sampleChars))
                    == UconfigValueType::Chars;
    success &= UconfigIO::guessValueType(sampleInteger, strlen(sampleInteger))
                    == UconfigValueType::Integer;
    success &= UconfigIO::guessValueType(sampleDouble, strlen(sampleDouble))
                    == UconfigValueType::Double;
    success &= UconfigIO::guessValueType(sampleBool, strlen(sampleBool))
                    == UconfigValueType::Bool;
    success &= UconfigIO::guessValueType(sampleRaw1, strlen(sampleRaw1))
                    == UconfigValueType::Raw;
    success &= UconfigIO::guessValueType(sampleRaw2, strlen(sampleRaw2))
                    == UconfigValueType::Raw;
    success &= UconfigIO::guessValueType(sampleRaw3, strlen(sampleRaw3))
                    == UconfigValueType::Raw;

    return success;
}

void testBasic()
{
    if (testEntry())
        printf("testEntry() passed.\n");
    else
        printf("testEntry() failed!\n");

    if (testGuessValueType())
        printf("testGuessValueType() passed.\n");
    else
        printf("testGuessValueType() failed!\n");
}
