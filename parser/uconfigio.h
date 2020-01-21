#ifndef UCONFIGIO_H
#define UCONFIGIO_H

#include "uconfigfile.h"


class UconfigIO
{
public:
    UconfigIO(){}
    ~UconfigIO(){}

    static bool readUconfig(const char* filename,
                            UconfigFile* config);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config);

    static UconfigValueType guessValueType(const char* expression, int length);
};

#endif // UCONFIGIO_H
