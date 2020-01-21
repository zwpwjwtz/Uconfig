#include <stdio.h>
#include <stdlib.h>
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

#define UCONFIG_IO_JSON_EXPRESSION_BOOL_FALSE   "false"
#define UCONFIG_IO_JSON_EXPRESSION_BOOL_TRUE    "true"


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

// Normally, all value names in JSON must be wrapped in a pair of quotes
// Here, we also accept strings without quotes as value names
bool UconfigJSONKey::parseName(const char* expression, int length)
{
    if (length > 0 &&
        expression[0] == UCONFIG_IO_JSON_CHAR_STRING &&
        expression[length - 1] == UCONFIG_IO_JSON_CHAR_STRING)
    {
        setName(&expression[1], length - 2);
    }
    else
    {
        setName(expression, length);
    }
    return true;
}

bool UconfigJSONKey::parseValue(const char* expression, int length)
{
    if (!expression)
        return false;

    char* pTail;

    UconfigValueType valueType = guessValueType(expression, length);
    switch (valueType)
    {
        case UconfigValueType::Bool:
        {
            bool tempBool = strcasestr(expression,
                                       UCONFIG_IO_JSON_EXPRESSION_BOOL_TRUE)
                            == expression;
            setValue((char*)(&tempBool), sizeof(bool));
            break;
        }
        case UconfigValueType::Chars:
        {
            // Ignore wrapping quotes when storing
            setValue(&expression[1], length - sizeof(char) * 2);
            break;
        }
        case UconfigValueType::Integer:
        {
            // Store the number as an "int"
            int tempInt = int(strtol(expression, &pTail, 0));
            if (pTail > expression)
                setValue((char*)(&tempInt), sizeof(int));
            break;
        }
        case UconfigValueType::Float:
        case UconfigValueType::Double:
        {
            // Store the number as a "double"
            double tempDouble = strtod(expression, &pTail);
            if (pTail > expression)
                setValue((char*)(&tempDouble), sizeof(double));
            break;
        }
        default:
            setValue(expression, length);
    }

    setType(valueType);
    return true;
}

int UconfigJSONKey::fwriteName(FILE *file)
{
    const UconfigKey& data = refData ? *refData : propData;

    fputc(UCONFIG_IO_JSON_CHAR_STRING, file);
    fwrite(data.name, sizeof(char), data.nameSize, file);
    fputc(UCONFIG_IO_JSON_CHAR_STRING, file);
}

int UconfigJSONKey::fwriteValue(FILE* file)
{
    char* buffer = NULL;

    int length = 0;
    switch (UconfigValueType(type()))
    {
        case UconfigValueType::Bool:
        {
            const char* boolExp = *(bool*)(value()) ?
                             UCONFIG_IO_JSON_EXPRESSION_BOOL_TRUE :
                             UCONFIG_IO_JSON_EXPRESSION_BOOL_FALSE;
            fwrite(boolExp, strlen(boolExp), sizeof(char), file);
            break;
        }
        case UconfigValueType::Chars:
            // Wrap string with quotes
            fputc(UCONFIG_IO_JSON_CHAR_STRING, file);
            fwrite(value(), valueSize(), sizeof(char), file);
            fputc(UCONFIG_IO_JSON_CHAR_STRING, file);
            break;
        case UconfigValueType::Integer:
            length = asprintf(&buffer, "%d", *(int*)(value()));
            fwrite(buffer, length, sizeof(char), file);
            break;
        case UconfigValueType::Float:
            length = asprintf(&buffer, "%f", *(float*)(value()));
            fwrite(buffer, length, sizeof(char), file);
            break;
        case UconfigValueType::Double:
            length = asprintf(&buffer, "%f", *(double*)(value()));
            fwrite(buffer, length, sizeof(char), file);
            break;
        default:
            fwrite(value(), valueSize(), sizeof(char), file);
    }
    if (buffer)
        free(buffer);

    return length;
}


bool UconfigJSONEntry::parseName(const char* expression, int length)
{
    if (length > 0 &&
        expression[0] == UCONFIG_IO_JSON_CHAR_STRING &&
        expression[length - 1] == UCONFIG_IO_JSON_CHAR_STRING)
    {
        setName(&expression[1], length - 2);
    }
    else
    {
        setName(expression, length);
    }
    return true;
}

int UconfigJSONEntry::fwriteName(FILE *file)
{
    const UconfigEntry& data = refData ? *refData : propData;

    fputc(UCONFIG_IO_JSON_CHAR_STRING, file);
    fwrite(data.name, sizeof(char), data.nameSize, file);
    fputc(UCONFIG_IO_JSON_CHAR_STRING, file);
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
    UconfigJSONKey tempKey;
    UconfigJSONEntry tempSubentry;

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
                tempKey.parseName(elementName.data(), elementName.size());
                tempKey.parseValue(buffer.data(), buffer.size());
                entry.addKey(&tempKey);
                buffer.clear();
            }
            else
            {
                tempSubentry.parseName(elementName.data(), elementName.size());
                entry.addSubentry(&tempSubentry);
            }

            tempKey.reset();
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
    UconfigJSONKey* keyList = (UconfigJSONKey*)(entry.keys());
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
                keyList[i].fwriteName(file);
                fwrite(UCONFIG_IO_JSON_DELIMITER_DEFINITION,
                       sizeof(char), dDLength, file);
            }

            // Write the value of the key
            keyList[i].fwriteValue(file);
        }

        delete[] keyList;
    }

    // Then write subentries as arrays and objects
    UconfigJSONEntry* subentryList = (UconfigJSONEntry*)(entry.subentries());
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
                subentryList[i].fwriteName(file);
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
