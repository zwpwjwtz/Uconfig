#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uconfigini.h"
#include "uconfigini_p.h"
#include "uconfigfile_metadata.h"
#include "utils.h"

#define UCONFIG_IO_INI_DELIMITER_LINE      "\n"
#define UCONFIG_IO_INI_DELIMITER_KEYVAL    "="
#define UCONFIG_IO_INI_DELIMITER_COMMENT   ";"
#define UCONFIG_IO_INI_DELIMITER_COMMENT2   "#"


bool UconfigINI::readUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    config->rootEntry.reset();

    UconfigEntryObject tempEntry, tempSubentry;
    UconfigKeyObject tempKey, tempComment;
    tempEntry.setType(UconfigINI::UnknownEntry);

    // Read INI file and parse its content line by line
    char* buffer;
    char* entryName;
    long unsigned int readlen, parsedLen;
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

            // See if the title line contains a comment
            if (UconfigINIPrivate::parseLineComment(buffer, tempKey, readlen))
            {
                tempSubentry.setType(UconfigINI::Comment);
                tempSubentry.addKey(&tempKey);
            }
        }
        else
        {
            // See if the line contains a comment
            parsedLen = UconfigINIPrivate::parseLineComment(buffer,
                                                            tempComment,
                                                            readlen);

            // See if the line contains an expression like KEY=VALUE
            if (UconfigINIPrivate::parseExpKeyValue(buffer,
                                                    tempKey,
                                                    readlen - parsedLen))
            {
                tempSubentry.setType(UconfigINI::KeyVal);
                tempSubentry.addKey(&tempKey);

            }

            if (parsedLen > 0)
            {
                tempSubentry.addKey(&tempComment);
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
                tempKey.setType(ValueType::Raw);
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

        if (buffer)
            free(buffer);
    }

    if (tempEntry.type() != UconfigINI::UnknownEntry)
    {
        // Save the last entry
        config->rootEntry.addSubentry(&tempEntry);
    }

    // Add meta-data
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_FILENAME);
    tempKey.setType(ValueType::Chars);
    tempKey.setValue(filename, strlen(filename) + 1);
    config->metadata.addKey(&tempKey);
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_FILETYPE);
    tempKey.setType(ValueType::Chars);
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

    const char* lineDelimiter = UCONFIG_IO_INI_DELIMITER_LINE;
    const char* commentDelimiter = UCONFIG_IO_INI_DELIMITER_COMMENT;
    bool success = UconfigINIPrivate::fwriteEntry(outputFile,
                                                  config->rootEntry,
                                                  lineDelimiter,
                                                  commentDelimiter);
    fclose(outputFile);

    return success;
}

int UconfigINIPrivate::parseLineComment(const char* expression,
                                        UconfigKeyObject& key,
                                        int expressionlength,
                                        const char* delimiter)
{
    int parsedLength;

    // Try all possible delimiters
    if (delimiter)
    {
        parsedLength =
            UconfigKeyValuePrivate::parseLineComment(expression,
                                                     key,
                                                     expressionlength,
                                                     delimiter);
        if (parsedLength > 0)
            return parsedLength;
    }

    delimiter = UCONFIG_IO_INI_DELIMITER_COMMENT;
    parsedLength =
            UconfigKeyValuePrivate::parseLineComment(expression,
                                                     key,
                                                     expressionlength,
                                                     delimiter);
    if (parsedLength > 0)
        return parsedLength;

    delimiter = UCONFIG_IO_INI_DELIMITER_COMMENT2;
    parsedLength =
            UconfigKeyValuePrivate::parseLineComment(expression,
                                                     key,
                                                     expressionlength,
                                                     delimiter);
    if (parsedLength > 0)
        return parsedLength;

    return 0;
}

bool UconfigINIPrivate::fwriteEntry(FILE* file,
                                    UconfigEntryObject& entry,
                                    const char* lineDelimiter,
                                    const char* keyValueDelimiter,
                                    const char* commentDelimiter)
{
    bool success;
    UconfigEntryObject* entryList = entry.subentries();
    int lDLength = strlen(lineDelimiter);
    keyValueDelimiter = UCONFIG_IO_INI_DELIMITER_KEYVAL;

    for (int i=0; i<entry.subentryCount(); i++)
    {
        if (entryList[i].type() == UconfigINI::NormalEntry)
        {
            // Write entry header
            fwrite("[", sizeof(char), 1, file);
            fwrite(entryList[i].name(),
                   sizeof(char),
                   strlen(entryList[i].name()),
                   file);
            fwrite("]", sizeof(char), 1, file);
            fwrite(lineDelimiter, sizeof(char), lDLength, file);
        }

        success = UconfigKeyValuePrivate::fwriteEntry(file,
                                                      entryList[i],
                                                      lineDelimiter,
                                                      keyValueDelimiter,
                                                      commentDelimiter);
        if (!success)
            break;

        if (entryList[i].type() == UconfigINI::NormalEntry)
        {
            // An extra line-break for the INI entry
            fwrite(lineDelimiter, sizeof(char), lDLength, file);
        }
    }

    if (entryList)
        delete[] entryList;
    return success;
}
