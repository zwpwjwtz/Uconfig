#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStack>
#include "uconfigeditor.h"
#include "ui_uconfigeditor.h"
#include "hexeditdialog.h"
#include "valueeditordelegate.h"
#include "parser/uconfigini.h"
#include "parser/uconfigcsv.h"
#include "parser/uconfigjson.h"
#include "parser/uconfigxml.h"

#define UCONFIG_EDITOR_ENTRY_NAME_PREFIX    "UCONFIGEDITOR_ENTRY_"
#define UCONFIG_EDITOR_KEY_NAME_PREFIX      "UCONFIGEDITOR_KEY_"

#define UCONFIG_EDITOR_TREEVIEW_TEXT_ROOT   "/"
#define UCONFIG_EDITOR_TREEVIEW_TEXT_NONAME "(No name)"
#define UCONFIG_EDITOR_TREEVIEW_TEXT_NEW    "New entry"

#define UCONFIG_EDITOR_LISTVIEW_TEXT_NONAME "(No name)"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_NEW    "New key"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_T "True"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_F "False"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_MAXLEN 32

#define UCONFIG_EDITOR_FILE_SUFFIX_ALL "All (*.*)(*)"
#define UCONFIG_EDITOR_FILE_SUFFIX_TXT "Plain text (*.txt)(*.txt)"
#define UCONFIG_EDITOR_FILE_SUFFIX_CSV "CSV (*.csv)(*.csv)"
#define UCONFIG_EDITOR_FILE_SUFFIX_INI "INI (*.ini)(*.ini)"
#define UCONFIG_EDITOR_FILE_SUFFIX_JSON "JSON (*.json)(*.json)"
#define UCONFIG_EDITOR_FILE_SUFFIX_XML "XML (*.xml)(*.xml)"


UconfigEditor::UconfigEditor(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::UconfigEditor)
{
    reset();

    menuTreeSubentry = NULL;
    menuListKey = NULL;
    hexEditor = NULL;
    valueEditor = new ValueEditorDelegate(this);

    ui->setupUi(this);
    ui->treeSubentry->setModel(&modelEntryList);
    ui->treeSubentry->setHeaderHidden(true);
    ui->treeSubentry->setExpanded(entryListRoot->index(), true);
    ui->listKey->setModel(&modelKeyList);
    ui->listKey->setItemDelegateForColumn(2, valueEditor);
    updateWindowTitle();

    connect(ui->treeSubentry, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(onEntryListItemClicked(const QModelIndex&)));
    connect(&modelEntryList, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(onEntryListItemChanged(QStandardItem*)));
    connect(&modelKeyList, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(onKeyListItemChanged(QStandardItem*)));
}

UconfigEditor::~UconfigEditor()
{
    delete ui;
    delete valueEditor;

    if (currentEntry)
        delete currentEntry;
}

void UconfigEditor::reset()
{
    modified = false;
    fileName.clear();
    lastSavingFilter.clear();
    lastSavingPath = QApplication::applicationDirPath();

    currentFile.rootEntry.reset();
    currentFile.metadata.reset();

    currentEntry = NULL;

    resetEntryList();
    resetKeyList();
}

bool UconfigEditor::confirmSaving()
{
    if (!modified)
        return true;

    QMessageBox::StandardButton response =
        QMessageBox::warning(this, "File not saved",
                             "File has been modified but not saved. \n"
                             "Save it before leaving?",
                             QMessageBox::Yes |
                             QMessageBox::No |
                             QMessageBox::Cancel);
    if (response == QMessageBox::Yes)
        return saveFile();
    else if (response == QMessageBox::No)
        return true;
    else
        return false;
}

