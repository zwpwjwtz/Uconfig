#include "uconfigcsv.h"
#include "uconfigfile_metadata.h"

#define UCONFIG_IO_CSV_DELIMITER_ROW    "\n"
#define UCONFIG_IO_CSV_DELIMITER_COL    ","
#define UCONFIG_IO_CSV_TYPE_HEADER      "Header"


bool UconfigCSV::readUconfig(const char* filename, UconfigFile* config)
{
    return readUconfig(filename, config,
                       UCONFIG_IO_CSV_DELIMITER_ROW,
                       UCONFIG_IO_CSV_DELIMITER_COL,
                       ValueType::Chars);
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

bool UconfigCSV::readUconfig(const char* filename,
                             UconfigFile* config,
                             const char* rowDelimiter,
                             const char* columnDelimiter,
                             bool readColumnNames,
                             bool readRowNames)
{
    if (!Uconfig2DTable::readUconfig(filename, config,
                                     rowDelimiter, columnDelimiter,
                                     true, true))
        return false;

    // Extract row names and column names
    int i, j, k;
    int keyCount;
    UconfigKeyObject* keyList;
    UconfigKeyObject* valueList;
    UconfigEntryObject* entryList = config->rootEntry.subentries();
    UconfigEntryObject* rows;
    if (readRowNames || readColumnNames)
    {
        for (i=0; i<config->rootEntry.subentryCount(); i++)
        {
            if (entryList[i].type() != EntryType::NormalEntry ||
                entryList[i].subentryCount() < 1)
                continue;

            rows = entryList[i].subentries();
            keyList = rows[0].keys();
            keyCount = rows[0].keyCount();
            for (j=0; j<entryList[i].subentryCount(); j++)
            {
                if (rows[j].keyCount() < 1)
                    continue;
                valueList = rows[j].keys();

                if (readRowNames)
                    rows[j].setName(valueList[0].value(),
                                    valueList[0].valueSize());

                if (readColumnNames)
                {
                    for (k=0; k<rows[j].keyCount(); k++)
                    {
                        if (k >= keyCount)
                            break;
                        valueList[k].setName(keyList[k].value(),
                                             keyList[k].valueSize());
                    }
                }
            }
            if (readColumnNames)
                rows[0].setName(UCONFIG_IO_CSV_TYPE_HEADER);
        }
    }
    return true;
}
