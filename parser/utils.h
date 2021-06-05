#ifndef UTILS_H
#define UTILS_H


typedef struct _IO_FILE FILE;

// Utility functions for string processing, etc.

// Find the position of the first occurance of a substring in string
extern int Uconfig_strpos(const char* haystack, const char* needle);

// Find the position of the first occurance of a line delimitor
// (\n, \r or \r\n) in a string
extern int Uconfig_findLineDelimiter(const char* str);

// Like strstr, but search up to LENGTH bytes
extern char* Uconfig_strnstr(const char* haystack,
                             const char* needle,
                             int length);

// Like isspace, applicable to a whole string
extern bool Uconfig_isspace(const char* str, int length = 1);

// Like strncpy, with a "\0" appended to the destination
extern char* Uconfig_strncpy (char* dest, const char* src, int count);

// Like getdelim, read up to (and including) a DELIMITER
// from STREAM into *LINEPTR. The DELIMITER can be a multi-char string.
// If *N > 0, the maximum length of string read from STREAM is *N;
// otherwise, its length is unlimited (except by the file size).
extern int Uconfig_getdelim(char** __restrict lineptr,
                            int* __restrict n,
                            const char* __restrict delimiter,
                            FILE* __restrict stream);

// Read N chars from STREAM, the go back to where we were
// before the read
extern int Uconfig_fpeek(FILE* stream, char* buffer, int n = 1);

// Read N chars from STREAM, and compare it with a given string.
// Return the number of char read if strcmp()==0; otherwise return 0.
// Assuming strlen(string) if N is not specified.
extern int Uconfig_freadCmp(FILE* stream, const char* string, int n = 0);

// Like Uconfig_freadCmp, but go back to where we were before the read
extern int Uconfig_fpeekCmp(FILE* stream, const char* string, int n = 0);

// Write multiple spaces (0x20) or tabs (0x09) as indentation of text
extern int Uconfig_fwriteIndentation(FILE* stream,
                                     int level,
                                     bool usingTabs = false);


#endif // UTILS_H
