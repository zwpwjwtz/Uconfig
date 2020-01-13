#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "uconfigxml.h"
#include "uconfigxml_p.h"
#include "uconfigfile_metadata.h"
#include "utils.h"

// Chars: for stream parsing only
#define UCONFIG_IO_XML_CHAR_TAG_BEGIN           '<'
#define UCONFIG_IO_XML_CHAR_TAG_MID             '/'
#define UCONFIG_IO_XML_CHAR_TAG_END             '>'
#define UCONFIG_IO_XML_CHAR_KEY_DEFINITION      '='
#define UCONFIG_IO_XML_CHAR_STRING              '"'

// Delimitors as strings
#define UCONFIG_IO_XML_DELIMITER_NEWLINE        "\n"
#define UCONFIG_IO_XML_DELIMITER_KEYVAL         "="
#define UCONFIG_IO_XML_DELIMITER_ATTRIBUTE      " "
#define UCONFIG_IO_XML_DELIMITER_TAG_BEGINOPEN  "<"
#define UCONFIG_IO_XML_DELIMITER_TAG_BEGINCLOSE "</"
#define UCONFIG_IO_XML_DELIMITER_TAG_END        ">"
#define UCONFIG_IO_XML_DELIMITER_TAG_ENDSELF    " />"
#define UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN  "<!--"
#define UCONFIG_IO_XML_DELIMITER_COMMENT_END    "-->"
#define UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN    "<![CDATA["
#define UCONFIG_IO_XML_DELIMITER_CDATA_END      "]]>"
#define UCONFIG_IO_XML_DELIMITER_XML_BEGIN      "<?xml"
#define UCONFIG_IO_XML_DELIMITER_XML_END        "?>"
#define UCONFIG_IO_XML_DELIMITER_DOCTYPE_BEGIN  "<!DOCTYPE"
#define UCONFIG_IO_XML_DELIMITER_DOCTYPE_END    ">"
#define UCONFIG_IO_XML_NAME_XML                 "?xml"
#define UCONFIG_IO_XML_NAME_DOCTYPE             "!DOCTYPE"

#define UCONFIG_IO_XML_BUFFER_MIN               8


