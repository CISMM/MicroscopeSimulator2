#include <QModelObjectPropertyListItemDelegate.h>

#include <QPushButton>
#include <QModelObjectPropertyListTableModel.h>

#include <iostream>

QModelObjectPropertyListItemDelegate::QModelObjectPropertyListItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent) {
}


QWidget*
QModelObjectPropertyListItemDelegate
::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
               const QModelIndex& index) const {
  if (index.column() == 1) {
    QPushButton* button = new QPushButton(parent);
    button->setText("Hi");
    return button;
  }

  return QStyledItemDelegate::createEditor(parent, option, index);
}


void
QModelObjectPropertyListItemDelegate
::setEditorData(QWidget* editor, const QModelIndex& index) const {
  QModelObjectPropertyListTableModel *model =
    static_cast<QModelObjectPropertyListTableModel*>(const_cast<QAbstractItemModel*>(index.model()));

  QString value = model->data(index, Qt::EditRole).toString();

  std::cout << value.toStdString() << std::endl;

  QPushButton *button = static_cast<QPushButton*>(editor);
  button->setText(value);
}

