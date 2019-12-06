#include <stdio.h>
#include <string.h>
#include "uconfigini.h"
#include "uconfigfile_metadata.h"
#include "utils.h"

#define UCONFIG_IO_INI_BUFFER_MAXLEN    16384


bool Uconfig_fwriteINIEntry(FILE* file,
                         UconfigEntryObject& entry,
                         const char* lineDelimiter = NULL);

bool UconfigINI::readUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    UconfigEntryObject tempEntry, tempSubentry;
    UconfigKeyObject tempKey;
    tempEntry.setType(UconfigINI::UnknownEntry);

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

        // Get the length of line without line delimiter
        readlen = Uconfig_findLineDelimiter(buffer);

        // See if the line opens a new entry
        if (buffer[0] == '[' && buffer[readlen - 1] == ']')
        {
            if (tempEntry.type() != UconfigINI::UnknownEntry)
            {
                // Save previous entry
                config->rootEntry.addSubentry(&tempEntry);
            }

            // Create a new entry
            tempEntry.reset();
            tempEntry.setType(UconfigINI::NormalEntry);

            // Set entry's name
            entryName = new char[readlen - 1];
            Uconfig_strncpy(entryName, &buffer[1], readlen - 2);
            tempEntry.setName(entryName);
            delete entryName;

            // See if the line contains a comment
            if (parseLineComment(buffer, tempKey, readlen))
            {
                tempSubentry.setType(UconfigINI::Comment);
                tempSubentry.addKey(&tempKey);
            }
        }
        else
        {
            // See if the line contains an expression like KEY=VALUE
            if (parseExpKeyValue(buffer, tempKey, readlen))
            {
                tempSubentry.setType(UconfigINI::KeyVal);
                tempSubentry.addKey(&tempKey);
            }

            // See if the line contains a comment
            if (parseLineComment(buffer, tempKey, readlen))
            {
                tempSubentry.addKey(&tempKey);
                if (tempSubentry.keyCount() < 2)
                {
                    // Standalone comment line
                    tempSubentry.setType(UconfigINI::Comment);
                }
            }

            if (tempSubentry.keyCount() < 1)
            {
                // See the whole line as RAW content
                tempKey.reset();
                tempKey.setValue(buffer, readlen);
                tempKey.setType(UconfigValueType::Raw);
                tempSubentry.addKey(&tempKey);
                tempSubentry.setType(UconfigINI::Raw);
            }
        }

        if (tempSubentry.keyCount() > 0)
        {
            if (tempEntry.type() == UconfigINI::NormalEntry)
            {
                // Normal entry, use the name of the first key as its name
                tempSubentry.setName(tempSubentry.keys()[0].name());
                tempEntry.addSubentry(&tempSubentry);
            }
            else
            {
                // Standalone comment entry or raw content block
                // (at the beginning of the file)
                tempEntry.addSubentry(&tempSubentry);
                tempEntry.setType(UconfigINI::CommentEntry);
            }
            tempSubentry.reset();
        }
    }

    if (tempEntry.type() != UconfigINI::UnknownEntry)
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

    FILE* outputFile = fopen(filename, "w");
    if (!outputFile)
        return false;

    bool success;
    UconfigEntryObject* entryList = config->rootEntry.subentries();
    for (int i=0; i<config->rootEntry.subentryCount(); i++)
    {
        success = Uconfig_fwriteINIEntry(outputFile, entryList[i]);
        if (!success)
            break;
    }

    if (entryList)
        delete[] entryList;
    fclose(outputFile);

    return success;
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
    // See if the comment delimiter appears in a right place
    int p = Uconfig_strpos(expression, "#");
    if (p < 0 || (p > 0 && expression[p - 1] != ' ') || p >= length - 1)
        p = Uconfig_strpos(expression, ";");
    if (p < 0 || (p > 0 && expression[p - 1] != ' ') || p >= length - 1)
        return false;

    key.reset();
    key.setType(UconfigValueType::Chars);
    if (length <= 0)
        length = strlen(expression);
    if (p < length - 1)
    {
        key.setValue(&expression[p + 1], length - p - 1);
    }
    return true;
}

bool Uconfig_fwriteINIEntry(FILE* file,
                         UconfigEntryObject& entry,
                         const char* lineDelimiter)
{
    if (entry.subentryCount() < 1)
        return true;

    int i, j;
    int entryType = entry.type();
    UconfigKeyObject* keyList = NULL;
    UconfigEntryObject* subentryList = entry.subentries();
    const char* delimiter = lineDelimiter ? lineDelimiter : "\n";

    if (entryType == UconfigINI::NormalEntry)
    {
        // Write entry header
        fwrite("[", sizeof(char), 1, file);
        fwrite(entry.name(), strlen(entry.name()), 1, file);
        fwrite("]", sizeof(char), 1, file);
        fwrite(delimiter, sizeof(char), 1, file);

        for (i=0; i<entry.subentryCount(); i++)
        {
            // Add leading ";" for each comment line
            if (subentryList[i].type() == UconfigINI::Comment)
                fwrite(";", sizeof(char), 1, file);

            if (subentryList[i].keyCount() > 0)
            {
                keyList = subentryList[i].keys();
                for (j=0; j<subentryList[i].keyCount(); j++)
                {
                    // Take the first key as a key-value pair
                    if (subentryList[i].type() == UconfigINI::KeyVal && j == 0)
                    {
                        fwrite(keyList[0].name(),
                               sizeof(char),
                               strlen(keyList[0].name()),
                               file);
                        fwrite("=", sizeof(char), 1, file);
                    }
                    fwrite(keyList[0].value(),
                           sizeof(char),
                           keyList[0].valueSize(),
                           file);
                }
                delete[] keyList;
            }

            fwrite(delimiter, sizeof(char), 1, file);
        }

        fwrite(delimiter, sizeof(char), 1, file);
    }
    else if (entryType == UconfigINI::CommentEntry)
    {
        for (i=0; i<entry.subentryCount(); i++)
        {
            // Add leading ";" for each comment line
            if (subentryList[i].type() == UconfigINI::Comment)
                fwrite(";", sizeof(char), 1, file);

            if (subentryList[i].keyCount() > 0)
            {
                keyList = subentryList[i].keys();
                for (j=0; j<subentryList[i].keyCount(); j++)
                {
                    // Write each comment line or raw content line directly
                    fwrite(keyList[0].value(),
                           sizeof(char),
                           keyList[0].valueSize(),
                           file);
                }
                delete[] keyList;
            }

            fwrite(delimiter, sizeof(char), 1, file);
        }
    }

    if (subentryList)
        delete[] subentryList;
    return true;
}
