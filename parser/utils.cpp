#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

#define UCONFIG_UTILS_LINEDELIMITER_UNIX    "\n"
#define UCONFIG_UTILS_LINEDELIMITER_OSX     "\r"
#define UCONFIG_UTILS_LINEDELIMITER_WIN     "\r\n"

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
    if (*lineptr == NULL || *n == 0)
    {
        *n = UCONFIG_UTILS_FILE_BUFFER_MAX;
        *lineptr = (char*)(malloc(*n));
        if (*lineptr == NULL)
            return -1;
    }

    char* p1 = *lineptr;
    char* p2 = *lineptr + *n;
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
            newBufferSize = *n * 2;
            newBuffer = (char*)(realloc(*lineptr, newBufferSize));
            if (newBuffer == NULL)
                return -1;
            p1 += newBuffer - *lineptr;
            p2 = newBuffer + newBufferSize;
            *lineptr = newBuffer;
            *n = newBufferSize;
        }
    }
}
