#ifndef HEXEDITDIALOG_H
#define HEXEDITDIALOG_H

#include <QDialog>

#include "qhexedit2/qhexedit.h"

namespace Ui {
class HexEditDialog;
}

class HexEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HexEditDialog(QWidget *parent = 0);
    ~HexEditDialog();

    QByteArray getData();
    void setData(const QByteArray& data);
    bool isModified();

protected:
    void closeEvent(QCloseEvent* event);

private:
    Ui::HexEditDialog *ui;
    QHexEdit* hexEdit;

    bool modified;

private slots:
    void onHexEditDataChanged();
    void on_buttonBox_rejected();
};

#endif // HEXEDITDIALOG_H
