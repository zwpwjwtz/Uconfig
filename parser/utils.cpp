#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
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
    char* pTail;
    char* newBuffer;
    int newBufferSize;
    int delimiterLength = strlen(delimiter);
    while (true)
    {
        // Read from stream char by char
        int c = fgetc(stream);
        if (c == -1)
        {
            if (feof(stream))
                return p1 == *lineptr ? -1 : p1 - *lineptr;
            else
                return -1;
        }
        *p1++ = c;

        // See if there is a delimiter at the end of the buffer
        pTail = p1 - delimiterLength;
        if (strstr(pTail, delimiter) == pTail)
        {
            *pTail = '\0';
            return pTail - *lineptr;
        }

        // See if we need to increase the size of the buffer
        if (p1 + 2 >= p2)
        {
            if (n != NULL && *n > 0)
            {
                // Size limited by caller; stop reading
                *pTail = '\0';
                return pTail - *lineptr;
            }

            newBufferSize = bufferSize * 2;
            newBuffer = (char*)(realloc(*lineptr, newBufferSize));
            if (newBuffer == NULL)
                return -1;
            p1 += newBuffer - *lineptr;
            p2 = newBuffer + newBufferSize;
            *lineptr = newBuffer;
            bufferSize = newBufferSize;
        }
    }
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
