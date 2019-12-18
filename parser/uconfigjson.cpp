#include <stdio.h>
#include <string.h>
#include <vector>
#include "uconfigjson.h"
#include "uconfigjson_p.h"
#include "uconfigfile_metadata.h"
#include "utils.h"

#define UCONFIG_IO_JSON_CHAR_OBJECT_BEGIN       '{'
#define UCONFIG_IO_JSON_CHAR_OBJECT_END         '}'
#define UCONFIG_IO_JSON_CHAR_ARRAY_BEGIN        '['
#define UCONFIG_IO_JSON_CHAR_ARRAY_END          ']'
#define UCONFIG_IO_JSON_CHAR_DEFINITION         ':'
#define UCONFIG_IO_JSON_CHAR_ELEMENT_NEXT       ','
#define UCONFIG_IO_JSON_CHAR_STRING             '"'
#define UCONFIG_IO_JSON_DELIMITER_NEWLINE       "\n"
#define UCONFIG_IO_JSON_DELIMITER_DEFINITION    ": "


bool UconfigJSON::readUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    bool success = UconfigJSONPrivate::freadEntry(inputFile,
                                                  config->rootEntry) > 0;

    if (success)
    {
        config->rootEntry.setType(UconfigJSON::ObjectEntry);

        // Add meta-data
        UconfigKeyObject tempKey;
        /* Basic information */
        tempKey.reset();
        tempKey.setName(UCONFIG_METADATA_KEY_FILENAME);
        tempKey.setType(UconfigValueType::Chars);
        tempKey.setValue(filename, strlen(filename) + 1);
        config->metadata.addKey(&tempKey);
        tempKey.reset();
        tempKey.setName(UCONFIG_METADATA_KEY_FILETYPE);
        tempKey.setType(UconfigValueType::Chars);
        tempKey.setValue(UCONFIG_METADATA_VALUE_JSON,
                         strlen(UCONFIG_METADATA_VALUE_JSON) + 1);
        config->metadata.addKey(&tempKey);
    }

    fclose(inputFile);
    return success;
}

bool UconfigJSON::writeUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* outputFile = fopen(filename, "w");
    if (!outputFile)
        return false;

    bool success = true;
    int nlDLength = strlen(UCONFIG_IO_JSON_DELIMITER_NEWLINE);

    // According to specification, a JSON file must contain either
    // exactly one value or exactly one obejct, or simply be empty.
    // Here, we will output all the values and objects attached to the root.
    UconfigKeyObject* keyList = config->rootEntry.keys();
    if (keyList)
    {
        for (int i=0; i<config->rootEntry.keyCount(); i++)
        {
            if (i > 0)
                fputc(UCONFIG_IO_JSON_CHAR_ELEMENT_NEXT, outputFile);

            success &= UconfigJSONPrivate::fwriteEntry(outputFile,
                                                       config->rootEntry) > 0;
            fwrite(UCONFIG_IO_JSON_DELIMITER_NEWLINE,
                   sizeof(char), nlDLength, outputFile);
        }
        delete[] keyList;
    }

    UconfigEntryObject* entryList = config->rootEntry.subentries();
    if (entryList)
    {
        for (int i=0; i<config->rootEntry.subentryCount(); i++)
        {
            if (i > 0)
                fputc(UCONFIG_IO_JSON_CHAR_ELEMENT_NEXT, outputFile);

            success &= UconfigJSONPrivate::fwriteEntry(outputFile,
                                                       entryList[i]) > 0;
            fwrite(UCONFIG_IO_JSON_DELIMITER_NEWLINE,
                   sizeof(char), nlDLength, outputFile);
        }
        delete[] entryList;
    }

    fclose(outputFile);
    return success;
}


UconfigValueType UconfigJSONPrivate::getValueType(const char* value,
                                                  int length)
{
    return UconfigValueType::Chars;
}

