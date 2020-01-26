#include <cstdio>

#include "parser/uconfigini.h"
#include "parser/uconfigjson.h"
#include "parser/uconfigxml.h"


bool testKeyValue2INI()
{
    const char* filename = "./SampleConfigs/grub";
    const char* outputFileName = "./SampleConfigs/grub.ini";
    const char* rootElementName = "grub";

    UconfigFile config;
    if (!UconfigKeyValue::readUconfig(filename, &config))
        return false;

    // Adjust the hierarchy of data to INI syntax
    // by wrapping all entries into a pseudo root entry (object)
    UconfigFile newConfig;
    config.rootEntry.setType(UconfigJSON::ObjectEntry);
    config.rootEntry.setName(rootElementName);
    newConfig.rootEntry.addSubentry(&config.rootEntry);

    // Export the data to a JSON
    return UconfigINI::writeUconfig(outputFileName, &newConfig);
}

bool testINI2KeyValue()
{
    const char* filename = "./SampleConfigs/QMLPlayer.ini";
    const char* outputFileName = "./SampleConfigs/QMLPlayer.ini.txt";

    UconfigFile config;
    if (!UconfigINI::readUconfig(filename, &config))
        return false;

    // Adjust the hierarchy of data to simple KEY=VALUE pattern
    // by extract all subentries into the root entry
    UconfigFile newConfig;
    UconfigEntryObject* entryList = config.rootEntry.subentries();
    for (int i=0; i<config.rootEntry.subentryCount(); i++)
    {
        UconfigEntryObject* subentryList = entryList[i].subentries();
        for (int j=0; j<entryList[i].subentryCount(); j++)
            newConfig.rootEntry.addSubentry(&subentryList[j]);
        if (subentryList)
            delete[] subentryList;
    }

    // Export the data to a JSON
    return UconfigKeyValue::writeUconfig(outputFileName, &newConfig);
}


bool testINI2JSON()
{
    const char* filename = "./SampleConfigs/QMLPlayer.ini";
    const char* outputFileName = "./SampleConfigs/QMLPlayer.ini.json";

    UconfigFile config;
    if (!UconfigINI::readUconfig(filename, &config))
        return false;

    // Adjust the hierarchy of data to JSON syntax
    // by wrapping all entries into a pseudo root entry (object)
    UconfigFile newConfig;
    config.rootEntry.setType(UconfigJSON::ObjectEntry);
    newConfig.rootEntry.addSubentry(&config.rootEntry);

    // Export the data to a JSON
    return UconfigJSON::writeUconfig(outputFileName, &newConfig);
}

bool testJSON2INI()
{
    const char* filename = "./SampleConfigs/firefox.json";
    const char* outputFileName = "./SampleConfigs/firefox.json.ini";
    const char* rootElementName = "firefox";

    UconfigFile config;
    if (!UconfigJSON::readUconfig(filename, &config))
        return false;

    // Give the outer element a name
    config.rootEntry.subentries()[0].setName(rootElementName);

    // Export the data to an INI
    return UconfigINI::writeUconfig(outputFileName, &config);
}

bool testJSON2XML()
{
    const char* filename = "./SampleConfigs/firefox.json";
    const char* outputFileName = "./SampleConfigs/firefox.json.xml";
    const char* rootElementName = "firefox";

    UconfigFile config;
    if (!UconfigJSON::readUconfig(filename, &config))
        return false;

    // Give the outer element a name
    config.rootEntry.subentries()[0].setName(rootElementName);

    // Export the data to a XML
    return UconfigXML::writeUconfig(outputFileName, &config);
}

bool testXML2JSON()
{
    const char* filename = "./SampleConfigs/config.xml";
    const char* outputFileName = "./SampleConfigs/config.xml.json";

    UconfigFile config;
    if (!UconfigXML::readUconfig(filename, &config))
        return false;

    // Adjust the hierarchy of data to JSON syntax
    // by removing all prologue sections
    int entryCount = config.rootEntry.subentryCount();
    UconfigEntryObject* entryList = config.rootEntry.subentries();
    for (int i=0; i<entryCount; i++)
    {
        if (entryList[i].type() != UconfigXML::NormalEntry)
        {
            entryList[i].setName("unconvertible");
            config.rootEntry.deleteSubentry("unconvertible");
        }
    }

    // Export the data to a JSON
    return UconfigJSON::writeUconfig(outputFileName, &config);
}

void testConversion()
{
    if (testKeyValue2INI())
        printf("testKeyValue2INI() passed.\n");
    else
        printf("testKeyValue2INI() failed!\n");

    if (testINI2KeyValue())
        printf("testINI2KeyValue() passed.\n");
    else
        printf("testINI2KeyValue() failed!\n");

    if (testINI2JSON())
        printf("testINI2JSON() passed.\n");
    else
        printf("testINI2JSON() failed!\n");

    if (testJSON2INI())
        printf("testJSON2INI() passed.\n");
    else
        printf("testJSON2INI() failed!\n");

    if (testJSON2XML())
        printf("testJSON2XML() passed.\n");
    else
        printf("testJSON2XML() failed!\n");

    if (testXML2JSON())
        printf("testXML2JSON() passed.\n");
    else
        printf("testXML2JSON() failed!\n");
}