bool UconfigEditor::loadFile()
{
    QString filter;
    filter.append(UCONFIG_EDITOR_FILE_SUFFIX_ALL).append(";;")
          .append(UCONFIG_EDITOR_FILE_SUFFIX_TXT).append(";;")
          .append(UCONFIG_EDITOR_FILE_SUFFIX_CSV).append(";;")
          .append(UCONFIG_EDITOR_FILE_SUFFIX_INI).append(";;")
          .append(UCONFIG_EDITOR_FILE_SUFFIX_JSON).append(";;")
          .append(UCONFIG_EDITOR_FILE_SUFFIX_XML);

    QString newFileName = QFileDialog::getOpenFileName(
                                            this, "Load file...",
                                            lastSavingPath,
                                            filter, &lastSavingFilter);

    if (newFileName.isEmpty())
        return false;

    lastSavingPath = newFileName;

    bool success = false;
    QByteArray encodedFileName = newFileName.toLocal8Bit();
    const char* fileNameChars = encodedFileName.constData();
    if (newFileName.toLower().endsWith(".ini"))
        success = UconfigINI::readUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".csv"))
        success = UconfigCSV::readUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".json"))
        success = UconfigJSON::readUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".xml"))
        success = UconfigXML::readUconfig(fileNameChars, &currentFile);
    else
        success = UconfigKeyValue::readUconfig(fileNameChars, &currentFile);
    if (success)
    {
        fileName = newFileName;
        updateWindowTitle();
        reloadEntryList();
        return true;
    }
    else
        return false;
}

bool UconfigEditor::saveFile(bool forceSavingAs)
{
    if (!modified && !forceSavingAs)
        return true;

    QString newFileName;
    if (fileName.isEmpty() || forceSavingAs)
    {
        // Show "Save As" dialog
        QString filter;
        filter.append(UCONFIG_EDITOR_FILE_SUFFIX_ALL).append(";;")
              .append(UCONFIG_EDITOR_FILE_SUFFIX_TXT).append(";;")
              .append(UCONFIG_EDITOR_FILE_SUFFIX_CSV).append(";;")
              .append(UCONFIG_EDITOR_FILE_SUFFIX_INI).append(";;")
              .append(UCONFIG_EDITOR_FILE_SUFFIX_JSON).append(";;")
              .append(UCONFIG_EDITOR_FILE_SUFFIX_XML);

        newFileName = QFileDialog::getSaveFileName(
                                        this, "Save file as...",
                                        lastSavingPath,
                                        filter, &lastSavingFilter);

    }
    else
        newFileName = fileName;

    if (newFileName.isEmpty())
        return false;
    else
        lastSavingPath = newFileName;

    bool success = false;
    QByteArray encodedFileName = newFileName.toLocal8Bit();
    const char* fileNameChars = encodedFileName.constData();
    if (newFileName.toLower().endsWith(".ini"))
        success = UconfigINI::writeUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".csv"))
        success = UconfigCSV::writeUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".json"))
        success = UconfigJSON::writeUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".xml"))
        success = UconfigXML::writeUconfig(fileNameChars, &currentFile);
    else
        success = UconfigKeyValue::writeUconfig(fileNameChars, &currentFile);
    if (success)
    {
        fileName = newFileName;
        updateWindowTitle();
        modified = false;
        return true;
    }
    else
        return false;
}

void UconfigEditor::updateWindowTitle()
{
    QString windowTitle(QFileInfo(fileName).fileName());
    if (windowTitle.isEmpty())
        windowTitle = "Uconfig Editor";
    else
        windowTitle.append(" - Uconfig Editor");
    setWindowTitle(windowTitle);
}

void UconfigEditor::reloadEntryList()
{
    // Clear existing subentries
    entryListRoot->removeRows(0, entryListRoot->rowCount());

    // Append the subentries as child nodes
    UconfigEntryObject* subentryList = currentFile.rootEntry.subentries();
    if (subentryList)
    {
        for (int i=0; i<currentFile.rootEntry.subentryCount(); i++)
            loadEntry(entryListRoot, subentryList[i]);
        delete[] subentryList;
    }
}

bool UconfigEditor::addSubentry(const QModelIndex& parentIndex,
                                const UconfigEntryObject* newEntry)
{
    UconfigEntryObject* parent = modelIndexToEntry(parentIndex);
    if (!parent)
        parent = &currentFile.rootEntry;

    if (newEntry)
    {
        parent->addSubentry(newEntry);
        loadEntry(modelEntryList.itemFromIndex(parentIndex), *newEntry);
    }
    else
    {
        UconfigEntryObject tempEntry;
        tempEntry.setName(UCONFIG_EDITOR_TREEVIEW_TEXT_NEW);
        parent->addSubentry(&tempEntry);
        loadEntry(modelEntryList.itemFromIndex(parentIndex), tempEntry);
    }

    modified = true;
    return true;
}

