#include <string.h>
#include "utils.h"

#define UCONFIG_UTILS_LINEDELIMITOR_UNIX    "\n"
#define UCONFIG_UTILS_LINEDELIMITOR_OSX     "\r"
#define UCONFIG_UTILS_LINEDELIMITOR_WIN     "\r\n"


int Uconfig_strpos(const char* haystack, const char* needle)
{
    const char* pos = strstr(haystack, needle);
    if (pos)
        return (int)(pos - haystack);
    else
        return -1;
}

int Uconfig_findLineDelimitor(const char* str)
{
    const char* search = UCONFIG_UTILS_LINEDELIMITOR_UNIX;
    int pos = Uconfig_strpos(str, search);
    if (pos >= 0)
        return pos;

    search = UCONFIG_UTILS_LINEDELIMITOR_OSX;
    pos = Uconfig_strpos(str, search);
    if (pos >= 0)
        return pos;

    search = UCONFIG_UTILS_LINEDELIMITOR_WIN;
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
