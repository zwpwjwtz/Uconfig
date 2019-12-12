#include <string.h>
#include "uconfigentryobject.h"


// Declaration of private functions
UconfigKey* Uconfig_searchKeyByName(const char* name, UconfigEntry* entry);
UconfigEntry* Uconfig_searchEntryByName(const char* name,
                                        UconfigEntry* parent,
                                        bool recursive = false);


UconfigKeyObject::UconfigKeyObject()
{
    initialize();
}

UconfigKeyObject::UconfigKeyObject(UconfigKey* key, bool copy)
{
    if (key)
    {
        if (copy)
            copyKey(&propData, key);
        else
        {
            initialize();
            refData = key;
        }
    }
    else
        initialize();
}

UconfigKeyObject::UconfigKeyObject(const UconfigKeyObject& key)
{
    if (key.refData)
        copyKey(&propData, key.refData);
    else
        copyKey(&propData, &key.propData);
}

UconfigKeyObject::~UconfigKeyObject()
{
    reset();
}

void UconfigKeyObject::reset()
{
    refData = NULL;

    if (propData.name)
        delete propData.name;
    propData.name = NULL;

    if (propData.value)
        delete propData.value;
    propData.value = NULL;
    propData.valueSize = 0;
    propData.valueType = 0;
}

const char* UconfigKeyObject::name() const
{
    const UconfigKey& data = refData ? *refData : propData;
    return data.name;
}

void UconfigKeyObject::setName(const char* name)
{
    UconfigKey& data = refData ? *refData : propData;

    if (data.name)
        delete data.name;

    if (name)
    {
        data.name = new char[strlen(name) + 1];
        strcpy(data.name, name);
    }
    else
        data.name = NULL;
}

UconfigValueType UconfigKeyObject::type() const
{
    const UconfigKey& data = refData ? *refData : propData;
    return UconfigValueType(data.valueType);
}

void UconfigKeyObject::setType(UconfigValueType type)
{
    UconfigKey& data = refData ? *refData : propData;
    data.valueType = int(type);
}

const char* UconfigKeyObject::value() const
{
    const UconfigKey& data = refData ? *refData : propData;
    return data.value;
}

int UconfigKeyObject::valueSize() const
{
    const UconfigKey& data = refData ? *refData : propData;
    return data.valueSize;
}

void UconfigKeyObject::setValue(const char* value, int size)
{
    UconfigKey& data = refData ? *refData : propData;

    if (data.value)
        delete data.value;

    if (value && size > 0)
    {
        data.value = new char[size];
        memcpy(data.value, value, size);
        data.valueSize = size;
    }
    else
    {
        data.value = NULL;
        data.valueSize = 0;
    }
}

// Deep copy of a key
bool UconfigKeyObject::copyKey(UconfigKey* dest, const UconfigKey* src)
{
    // First do a shallow copy
    memcpy(dest, src, sizeof(UconfigKey));

    // Deep copy of the name
    if (src->name)
    {
        dest->name = new char[strlen(src->name) + 1];
        strcpy(dest->name, src->name);
    }

    // Deep copy of the value chunk
    if (src->value)
    {
        dest->value = new char[src->valueSize];
        memcpy(dest->value, src->value, src->valueSize);
        dest->valueSize = src->valueSize;
    }

    return true;
}

void UconfigKeyObject::deleteKey(UconfigKey* key)
{
    if (key->name)
        delete key->name;
    if (key->value)
        delete key->value;
    delete key;
}

void UconfigKeyObject::initialize()
{
    refData = NULL;

    propData.name = NULL;
    propData.value = NULL;
    propData.valueSize = 0;
    propData.valueType = 0;
}

void UconfigKeyObject::setReference(UconfigKey* reference)
{
    refData = reference;
}


UconfigEntryObject::UconfigEntryObject()
{
    initialize();
}

UconfigEntryObject::UconfigEntryObject(UconfigEntry* entry,
                                       bool copy,
                                       bool subentries)
{
    if (entry)
    {
        if (copy)
            copyEntry(&propData, entry, subentries);
        else
        {
            initialize();
            refData = entry;
        }
    }
    else
        initialize();
}

UconfigEntryObject::UconfigEntryObject(const UconfigEntryObject& entry)
{
    if (entry.refData)
        copyEntry(&propData, entry.refData);
    else
        copyEntry(&propData, &entry.propData);
    refData = NULL;
}

