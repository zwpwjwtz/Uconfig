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
    if (strlen(entryName) < 1)
        return d_ptr->rootEntry;

    UconfigEntry* parent = d_ptr->rootEntry;
    if (parentName)
    {
        parent = d_ptr->searchEntryByName(parent, parentName);
        if (!parent)
            return false;
    }

    UconfigEntry* entry = d_ptr->searchEntryByName(parent, entryName);
    return d_ptr->copyEntry(newEntry, entry, recursive);
}

bool UconfigFile::addEntry(const UconfigEntry* newEntry,
                           const char* parentName)
{
    UconfigEntry* parent = d_ptr->rootEntry;
    if (parentName)
    {
        parent = d_ptr->searchEntryByName(parent, parentName);
        if (!parent)
            return false;
    }

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

    // Update the list
    if (parent->subentries)
        free(parent->subentries);
    parent->subentries = newEntryList;
    parent->subentryCount++;

    return true;
}

bool UconfigFile::deleteEntry(const char* entryName,
                              const char* parentName)
{
    UconfigEntry* parent = d_ptr->rootEntry;
    if (parentName)
    {
        parent = d_ptr->searchEntryByName(parent, parentName);
        if (!parent)
            return false;
    }

    UconfigEntry* entry = d_ptr->searchEntryByName(parent, entryName);
    d_ptr->deleteEntry(entry);

    // Update the subentry list of parent
    int entryCount = parent->subentryCount;
    UconfigEntry** newEntryList =
            (UconfigEntry**)malloc(sizeof(UconfigEntry*) * (entryCount - 1));
    int i, j = 0;
    for (i=0; i<entryCount; i++)
    {
        if (parent->subentries[i] != entry)
            newEntryList[j++] = parent->subentries[i];
    }

    // Update the list
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
    UconfigEntry* parent = d_ptr->rootEntry;
    if (parentName)
    {
        parent = d_ptr->searchEntryByName(parent, parentName);
        if (!parent)
            return false;
    }

    UconfigEntry* entry = d_ptr->searchEntryByName(parent, entryName);
    if (!entry)
        return false;

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

    // Deep copy of subentries
    UconfigEntry** newEntries =
            (UconfigEntry**)malloc(sizeof(UconfigEntry*) * src->subentryCount);
    for (i=0; i<src->subentryCount; i++)
    {
        if (recursive)
        {
            newEntries[i] = (UconfigEntry*)malloc(sizeof(UconfigEntry));
            copyEntry(newEntries[i], src->subentries[i], true);
        }
        else
            newEntries[i] = NULL;
    }
    dest->subentries = newEntries;

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

UconfigEntry* UconfigFilePrivate::searchEntryByName(UconfigEntry* parent,
                                                    const char* name)
{
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
            entry = searchEntryByName(parent->subentries[i], name);
            if (entry)
                return entry;
        }
    }
    // If still no found, return a NULL pointer
    return NULL;
}