bool UconfigXML::readUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    bool success = UconfigXMLPrivate::freadEntry(inputFile,
                                                  config->rootEntry) > 0;

    if (success)
    {
        config->rootEntry.setType(UconfigXML::NormalEntry);

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

bool UconfigXML::writeUconfig(const char* filename, UconfigFile* config)
{
    if (!config)
        return false;

    FILE* outputFile = fopen(filename, "w");
    if (!outputFile)
        return false;

    bool success = true;
    if (config->rootEntry.subentryCount() > 0)
    {
        UconfigEntryObject* entryList = config->rootEntry.subentries();
        for (int i=0; i<config->rootEntry.subentryCount(); i++)
        {
            success &= UconfigXMLPrivate::fwriteEntry(outputFile,
                                                      entryList[i]) > 0;
        }
        delete[] entryList;
    }

    fclose(outputFile);
    return success;
}


UconfigValueType UconfigXMLPrivate::getValueType(const char* value,
                                                  int length)
{
    return UconfigValueType::Chars;
}

int UconfigXMLPrivate::freadEntry(FILE* file,
                                   UconfigEntryObject& entry)
{
    int retValue;
    int parsedLen = 0;
    bool nextKey = false;
    bool nextElement = false;
    bool preOpening = false;
    bool closing = false;
    bool parsingString = false;
    char bufferChar;
    std::vector<char> buffer, keyName;
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
            break;
        }

        parsedLen++;
        bufferChar = char(retValue);
        if (parsingString)
        {
            // Quit string parsing mode
            buffer.push_back(bufferChar);
            if (bufferChar == UCONFIG_IO_XML_CHAR_STRING)
                parsingString = false;
        }
        else
        switch (bufferChar)
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                // White spaces: possible delimitors for attributes
                nextKey = true;
                break;
            case UCONFIG_IO_XML_CHAR_STRING:
                // Enter string parsing mode
                parsingString = true;
                buffer.push_back(bufferChar);
                break;
            case UCONFIG_IO_XML_CHAR_TAG_BEGIN:
                // Both opening and closing tag are possible
                // We need to read one more char to determine
                preOpening = true;
                break;
            case UCONFIG_IO_XML_CHAR_TAG_END:
                nextKey = true;
                nextElement = true;
                break;
            case UCONFIG_IO_XML_CHAR_TAG_MID:
                nextKey = true;
                closing = true;
                break;
            case UCONFIG_IO_XML_CHAR_KEY_DEFINITION:
                keyName = buffer;
                keyName.push_back('\0');
                buffer.clear();
                break;
            default:
                buffer.push_back(bufferChar);
        }

        if (preOpening)
        {
            // Determine the tag's nature from its left part
            fseek(file, -1, SEEK_CUR);
            parsedLen--;

            retValue = parseComment(file, tempSubentry);
            if (retValue <= 0)
                retValue = parseCDATA(file, tempSubentry);
            if (retValue <= 0)
                retValue = parseXMLDelcaration(file, tempSubentry);
            if (retValue <= 0)
                retValue = parseDoctype(file, tempSubentry);

            if (retValue <= 0)
            {
                // Normal element
                // Try to read one more char to determine its nature

                fseek(file, 1, SEEK_CUR);
                parsedLen++;

                retValue = fgetc(file);
                if (retValue == -1)
                {
                    // EOF
                    break;
                }
                parsedLen++;
                bufferChar = char(retValue);

                if (bufferChar == UCONFIG_IO_XML_CHAR_TAG_MID)
                {
                    // Closing tag of an element
                    closing = true;
                }
                else
                {
                    // Opening tag of an element, ignore it
                    fseek(file, -1, SEEK_CUR);
                    parsedLen--;

                    retValue = freadEntry(file, tempSubentry);
                    if (retValue > 0)
                    {
                        tempSubentry.setType(UconfigXML::NormalEntry);
                        entry.addSubentry(&tempSubentry);
                        parsedLen += retValue;
                    }
                }
            }
            else
            {
                entry.addSubentry(&tempSubentry);
                parsedLen += retValue;
            }

            preOpening = false;
        }

        if (nextKey)
        {
            if (buffer.size() > 0)
            {
                // Deal with element names, attribute names and values
                if (!entry.name())
                {
                    buffer.push_back('\0');
                    entry.setName(buffer.data());
                }
                else if (keyName.size() == 0)
                {
                    keyName = buffer;
                    keyName.push_back('\0');
                }
                else
                {
                    valueType = getValueType(buffer.data(), buffer.size());
                    switch (valueType)
                    {
                        case UconfigValueType::Bool:
                        case UconfigValueType::Chars:
                        case UconfigValueType::Integer:
                        case UconfigValueType::Float:
                        case UconfigValueType::Double:
                            tempKey.setType(UconfigValueType::Chars);
                            break;
                        default:
                            tempKey.setType(UconfigValueType::Raw);
                    }
                    tempKey.setName(keyName.data());
                    tempKey.setValue(buffer.data(), buffer.size());
                    entry.addKey(&tempKey);
                    keyName.clear();
                }

                buffer.clear();
            }

            nextKey = false;
        }

        if (nextElement)
        {
            // Deal with the right part of opening/closing tags
            if (closing)
            {
                if (buffer.size() > 0)
                {
                    // Extra syntax check for non self-closing tags:
                    // See if the name in the closing tag matches
                    // that in the opening tag
                    buffer.push_back('\0');
                    if (strcmp(entry.name(), buffer.data()) != 0)
                    {
                        // Tags not matching; ignored
                        // Further error information should be printed
                    }
                }
                break;
            }

            nextElement = false;
        }
    }

    return parsedLen;
}

