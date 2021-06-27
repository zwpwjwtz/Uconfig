#ifndef UCONFIGENTRYOBJECT_H
#define UCONFIGENTRYOBJECT_H

/*
 * This file offers wrapper classes for UconfigKey and UconfigEntry.
 * When destructed, these class will try to free the resource
 * allocated to their properties.
 */

#include "uconfigentry.h"


class UconfigKeyObject
{
public:
    UconfigKeyObject();
    UconfigKeyObject(UconfigKey* key, bool copy = true);
    UconfigKeyObject(const UconfigKeyObject& key);
    ~UconfigKeyObject();

    UconfigKeyObject& operator=(const UconfigKeyObject& key);

    void reset();

    // Key name
    const char* name() const;
    int nameSize() const;
    void setName(const char* name, int size = 0);

    // Key type
    int type() const;
    void setType(int type);

    // Key value
    const char* value() const;
    int valueSize() const;
    void setValue(const char* value, int size);

    // Helper functions
    static bool copyKey(UconfigKey* dest, const UconfigKey* src);
    static void deleteKey(UconfigKey* key);

protected:
    UconfigKey propData;
    UconfigKey* refData;

    void initialize();
    void setReference(UconfigKey *reference);

    friend class UconfigEntryObject;
};


class UconfigEntryObject
{
public:
    UconfigEntryObject();
    UconfigEntryObject(UconfigEntry* entry,
                       bool copy = true,
                       bool subentries = false);
    UconfigEntryObject(const UconfigEntryObject& entry);
    ~UconfigEntryObject();

    UconfigEntryObject& operator=(const UconfigEntryObject& entry);

    void reset();

    // Entry name
    const char* name() const;
    int nameSize() const;
    void setName(const char* name, int size = 0);

    // Entry type
    int type() const;
    void setType(int type);

    // Entry's keys
    int keyCount() const;
    UconfigKeyObject* keys();

    bool existKey(const char* keyName, int nameSize = 0) const;
    UconfigKeyObject searchKey(const char* keyName, int nameSize = 0);

    bool addKey(const UconfigKeyObject* newKey);
    bool deleteKey(const char* keyName, int nameSize = 0);
    bool modifyKey(const UconfigKeyObject* newKey,
                   const char* keyName,
                   int nameSize = 0);

    // Subentries
    int subentryCount() const;
    UconfigEntryObject* subentries();

    bool existSubentry(const char* entryName, int nameSize = 0) const;
    UconfigEntryObject searchSubentry(const char* entryName = NULL,
                                      const char* parentName = NULL,
                                      bool recursive = false,
                                      int entryNameSize = 0,
                                      int parentNameSize = 0);

    bool addSubentry(const UconfigEntryObject *newEntry);
    bool appendSubentry(UconfigEntryObject* newEntry);
    bool deleteSubentry(const char* entryName, int nameSize = 0);
    bool modifySubentry(const UconfigEntryObject* newEntry,
                        const char* entryName,
                        int nameSize);

    // Entry's parent
    UconfigEntryObject parentEntry();

    // Helper functions
    static bool copyEntry(UconfigEntry* dest,
                           const UconfigEntry* src,
                           bool recursive = false);
    static void deleteEntry(UconfigEntry* entry);

protected:
    UconfigEntry* propData;
    UconfigEntry* refData;

    void initialize();
    void setReference(UconfigEntry* reference);
};

#endif
