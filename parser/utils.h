#ifndef UTILS_H
#define UTILS_H


typedef struct _IO_FILE FILE;

// Utility functions for string processing, etc.

// Find the position of the first occurance of a substring in string
extern int Uconfig_strpos(const char* haystack, const char* needle);

// Find the position of the first occurance of a line delimitor
// (\n, \r or \r\n) in a string
extern int Uconfig_findLineDelimiter(const char* str);

// Like strncpy, with a "\0" appended to the destination
extern char* Uconfig_strncpy (char* dest, const char* src, int count);

// Like getdelim, read up to (and including) a DELIMITER
// from STREAM into *LINEPTR. The DELIMITER can be a multi-char string.
extern int Uconfig_getdelim(char** __restrict lineptr,
                            int* __restrict n,
                            const char* __restrict delimiter,
                            FILE* __restrict stream);

// Write multiple spaces (0x20) or tabs (0x09) as indentation of text
extern int Uconfig_fwriteIndentation(FILE* __restrict stream,
                                     int level,
                                     bool usingTabs = false);

#endif // UTILS_H
