#include <stdio.h>
#include <string.h>
#include "uconfig2dtable.h"
#include "uconfigfile_metadata.h"
#include "utils.h"

#define UCONFIG_IO_2DTABLE_DELIMITER_ROW    "\n"
#define UCONFIG_IO_2DTABLE_DELIMITER_COL    " "
#define UCONFIG_IO_2DTABLE_TYPE_COMMENT     "Comment"
#define UCONFIG_IO_2DTABLE_TYPE_TABLE       "Table"


static bool Uconfig_fwrite2DTableEntry(FILE* file,
                                       UconfigEntryObject& entry,
                                       const char* rowDelimiter,
                                       const char* columnDelimiter);
static bool Uconfig_fwrite2DTableSubentry(FILE* file,
                                          UconfigEntryObject& subentry,
                                          const char* columnDelimiter);

bool Uconfig2DTable::readUconfig(const char* filename, UconfigFile* config)
{
    return readUconfig(filename, config,
                       UCONFIG_IO_2DTABLE_DELIMITER_ROW,
                       UCONFIG_IO_2DTABLE_DELIMITER_COL);
}

bool Uconfig2DTable::writeUconfig(const char* filename, UconfigFile* config)
{
    // If there are delimiters previously specified during file parsing,
    // then take them directly
    const char* rowDelimiter =
            config->metadata.searchKey(UCONFIG_METADATA_KEY_ROWDELIM).value();
    const char* columnDelimitor =
            config->metadata.searchKey(UCONFIG_METADATA_KEY_COLDELIM).value();

    // Otherwise, use default delimiters
    if (!rowDelimiter)
        rowDelimiter = UCONFIG_IO_2DTABLE_DELIMITER_ROW;
    if (!columnDelimitor)
        columnDelimitor = UCONFIG_IO_2DTABLE_DELIMITER_COL;

    return writeUconfig(filename, config, rowDelimiter, columnDelimitor);
}

bool Uconfig2DTable::readUconfig(const char* filename,
                                 UconfigFile* config,
                                 const char* rowDelimiter,
                                 const char* columnDelimiter,
                                 bool skipEmptyRow,
                                 bool skipEmptyColumn,
                                 ValueType defaultValueType)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    config->rootEntry.reset();

    UconfigEntryObject tempEntry, tempSubentry;
    UconfigKeyObject tempKey;
    tempEntry.setType(Uconfig2DTable::UnknownEntry);

    // Read a 2D table file and parse its content "line" by "line"
    int readLen;
    char* buffer;
    if (!rowDelimiter)
        rowDelimiter = UCONFIG_IO_2DTABLE_DELIMITER_ROW;
    if (!columnDelimiter)
        columnDelimiter = UCONFIG_IO_2DTABLE_DELIMITER_COL;
    while(!feof(inputFile))
    {
        readLen = 0;
        buffer = NULL;
        readLen = Uconfig_getdelim(&buffer, &readLen, rowDelimiter, inputFile);

        // Omit empty (incomplete) "lines" if required
        if (readLen < 1 && skipEmptyRow)
            continue;

        parseValues(buffer, tempSubentry, readLen,
                    columnDelimiter, skipEmptyColumn,
                    defaultValueType);

        if (tempSubentry.keyCount() > 0)
        {
            if (tempEntry.type() == Uconfig2DTable::CommentEntry)
            {
                // Save previous entry, then create a new entry
                config->rootEntry.addSubentry(&tempEntry);
                tempEntry.reset();
            }
            tempEntry.setName(UCONFIG_IO_2DTABLE_TYPE_TABLE);
            tempEntry.setType(Uconfig2DTable::NormalEntry);
        }
        else
        {
            // See the whole "line" as RAW content
            tempKey.setType(ValueType::Raw);
            tempKey.setValue(buffer, readLen);
            tempSubentry.addKey(&tempKey);
            tempSubentry.setType(Uconfig2DTable::Raw);

            if (tempEntry.type() == Uconfig2DTable::UnknownEntry)
            {
                // Standalone comment entry or raw content block
                // (at the beginning of the file)
                tempEntry.setName(UCONFIG_IO_2DTABLE_TYPE_COMMENT);
                tempEntry.setType(Uconfig2DTable::CommentEntry);
            }
        }
        tempEntry.addSubentry(&tempSubentry);
        tempSubentry.reset();
    }

    if (tempEntry.type() != Uconfig2DTable::UnknownEntry)
    {
        // Save the last entry
        config->rootEntry.addSubentry(&tempEntry);
    }

    // Add meta-data
    /* Basic information */
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_FILENAME);
    tempKey.setType(ValueType::Chars);
    tempKey.setValue(filename, strlen(filename) + 1);
    config->metadata.addKey(&tempKey);
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_FILETYPE);
    tempKey.setType(ValueType::Chars);
    tempKey.setValue(UCONFIG_METADATA_VALUE_2DTABLE,
                     strlen(UCONFIG_METADATA_VALUE_2DTABLE) + 1);
    config->metadata.addKey(&tempKey);
    /* Delimiter strings */
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_ROWDELIM);
    tempKey.setType(ValueType::Chars);
    tempKey.setValue(rowDelimiter, strlen(rowDelimiter) + 1);
    config->metadata.addKey(&tempKey);
    tempKey.reset();
    tempKey.setName(UCONFIG_METADATA_KEY_COLDELIM);
    tempKey.setType(ValueType::Chars);
    tempKey.setValue(columnDelimiter, strlen(columnDelimiter) + 1);
    config->metadata.addKey(&tempKey);

    fclose(inputFile);
    return true;
}