bool UconfigEditor::removeEntry(const QModelIndex& index)
{
    if (!index.isValid() || index == entryListRoot->index())
        return false;

    UconfigEntryObject* entry = modelIndexToEntry(index);
    if (entry)
    {
        // First assign the entry a temporary but unique name
        UconfigEntryObject parent = entry->parentEntry();
        QByteArray tempName;
        while (true)
        {
            tempName = UCONFIG_EDITOR_ENTRY_NAME_PREFIX;
            tempName.append(QByteArray::number(qrand()));
            if (!parent.existSubentry(tempName.constData(), tempName.size()))
                break;
        }
        entry->setName(tempName.constData(), tempName.size());

        // Then delete the entry by its name
        parent.deleteSubentry(tempName.constData(), tempName.size());
    }
    modelEntryList.removeRow(index.row(), index.parent());

    modified = true;
    return true;
}

void UconfigEditor::reloadKeyList(UconfigEntryObject& entry)
{
    resetKeyList();

    UconfigKeyObject* keyList = entry.keys();
    if (keyList)
    {
        for (int i=0; i<entry.keyCount(); i++)
            loadKey(keyList[i]);
        delete[] keyList;
    }
}

bool UconfigEditor::addKey(const UconfigKeyObject* newKey)
{
    if (!currentEntry)
        return false;

    if (newKey)
    {
        currentEntry->addKey(newKey);
        loadKey(*newKey);
    }
    else
    {
        UconfigKeyObject tempKey;
        tempKey.setName(UCONFIG_EDITOR_LISTVIEW_TEXT_NEW);
        currentEntry->addKey(&tempKey);
        loadKey(tempKey);
    }

    modified = true;
    return true;
}

bool UconfigEditor::removeKey(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    UconfigKeyObject* key = modelIndexToKey(index);
    if (key && currentEntry)
    {
        // First assign the key a temporary but unique name
        QByteArray tempName;
        while (true)
        {
            tempName = UCONFIG_EDITOR_KEY_NAME_PREFIX;
            tempName.append(QByteArray::number(qrand()));
            if (!currentEntry->existKey(tempName.constData(), tempName.size()))
                break;
        }
        key->setName(tempName.constData(), tempName.size());

        // Then delete the key by its name
        currentEntry->deleteKey(tempName.constData(), tempName.size());
    }
    modelKeyList.removeRow(index.row(), index.parent());

    modified = true;
    return true;
}

QString UconfigEditor::keyTypeToString(int valueType)
{
    typedef UconfigIO::ValueType ValueType;

    QString text;
    switch (ValueType(valueType))
    {
        case ValueType::Chars:
            text = "Chars";
            break;
        case ValueType::Integer:
            text = "Integer";
            break;
        case ValueType::Float:
            text = "Float";
            break;
        case ValueType::Double:
            text = "Double";
            break;
        case ValueType::Bool:
            text = "Boolean";
            break;
        case ValueType::Raw:
        default:
            text = "Raw";
    }

    return text;
}

QString UconfigEditor::keyValueToString(const UconfigKeyObject& key)
{
    typedef UconfigIO::ValueType ValueType;

    QString text;
    switch (ValueType(key.type()))
    {
        case ValueType::Integer:
            text = QString::number(*((int*)(key.value())));
            break;
        case ValueType::Float:
            text = QString::number(*((float*)(key.value())));
            break;
        case ValueType::Double:
            text = QString::number(*((double*)(key.value())));
            break;
        case ValueType::Bool:
            text = *((bool*)(key.value())) ?
                   UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_T :
                   UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_F;
            break;
        case ValueType::Chars:
        case ValueType::Raw:
        default:
            if (key.valueSize() > UCONFIG_EDITOR_LISTVIEW_TEXT_MAXLEN)
            {
                text = QByteArray(key.value(),
                                  UCONFIG_EDITOR_LISTVIEW_TEXT_MAXLEN);
                text.append("...");
            }
            else
                text = QByteArray(key.value(), key.valueSize());
    }

    return text;
}

