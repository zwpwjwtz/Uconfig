#include <stdlib.h>
#include <string.h>
#include "uconfigfile_p.h"


UconfigFile::UconfigFile()
{
    this->d_ptr = new UconfigFilePrivate(this);
}

UconfigFile::~UconfigFile()
{
    delete this->d_ptr;
}

char* UconfigFile::fileName()
{
    char* buf = (char*)malloc(sizeof(char) * (strlen(d_ptr->fileName) + 1));
    strcpy(buf, d_ptr->fileName);
    return buf;
}

int UconfigFile::configType()
{
    return d_ptr->fileType;
}

bool UconfigFile::getEntry(UconfigEntry* newEntry,
                           const char* entryName,
                           const char* parentName,
                           bool recursive)
{
    if (!newEntry)
        return false;

    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    UconfigEntry* entry = d_ptr->searchEntryByName(entryName, parent);
    return d_ptr->copyEntry(newEntry, entry, recursive);
}

bool UconfigFile::addEntry(const UconfigEntry* newEntry,
                           const char* parentName)
{
    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    if (!parent)
            return false;

    // Create a new list for the subentries
    int entryCount = parent->subentryCount;
    UconfigEntry** newEntryList =
            (UconfigEntry**)malloc(sizeof(UconfigEntry*) * (entryCount + 1));
    if (entryCount > 0)
        memcpy(newEntryList,
               parent->subentries,
               sizeof(UconfigEntry*) * entryCount);

    // Insert the new entry into the list
    newEntryList[entryCount] = (UconfigEntry*)malloc(sizeof(UconfigEntry));
    if (!d_ptr->copyEntry(newEntryList[entryCount], newEntry, true))
        return false;

    // Update the list for the parent
    if (parent->subentries)
        free(parent->subentries);
    parent->subentries = newEntryList;
    parent->subentryCount++;

    return true;
}

bool UconfigFile::deleteEntry(const char* entryName,
                              const char* parentName)
{
    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    UconfigEntry* entry = d_ptr->searchEntryByName(entryName, parent);
    if (!entry)
        return false;
    d_ptr->deleteEntry(entry);

    // Create a new list for the subentries
    int entryCount = parent->subentryCount;
    UconfigEntry** newEntryList =
            (UconfigEntry**)malloc(sizeof(UconfigEntry*) * (entryCount - 1));
    int i, j = 0;
    for (i=0; i<entryCount; i++)
    {
        if (parent->subentries[i] != entry)
            newEntryList[j++] = parent->subentries[i];
    }

    // Update the list for the parent
    if (parent->subentries)
        free(parent->subentries);
    parent->subentries = newEntryList;
    parent->subentryCount--;

    return true;
}

bool UconfigFile::modifyEntry(const UconfigEntry* newEntry,
                              const char* entryName,
                              const char* parentName)
{
    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    UconfigEntry* entry = d_ptr->searchEntryByName(entryName, parent);
    if (!entry)
        return false;

    // Duplicate the given entry
    UconfigEntry* tempEntry = (UconfigEntry*)malloc(sizeof(UconfigEntry));
    if (!d_ptr->copyEntry(tempEntry, newEntry, true))
        return false;

    // Update the subentry list of parent
    for (int i=0; i<parent->subentryCount; i++)
    {
        if (parent->subentries[i] == entry)
        {
            parent->subentries[i] = tempEntry;
            d_ptr->deleteEntry(entry);
            return true;
        }
    }

    // We shall never reach here
    return false;
}

bool UconfigFile::getKey(UconfigKey* newKey,
                         const char* keyName,
                         const char* entryName,
                         const char* parentName)
{
    if (!newKey)
        return false;

    // Find the entry first
    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    UconfigEntry* entry = d_ptr->searchEntryByName(entryName, parent);
    if (!entry || !entry->keys)
        return false;

    // Then find the key
    UconfigKey* key = d_ptr->searchKeyByName(keyName, entry);
    return d_ptr->copyKey(newKey, key);
}

bool UconfigFile::addKey(UconfigKey* newKey,
                         const char* entryName,
                         const char* parentName)
{
    // Find the entry first
    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    UconfigEntry* entry = d_ptr->searchEntryByName(entryName, parent);
    if (!entry)
        return false;

    // Create a new list for the keys
    int keyCount = entry->keyCount;
    UconfigKey** newKeyList =
                    (UconfigKey**)malloc(sizeof(UconfigKey) * (keyCount + 1));
    if (keyCount > 0)
        memcpy(newKeyList,
               entry->keys,
               sizeof(UconfigKey*) * keyCount);

    // Insert the new key into list
    newKeyList[keyCount] = (UconfigKey*)malloc(sizeof(UconfigKey));
    if (!d_ptr->copyKey(newKeyList[keyCount], newKey))
        return false;

    // Update the key list for the entry
    if (entry->keys)
        free(entry->keys);
    entry->keys = newKeyList;
    entry->keyCount++;

    return true;
}

bool UconfigFile::deleteKey(const char* keyName,
                            const char* entryName,
                            const char* parentName)
{
    // Find the entry first
    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    UconfigEntry* entry = d_ptr->searchEntryByName(entryName, parent);
    if (!entry || !entry->keys)
        return false;

    // Then find the key
    UconfigKey* key = d_ptr->searchKeyByName(keyName, entry);
    if (!key)
        return false;

    // Create a new list for the keys
    int keyCount = entry->keyCount;
    UconfigKey** newKeyList =
            (UconfigKey**)malloc(sizeof(UconfigKey*) * (keyCount - 1));
    int i, j = 0;
    for (i=0; i<keyCount; i++)
    {
        if (entry->keys[i] == key)
            newKeyList[j++] = entry->keys[i];
    }

    // Update the key list for the entry
    if (entry->keys)
        free(entry->keys);
    entry->keys = newKeyList;
    entry->keyCount--;

    return true;
}

