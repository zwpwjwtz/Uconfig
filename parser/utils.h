#ifndef UTILS_H
#define UTILS_H

// Utility functions for string processing, etc.

// Find the position of the first occurance of a substring in string
extern int Uconfig_strpos(const char* haystack, const char* needle);

// Find the position of the first occurance of a line delimitor
// (\n, \r or \r\n) in a string
extern int Uconfig_findLineDelimitor(const char* str);

// Like strncpy, with a "\0" appended to the destination
extern char* Uconfig_strncpy (char* dest, const char* src, int count);

#endif // UTILS_H
