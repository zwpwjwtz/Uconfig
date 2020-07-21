#include <cstring>
#include <cstdio>

#include "parser/uconfigfile_metadata.h"
#include "parser/uconfigini.h"
#include "parser/uconfigcsv.h"
#include "parser/uconfigjson.h"
#include "parser/uconfigxml.h"


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
        strcmp(config.rootEntry.searchSubentry("GRUB_THEME")
                     .keys()[0].value(),
               "/boot/grub/themes/deepin/theme.txt") == 0;

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

bool testParserCSV()
{
    const char* filename = "./SampleConfigs/population.csv";
    const char* outputFileName = "./SampleConfigs/population.txt";

    UconfigFile config;
    if (!UconfigCSV::readUconfig(filename, &config,
                                 "\n", NULL))
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
                     .subentryCount() == 2;

    success &= config.rootEntry.searchSubentry("handler", NULL, true)
                     .keyCount() == 3;

    UconfigKeyObject* keyList =
            config.rootEntry.searchSubentry("contents", NULL, true)
                  .subentries()[0].keys();
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

void testParser()
{
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

    if (testParserCSV())
        printf("testParserCSV() passed.\n");
    else
        printf("testParserCSV() failed!\n");

    if (testParserJSON())
        printf("testParserJSON() passed.\n");
    else
        printf("testParserJSON() failed!\n");

    if (testParserXML())
        printf("testParserXML() passed.\n");
    else
        printf("testParserXML() failed!\n");
}
