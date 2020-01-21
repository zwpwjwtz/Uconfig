#include <string.h>
#include <stdio.h>
#include "parser/uconfigfile_metadata.h"
#include "parser/uconfigini.h"
#include "parser/uconfig2dtable.h"
#include "parser/uconfigjson.h"
#include "parser/uconfigxml.h"


bool testFileContainer()
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


bool testParserKeyValue()
{
    const char* filename = "./SampleConfigs/grub";
    const char* outputFileName = "./SampleConfigs/grub.txt";

    UconfigFile config;
    if (!UconfigKeyValue::readUconfig(filename, &config))
        return false;

    bool success = true;
    success &=
        strcmp(config.metadata.searchKey(UCONFIG_METADATA_KEY_FILENAME).value(),
               filename) == 0;
    success &=
        config.rootEntry.searchSubentry("GRUB_GFXMODE").keyCount() == 2;
    success &=
        strcmp(config.rootEntry.searchSubentry("GRUB_CMDLINE_LINUX")
                     .keys()[0].value(), "\"\"") == 0;

    success &= UconfigKeyValue::writeUconfig(outputFileName, &config);

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
               " ") == 0;

    success &= Uconfig2DTable::writeUconfig(outputFileName, &config);

    return success;
}

bool testParserJSON()
{
    const char* filename = "./SampleConfigs/firefox.json";
    const char* outputFileName = "./SampleConfigs/firefox.json.json";

    UconfigFile config;
    if (!UconfigJSON::readUconfig(filename, &config))
        return false;

    bool success = true;
    success &=
        strcmp(config.metadata.searchKey(UCONFIG_METADATA_KEY_FILENAME).value(),
               filename) == 0;

    success &= config.rootEntry.searchSubentry("Apply", NULL, true, 5)
                     .subentryCount() == 1;

    success &= config.rootEntry.searchSubentry("Capture", NULL, true, 7)
                     .keyCount() == 1;

    UconfigKeyObject* keyList =
            config.rootEntry.searchSubentry("FileList", NULL, true, 8).keys();
    success &=
        strncmp(keyList[4].value(),
                "/usr/lib/python3.5/",
                keyList[4].valueSize()) == 0;

    success &= UconfigJSON::writeUconfig(outputFileName, &config);

    return success;
}

bool testParserXML()
{
    const char* filename = "./SampleConfigs/config.xml";
    const char* outputFileName = "./SampleConfigs/config.xml.xml";

    UconfigFile config;
    if (!UconfigXML::readUconfig(filename, &config))
        return false;

    bool success = true;
    success &=
        strcmp(config.metadata.searchKey(UCONFIG_METADATA_KEY_FILENAME).value(),
               filename) == 0;

    success &= config.rootEntry.searchSubentry("configBlock", NULL, true)
                     .subentryCount() == 5;

    success &= config.rootEntry.searchSubentry("handler", NULL, true)
                     .keyCount() == 3;

    UconfigKeyObject* keyList =
            config.rootEntry.searchSubentry("contents", NULL, true)
                  .subentries()[1].keys();
    success &=
        strncmp(keyList[0].value(),
                "\n<add verb=\"*\" path=\"settings.map\" "
                "type=\"System.Web.HttpForbiddenHandler, System.Web, "
                "Version=2.0.0.0, Culture=neutral, "
                "PublicKeyToken=Token\" />\n",
                keyList[0].valueSize()) == 0;

    success &= UconfigXML::writeUconfig(outputFileName, &config);

    return success;
}

bool testJSON2XML()
{
    const char* filename = "./SampleConfigs/firefox.json";
    const char* outputFileName = "./SampleConfigs/firefox.json.xml";
    const char* rootElementName = "firefox";

    UconfigFile config;
    if (!UconfigJSON::readUconfig(filename, &config))
        return false;

    bool success = true;

    // Give the outer element a name
    config.rootEntry.subentries()[0].setName(rootElementName);

    // Export the data to a XML
    success &= UconfigXML::writeUconfig(outputFileName, &config);

    return success;
}

int main(int argc, char *argv[])
{
    if (testFileContainer())
        printf("testFileContainer() passed.\n");
    else
        printf("testFileContainer() failed!\n");

    if (testGuessValueType())
        printf("testGuessValueType() passed.\n");
    else
        printf("testGuessValueType() failed!\n");

    if (testParserKeyValue())
        printf("testParserKeyValue() passed.\n");
    else
        printf("testParserKeyValue() failed!\n");

    if (testParserINI())
        printf("testParserINI() passed.\n");
    else
        printf("testParserINI() failed!\n");

    if (testParser2DTable())
        printf("testParser2DTable() passed.\n");
    else
        printf("testParser2DTable() failed!\n");

    if (testParserJSON())
        printf("testParserJSON() passed.\n");
    else
        printf("testParserJSON() failed!\n");

    if (testParserXML())
        printf("testParserXML() passed.\n");
    else
        printf("testParserXML() failed!\n");

    if (testJSON2XML())
        printf("testJSON2XML() passed.\n");
    else
        printf("testJSON2XML() failed!\n");

    return 0;
}