bool UconfigXMLPrivate::fwriteEntry(FILE* file,
                                    UconfigEntryObject& entry,
                                    int level)
{
    int i;
    const int nlDLength = strlen(UCONFIG_IO_XML_DELIMITER_NEWLINE);
    const int tboDLength = strlen(UCONFIG_IO_XML_DELIMITER_TAG_BEGINOPEN);
    const int tbcDLength = strlen(UCONFIG_IO_XML_DELIMITER_TAG_BEGINCLOSE);
    const int teDLength = strlen(UCONFIG_IO_XML_DELIMITER_TAG_END);
    const int kvDlength = strlen(UCONFIG_IO_XML_DELIMITER_KEYVAL);

    if (entry.type() == UconfigXML::NormalEntry ||
        entry.type() == UconfigXML::DoctypeEntry ||
        entry.type() == UconfigXML::XMLDeclEntry)
    {
        // First write the opening tag of the entry
        Uconfig_fwriteIndentation(file, level);
        fwrite(UCONFIG_IO_XML_DELIMITER_TAG_BEGINOPEN,
               sizeof(char), tboDLength, file);
        if (entry.type() == UconfigXML::NormalEntry)
            fwrite(entry.name(), sizeof(char), strlen(entry.name()), file);

        // Then write the attributes (keys)
        UconfigKeyObject* keyList = entry.keys();
        if (keyList)
        {
            for (i=0; i<entry.keyCount(); i++)
            {
                fwrite(UCONFIG_IO_XML_DELIMITER_ATTRIBUTE,
                       sizeof(char),
                       strlen(UCONFIG_IO_XML_DELIMITER_ATTRIBUTE),
                       file);

                // Write the name of the attribute if any
                if (keyList[i].name())
                {
                    fwrite(keyList[i].name(),
                           sizeof(char),
                           strlen(keyList[i].name()),
                           file);
                }

                // Write the value of the attribute if any
                if (keyList[i].value())
                {
                    fwrite(UCONFIG_IO_XML_DELIMITER_KEYVAL,
                           sizeof(char), kvDlength, file);
                    fwrite(keyList[i].value(),
                           sizeof(char),
                           keyList[i].valueSize(),
                           file);
                }
            }

            delete[] keyList;
        }

        // Then write subentries
        UconfigEntryObject* subentryList = entry.subentries();
        if (subentryList)
        {
            fwrite(UCONFIG_IO_XML_DELIMITER_TAG_END,
                   sizeof(char), teDLength, file);

            for (i=0; i<entry.subentryCount(); i++)
            {
                fwrite(UCONFIG_IO_XML_DELIMITER_NEWLINE,
                       sizeof(char), nlDLength, file);
                fwriteEntry(file, subentryList[i], level + 1);
            }

            // Write an additional newline if the body is no empty
            fwrite(UCONFIG_IO_XML_DELIMITER_NEWLINE,
                   sizeof(char), nlDLength, file);

            // Finally write the closing tag of the entry
            Uconfig_fwriteIndentation(file, level);
            fwrite(UCONFIG_IO_XML_DELIMITER_TAG_BEGINCLOSE,
                   sizeof(char), tbcDLength, file);
            fwrite(entry.name(), sizeof(char), strlen(entry.name()), file);
            fwrite(UCONFIG_IO_XML_DELIMITER_TAG_END,
                   sizeof(char), teDLength, file);

            delete[] subentryList;
        }
        else
        {
            // Use self-closing tag
            fwrite(UCONFIG_IO_XML_DELIMITER_TAG_ENDSELF,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_TAG_ENDSELF),
                   file);
        }
    }
    else if (entry.type() == UconfigXML::CommentEntry)
    {
        Uconfig_fwriteIndentation(file, level);
        fwrite(UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN,
               sizeof(char),
               strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN),
               file);

        UconfigKeyObject* keyList = entry.keys();
        if (keyList)
        {
            for (i=0; i<entry.keyCount(); i++)
            {
                // Write an additional newline between comments
                if (i > 0)
                {
                    fwrite(UCONFIG_IO_XML_DELIMITER_NEWLINE,
                           sizeof(char), nlDLength, file);
                }

                // Write the value of the key directly
                if (keyList[i].value())
                {
                    fwrite(keyList[i].value(),
                           sizeof(char),
                           keyList[i].valueSize(),
                           file);
                }
            }
            delete[] keyList;
        }

        fwrite(UCONFIG_IO_XML_DELIMITER_COMMENT_END,
               sizeof(char),
               strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_END),
               file);
    }
    else if (entry.type() == UconfigXML::CDATAEntry)
    {
        fwrite(UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN,
               sizeof(char),
               strlen(UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN),
               file);

        UconfigKeyObject* keyList = entry.keys();
        if (keyList)
        {
            for (i=0; i<entry.keyCount(); i++)
            {
                // Write the value of the key directly
                if (keyList[i].value())
                {
                    fwrite(keyList[i].value(),
                           sizeof(char),
                           keyList[i].valueSize(),
                           file);
                }
            }
            delete[] keyList;
        }

        fwrite(UCONFIG_IO_XML_DELIMITER_CDATA_END,
               sizeof(char),
               strlen(UCONFIG_IO_XML_DELIMITER_CDATA_END),
               file);
    }

    return true;
}

