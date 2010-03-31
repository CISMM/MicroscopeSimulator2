#include <QModelObjectListModel.h>

#include <ModelObjectList.h>


QModelObjectListModel::QModelObjectListModel(QObject* parent) :
  QAbstractListModel(parent) {

  m_ModelObjectList = NULL;
}


QModelObjectListModel::~QModelObjectListModel() {


}


void QModelObjectListModel::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;
}


void QModelObjectListModel::Refresh() {
  reset();
}


int QModelObjectListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }

  int size = static_cast<int>(m_ModelObjectList->GetSize());
  return size;
}


QVariant QModelObjectListModel::data(const QModelIndex& index, int role) const {
  int row = index.row();

  if (row < 0 || row > static_cast<int>(m_ModelObjectList->GetSize())-1)
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return QVariant(m_ModelObjectList->GetModelObjectAtIndex(row)->GetName().c_str());
  }

  return QVariant();
}
