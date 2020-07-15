#ifndef UCONFIGCSV_H
#define UCONFIGCSV_H

#include <stddef.h>
#include "uconfig2dtable.h"


class UconfigCSV : public Uconfig2DTable
{
public:
    static bool readUconfig(const char* filename, UconfigFile* config);
    static bool writeUconfig(const char* filename, UconfigFile* config);
};

#endif // UCONFIGCSV_H