// Parse an attribute expression of format "NAME=VALUE"
// Return the number of chars(bytes) that have been parsed
int UconfigXMLPrivate::parseTagAttribute(const char* expression,
                                         UconfigKeyObject& key,
                                         int expressionLength)
{
    if (expressionLength <= 0)
        expressionLength = strlen(expression);

    // Try to find the delimitor between NAME and VALUE
    int pos = Uconfig_strpos(expression, UCONFIG_IO_XML_DELIMITER_KEYVAL);
    if (pos <= 0 || pos + 1 >= expressionLength)
        return 0;

    key.reset();

    // Extract attribute's name
    char* keyName = new char[pos + 1];
    Uconfig_strncpy(keyName, expression, pos);
    key.setName(keyName);
    delete keyName;

    // Try to find the end position of VALUE
    pos += strlen(UCONFIG_IO_XML_DELIMITER_KEYVAL);
    int pos2 = pos;
    bool end = false;
    bool parsingString = false;
    while (pos2 < expressionLength)
    {
        if (parsingString)
        {
            if (expression[pos2] == '"' || expression[pos2] == '\'')
                parsingString = false;
        }
        else
        switch (expression[pos2])
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '/':
            case '>':
                end = true;
                break;
            case '"':
            case '\'':
                parsingString = true;
                break;
            default:;
        }
        if (end)
            break;

        pos2++;
    }
    // Extract attribute's value
    if (pos2 > pos)
    {
        key.setType(UconfigValueType::Raw);
        key.setValue(&expression[pos], pos2 - pos);
    }

    return pos2;
}

// Try to parse a comment section from the current reading position
// of the file, with a constraint of maximum parsing length.
// Any parsed comment is stored as a key attached to the entry.
int UconfigXMLPrivate::parseComment(FILE* file,
                                    UconfigEntryObject& entry,
                                    int maxLength)
{
    if (!file)
        return 0;

    // Check the opening part of the comment section
    int readLength = Uconfig_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::CommentEntry);

    // Find the closing part of the comment section
    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = Uconfig_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_COMMENT_END,
                                     file);
    if (contentLength > 0)
    {
        // Extract the comment
        UconfigKeyObject key;
        key.setType(UconfigValueType::Raw);
        key.setValue(buffer, contentLength);
        entry.addKey(&key);
        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    // Checking the closing part of the comment section,
    // even we have already searched for it in Uconfig_getdelim().

    if (Uconfig_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_COMMENT_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_END)) < 0)
    {
        // Valid closing part
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_END);
    }
    else
    {
        // Premature ending: the comment section is incomplete
        // We shall report this error in the future
    }

    return readLength;
}

// Try to parse a CDATA section from the file.
// Similar to the function UconfigXMLPrivate::parseComment().
// See its comment for detailed explanation
int UconfigXMLPrivate::parseCDATA(FILE* file,
                                  UconfigEntryObject& entry,
                                  int maxLength)
{
    if (!file)
        return 0;

    int readLength = Uconfig_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::CDATAEntry);

    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = Uconfig_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_CDATA_END,
                                     file);
    if (contentLength > 0)
    {
        UconfigKeyObject key;
        key.setType(UconfigValueType::Raw);
        key.setValue(buffer, contentLength);
        entry.addKey(&key);
        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    if (Uconfig_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_CDATA_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_CDATA_END)) < 0)
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_CDATA_END);

    return readLength;
}

// Try to parse a XML declaration section from the file.
// Similar to the function UconfigXMLPrivate::parseComment().
int UconfigXMLPrivate::parseXMLDelcaration(FILE* file,
                                           UconfigEntryObject& entry,
                                           int maxLength)
{
    if (!file)
        return 0;

    int readLength = Uconfig_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_XML_BEGIN);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::XMLDeclEntry);

    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = Uconfig_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_XML_END,
                                     file);
    if (contentLength > 0)
    {
        // Try to parse attributes in the tag one by one
        int pos = 0;
        int attributeLength;
        UconfigKeyObject key;
        while (true)
        {
            attributeLength = parseTagAttribute(&buffer[pos],
                                                key,
                                                contentLength - pos);
            if (attributeLength > 0)
            {
                entry.addKey(&key);
                pos += attributeLength;
            }
            else
                break;
        }

        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    if (Uconfig_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_XML_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_XML_END)) < 0)
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_XML_END);

    return readLength;
}

// Try to parse a DOCTYPE section from the file.
// Similar to the function UconfigXMLPrivate::parseXMLDelcaration().
// See its comment for detailed explanation
int UconfigXMLPrivate::parseDoctype(FILE* file,
                                    UconfigEntryObject& entry,
                                    int maxLength)
{
    if (!file)
        return 0;

    int readLength = Uconfig_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_DOCTYPE_BEGIN);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::DoctypeEntry);

    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = Uconfig_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_DOCTYPE_END,
                                     file);
    if (contentLength > 0)
    {
        int pos = 0;
        int attributeLength;
        UconfigKeyObject key;
        while (true)
        {
            attributeLength = parseTagAttribute(&buffer[pos],
                                                key,
                                                contentLength - pos);
            if (attributeLength > 0)
            {
                entry.addKey(&key);
                pos += attributeLength;
            }
            else
                break;
        }

        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    if (Uconfig_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_DOCTYPE_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_DOCTYPE_END)) < 0)
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_DOCTYPE_END);

    return readLength;
}
