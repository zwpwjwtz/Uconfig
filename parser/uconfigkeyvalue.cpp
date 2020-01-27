#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uconfigkeyvalue.h"
#include "uconfigkeyvalue_p.h"
#include "uconfigfile_metadata.h"
#include "utils.h"

#define UCONFIG_IO_KEYVALUE_DELIMITER_LINE      "\n"
#define UCONFIG_IO_KEYVALUE_DELIMITER_KEYVAL    "="
#define UCONFIG_IO_KEYVALUE_DELIMITER_COMMENT   "#"

bool UconfigKeyValue::readUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    config->rootEntry.reset();

    UconfigKeyObject tempKey, tempComment;
    UconfigEntryObject tempSubentry;
    UconfigEntryObject& rootEntry = config->rootEntry;
    rootEntry.setType(UconfigKeyValue::UnknownEntry);

    // Read a file and parse its content line by line
    // As there is no pre-defined hierachy, all lines
    // are seen as subentries of a single root entry
    char* buffer;
    long unsigned int readlen, parsedLen;
    while(!feof(inputFile))
    {
        readlen = 0;
        buffer = NULL;
        readlen = getline(&buffer, &readlen, inputFile);

        // Omit empty (incomplete) lines
        readlen = Uconfig_findLineDelimiter(buffer);
        if (readlen < 1)
            continue;

        // See if the line contains a comment
        // By definition, a comment section must appear after
        // the KEY=VALUE section. However, we need to extract first
        // the comment before being able to determine the length of
        // the KEY=VALUE section. Thus we keep the extract comment and
        // append it to the subentry later.
        parsedLen = UconfigKeyValuePrivate::parseLineComment(buffer,
                                                             tempComment,
                                                             readlen);

        // See if the line contains an expression like KEY=VALUE
        if (UconfigKeyValuePrivate::parseExpKeyValue(buffer,
                                                     tempKey,
                                                     readlen - parsedLen))
        {
            tempSubentry.setType(UconfigKeyValue::KeyVal);
            tempSubentry.addKey(&tempKey);

            if (!tempSubentry.name())
            {
                // Use the name of the first key as this subentry's name
                tempSubentry.setName(tempKey.name());
            }
        }

        // Append the comment section, and change the type of
        // the subentry if it is the only section within that line
        if (parsedLen > 0)
        {
            tempSubentry.addKey(&tempComment);
            if (tempSubentry.keyCount() < 2)
            {
                // Standalone comment line
                tempSubentry.setType(UconfigKeyValue::Comment);
            }
        }

        if (tempSubentry.keyCount() < 1)
        {
            // See the whole line as RAW content
            tempKey.reset();
            tempKey.setValue(buffer, readlen);
            tempKey.setType(UconfigValueType::Raw);
            tempSubentry.addKey(&tempKey);
            tempSubentry.setType(UconfigKeyValue::Raw);
        }

        // Append the subentry to the root entry
        if (rootEntry.type() == UconfigKeyValue::UnknownEntry)
        {
            rootEntry.reset();
            rootEntry.setType(UconfigKeyValue::NormalEntry);
        }
        rootEntry.addSubentry(&tempSubentry);
        tempSubentry.reset();

        if (buffer)
            free(buffer);
    }

    // Add meta-data
    /* Basic information */
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

bool UconfigKeyValue::writeUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* outputFile = fopen(filename, "w");
    if (!outputFile)
        return false;

    bool success = UconfigKeyValuePrivate::fwriteEntry(outputFile,
                                                       config->rootEntry);
    fclose(outputFile);
    return success;
}

// Parse an expression string of format "KEY=VALUE"
// Return the number of chars(bytes) that have been parsed
int UconfigKeyValuePrivate::parseExpKeyValue(const char* expression,
                                             UconfigKeyObject& key,
                                             int expressionLength,
                                             const char* delimiter)
{
    if (expressionLength <= 0)
        expressionLength = strlen(expression);

    if (!delimiter)
        delimiter = UCONFIG_IO_KEYVALUE_DELIMITER_KEYVAL;

    // See if the comment delimiter appears in a right place
    int pos = Uconfig_strpos(expression, delimiter);
    if (pos <= 0 || pos >= expressionLength)
        return 0;

    key.reset();

    // Extract key name
    char* keyName = new char[pos + 1];
    Uconfig_strncpy(keyName, expression, pos);
    key.setName(keyName);
    delete keyName;

    // Extract value
    int pos2 = pos + strlen(delimiter);
    key.setType(UconfigValueType::Raw);
    key.setValue(&expression[pos2], expressionLength - pos2);

    // Assuming the entire expression is parsed
    return expressionLength;
}

// Parse a line comment beginning with a given string
// Return the number of chars(bytes) that have been parsed
int UconfigKeyValuePrivate::parseLineComment(const char* expression,
                                             UconfigKeyObject& key,
                                             int expressionLength,
                                             const char* delimiter)
{
    if (expressionLength <= 0)
        expressionLength = strlen(expression);

    if (!delimiter)
        delimiter = UCONFIG_IO_KEYVALUE_DELIMITER_COMMENT;

    // See if the comment delimiter appears in a right place
    int pos = Uconfig_strpos(expression, delimiter);
    if (pos < 0 ||
        (pos > 0 && expression[pos - 1] != ' ') ||
        pos >= expressionLength - 1)
        return 0;

    // Extract the comment text
    int pos2 = pos + strlen(delimiter);
    key.reset();
    key.setType(UconfigValueType::Chars);
    key.setValue(&expression[pos2], expressionLength - pos2);

    return expressionLength - pos;
}

bool UconfigKeyValuePrivate::fwriteEntry(FILE* file,
                                         UconfigEntryObject& entry,
                                         const char* lineDelimiter,
                                         const char* keyValueDelimiter,
                                         const char* commentDelimiter)
{
    if (entry.subentryCount() < 1)
        return true;

    if (!lineDelimiter)
        lineDelimiter = UCONFIG_IO_KEYVALUE_DELIMITER_LINE;
    if (!keyValueDelimiter)
        keyValueDelimiter = UCONFIG_IO_KEYVALUE_DELIMITER_KEYVAL;
    if (!commentDelimiter)
        commentDelimiter = UCONFIG_IO_KEYVALUE_DELIMITER_COMMENT;

    int i, j;
    int lDLength = strlen(lineDelimiter);
    int kvDLength = strlen(keyValueDelimiter);
    int cDLength = strlen(commentDelimiter);
    UconfigKeyObject* keyList = NULL;
    UconfigEntryObject* subentryList = entry.subentries();

    for (i=0; i<entry.subentryCount(); i++)
    {
        if (subentryList[i].keyCount() > 0)
        {
            keyList = subentryList[i].keys();
            for (j=0; j<subentryList[i].keyCount(); j++)
            {
                if (subentryList[i].type() == UconfigKeyValue::KeyVal &&
                    keyList[j].name())
                {
                    fwrite(keyList[j].name(),
                           sizeof(char),
                           keyList[j].nameSize(),
                           file);
                    fwrite(keyValueDelimiter, sizeof(char), kvDLength, file);
                }
                else if (subentryList[i].type() == UconfigKeyValue::Comment ||
                         j >= 1)
                    fwrite(commentDelimiter, sizeof(char), cDLength, file);
                fwrite(keyList[j].value(),
                       sizeof(char),
                       keyList[j].valueSize(),
                       file);
            }
            delete[] keyList;
        }

        fwrite(lineDelimiter, sizeof(char), lDLength, file);
    }

    if (subentryList)
        delete[] subentryList;
    return true;
}
