#include <QPointSpreadFunctionPropertyTableModel.h>

#include <PointSpreadFunction.h>

QPointSpreadFunctionPropertyTableModel
::QPointSpreadFunctionPropertyTableModel(QObject* parent)
  : QAbstractTableModel(parent) {

  m_PSF = NULL;
}


QPointSpreadFunctionPropertyTableModel
::~QPointSpreadFunctionPropertyTableModel() {

}


void
QPointSpreadFunctionPropertyTableModel
::SetPointSpreadFunction(PointSpreadFunction* psf) {
  m_PSF = psf;

  Refresh();
}


PointSpreadFunction*
QPointSpreadFunctionPropertyTableModel
::GetPointSpreadFunction() {
  return m_PSF;
}


bool
QPointSpreadFunctionPropertyTableModel
::setData(const QModelIndex& index, const QVariant& value, int role) {
  m_PSF->SetParameterValue(index.row(), value.toDouble());

  reset();

  // Notify Qt items that the data has changed.
  emit dataChanged(index, index);

  return true;
}


QVariant
QPointSpreadFunctionPropertyTableModel
::data(const QModelIndex& index, int role) const {
  if (m_PSF == NULL) {
    return QVariant();
  }

  int row = index.row();
  int col = index.column();
  if (row < 0 || row >= this->rowCount() || col < 0 || col >= this->columnCount())
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (col == 0) {
      return QVariant(m_PSF->GetParameterName(row).c_str());
    } else if (col == 1) {
      return QVariant(m_PSF->GetParameterValue(row));
    } else {
      return QVariant();
    }
  }

  return QVariant();
}


Qt::ItemFlags
QPointSpreadFunctionPropertyTableModel
::flags(const QModelIndex& index) const {
  if (index.column() == 1) {
    Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return flag;
  }

  return Qt::ItemIsEnabled;
}


QVariant
QPointSpreadFunctionPropertyTableModel
::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0: return QVariant("Property"); break;
    case 1: return QVariant("Value"); break;
    default: return QVariant(section); break;
    }
  } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
    return QVariant(section);
  } else {
    return QVariant();
  }
}


int
QPointSpreadFunctionPropertyTableModel
::rowCount(const QModelIndex& parent) const {
  if (m_PSF == NULL)
    return 0;
  return m_PSF->GetNumberOfProperties();
}


int
QPointSpreadFunctionPropertyTableModel
::columnCount(const QModelIndex& parent) const {
  return 2;
}


void
QPointSpreadFunctionPropertyTableModel
::Refresh() {
  reset();
}