UconfigEntryObject::~UconfigEntryObject()
{
    reset();
}

void UconfigEntryObject::reset()
{
    refData = NULL;

    if (propData.name)
        delete propData.name;
    propData.name = NULL;

    propData.type = 0;

    if (propData.keys)
    {
        for (int i=0; i<propData.keyCount; i++)
            UconfigKeyObject::deleteKey(propData.keys[i]);
        delete propData.keys;
    }
    propData.keys = NULL;
    propData.keyCount = 0;

    if (propData.subentries)
    {
        for (int i=0; i<propData.subentryCount; i++)
            deleteEntry(propData.subentries[i]);
        delete propData.subentries;
    }
    propData.subentries = NULL;
    propData.subentryCount = 0;
}

const char* UconfigEntryObject::name() const
{
    const UconfigEntry& data = refData ? *refData : propData;
    return data.name;
}

void UconfigEntryObject::setName(const char* name)
{
    UconfigEntry& data = refData ? *refData : propData;

    if (data.name)
        delete data.name;

    if (name)
    {
        data.name = new char[strlen(name) + 1];
        strcpy(data.name, name);
    }
    else
        data.name = NULL;
}

int UconfigEntryObject::type() const
{
    const UconfigEntry& data = refData ? *refData : propData;
    return data.type;
}

void UconfigEntryObject::setType(int type)
{
    UconfigEntry& data = refData ? *refData : propData;
    data.type = type;
}

int UconfigEntryObject::keyCount() const
{
    const UconfigEntry& data = refData ? *refData : propData;
    return data.keyCount;
}

UconfigKeyObject* UconfigEntryObject::keys()
{
    UconfigEntry& data = refData ? *refData : propData;
    if (data.keyCount < 1)
        return NULL;

    UconfigKeyObject* keyList = new UconfigKeyObject[data.keyCount];
    for (int i=0; i<data.keyCount; i++)
        keyList[i].setReference(data.keys[i]);
    return keyList;
}

// Find a key with given name under a given entry
// Return NULL if no such key can be found
UconfigKeyObject UconfigEntryObject::searchKey(const char* keyName)
{
    if (!keyName)
        return UconfigKeyObject();

    UconfigEntry& entry = refData ? *refData : propData;

    UconfigKey* key = NULL;
    for (int i=0; i<entry.keyCount; i++)
    {
        if (entry.keys[i] &&
            entry.keys[i]->name &&
            strcmp(entry.keys[i]->name, keyName) == 0)
        {
            key = entry.keys[i];
            break;
        }
    }

    return UconfigKeyObject(key, false);
}
bool UconfigEntryObject::addKey(const UconfigKeyObject* newKey)
{
    UconfigEntry& entry = refData ? *refData : propData;

    // Create a new list for the keys
    int keyCount = entry.keyCount;
    UconfigKey** newKeyList = new UconfigKey*[keyCount + 1];
    if (keyCount > 0)
        memcpy(newKeyList,
               entry.keys,
               sizeof(UconfigKey*) * keyCount);

    // Insert the new key into list
    newKeyList[keyCount] = new UconfigKey;
    const UconfigKey* newData =
                        newKey->refData ? newKey->refData : &newKey->propData;
    if (!UconfigKeyObject::copyKey(newKeyList[keyCount], newData))
        return false;

    // Update the key list for the entry
    if (entry.keys)
        delete entry.keys;
    entry.keys = newKeyList;
    entry.keyCount++;

    return true;
}

bool UconfigEntryObject::deleteKey(const char* keyName)
{
    UconfigEntry& entry = refData ? *refData : propData;
    UconfigKey* key = Uconfig_searchKeyByName(keyName, &entry);
    if (!key)
        return false;

    // Create a new list for the keys
    int keyCount = entry.keyCount;
    UconfigKey** newKeyList = new UconfigKey*[keyCount - 1];
    int i, j = 0;
    for (i=0; i<keyCount; i++)
    {
        if (entry.keys[i] == key)
            newKeyList[j++] = entry.keys[i];
    }

    // Update the key list for the entry
    if (entry.keys)
        delete entry.keys;
    entry.keys = newKeyList;
    entry.keyCount--;

    return true;
}

