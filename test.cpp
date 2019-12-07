#include <string.h>
#include <stdio.h>
#include "parser/uconfigfile_metadata.h"
#include "parser/uconfigini.h"
#include "parser/uconfig2dtable.h"


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

bool testParserINI()
{
    const char* filename = "./SampleConfigs/QMLPlayer.ini";
    const char* outputFileName = "./SampleConfigs/QMLPlayer.ini.ini";

    UconfigFile config;
    if (!UconfigINI::readUconfig(filename, &config))
        return false;

    bool success = true;
    success &=
        strcmp(config.metadata.searchKey(UCONFIG_METADATA_KEY_FILENAME).value(),
               filename) == 0;
    success &=
        config.rootEntry.searchSubentry("avfilterAudio").subentryCount() == 2;
    success &=
        strcmp(config.rootEntry.searchSubentry("capture").searchSubentry("dir")
                     .keys()[0].value(), "/home/user") == 0;
    success &=
        strcmp(config.rootEntry.searchSubentry("decoder").subentries()[0]
                     .keys()[0].name(), "video\\priority") == 0;

    success &= UconfigINI::writeUconfig(outputFileName, &config);

    return success;
}

bool testParser2DTable()
{
    const char* filename = "./SampleConfigs/fstab";
    const char* outputFileName = "./SampleConfigs/fstab.txt";

    UconfigFile config;
    if (!Uconfig2DTable::readUconfig(filename, &config,
                                     "\n", NULL, false, true))
        return false;

    bool success = true;
    success &=
        strcmp(config.metadata.searchKey(UCONFIG_METADATA_KEY_FILENAME).value(),
               filename) == 0;
    success &=
        strcmp(config.metadata.searchKey(UCONFIG_METADATA_KEY_COLDELIM).value(),
               "\t") == 0;

    Uconfig2DTable configParser;
    success &= configParser.writeUconfig(outputFileName, &config);

    return success;
}

int main(int argc, char *argv[])
{
    if (testFileContainer())
        printf("testFileContainer() passed.\n");
    else
        printf("testFileContainer() failed!\n");

    if (testParserINI())
        printf("testParserINI() passed.\n");
    else
        printf("testParserINI() failed!\n");

    if (testParser2DTable())
        printf("testParser2DTable() passed.\n");
    else
        printf("testParser2DTable() failed!\n");

    return 0;
}