void UconfigEditor::resetEntryList()
{
    entryListRoot = new QStandardItem;
    entryListRoot->setText(UCONFIG_EDITOR_TREEVIEW_TEXT_ROOT);
    entryListRoot->setEditable(false);
    modelEntryList.clear();
    modelEntryList.appendRow(entryListRoot);
}

void UconfigEditor::resetKeyList()
{
    modelKeyList.clear();
    modelKeyList.setColumnCount(3);
    modelKeyList.setHeaderData(0, Qt::Horizontal, "Name");
    modelKeyList.setHeaderData(1, Qt::Horizontal, "Type");
    modelKeyList.setHeaderData(2, Qt::Horizontal, "Value");
}

void UconfigEditor::loadEntry(QStandardItem* parent,
                              const UconfigEntryObject& entry)
{
    // Load entries into node recursively
    static QString nodeName;

    if (entry.nameSize() > 0)
        nodeName = QByteArray(entry.name(), entry.nameSize());
    else
        nodeName = UCONFIG_EDITOR_TREEVIEW_TEXT_NONAME;

    QStandardItem* childNode = new QStandardItem;
    childNode->setText(nodeName);
    childNode->setIcon(QIcon(":/icons/directory.png"));
    parent->appendRow(childNode);

    // Deal with subentries...
    UconfigEntryObject* subentryList =
                        const_cast<UconfigEntryObject&>(entry).subentries();
    if (subentryList)
    {
        for (int i=0; i<entry.subentryCount(); i++)
            loadEntry(childNode, subentryList[i]);
        delete[] subentryList;
    }
}


void UconfigEditor::loadKey(const UconfigKeyObject& key)
{
    static QString rowName;
    static QList<QStandardItem*> itemList;

    if (key.name())
        rowName = QByteArray(key.name(), key.nameSize());
    else
        rowName = UCONFIG_EDITOR_LISTVIEW_TEXT_NONAME;

    itemList.clear();
    itemList.append(new QStandardItem(rowName));
    itemList.append(new QStandardItem(keyTypeToString(key.type())));
    itemList.append(new QStandardItem(keyValueToString(key)));
    itemList[0]->setIcon(QIcon(":/icons/file.png"));
    itemList[1]->setEditable(false);
    itemList[2]->setEditable(key.type() != UconfigIO::ValueType::Raw);

    modelKeyList.appendRow(itemList);
}

void UconfigEditor::updateKey(const UconfigKeyObject& key, int row)
{
    if (modelKeyList.rowCount() <= row)
        return;

    static QString rowName;
    if (key.name())
        rowName = QByteArray(key.name(), key.nameSize());
    else
        rowName = UCONFIG_EDITOR_LISTVIEW_TEXT_NONAME;

    modelKeyList.item(row, 0)->setText(rowName);
    modelKeyList.item(row, 1)->setText(keyTypeToString(key.type()));
    modelKeyList.item(row, 2)->setText(keyValueToString(key));
    modelKeyList.item(row, 2)->
                        setEditable(key.type() != UconfigIO::ValueType::Raw);
}

UconfigEntryObject* UconfigEditor::modelIndexToEntry(const QModelIndex& index)
{
    // Reverse mapping from a QStandardItemModel index to an Uconfig entry

    // First, find a path from the node of given index to the root node
    // Record the row indexes along the path
    QStack<int> rowIndexes;
    QModelIndex currentIndex = index;
    QModelIndex rootIndex = entryListRoot->index();
    while (!(currentIndex == rootIndex))
    {
        rowIndexes.push(currentIndex.row());
        currentIndex = currentIndex.parent();
    }

    // Then locate the entry in file by reversing the row indexes
    UconfigEntryObject* currentEntry = NULL;
    UconfigEntryObject* subentryList = currentFile.rootEntry.subentries();
    while (!rowIndexes.isEmpty() && subentryList)
    {
        // At each level, create a new copy of the reference of
        // the entry found, as the original reference in subentryList
        // will be destroyed once subentryList is deleted.
        // Here we use the copy-assignment operator "=" to create a copy,
        // which generates a new reference to the original reference.
        if (!currentEntry)
            currentEntry = new UconfigEntryObject;
        *currentEntry = subentryList[rowIndexes.pop()];

        // Step into next level
        delete[] subentryList;
        subentryList = currentEntry->subentries();
    }

    if (subentryList)
        delete[] subentryList;

    return currentEntry;
}

