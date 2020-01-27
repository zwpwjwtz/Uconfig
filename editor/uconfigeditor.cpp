#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStack>
#include "uconfigeditor.h"
#include "ui_uconfigeditor.h"
#include "parser/uconfigini.h"
#include "parser/uconfigjson.h"
#include "parser/uconfigxml.h"

#define UCONFIG_EDITOR_TREEVIEW_TEXT_ROOT   "/"
#define UCONFIG_EDITOR_TREEVIEW_TEXT_NONAME "(No name)"

#define UCONFIG_EDITOR_LISTVIEW_TEXT_NONAME "(No name)"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_RAW    "(Raw)"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_T "True"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_F "False"
#define UCONFIG_EDITOR_LISTVIEW_TEXT_MAXLEN 32

#define UCONFIG_EDITOR_FILE_SUFFIX_ALL "All (*.*)(*.*)"
#define UCONFIG_EDITOR_FILE_SUFFIX_TXT "Plain text (*.txt)(*.txt)"
#define UCONFIG_EDITOR_FILE_SUFFIX_INI "INI (*.ini)(*.ini)"
#define UCONFIG_EDITOR_FILE_SUFFIX_JSON "JSON (*.json)(*.json)"
#define UCONFIG_EDITOR_FILE_SUFFIX_XML "XML (*.xml)(*.xml)"


UconfigEditor::UconfigEditor(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::UconfigEditor)
{
    reset();

    ui->setupUi(this);
    ui->treeSubentry->setModel(&modelEntryList);
    ui->treeSubentry->setHeaderHidden(true);
    ui->treeSubentry->setExpanded(modelEntryList.index(0, 0), true);
    ui->listKey->setModel(&modelKeyList);

    currentEntry = NULL;
    lastSavingPath = QApplication::applicationDirPath();

    connect(ui->treeSubentry, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(onEntryListItemClicked(const QModelIndex&)));
}

UconfigEditor::~UconfigEditor()
{
    delete ui;

    if (currentEntry)
        delete currentEntry;
}

void UconfigEditor::reset()
{
    modified = false;

    currentFile.rootEntry.reset();
    currentFile.metadata.reset();

    resetEntryList();
    resetKeyList();

    setWindowTitle("New file");
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
    {
        return true;
    }
    else if (response == QMessageBox::No)
        return saveFile();
    else
        return false;
}

bool UconfigEditor::loadFile()
{
    QString filter;
    filter.append(UCONFIG_EDITOR_FILE_SUFFIX_ALL).append(";;")
          .append(UCONFIG_EDITOR_FILE_SUFFIX_TXT).append(";;")
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
    const char* fileNameChars = newFileName.toLocal8Bit().constData();
    if (newFileName.toLower().endsWith(".ini"))
        success = UconfigINI::readUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".json"))
        success = UconfigJSON::readUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".xml"))
        success = UconfigXML::readUconfig(fileNameChars, &currentFile);
    else
        success = UconfigKeyValue::readUconfig(fileNameChars, &currentFile);
    if (success)
    {
        fileName = newFileName;
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
    const char* fileNameChars = newFileName.toLocal8Bit().constData();
    if (newFileName.toLower().endsWith(".ini"))
        success = UconfigINI::writeUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".json"))
        success = UconfigJSON::writeUconfig(fileNameChars, &currentFile);
    else if (newFileName.toLower().endsWith(".xml"))
        success = UconfigXML::writeUconfig(fileNameChars, &currentFile);
    else
        success = UconfigKeyValue::writeUconfig(fileNameChars, &currentFile);
    if (success)
    {
        fileName = newFileName;
        return true;
    }
    else
        return false;
}

void UconfigEditor::reloadEntryList()
{
    // Clear existing subentries
    QStandardItem* root = modelEntryList.item(0, 0);
    root->removeRows(0, root->rowCount());

    // Append the subentries as child nodes
    UconfigEntryObject* subentryList = currentFile.rootEntry.subentries();
    if (subentryList)
    {
        for (int i=0; i<currentFile.rootEntry.subentryCount(); i++)
            loadEntry(root, subentryList[i]);
        delete[] subentryList;
    }
}