int UconfigJSONPrivate::freadEntry(FILE* file,
                                   UconfigEntryObject& entry)
{
    int retValue;
    int parsedLen = 0;
    bool finished = false;
    bool nextElement = false;
    bool parsingString = false;
    char bufferChar;
    std::vector<char> buffer, elementName;
    UconfigValueType valueType;
    UconfigKeyObject tempKey;
    UconfigEntryObject tempSubentry;

    entry.reset();
    while (true)
    {
        // Read from file char by char
        retValue = fgetc(file);
        if (retValue == -1)
        {
            // EOF
            nextElement = true;
            finished = true;
        }
        else
        {
            parsedLen++;
            bufferChar = char(retValue);
            if (parsingString)
            {
                buffer.push_back(bufferChar);
                if (bufferChar == UCONFIG_IO_JSON_CHAR_STRING)
                    parsingString = false;
            }
            else
            switch (bufferChar)
            {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    break;
                case UCONFIG_IO_JSON_CHAR_ARRAY_BEGIN:
                    retValue = freadEntry(file, tempSubentry);
                    if (retValue > 0)
                    {
                        tempSubentry.setType(UconfigJSON::ArrayEntry);
                        parsedLen += retValue;
                    }
                    break;
                case UCONFIG_IO_JSON_CHAR_OBJECT_BEGIN:
                    retValue = freadEntry(file, tempSubentry);
                    if (retValue > 0)
                    {
                        tempSubentry.setType(UconfigJSON::ObjectEntry);
                        parsedLen += retValue;
                    }
                    break;
                case UCONFIG_IO_JSON_CHAR_ARRAY_END:
                case UCONFIG_IO_JSON_CHAR_OBJECT_END:
                    finished = true;
                case UCONFIG_IO_JSON_CHAR_ELEMENT_NEXT:
                    nextElement = true;
                    break;
                case UCONFIG_IO_JSON_CHAR_DEFINITION:
                    elementName = buffer;
                    elementName.push_back('\0');
                    buffer.clear();
                    break;
                case UCONFIG_IO_JSON_CHAR_STRING:
                    parsingString = true;
                default:
                    buffer.push_back(bufferChar);
            }
        }

        if (nextElement)
        {
            if (tempSubentry.type() == UconfigJSON::UnknownEntry)
            {
                // Non object/array value: store it as a key
                valueType = getValueType(buffer.data(), buffer.size());
                switch (valueType)
                {
                    case UconfigValueType::Bool:
                    case UconfigValueType::Chars:
                    case UconfigValueType::Integer:
                    case UconfigValueType::Float:
                    case UconfigValueType::Double:
                        tempKey.setType(UconfigValueType::Chars);
                        tempKey.setValue(buffer.data(), buffer.size());
                        break;
                    default:
                        tempKey.setType(UconfigValueType::Raw);
                        tempKey.setValue(buffer.data(), buffer.size());
                }
                tempKey.setName(elementName.data());
                entry.addKey(&tempKey);
                tempKey.reset();
                buffer.clear();
            }
            else
            {
                tempSubentry.setName(elementName.data());
                entry.addSubentry(&tempSubentry);
            }

            elementName.clear();
            tempSubentry.setType(UconfigJSON::UnknownEntry);

            // Reset the flag for next iteration of parsing
            nextElement = false;
        }
        if (finished)
            break;
    }

    return parsedLen;
}

bool UconfigJSONPrivate::fwriteEntry(FILE* file,
                                     UconfigEntryObject& entry,
                                     int level)
{
    int i;
    int nlDLength = strlen(UCONFIG_IO_JSON_DELIMITER_NEWLINE);
    int dDLength = strlen(UCONFIG_IO_JSON_DELIMITER_DEFINITION);

    UconfigJSON::EntryType entryType = UconfigJSON::EntryType(entry.type());

    // The opening tag
    switch (entryType)
    {
        case UconfigJSON::ArrayEntry:
            fputc(UCONFIG_IO_JSON_CHAR_ARRAY_BEGIN, file);
            break;
        case UconfigJSON::ObjectEntry:
            fputc(UCONFIG_IO_JSON_CHAR_OBJECT_BEGIN, file);
            break;
        default:;
    }

    // First write the values of keys
    UconfigKeyObject* keyList = entry.keys();
    if (keyList)
    {
        for (i=0; i<entry.keyCount(); i++)
        {
            if (i > 0)
                fputc(UCONFIG_IO_JSON_CHAR_ELEMENT_NEXT, file);

            fwrite(UCONFIG_IO_JSON_DELIMITER_NEWLINE,
                   sizeof(char), nlDLength, file);
            Uconfig_fwriteIndentation(file, level + 1);

            // Write the name of the key if any
            if (keyList[i].name())
            {
                fwrite(keyList[i].name(),
                       sizeof(char),
                       strlen(keyList[i].name()),
                       file);
                fwrite(UCONFIG_IO_JSON_DELIMITER_DEFINITION,
                       sizeof(char), dDLength, file);
            }

            // Write the value of the key
            fwrite(keyList[i].value(),
                   sizeof(char),
                   keyList[i].valueSize(),
                   file);
        }

        delete[] keyList;
    }

    // Then write subentries as arrays and objects
    UconfigEntryObject* subentryList = entry.subentries();
    if (subentryList)
    {
        // Add an additional comma between keys and subentries
        // if necessary
        if (entry.keyCount() > 0)
            fputc(UCONFIG_IO_JSON_CHAR_ELEMENT_NEXT, file);

        for (i=0; i<entry.subentryCount(); i++)
        {
            if (i > 0)
                fputc(UCONFIG_IO_JSON_CHAR_ELEMENT_NEXT, file);

            fwrite(UCONFIG_IO_JSON_DELIMITER_NEWLINE,
                   sizeof(char), nlDLength, file);
            Uconfig_fwriteIndentation(file, level + 1);

            // Write the name of the subentry if any and
            // if we are in an object
            if (subentryList[i].name() &&
                entry.type() == UconfigJSON::ObjectEntry)
            {
                fwrite(subentryList[i].name(),
                       sizeof(char),
                       strlen(subentryList[i].name()),
                       file);
                fwrite(UCONFIG_IO_JSON_DELIMITER_DEFINITION,
                       sizeof(char), dDLength, file);
            }

            // Write the body of the subentry
            fwriteEntry(file, subentryList[i], level + 1);
        }

        delete[] subentryList;
    }

    // The closing tag
    fwrite(UCONFIG_IO_JSON_DELIMITER_NEWLINE,
           sizeof(char), nlDLength, file);
    Uconfig_fwriteIndentation(file, level);
    switch (entryType)
    {
        case UconfigJSON::ArrayEntry:
            fputc(UCONFIG_IO_JSON_CHAR_ARRAY_END, file);
            break;
        case UconfigJSON::ObjectEntry:
            fputc(UCONFIG_IO_JSON_CHAR_OBJECT_END, file);
            break;
        default:;
    }

    return true;
}
