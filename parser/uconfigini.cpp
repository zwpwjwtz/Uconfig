#include <stdio.h>
#include <string.h>
#include "uconfigini.h"
#include "uconfigfile_metadata.h"
#include "utils.h"

#define UCONFIG_IO_INI_ENTRY_UNKNOWN    0
#define UCONFIG_IO_INI_ENTRY_NORMAL     1
#define UCONFIG_IO_INI_ENTRY_COMMENT    2
#define UCONFIG_IO_INI_SUBENTRY_KEYVAL  3
#define UCONFIG_IO_INI_SUBENTRY_COMMENT 4
#define UCONFIG_IO_INI_BUFFER_MAXLEN    16384


bool UconfigINI::readUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    UconfigEntryObject tempEntry, tempSubentry;
    UconfigKeyObject tempKey;
    tempEntry.setType(UCONFIG_IO_INI_ENTRY_UNKNOWN);

    // Read INI file and parse its content line by line
    long unsigned int readlen;
    char* buffer;
    char* entryName;
    while(!feof(inputFile))
    {
        readlen = 0;
        buffer = NULL;
        readlen = getline(&buffer, &readlen, inputFile);

        // Omit empty (incomplete) lines
        if (readlen <= 2)
            continue;

        // Get the length of line without line delimitor
        readlen = Uconfig_findLineDelimitor(buffer);

        // See if the line opens a new entry
        if (buffer[0] == '[' && buffer[readlen - 1] == ']')
        {
            if (tempEntry.type() != UCONFIG_IO_INI_ENTRY_UNKNOWN)
            {
                // Save previous entry
                config->rootEntry.addSubentry(&tempEntry);
            }

            // Create a new entry
            tempEntry.reset();
            tempEntry.setType(UCONFIG_IO_INI_ENTRY_NORMAL);

            // Set entry's name
            entryName = new char[readlen - 1];
            Uconfig_strncpy(entryName, &buffer[1], readlen - 2);
            tempEntry.setName(entryName);
            delete entryName;

            // See if the line contains a comment
            if (parseLineComment(buffer, tempKey, readlen))
            {
                tempSubentry.setType(UCONFIG_IO_INI_SUBENTRY_COMMENT);
                tempSubentry.addKey(&tempKey);
            }
        }
        else
        {
            // See if the line contains an expression like KEY=VALUE
            if (parseExpKeyValue(buffer, tempKey, readlen))
            {
                tempSubentry.setType(UCONFIG_IO_INI_SUBENTRY_KEYVAL);
                tempSubentry.addKey(&tempKey);
            }

            // See if the line is a comment
            if (parseLineComment(buffer, tempKey, readlen))
                tempSubentry.addKey(&tempKey);

            if (tempSubentry.keyCount() < 1)
            {
                // See the whole line as RAW content
                tempKey.reset();
                tempKey.setValue(buffer, readlen);
                tempKey.setType(UconfigValueType::Raw);
                tempSubentry.addKey(&tempKey);
            }
        }

        if (tempSubentry.keyCount() > 0)
        {
            if (tempEntry.type() == UCONFIG_IO_INI_ENTRY_NORMAL)
            {
                // Normal entry, use the name of the first key as its name
                tempSubentry.setName(tempSubentry.keys()[0].name());
                tempEntry.addSubentry(&tempSubentry);
                tempSubentry.reset();
            }
            else
            {
                // Comment entry (at the beginning of the file)
                tempSubentry.setType(UCONFIG_IO_INI_SUBENTRY_COMMENT);
                tempEntry.addSubentry(&tempSubentry);
                tempEntry.setType(UCONFIG_IO_INI_ENTRY_COMMENT);
            }
        }
    }

    if (tempEntry.type() != UCONFIG_IO_INI_ENTRY_UNKNOWN)
    {
        // Save the last entry
        config->rootEntry.addSubentry(&tempEntry);
    }

    // Add meta-data
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_FILENAME);
    tempKey.setType(UconfigValueType::Chars);
    tempKey.setValue(filename, strlen(filename) + 1);
    config->metadata.addKey(&tempKey);
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_FILETYPE);
    tempKey.setType(UconfigValueType::Chars);
    tempKey.setValue(UCONFIG_METADATA_VALUE_INIFILE,
                     strlen(UCONFIG_METADATA_VALUE_INIFILE) + 1);
    config->metadata.addKey(&tempKey);

    fclose(inputFile);
    return true;
}

bool UconfigINI::writeUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* outputFile = fopen(filename, "r+");
    if (!outputFile)
        return false;

    fclose(outputFile);
    return true;
}

// Parse an expression string of format "KEY=VALUE"
bool UconfigINI::parseExpKeyValue(const char* expression,
                                  UconfigKeyObject &key,
                                  int length)
{
    if (length <= 0)
        length = strlen(expression);

    int p = Uconfig_strpos(expression, "=");
    if (p <= 0)
        return false;

    char* keyName = new char[p + 1];
    Uconfig_strncpy(keyName, expression, p);
    key.reset();
    key.setName(keyName);
    key.setType(UconfigValueType::Raw);
    key.setValue(&expression[p + 1], length - p - 1);
    delete keyName;
    return true;
}

// Parse a line comment beginning with a given string
bool UconfigINI::parseLineComment(const char* expression,
                                  UconfigKeyObject& key,
                                  int length)
{
    int p = Uconfig_strpos(expression, "#");
    if (p < 0 || p >= length - 1)
        p = Uconfig_strpos(expression, ";");
    if (p < 0 || p >= length - 1)
        return false;

    key.reset();
    key.setType(UconfigValueType::Chars);
    if (length <= 0)
        length = strlen(expression);
    if (p < length - 1)
    {
        key.setValue(&expression[p + 1], length - p);
    }
    return true;
}
