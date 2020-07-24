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
    int columnCount;
    bool* hasColumnName;
    UconfigKeyObject* firstLine;
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

            if (readColumnNames)
            {
                // Use the key names of the first line as column names
                firstLine = rows[0].keys();
                columnCount = rows[0].keyCount();
                hasColumnName = new bool[columnCount];

                // Exclude key names of non-char type
                for (j=0; j<columnCount; j++)
                {
                    hasColumnName[j] = firstLine[j].type() == ValueType::Raw ||
                                       firstLine[j].type() == ValueType::Chars;
                }
            }
            for (j=0; j<entryList[i].subentryCount(); j++)
            {
                if (rows[j].keyCount() < 1)
                    continue;
                valueList = rows[j].keys();

                // Use the name of the first key as the row name
                if (readRowNames &&
                    (valueList[0].type() == ValueType::Raw ||
                     valueList[0].type() == ValueType::Chars))
                    rows[j].setName(valueList[0].value(),
                                    valueList[0].valueSize());

                // Apply column names to each key
                if (readColumnNames)
                {
                    for (k=0; k<rows[j].keyCount(); k++)
                    {
                        if (k >= columnCount)
                            break;
                        if (hasColumnName[k])
                            valueList[k].setName(firstLine[k].value(),
                                                 firstLine[k].valueSize());
                    }
                }

                delete[] valueList;
            }
            if (readColumnNames)
            {
                // Additional remark for the first row
                rows[0].setName(UCONFIG_IO_CSV_TYPE_HEADER);
                delete[] hasColumnName;
            }

            delete[] rows;
        }
    }
    if (entryList)
        delete[] entryList;
    return true;
}
