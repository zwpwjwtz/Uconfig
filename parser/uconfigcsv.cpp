#include "uconfigcsv.h"
#include "uconfigfile_metadata.h"

#define UCONFIG_IO_CSV_DELIMITER_ROW    "\n"
#define UCONFIG_IO_CSV_DELIMITER_COL    ","


bool UconfigCSV::readUconfig(const char* filename, UconfigFile* config)
{
    return Uconfig2DTable::readUconfig(filename, config,
                                       UCONFIG_IO_CSV_DELIMITER_ROW,
                                       UCONFIG_IO_CSV_DELIMITER_COL);
}

bool UconfigCSV::writeUconfig(const char* filename, UconfigFile* config)
{
    // If there are delimiters previously specified during file parsing,
    // then take them directly
    const char* rowDelimiter =
            config->metadata.searchKey(UCONFIG_METADATA_KEY_ROWDELIM).value();
    const char* columnDelimitor =
            config->metadata.searchKey(UCONFIG_METADATA_KEY_COLDELIM).value();

    // Otherwise, use default delimiters
    if (!rowDelimiter)
        rowDelimiter = UCONFIG_IO_CSV_DELIMITER_ROW;
    if (!columnDelimitor)
        columnDelimitor = UCONFIG_IO_CSV_DELIMITER_COL;

    return Uconfig2DTable::writeUconfig(filename, config,
                                        rowDelimiter, columnDelimitor);
}
