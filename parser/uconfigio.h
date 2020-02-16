#ifndef UCONFIGIO_H
#define UCONFIGIO_H

#include "uconfigfile.h"


class UconfigIO
{
public:
    enum ValueType
    {
        Raw = 0, // Array of bytes
        Bool = 1,
        Chars = 2, // Array of ASCII characters terminated by 0x00
        Integer = 3,
        Float = 4,
        Double = 5,
        List = 128
    };

    UconfigIO(){}
    ~UconfigIO(){}

    static bool readUconfig(const char* filename,
                            UconfigFile* config);
    static bool writeUconfig(const char* filename,
                             UconfigFile* config);

    static ValueType guessValueType(const char* expression, int length);
};

#endif // UCONFIGIO_H