bool UconfigEntryObject::modifyKey(const char* keyName,
                                   const UconfigKeyObject* newKey)
{
    UconfigEntry& entry = refData ? *refData : propData;
    UconfigKey* key = Uconfig_searchKeyByName(keyName, &entry);
    if (!key)
        return false;

    // Duplicate the given key
    UconfigKey* tempKey = new UconfigKey;
    const UconfigKey* newData =
                        newKey->refData ? newKey->refData : &newKey->propData;
    if (!UconfigKeyObject::copyKey(tempKey, newData))
        return false;

    // Update the key list for the entry
    for (int i=0; i<entry.keyCount; i++)
    {
        if (entry.keys[i] == key)
        {
            entry.keys[i] = tempKey;
            UconfigKeyObject::deleteKey(key);
            return true;
        }
    }

    // We shall never reach here
    return false;
}

int UconfigEntryObject::subentryCount() const
{
    const UconfigEntry& entry = refData ? *refData : propData;
    return entry.subentryCount;
}

UconfigEntryObject* UconfigEntryObject::subentries()
{
    UconfigEntry& entry = refData ? *refData : propData;
    if (entry.subentryCount < 1)
        return NULL;

    UconfigEntryObject* entryList =
                            new UconfigEntryObject[entry.subentryCount];
    for (int i=0; i<entry.subentryCount; i++)
        entryList[i].setReference(entry.subentries[i]);
    return entryList;
}

// Find an entry with given name under a given parent recursively
// Return NULL if no such entry can be found
UconfigEntryObject
UconfigEntryObject::searchSubentry(const char* entryName,
                                   const char* parentName,
                                   bool recursive)
{
    UconfigEntryObject entryObject;

    UconfigEntry* entry = refData ? refData : &propData;
    UconfigEntry* parent;
    if (parentName)
        parent = Uconfig_searchEntryByName(parentName, entry);
    else
        parent = entry;

    if (parent)
    {
        entry = Uconfig_searchEntryByName(entryName, parent, recursive);
        if (entry)
            entryObject.setReference(entry);
    }

    return entryObject;
}

bool UconfigEntryObject::addSubentry(const UconfigEntryObject* newEntry)
{
    UconfigEntry& entry = refData ? *refData : propData;

    // Create a new list for the subentries
    int entryCount = entry.subentryCount;
    UconfigEntry** newEntryList = new UconfigEntry*[entryCount + 1];
    if (entryCount > 0)
        memcpy(newEntryList,
               entry.subentries,
               sizeof(UconfigEntry*) * entryCount);

    // Insert the new entry into the list
    const UconfigEntry* newData =
                    newEntry->refData ? newEntry->refData : &newEntry->propData;
    newEntryList[entryCount] = new UconfigEntry;
    if (!copyEntry(newEntryList[entryCount], newData, true))
        return false;
    else
        newEntryList[entryCount]->parentEntry = &entry;

    // Update the list for the parent
    if (entry.subentries)
        delete entry.subentries;
    entry.subentries = newEntryList;
    entry.subentryCount++;

    return true;
}

bool UconfigEntryObject::deleteSubentry(const char* entryName)
{
    UconfigEntry& data = refData ? *refData : propData;
    UconfigEntry* entry = Uconfig_searchEntryByName(entryName, &data);
    if (!entry)
        return false;
    deleteEntry(entry);

    // Create a new list for the subentries
    int entryCount = entry->subentryCount;
    UconfigEntry** newEntryList = new UconfigEntry*[entryCount - 1];
    int i, j = 0;
    for (i=0; i<entryCount; i++)
    {
        if (entry->subentries[i] != entry)
            newEntryList[j++] = entry->subentries[i];
    }

    // Update the list for the parent
    if (entry->subentries)
        delete entry->subentries;
    entry->subentries = newEntryList;
    entry->subentryCount--;

    return true;
}