bool Uconfig2DTable::writeUconfig(const char* filename,
                                  UconfigFile* config,
                                  const char* rowDelimiter,
                                  const char* columnDelimiter)
{
    if (!config)
        return false;

    FILE* outputFile = fopen(filename, "w");
    if (!outputFile)
        return false;

    bool success = true;
    UconfigEntryObject* entryList = config->rootEntry.subentries();
    for (int i=0; i<config->rootEntry.subentryCount(); i++)
    {
        success = Uconfig_fwrite2DTableEntry(outputFile,
                                             entryList[i],
                                             rowDelimiter,
                                             columnDelimiter);
        if (!success)
            break;
    }

    if (entryList)
        delete[] entryList;
    fclose(outputFile);

    return success;
}

// Parse an expression containing values separated by delimiters
int Uconfig2DTable::parseValues(const char* expression,
                                UconfigEntryObject& entry,
                                int expressionLength,
                                const char* delimiter,
                                bool skipEmptyValue,
                                ValueType defaultType)
{
    if (!expression)
        return 0;
    if (!delimiter)
        delimiter = UCONFIG_IO_2DTABLE_DELIMITER_COL;

    int p1 = 0, p2, substrLen;
    int keyCount = 0;
    int delimiterLength = strlen(delimiter);
    UconfigKeyObject tempKey;

    while (p1 < expressionLength)
    {
        substrLen = Uconfig_strpos(&expression[p1], delimiter);
        p2 = p1 + substrLen;
        if (substrLen < 0 || p2 + 1 >= expressionLength)
        {
            if (p1 == 0)
            {
                // No delimiter presents in the expression, halting.
                break;
            }
            substrLen = expressionLength - p1;
            p2 = p1 + substrLen;
        }

        if (substrLen > 0 || !skipEmptyValue)
        {
            tempKey.reset();
            tempKey.setType(defaultType);
            if (defaultType == ValueType::Chars &&
                expression[p1] == '"' &&
                expression[p1 + substrLen - 1] == '"')
            {
                // Ignore wrapping quotes when storing
                tempKey.setValue(&expression[p1 + 1],
                                 substrLen - sizeof(char) * 2);
            }
            else
                tempKey.setValue(&expression[p1], substrLen);
            entry.setType(Uconfig2DTable::Row);
            entry.addKey(&tempKey);
            keyCount++;
        }

        p1 = p2 + delimiterLength;
    }

    return keyCount;
}

bool Uconfig_fwrite2DTableEntry(FILE* file,
                                UconfigEntryObject& entry,
                                const char* rowDelimiter,
                                const char* columnDelimiter)
{
    if (entry.subentryCount() < 1)
        return true;

    int i, j;
    int entryType = entry.type();
    int delimiterLength = strlen(rowDelimiter);
    UconfigKeyObject* keyList = NULL;
    UconfigEntryObject* subentryList = entry.subentries();

    if (entryType == Uconfig2DTable::NormalEntry)
    {
        for (i=0; i<entry.subentryCount(); i++)
        {
            if (subentryList[i].type() == Uconfig2DTable::Row)
            {
                // Write keys as columns
                Uconfig_fwrite2DTableSubentry(file,
                                              subentryList[i],
                                              columnDelimiter);
            }
            else
            {
                // Write raw content directly
                keyList = subentryList[i].keys();
                for (j=0; j<subentryList[i].keyCount(); j++)
                {
                    fwrite(keyList[0].value(),
                           sizeof(char),
                           keyList[0].valueSize(),
                           file);
                }
                delete[] keyList;
            }
            fwrite(rowDelimiter, sizeof(char), delimiterLength, file);
        }
    }
    else if (entryType == Uconfig2DTable::CommentEntry)
    {
        for (i=0; i<entry.subentryCount(); i++)
        {
            // Write raw content directly
            keyList = subentryList[i].keys();
            for (j=0; j<subentryList[i].keyCount(); j++)
            {
                fwrite(keyList[0].value(),
                       sizeof(char),
                       keyList[0].valueSize(),
                       file);
            }
            delete[] keyList;

            fwrite(rowDelimiter, sizeof(char), delimiterLength, file);
        }
    }

    if (subentryList)
        delete[] subentryList;
    return true;
}

bool Uconfig_fwrite2DTableSubentry(FILE* file,
                                   UconfigEntryObject& subentry,
                                   const char* columnDelimiter)
{
    if (subentry.keyCount() > 0)
    {
        int delimiterLength = strlen(columnDelimiter);
        UconfigKeyObject* keyList = subentry.keys();
        for (int i=0; i<subentry.keyCount(); i++)
        {
            fwrite(keyList[i].value(),
                   sizeof(char),
                   keyList[i].valueSize(),
                   file);
            if (i + 1 < subentry.keyCount())
            {
                // Only write column delimiters between values
                fwrite(columnDelimiter,
                       sizeof(char),
                       delimiterLength,
                       file);
            }
        }
        delete[] keyList;
    }

    return true;
}
