#ifndef VALUEEDITORDELEGATE_H
#define VALUEEDITORDELEGATE_H

#include <QItemDelegate>
#include "hexeditdialog.h"


class UconfigEditor;

class ValueEditorDelegate : public QItemDelegate
{
public:
    ValueEditorDelegate(UconfigEditor* parent);

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const;

private:
    UconfigEditor* mainEditor;
};

#endif // VALUEEDITORDELEGATE_H
