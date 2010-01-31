#ifndef _Q_MODEL_OBJECT_PROPERTY_LIST_ITEM_DELEGATE_H_
#define _Q_MODEL_OBJECT_PROPERTY_LIST_ITEM_DELEGATE_H_

#include <QStyledItemDelegate>

class QModelObjectPropertyListItemDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  QModelObjectPropertyListItemDelegate(QObject* parent = 0);
  
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const;

  void setEditorData(QWidget* editor, const QModelIndex& index) const;

};


#endif // _Q_MODEL_OBJECT_PROPERTY_LIST_ITEM_DELEGATE_H_
