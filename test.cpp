#include <string.h>
#include <stdio.h>
#include "parser/uconfigfile.h"


bool testFileContainer()
{
    const char bigdata[] = "235\x00\x00735689067458\x00934890346";

    UconfigEntry e1;

    e1.name = new char[32];
    strcpy(e1.name, "TestEntry");

    e1.keyCount = 2;
    e1.keys = new UconfigKey*[2];
    e1.keys[0] = new UconfigKey;
    e1.keys[0]->name = new char[16];
    strcpy(e1.keys[0]->name, "Hello");
    e1.keys[1] = new UconfigKey;
    e1.keys[1]->valueSize = 32;
    e1.keys[1]->value = new char[e1.keys[1]->valueSize];
    memcpy(e1.keys[1]->value, bigdata, 32);
    e1.subentryCount = 0;

    UconfigEntryObject e(&e1);
    UconfigFile f;
    f.rootEntry.addSubentry(&e);

    UconfigEntryObject e2 = f.rootEntry.searchSubentry("TestEntry");
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

int main(int argc, char *argv[])
{
    if (testFileContainer())
        printf("testFileContainer() passed.\n");
    else
        printf("testFileContainer() failed!\n");
    return 0;
}
