#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

#define UCONFIG_UTILS_LINEDELIMITER_UNIX    "\n"
#define UCONFIG_UTILS_LINEDELIMITER_OSX     "\r"
#define UCONFIG_UTILS_LINEDELIMITER_WIN     "\r\n"
#define UCONFIG_UTILS_INDENTATION_SPACE     "    "
#define UCONFIG_UTILS_INDENTATION_TAB       "   "

#define UCONFIG_UTILS_FILE_BUFFER_MAX   1024


int Uconfig_strpos(const char* haystack, const char* needle)
{
    const char* pos = strstr(haystack, needle);
    if (pos)
        return (int)(pos - haystack);
    else
        return -1;
}

int Uconfig_findLineDelimiter(const char* str)
{
    const char* search = UCONFIG_UTILS_LINEDELIMITER_UNIX;
    int pos = Uconfig_strpos(str, search);
    if (pos >= 0)
        return pos;

    search = UCONFIG_UTILS_LINEDELIMITER_OSX;
    pos = Uconfig_strpos(str, search);
    if (pos >= 0)
        return pos;

    search = UCONFIG_UTILS_LINEDELIMITER_WIN;
    pos = Uconfig_strpos(str, search);
    if (pos >= 0)
        return pos;

    return -1;
}

char* Uconfig_strnstr(const char* haystack, const char* needle, int length)
{
    const char* pos = strstr(haystack, needle);
    if (pos - haystack + strlen(needle) <= length)
        return (char*)(pos);
    else
        return NULL;
}

bool Uconfig_isspace(const char* str, int length)
{
    while (length > 0)
    {
        if (*str != ' ' || *str != '\0' ||
            *str != '\t' || *str != '\v'  ||
            *str != '\n' || *str != '\r' || *str != '\f')
            break;
        str++;
        length--;
    }
    return length == 0;
}

char* Uconfig_strncpy (char* dest, const char* src, int count)
{
    dest[count] = '\0';
    return strncpy(dest, src, count);
}

int Uconfig_getdelim(char** lineptr, int* n,
                     const char* delimiter, FILE* stream)
{
    int bufferSize;

    // See if the size limit is valid
    if (n != NULL && *n > 0)
        bufferSize = *n;
    else
        bufferSize = UCONFIG_UTILS_FILE_BUFFER_MAX;

    // Reallocating memory for the buffer
    if (*lineptr == NULL)
    {
        *lineptr = (char*)(malloc(bufferSize));
        if (*lineptr == NULL)
            return -1;
    }

    char* p1 = *lineptr;
    char* p2 = *lineptr + bufferSize;
    char* newBuffer;
    int newBufferSize;
    int allocationFailed = false;

    long readLength;
    const long delimiterLength = strlen(delimiter);
    const long seekLength = UCONFIG_UTILS_FILE_BUFFER_MAX + delimiterLength;
    char* buffer = (char*)(malloc(seekLength * sizeof(char)));
    char* pos;

    while (true)
    {
        readLength = fread(buffer, 1, seekLength, stream);
        if (readLength <= 0)
            break;

        pos = Uconfig_strnstr(buffer, delimiter, readLength);
        if (pos)
        {
            // See if we need to increase the size of the buffer
            if (p1 + (pos - buffer) + 1 >= p2)
            {
                if (n != NULL && *n > 0)
                {
                    // Size limited by caller; stop reading
                    Uconfig_strncpy(p1, buffer, p2 - p1 - 1);
                    p1 = p2;
                    break;
                }

                newBufferSize = bufferSize * 2;
                newBuffer = (char*)(realloc(*lineptr, newBufferSize));
                if (newBuffer == NULL)
                {
                    allocationFailed = true;
                    break;
                }
                p1 += newBuffer - *lineptr;
                p2 = newBuffer + newBufferSize;
                *lineptr = newBuffer;
                bufferSize = newBufferSize;
            }
            Uconfig_strncpy(p1, buffer, pos - buffer);
            p1 += pos - buffer;
            fseek(stream, pos - buffer + delimiterLength - readLength,
                  SEEK_CUR);
            break;
        }

        if (feof(stream))
            break;

        // See if we need to increase the size of the buffer
        if (p1 + readLength - delimiterLength + 1 >= p2)
        {
            if (n != NULL && *n > 0)
            {
                // Size limited by caller; stop reading
                Uconfig_strncpy(p1, buffer, p2 - p1 - 1);
                p1 = p2;
                break;
            }

            newBufferSize = bufferSize * 2;
            newBuffer = (char*)(realloc(*lineptr, newBufferSize));
            if (newBuffer == NULL)
            {
                readLength = 0;
                break;
            }
            p1 += newBuffer - *lineptr;
            p2 = newBuffer + newBufferSize;
            *lineptr = newBuffer;
            bufferSize = newBufferSize;
        }
        Uconfig_strncpy(p1, buffer, readLength - delimiterLength);
        p1 += readLength - delimiterLength;
        fseek(stream, -delimiterLength, SEEK_CUR);

    }
    free(buffer);

    if (allocationFailed)
        return -1;
    else
        return p1 - *lineptr;
}

int Uconfig_fpeek(FILE* stream, char* buffer, int n)
{
    int readLength;
    if (n >= 0)
    {
        readLength = fread(buffer, sizeof(char), n, stream);
        fseek(stream, -readLength, SEEK_CUR);
    }
    else
    {
        long oldPos = ftell(stream);
        fseek(stream, n, SEEK_CUR);
        long newPos = ftell(stream);
        readLength = -fread(buffer, sizeof(char), oldPos - newPos, stream);
    }
    return readLength;
}


int Uconfig_freadCmp(FILE *stream, const char* string, int n)
{
    if (n == 0)
    {
        n = strlen(string);
        if (n == 0)
            return 0;
    }
    else if (n < 0)
    {
        int oldPos = ftell(stream);
        if (fseek(stream, n, SEEK_CUR) == oldPos)
            return 0;
    }

    // Read stream by standard fread():
    // the size parameter and the return value must be positive
    char* buffer = new char[abs(n)];
    int readLength = fread(buffer, sizeof(char), abs(n), stream);
    if (strncmp(buffer, string, abs(n)) != 0)
        readLength = 0;

    delete[] buffer;
    return n >= 0 ? readLength : -readLength;
}

int Uconfig_fpeekCmp(FILE *stream, const char* string, int n)
{
    if (n == 0)
    {
        n = strlen(string);
        if (n == 0)
            return 0;
    }

    // Read stream by customized fpeek():
    // the size parameter and the return value could be negative
    char* buffer = new char[abs(n)];
    int readLength = Uconfig_fpeek(stream, buffer, n);
    if (strncmp(buffer, string, abs(n)) != 0)
        readLength = 0;

    delete[] buffer;
    return readLength;
}

int Uconfig_fwriteIndentation(FILE* __restrict stream,
                              int level,
                              bool usingTabs)
{
    int charCount = 0;
    const char* indentString = usingTabs ?
                               UCONFIG_UTILS_INDENTATION_TAB :
                               UCONFIG_UTILS_INDENTATION_SPACE;
    const int indentLength = strlen(indentString);

    while (level > 0)
    {
        level--;
        fwrite(indentString, sizeof(char), indentLength, stream);
        charCount += indentLength;
    }

    return charCount;
}
