#ifndef _Q_MODEL_OBJECT_LIST_MODEL_H_
#define _Q_MODEL_OBJECT_LIST_MODEL_H_

#include <QAbstractListModel>

// Forward declarations
class ModelObjectList;


class QModelObjectListModel : public QAbstractListModel {
  Q_OBJECT

 public:
  QModelObjectListModel(QObject* parent=0);
  virtual ~QModelObjectListModel();

  void SetModelObjectList(ModelObjectList* list);

  void Refresh();

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

 protected:

  ModelObjectList* m_ModelObjectList;

};


#endif // _Q_MODEL_OBJECT_LIST_MODEL_H_
