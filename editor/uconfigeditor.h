#ifndef UCONFIGEDITOR_H
#define UCONFIGEDITOR_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "parser/uconfigfile.h"


namespace Ui {
class UconfigEditor;
}

class UconfigEditor : public QMainWindow
{
    Q_OBJECT

public:
    enum class FileType
    {
        Unknown = 0,
        KeyValue = 1,
        WinINI = 2,
        TwoDimTable = 3,
        JSON = 4,
        XML = 5
    };

    QString fileName;
    QString lastSavingPath;
    QString lastSavingFilter;

    explicit UconfigEditor(QWidget* parent = 0);
    ~UconfigEditor();

    void reset();

    // File I/O
    bool confirmSaving();
    bool loadFile();
    bool saveFile(bool forceSavingAs = false);

    // UI behavior
    void updateWindowTitle();

    // Entry list operations
    void reloadEntryList();
    bool addSubentry(const QModelIndex& parentIndex,
                     const UconfigEntryObject* newEntry = NULL);
    bool removeEntry(const QModelIndex& index);

    // Key list operations
    void reloadKeyList(UconfigEntryObject& entry);
    bool addKey(const UconfigKeyObject* newKey = NULL);
    bool removeKey(const QModelIndex& index);

    // Helper functions
    static QString keyTypeToString(UconfigValueType valueType);
    static QString keyValueToString(const UconfigKeyObject& key);

private:
    Ui::UconfigEditor *ui;
    QMenu* menuTreeSubentry;
    QMenu* menuListKey;

protected:
    bool modified = false;
    UconfigFile currentFile;
    UconfigEntryObject* currentEntry;

    QStandardItem* entryListRoot;
    QStandardItemModel modelEntryList;
    QStandardItemModel modelKeyList;
    QStandardItemModel modelKeyListHeader;

    void resetEntryList();
    void resetKeyList();

    void loadEntry(QStandardItem* parent, const UconfigEntryObject &entry);
    void loadKey(const UconfigKeyObject& key);

    UconfigEntryObject* modelIndexToEntry(const QModelIndex& item);
    UconfigKeyObject* modelIndexToKey(const QModelIndex& index);

    void closeEvent(QCloseEvent* event);

private slots:
    // Auto-connected slots
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionQuit_triggered();
    void on_actionFind_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionPaste_triggered();
    void on_actionAbout_triggered();
    void on_treeSubentry_customContextMenuRequested(const QPoint &pos);
    void on_listKey_customContextMenuRequested(const QPoint &pos);

    // Manually connected slots
    void onEntryListItemClicked(const QModelIndex& index);
    void onActionAddSubentry_triggered();
    void onActionDuplicateEntry_triggered();
    void onActionDeleteEntry_triggered();
    void onActionAddKey_triggered();
    void onActionDuplicateKey_triggered();
    void onActionDeleteKey_triggered();
};

#endif // UCONFIGEDITOR_H
