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

    bool confirmSaving();
    bool loadFile();
    bool saveFile(bool forceSavingAs = false);

    void reloadEntryList();
    void reloadKeyList(UconfigEntryObject& entry);

    static QString keyTypeToString(UconfigValueType valueType);
    static QString keyValueToString(const UconfigKeyObject& key);

private:
    Ui::UconfigEditor *ui;

protected:
    bool modified = false;
    UconfigFile currentFile;
    UconfigEntryObject* currentEntry;

    QStandardItemModel modelEntryList;
    QStandardItemModel modelKeyList;
    QStandardItemModel modelKeyListHeader;

    void resetEntryList();
    void resetKeyList();

    void loadEntry(QStandardItem* parent, UconfigEntryObject& entry);
    UconfigEntryObject* modelIndexToEntry(const QModelIndex& item);

    void closeEvent(QCloseEvent* event);

private slots:
    void onEntryListItemClicked(const QModelIndex& index);

    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionQuit_triggered();
    void on_actionFind_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionPaste_triggered();
    void on_actionDuplicate_triggered();
    void on_actionAbout_triggered();
};

#endif // UCONFIGEDITOR_H
