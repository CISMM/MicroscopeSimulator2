#ifndef _Q_IMAGE_LIST_MODEL_H_
#define _Q_IMAGE_LIST_MODEL_H_

#include <QAbstractListModel>

// Forward declarations
class ModelObjectList;


class QImageListModel : public QAbstractListModel {
Q_OBJECT

 public:
  QImageListModel(QObject* parent = 0);
  virtual ~QImageListModel();

  void SetModelObjectList(ModelObjectList* list);
  
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  void Refresh();

 protected:
  ModelObjectList* m_ModelObjectList;
};

#endif // _Q_IMAGE_LIST_MODEL_H_
