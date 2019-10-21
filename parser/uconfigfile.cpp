#include <string.h>
#include "uconfigfile.h"


UconfigFile::UconfigFile()
{
}

UconfigFile::UconfigFile(const UconfigFile& file) :
    rootEntry(file.rootEntry)
{
}

UconfigFile::~UconfigFile()
{
}
