#include <PointSpreadFunctionList.h>

#include <QPSFListModel.h>
#include <ImportedPointSpreadFunction.h>
#include <PointSpreadFunction.h>


QPSFListModel
::QPSFListModel(QObject* parent) : QAbstractListModel(parent) {
  m_PSFList = NULL;
  m_HasNonePSF = false;
}


QPSFListModel
::~QPSFListModel() {

}


void
QPSFListModel
::SetPSFList(PointSpreadFunctionList* list) {
  m_PSFList = list;
}


PointSpreadFunctionList*
QPSFListModel
::GetPSFList() {
  return m_PSFList;
}


void
QPSFListModel
::SetHasNone(bool hasNone) {
  m_HasNonePSF = hasNone;
}


bool
QPSFListModel
::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (m_PSFList == NULL)
    return false;

  std::string strValue = value.toString().toStdString();
  if (strValue == "")
    return false;

  m_PSFList->SetPointSpreadFunctionName(index.row(), strValue);

  return true;
}


QVariant
QPSFListModel
::data(const QModelIndex& index, int role) const {
  if (m_PSFList == NULL) {
    return QVariant("Error: no PSF list");
  }

  int row = index.row();
  int max = m_HasNonePSF ? this->rowCount() + 1 : this->rowCount();
  if (row < 0 || row >= max) {
    return QVariant();
  }

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    PointSpreadFunction* psf = NULL;
    if (m_HasNonePSF) {
      if (row == 0) {
        return QVariant("None");
      } else {
        psf = m_PSFList->GetPointSpreadFunctionAt(row-1);
      }
    } else {
      psf = m_PSFList->GetPointSpreadFunctionAt(row);
    }
    if (psf) {
      return QVariant(psf->GetName().c_str());
    } else {
      return QVariant();
    }
  }

  // Activate selected PSF

  return QVariant();
}


Qt::ItemFlags
QPSFListModel
::flags(const QModelIndex& index) const {
  if (index.column() == 0) {
    Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return flag;
  }
  return Qt::ItemIsEnabled;
}


int
QPSFListModel
::rowCount(const QModelIndex& parent) const {
  if (m_PSFList == NULL) {
    return 0;
  } else if (m_HasNonePSF) {
    return m_PSFList->GetSize() + 1;
  }
  return m_PSFList->GetSize();
}


void
QPSFListModel
::Refresh() {
  reset();
}