UconfigKeyObject* UconfigEditor::modelIndexToKey(const QModelIndex& index)
{
    // Reverse mapping from a QStandardItemModel index to an Uconfig key
    UconfigKeyObject* currentKey = NULL;

    if (!currentEntry)
        return currentKey;

    UconfigKeyObject* keyList = currentEntry->keys();
    if (keyList)
    {
        currentKey = new UconfigKeyObject;
        *currentKey = keyList[index.row()]; // Do a shallow copy
        delete[] keyList;
    }
    return currentKey;
}

void UconfigEditor::closeEvent(QCloseEvent* event)
{
    if (!confirmSaving())
        event->ignore();
}

void UconfigEditor::on_actionNew_triggered()
{
    if (confirmSaving())
        reset();
}

void UconfigEditor::on_actionOpen_triggered()
{
    if (confirmSaving())
        loadFile();
}

void UconfigEditor::on_actionSave_triggered()
{
    saveFile();
}

void UconfigEditor::on_actionSaveAs_triggered()
{
    saveFile(true);
}

void UconfigEditor::on_actionQuit_triggered()
{
    close();
}

void UconfigEditor::on_actionFind_triggered()
{

}

void UconfigEditor::on_actionCopy_triggered()
{

}

void UconfigEditor::on_actionCut_triggered()
{

}

void UconfigEditor::on_actionPaste_triggered()
{

}

void UconfigEditor::on_actionAbout_triggered()
{
    QMessageBox::about(this, "Uconfig Editor",
                       "This is a simple GUI for editing config files "
                       "with Uconfig library. \n"
                       "No guarantee for data safety. "
                       "Back up your data before using!");
}

void
UconfigEditor::on_treeSubentry_customContextMenuRequested(const QPoint &pos)
{
    if (pos.isNull())
        return;

    if (menuTreeSubentry == NULL)
    {
        menuTreeSubentry = new QMenu(ui->treeSubentry);

        QAction* action;

        action = new QAction("&Add subentry", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionAddSubentry_triggered()));
        menuTreeSubentry->addAction(action);

        menuTreeSubentry->addSeparator();

        action = new QAction("&Duplicate", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionDuplicateEntry_triggered()));
        menuTreeSubentry->addAction(action);

        action = new QAction("Dele&te", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionDeleteEntry_triggered()));
        menuTreeSubentry->addAction(action);

        action = new QAction("&Rename", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionRenameEntry_triggered()));
        menuTreeSubentry->addAction(action);
    }
    menuTreeSubentry->exec(QCursor::pos());
}

void UconfigEditor::on_treeSubentry_doubleClicked(const QModelIndex &index)
{
    onActionRenameEntry_triggered();
}

void
UconfigEditor::on_listKey_customContextMenuRequested(const QPoint &pos)
{
    if (pos.isNull())
        return;

    if (menuListKey == NULL)
    {
        menuListKey = new QMenu(ui->listKey);

        QAction* action;

        action = new QAction("&Add key", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionAddKey_triggered()));
        menuListKey->addAction(action);

        action = new QAction("&Duplicate", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionDuplicateKey_triggered()));
        menuListKey->addAction(action);

        menuListKey->addSeparator();

        action = new QAction("Dele&te", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionDeleteKey_triggered()));
        menuListKey->addAction(action);

        action = new QAction("&Rename", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(onActionRenameKey_triggered()));
        menuListKey->addAction(action);
    }

    menuListKey->exec(QCursor::pos());
}

void UconfigEditor::on_listKey_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (index.column() == 0)
    {
        // Key name is clicked
        onActionRenameKey_triggered();
    }
    else if (index.column() == 2)
    {
        // Value content is clicked
        UconfigKeyObject* key = modelIndexToKey(index);
        if (key->type() == UconfigIO::ValueType::Raw)
        {
            // Launch QHexEdit Dialog
            if (!hexEditor)
                hexEditor = new HexEditDialog(this);
            hexEditor->setData(QByteArray(key->value(), key->valueSize()));
            hexEditor->exec();

            if (hexEditor->isModified())
            {
                // Update the value
                QByteArray newData = hexEditor->getData();
                key->setValue(newData.constData(), newData.size());
                updateKey(*key, index.row());
            }
        }
    }
}