bool UconfigFile::modifyKey(UconfigKey* newKey,
                            const char* entryName,
                            const char* parentName)
{
    // Find the entry first
    UconfigEntry* parent = d_ptr->searchEntryByName(parentName, NULL);
    UconfigEntry* entry = d_ptr->searchEntryByName(entryName, parent);
    if (!entry || !entry->keys)
        return false;

    // Then find the key
    UconfigKey* key = d_ptr->searchKeyByName(newKey->name, entry);
    if (!key)
        return false;

    // Duplicate the given key
    UconfigKey* tempKey = (UconfigKey*)malloc(sizeof(UconfigKey));
    if (!d_ptr->copyKey(tempKey, newKey))
        return false;

    // Update the key list for the entry
    for (int i=0; i<entry->keyCount; i++)
    {
        if (entry->keys[i] == key)
        {
            entry->keys[i] = tempKey;
            d_ptr->deleteKey(key);
            return true;
        }
    }

    // We shall never reach here
    return false;
}


UconfigFilePrivate::UconfigFilePrivate(UconfigFile* parent)
{
    this->p_ptr = parent;

    fileName = NULL;
    rootEntry = (UconfigEntry*)calloc(1, sizeof(UconfigEntry));
}

UconfigFilePrivate::~UconfigFilePrivate()
{
    if (fileName)
        delete fileName;
    deleteEntry(rootEntry);
}

// Deep copy of a key
bool UconfigFilePrivate::copyKey(UconfigKey* dest, const UconfigKey* src)
{
    // First do a shallow copy
    memcpy(dest, src, sizeof(UconfigKey));

    // Deep copy of the name
    if (src->name)
    {
        dest->name = (char*)malloc(sizeof(char) * (strlen(src->name) + 1));
        strcpy(dest->name, src->name);
    }

    // Deep copy of the value chunk
    if (src->value)
    {
        dest->value = (char*)malloc(sizeof(char) * src->valueSize);
        memcpy(dest->value, src->value, src->valueSize);
    }

    return true;
}

void UconfigFilePrivate::deleteKey(UconfigKey* key)
{
    if (key->name)
        free(key->name);
    if (key->value)
        free(key->value);
    free(key);
}

// Deep copy of an entry and its subentries
// Warning: do not free any allocated resource in the destination!
bool UconfigFilePrivate::copyEntry(UconfigEntry* dest,
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
        dest->name = (char*)malloc(sizeof(char) * (strlen(src->name) + 1));
        strcpy(dest->name, src->name);
    }

    // Deep copy of keys
    int i;
    UconfigKey** newKeys =
                    (UconfigKey**)malloc(sizeof(UconfigKey*) * src->keyCount);
    for (i=0; i<src->keyCount; i++)
    {
        newKeys[i] = (UconfigKey*)malloc(sizeof(UconfigKey));
        copyKey(newKeys[i], src->keys[i]);
    }
    dest->keys = newKeys;

    if (recursive)
    {
        // Deep copy of subentries
        UconfigEntry** newEntries =
            (UconfigEntry**)malloc(sizeof(UconfigEntry*) * src->subentryCount);
        for (i=0; i<src->subentryCount; i++)
        {
            newEntries[i] = (UconfigEntry*)malloc(sizeof(UconfigEntry));
            copyEntry(newEntries[i], src->subentries[i], true);
        }
        dest->subentries = newEntries;
    }
    else
        dest->subentries = NULL;

    return true;
}

void UconfigFilePrivate::deleteEntry(UconfigEntry* entry)
{
    if (entry->name)
        free(entry->name);
    if (entry->keys)
    {
        for (int i=0; i<entry->keyCount; i++)
            deleteKey(entry->keys[i]);
        free(entry->keys);
    }
    if (entry->subentries)
    {
        for (int i=0; i<entry->subentryCount; i++)
            deleteEntry(entry->subentries[i]);
        free(entry->subentries);
    }
    free(entry);
}

// Find an entry with given name under a given parent recursively
// Return NULL if no such entry can be found
UconfigEntry* UconfigFilePrivate::searchEntryByName(const char* name,
                                                    UconfigEntry* parent)
{
    if (!name || strlen(name) < 1)
        return rootEntry;

    if (!parent)
        parent = rootEntry;

    // Recursive search of an entry with given name and
    // attached to a specific parent
    if (parent && parent->subentries)
    {
        // See if one of the subentries's name match the search
        for (int i=0; i<parent->subentryCount; i++)
        {
            if (strcmp(parent->subentries[i]->name, name) == 0)
                return parent->subentries[i];
        }

        // If not, look into each subentry
        UconfigEntry* entry;
        for (int i=0; i<parent->subentryCount; i++)
        {
            entry = searchEntryByName(name, parent->subentries[i]);
            if (entry)
                return entry;
        }
    }
    // If still no found, return a NULL pointer
    return NULL;
}

// Find a key with given name under a given entry
// Return NULL if no such key can be found
UconfigKey* UconfigFilePrivate::searchKeyByName(const char* name,
                                                UconfigEntry* entry)
{
    if (!entry || !entry->keys)
        return NULL;

    UconfigKey* key = NULL;
    for (int i=0; i<entry->keyCount; i++)
    {
        if (entry->keys[i] && strcmp(entry->keys[i]->name, name) == 0)
        {
            key = entry->keys[i];
            break;
        }
    }
    return key;
}
