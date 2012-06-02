#ifndef _Q_MODEL_OBJECT_PROPERTY_LIST_TABLE_MODEL_H_
#define _Q_MODEL_OBJECT_PROPERTY_LIST_TABLE_MODEL_H_

#include <QAbstractTableModel>

// Forward declarations
class ModelObject;


class QModelObjectPropertyListTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  QModelObjectPropertyListTableModel(QObject* parent = 0);
  ~QModelObjectPropertyListTableModel();

  void SetModelObject(ModelObject* object);
  ModelObject* GetModelObject();

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  QVariant data(const QModelIndex& index, int role) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;

  void Refresh();

 protected:
  ModelObject* m_ModelObject;

};


#endif // _Q_MODEL_OBJECT_PROPERTY_LIST_TABLE_MODEL_H_