void UconfigEditor::onEntryListItemClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    UconfigEntryObject* entry = modelIndexToEntry(index);
    if (entry)
        reloadKeyList(*entry);
    else
        resetKeyList();

    if (currentEntry)
        delete currentEntry;
    currentEntry = entry;
}

void UconfigEditor::onEntryListItemChanged(QStandardItem* item)
{
    // Subentry name has been modified: write it back to UconfigEntryObject
    QByteArray newName = item->text().toLocal8Bit();
    modelIndexToEntry(item->index())->setName(newName.constData(),
                                              newName.size());
    modified = true;
}

void UconfigEditor::onKeyListItemChanged(QStandardItem* item)
{
    QModelIndex index = item->index();
    switch (index.column())
    {
        case 0:
        {
            // Key renamed
            QByteArray newName = item->text().toLocal8Bit();
            modelIndexToKey(index)->setName(newName.constData(),
                                            newName.size());
            modified = true;
            break;
        }
        case 1:
            // Type changed: not implemented yet
            break;
        case 2:
            // Value changed: handled in ValueEditorDelegate::setModelData()
        default:;
    }
}

void UconfigEditor::onActionAddSubentry_triggered()
{
    if (ui->treeSubentry->currentIndex().isValid())
        addSubentry(ui->treeSubentry->currentIndex());
}

void UconfigEditor::onActionDuplicateEntry_triggered()
{
    QModelIndex index = ui->treeSubentry->currentIndex();
    if (!index.isValid())
        return;
    if (index == entryListRoot->index())
    {
        QMessageBox::warning(this, "Illegal operation",
                             "Root entry cannot be duplicated.");
        return;
    }
    addSubentry(index.parent(), modelIndexToEntry(index));
}

void UconfigEditor::onActionDeleteEntry_triggered()
{
    QModelIndex index = ui->treeSubentry->currentIndex();
    if (!index.isValid())
        return;
    if (index == entryListRoot->index())
    {
        QMessageBox::warning(this, "Illegal operation",
                             "Root entry cannot be deleted.");
        return;
    }
    removeEntry(index);
}

void UconfigEditor::onActionRenameEntry_triggered()
{
    QModelIndex index = ui->treeSubentry->currentIndex();
    if (index.isValid() && index != entryListRoot->index())
    {
        ui->treeSubentry->openPersistentEditor(index);
        QWidget* inlineEditor = ui->treeSubentry->indexWidget(index);
        if (inlineEditor)
        {
            inlineEditor->setFocus();
            connect(inlineEditor, SIGNAL(editingFinished()),
                    inlineEditor, SLOT(deleteLater()));
        }
    }
}

void UconfigEditor::onActionAddKey_triggered()
{
    QModelIndex index = ui->treeSubentry->currentIndex();
    if (!index.isValid())
        return;
    if (index == entryListRoot->index())
    {
        QMessageBox::warning(this, "Illegal operation",
                             "Root entry cannot possess keys.");
        return;
    }
    addKey();
}

void UconfigEditor::onActionDuplicateKey_triggered()
{
    if (ui->listKey->currentIndex().isValid())
        addKey(modelIndexToKey(ui->listKey->currentIndex()));
}

void UconfigEditor::onActionDeleteKey_triggered()
{
    if (ui->listKey->currentIndex().isValid())
        removeKey(ui->listKey->currentIndex());
}

void UconfigEditor::onActionRenameKey_triggered()
{
    QModelIndex index = ui->listKey->currentIndex();
    if (index.isValid())
    {
        index = index.sibling(index.row(), 0);
        ui->listKey->openPersistentEditor(index);
        QWidget* inlineEditor = ui->listKey->indexWidget(index);
        if (inlineEditor)
        {
            inlineEditor->setFocus();
            connect(inlineEditor, SIGNAL(editingFinished()),
                    inlineEditor, SLOT(deleteLater()));
        }
    }
}
