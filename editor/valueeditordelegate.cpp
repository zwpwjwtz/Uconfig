#include <float.h>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include "valueeditordelegate.h"
#include "uconfigeditor.h"
#include "../parser/uconfigio.h"

#define UCONFIG_EDITOR_VALUE_TEXT_BOOL_T "True"
#define UCONFIG_EDITOR_VALUE_TEXT_BOOL_F "False"


ValueEditorDelegate::ValueEditorDelegate(UconfigEditor* parent)
{
    mainEditor = parent;
}

QWidget* ValueEditorDelegate::createEditor(QWidget* parent,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const
{
    QWidget* newEditor;
    using ValueType = UconfigIO::ValueType;
    UconfigKeyObject* key = mainEditor->modelIndexToKey(index);
    switch (key->type())
    {
        case ValueType::Bool:
        {
            QComboBox* boolValueEditor = new QComboBox(parent);
            boolValueEditor->addItem(UCONFIG_EDITOR_VALUE_TEXT_BOOL_T);
            boolValueEditor->addItem(UCONFIG_EDITOR_VALUE_TEXT_BOOL_F);
            newEditor = boolValueEditor;
            break;
        }
        case ValueType::Chars:
            newEditor = new QLineEdit(parent);
            break;
        case ValueType::Integer:
        {
            QSpinBox* intValueEditor = new QSpinBox(parent);
            intValueEditor->setMinimum(INT_MIN);
            intValueEditor->setMaximum(INT_MAX);
            newEditor = intValueEditor;
            break;
        }
        case ValueType::Double:
        {
            QDoubleSpinBox* doubleValueEditor = new QDoubleSpinBox(parent);
            doubleValueEditor->setMinimum(DBL_MIN);
            doubleValueEditor->setMaximum(DBL_MAX);
            newEditor = doubleValueEditor;
            break;
        }
        default:;
    }
    return newEditor;
}

void ValueEditorDelegate::setEditorData(QWidget* editor,
                                        const QModelIndex& index) const
{
    // Load value directly from UconfigKeyObject
    using ValueType = UconfigIO::ValueType;
    UconfigKeyObject* key = mainEditor->modelIndexToKey(index);
    switch (key->type())
    {
        case ValueType::Bool:
            bool boolValue;
            memcpy(&boolValue, key->value(), key->valueSize());
            static_cast<QComboBox*>(editor)->setCurrentIndex(boolValue ? 1 : 0);
            break;
        case ValueType::Chars:
            static_cast<QLineEdit*>(editor)->
                            setText(QByteArray(key->value(), key->valueSize()));
            break;
        case ValueType::Integer:
            int intValue;
            memcpy(&intValue, key->value(), key->valueSize());
            static_cast<QSpinBox*>(editor)->setValue(intValue);
            break;
        case ValueType::Double:
            double doubleValue;
            memcpy(&doubleValue, key->value(), key->valueSize());
            static_cast<QSpinBox*>(editor)->setValue(doubleValue);
            break;
        default:;
    }
}

void ValueEditorDelegate::setModelData(QWidget* editor,
                                       QAbstractItemModel* model,
                                       const QModelIndex& index) const
{
    // Write the new value back to UconfigKeyObject
    using ValueType = UconfigIO::ValueType;
    UconfigKeyObject* key = mainEditor->modelIndexToKey(index);
    switch (key->type())
    {
        case ValueType::Bool:
        {
            bool boolValue =
                    static_cast<QComboBox*>(editor)->currentIndex() > 0;
            key->setValue((char*)(&boolValue), sizeof(bool));
            break;
        }
        case ValueType::Chars:
        {
            QByteArray charValue =
                    static_cast<QLineEdit*>(editor)->text().toLocal8Bit();
            key->setValue(charValue.constData(),
                          charValue.size() * sizeof(char));
            break;
        }
        case ValueType::Integer:
        {
            int intValue = static_cast<QSpinBox*>(editor)->value();
            key->setValue((char*)(&intValue), sizeof(int));
            break;
        }
        case ValueType::Double:
        {
            double doubleValue = static_cast<QDoubleSpinBox*>(editor)->value();
            key->setValue((char*)(&doubleValue), sizeof(double));
            break;
        }
        default:;
    }
    mainEditor->modified = true;

    // Then update the displayed value
    mainEditor->updateKey(*key, index.row());
}
