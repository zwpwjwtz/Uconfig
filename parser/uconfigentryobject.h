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

    void reset();

    const char* name() const;
    void setName(const char* name);

    UconfigValueType type() const;
    void setType(UconfigValueType type);

    const char* value() const;
    int valueSize() const;
    void setValue(const char* value, int size);

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

    void reset();

    const char* name() const;
    void setName(const char* name);

    int type() const;
    void setType(int type);

    int keyCount() const;
    UconfigKeyObject* keys();
    UconfigKeyObject searchKey(const char* keyName);

    bool addKey(const UconfigKeyObject* newKey);
    bool deleteKey(const char* keyName);
    bool modifyKey(const char* keyName, const UconfigKeyObject* newKey);

    int subentryCount() const;
    UconfigEntryObject* subentries();
    UconfigEntryObject searchSubentry(const char* entryName = NULL,
                                      const char* parentName = NULL,
                                      bool recursive = false);

    bool addSubentry(const UconfigEntryObject *newEntry);
    bool deleteSubentry(const char* entryName);
    bool modifySubentry(const char* entryName,
                        const UconfigEntryObject* newEntry);

    static bool copyEntry(UconfigEntry* dest,
                           const UconfigEntry* src,
                           bool recursive = false);
    static void deleteEntry(UconfigEntry* entry);

protected:
    UconfigEntry propData;
    UconfigEntry* refData;

    void initialize();
    void setReference(UconfigEntry* reference);
};

#endif