bool UconfigEntryObject::modifySubentry(const char* entryName,
                                        const UconfigEntryObject* newEntry)
{
    UconfigEntry& data = refData ? *refData : propData;
    UconfigEntry* entry = Uconfig_searchEntryByName(entryName, &data);
    if (!entry)
        return false;

    // Duplicate the given entry
    const UconfigEntry* newData =
                    newEntry->refData ? newEntry->refData : &newEntry->propData;
    UconfigEntry* tempEntry = new UconfigEntry;
    if (!copyEntry(tempEntry, newData, true))
        return false;

    // Update the subentry list of parent
    for (int i=0; i<entry->subentryCount; i++)
    {
        if (entry->subentries[i] == entry)
        {
            entry->subentries[i] = tempEntry;
            tempEntry->parentEntry = entry;
            deleteEntry(entry);
            return true;
        }
    }

    // We shall never reach here
    return false;
}

// Deep copy of an entry and its subentries
// Warning: do not free any allocated resource in the destination!
bool UconfigEntryObject::copyEntry(UconfigEntry* dest,
                                   const UconfigEntry* src,
                                   bool recursive)
{
    if (!src || !dest)
        return false;

    // First do a shallow copy
    memcpy(dest, src, sizeof(UconfigEntry));

    // Deep copy of the name
    if (src->name)
    {
        dest->name = new char[strlen(src->name) + 1];
        strcpy(dest->name, src->name);
    }

    // Deep copy of keys
    int i;
    UconfigKey** newKeys = new UconfigKey*[src->keyCount];
    for (i=0; i<src->keyCount; i++)
    {
        newKeys[i] = new UconfigKey;
        UconfigKeyObject::copyKey(newKeys[i], src->keys[i]);
    }
    dest->keys = newKeys;

    if (recursive)
    {
        // Deep copy of subentries
        UconfigEntry** newEntries = new UconfigEntry*[src->subentryCount];
        for (i=0; i<src->subentryCount; i++)
        {
            newEntries[i] = new UconfigEntry;
            copyEntry(newEntries[i], src->subentries[i], true);
        }
        dest->subentries = newEntries;
    }
    else
        dest->subentries = NULL;

    return true;
}

void UconfigEntryObject::deleteEntry(UconfigEntry* entry)
{
    if (entry->name)
        delete entry->name;
    if (entry->keys)
    {
        for (int i=0; i<entry->keyCount; i++)
            UconfigKeyObject::deleteKey(entry->keys[i]);
        delete entry->keys;
    }
    if (entry->subentries)
    {
        for (int i=0; i<entry->subentryCount; i++)
            deleteEntry(entry->subentries[i]);
        delete entry->subentries;
    }
    delete entry;
}

void UconfigEntryObject::initialize()
{
    refData = NULL;

    propData.name = NULL;
    propData.type = 0;
    propData.keyCount = 0;
    propData.keys = NULL;
    propData.subentryCount = 0;
    propData.subentries = NULL;
    propData.parentEntry = NULL;
}

void UconfigEntryObject::setReference(UconfigEntry *reference)
{
    refData = reference;
}

// Find a key with given name under a given entry
// Return NULL if no such key can be found
UconfigKey* Uconfig_searchKeyByName(const char* name, UconfigEntry* entry)
{
    if (!entry || !entry->keys)
        return NULL;

    UconfigKey* key = NULL;
    for (int i=0; i<entry->keyCount; i++)
    {
        if (entry->keys[i] &&
            entry->keys[i]->name &&
            strcmp(entry->keys[i]->name, name) == 0)
        {
            key = entry->keys[i];
            break;
        }
    }
    return key;
}

// Find an entry with given name under a given parent
// Return NULL if no such entry can be found
UconfigEntry* Uconfig_searchEntryByName(const char* name,
                                        UconfigEntry* parent,
                                        bool recursive)
{
    if (!name || strlen(name) < 1)
        return parent;

    if (!parent)
        return NULL;

    // Recursive search of an entry with given name and
    // attached to a specific parent
    if (parent && parent->subentries)
    {
        // See if one of the subentries's name match the search
        for (int i=0; i<parent->subentryCount; i++)
        {
            if (parent->subentries[i]->name &&
                strcmp(parent->subentries[i]->name, name) == 0)
                return parent->subentries[i];
        }

        if (!recursive)
            return NULL;

        // If not, look into each subentry
        UconfigEntry* entry;
        for (int i=0; i<parent->subentryCount; i++)
        {
            entry = Uconfig_searchEntryByName(name,
                                              parent->subentries[i],
                                              true);
            if (entry)
                return entry;
        }
    }
    // If still no found, return a NULL pointer
    return NULL;
}