void UconfigEditor::reloadKeyList(UconfigEntryObject& entry)
{
    resetKeyList();

    UconfigKeyObject* keyList = entry.keys();
    if (keyList)
    {
        QString rowName;
        QList<QStandardItem*> itemList;
        for (int i=0; i<entry.keyCount(); i++)
        {
            if (keyList[i].name())
                rowName = QByteArray(keyList[i].name(),
                                     keyList[i].nameSize());
            else
                rowName = UCONFIG_EDITOR_LISTVIEW_TEXT_NONAME;

            itemList.append(new QStandardItem(rowName));
            itemList.append(new QStandardItem(
                                    keyTypeToString(keyList[i].type())));
            itemList.append(new QStandardItem(
                                    keyValueToString(keyList[i])));
            itemList[0]->setIcon(QIcon(":/icons/file.png"));

            modelKeyList.appendRow(itemList);
            itemList.clear();
        }
        delete[] keyList;
    }
}

QString UconfigEditor::keyTypeToString(UconfigValueType valueType)
{
    QString text;
    switch (valueType)
    {
        case Chars:
            text = "Chars";
            break;
        case Integer:
            text = "Integer";
            break;
        case Float:
            text = "Float";
            break;
        case Double:
            text = "Double";
            break;
        case Bool:
            text = "Boolean";
            break;
        case Raw:
        default:
            text = "Raw";
    }

    return text;
}

QString UconfigEditor::keyValueToString(const UconfigKeyObject& key)
{
    QString text;
    switch (UconfigValueType(key.type()))
    {
        case Chars:
            if (key.valueSize() > UCONFIG_EDITOR_LISTVIEW_TEXT_MAXLEN)
            {
                text = QByteArray(key.value(),
                                  UCONFIG_EDITOR_LISTVIEW_TEXT_MAXLEN);
                text.append("...");
            }
            else
                text = QByteArray(key.value(), key.valueSize());
            break;
        case Integer:
            text = QString::number(*((int*)(key.value())));
            break;
        case Float:
            text = QString::number(*((float*)(key.value())));
            break;
        case Double:
            text = QString::number(*((double*)(key.value())));
            break;
        case Bool:
            text = *((bool*)(key.value())) ?
                   UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_T :
                   UCONFIG_EDITOR_LISTVIEW_TEXT_BOOL_F;
            break;
        case Raw:
        default:
            text = UCONFIG_EDITOR_LISTVIEW_TEXT_RAW;
    }

    return text;
}

void UconfigEditor::resetEntryList()
{
    QStandardItem* root = new QStandardItem;
    root->setText(UCONFIG_EDITOR_TREEVIEW_TEXT_ROOT);
    root->setEditable(false);
    modelEntryList.clear();
    modelEntryList.appendRow(root);
}

void UconfigEditor::resetKeyList()
{
    modelKeyList.clear();
    modelKeyList.setColumnCount(3);
    modelKeyList.setHeaderData(0, Qt::Horizontal, "Name");
    modelKeyList.setHeaderData(1, Qt::Horizontal, "Type");
    modelKeyList.setHeaderData(2, Qt::Horizontal, "NameValue");
}

void UconfigEditor::loadEntry(QStandardItem* parent, UconfigEntryObject& entry)
{
    // Load entries into node recursively
    QString nodeName;
    if (entry.nameSize() > 0)
        nodeName = QByteArray(entry.name(), entry.nameSize());
    else
        nodeName = UCONFIG_EDITOR_TREEVIEW_TEXT_NONAME;

    QStandardItem* childNode = new QStandardItem;
    childNode->setText(nodeName);
    childNode->setIcon(QIcon(":/icons/directory.png"));
    parent->appendRow(childNode);

    // Deal with subentries...
    UconfigEntryObject* subentryList = entry.subentries();
    if (subentryList)
    {
        for (int i=0; i<entry.subentryCount(); i++)
            loadEntry(childNode, subentryList[i]);
        delete[] subentryList;
    }
}

UconfigEntryObject* UconfigEditor::modelIndexToEntry(const QModelIndex& index)
{
    // Reverse mapping from a QStandardItemModel index to an Uconfig entry

    // First, find a path from the node of given index to the root node
    // Record the row indexes along the path
    QStack<int> rowIndexes;
    QModelIndex currentIndex = index;
    QModelIndex rootIndex = modelEntryList.index(0, 0);
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

void UconfigEditor::closeEvent(QCloseEvent* event)
{
    if (!confirmSaving())
        event->ignore();
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

void UconfigEditor::on_actionDuplicate_triggered()
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
