#include <QImageListModel.h>

#include <ImageModelObject.h>
#include <ModelObjectList.h>


QImageListModel
::QImageListModel(QObject* parent) : QAbstractListModel(parent) {
  m_ModelObjectList = NULL;
}


QImageListModel
::~QImageListModel() {

}


void
QImageListModel
::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;
}


QVariant
QImageListModel
::data(const QModelIndex& index, int role) const {
  if (m_ModelObjectList == NULL) {
    return QVariant("Error: no ModelObjectList");
  }

  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    return QVariant();
  }

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (row == 0) {
      return QVariant("None");
    } else {
      ModelObjectPtr mo = m_ModelObjectList->
        GetModelObjectAtIndex(row-1, ImageModelObject::OBJECT_TYPE_NAME);
      if (mo)
        return QVariant(mo->GetName().c_str());
    }
  }

  return QVariant();
}


Qt::ItemFlags
QImageListModel
::flags(const QModelIndex& index) const {
  if (index.column() == 0) {
    Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return flag;
  }
  return Qt::ItemIsEnabled;
}


int
QImageListModel
::rowCount(const QModelIndex& parent) const {
  if (m_ModelObjectList == NULL) {
    return 1; // None image
  }

  return m_ModelObjectList->GetSize(ImageModelObject::OBJECT_TYPE_NAME)+1;
}


void
QImageListModel
::Refresh() {
  reset();
}
