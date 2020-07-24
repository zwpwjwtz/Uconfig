#include <QMessageBox>
#include "hexeditdialog.h"
#include "ui_hexeditdialog.h"


HexEditDialog::HexEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HexEditDialog)
{
    ui->setupUi(this);

    hexEdit = new QHexEdit;
    ui->layout->addWidget(hexEdit);

    modified = false;

    connect(hexEdit, SIGNAL(dataChanged()),
            this, SLOT(onHexEditDataChanged()));
}

HexEditDialog::~HexEditDialog()
{
    delete ui;
}

QByteArray HexEditDialog::getData()
{
    return hexEdit->data();
}

void HexEditDialog::setData(const QByteArray& data)
{
    hexEdit->setData(data);
    modified = false;
}

bool HexEditDialog::isModified()
{
    return modified;
}

void HexEditDialog::closeEvent(QCloseEvent* event)
{
    if (modified)
    {
        if (QMessageBox::question(this, "Value modified",
                                  "The value has been modified. "
                                  "Do you want to save it?")
                != QMessageBox::Yes)
            modified = false;
    }
}

void HexEditDialog::onHexEditDataChanged()
{
    modified = true;
}

void HexEditDialog::on_buttonBox_rejected()
{
    modified = false;
}
