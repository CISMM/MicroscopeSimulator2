#include <QFocalPlanePositionsTableModel.h>

#include <FluorescenceSimulation.h>


QFocalPlanePositionsTableModel
::QFocalPlanePositionsTableModel(QObject* parent) :
  QAbstractTableModel(parent) {

  m_Simulation = NULL;
}


QFocalPlanePositionsTableModel
::~QFocalPlanePositionsTableModel() {

}


void
QFocalPlanePositionsTableModel
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  m_Simulation = simulation;
  Refresh();
}


FluorescenceSimulation*
QFocalPlanePositionsTableModel
::GetFluorescenceSimulation() {
  return m_Simulation;
}


bool
QFocalPlanePositionsTableModel
::setData(const QModelIndex& index, const QVariant& value, int role) {
  m_Simulation->SetCustomFocalPlanePosition(index.row(), value.toDouble());

  reset();

  // Notify Qt items that the data has changed.
  emit dataChanged(index, index);

  return true;
}


QVariant
QFocalPlanePositionsTableModel
::data(const QModelIndex& index, int role) const {
  if (m_Simulation == NULL)
    return QVariant();

  int row = index.row();
  int col = index.column();
  if (row < 0 || row >= this->rowCount() || col < 0 || col >= this->columnCount())
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (col == 0) {
      return QVariant(row+1);
    } else if (col == 1) {
      return QVariant(m_Simulation->GetCustomFocalPlanePosition(row));
    } else {
      return QVariant();
    }
  }

  return QVariant();
}


Qt::ItemFlags
QFocalPlanePositionsTableModel
::flags(const QModelIndex& index) const {
  if (index.column() == 1) {
    Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return flag;
  }

  return Qt::ItemIsEnabled;
}


QVariant
QFocalPlanePositionsTableModel
::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0: return QVariant("Focal Plane"); break;
    case 1: return QVariant("Position (nm)"); break;
    default: return QVariant(); break;
    }
  }

  return QVariant();
}


int
QFocalPlanePositionsTableModel
::rowCount(const QModelIndex& parent) const {
  if (m_Simulation)
    return static_cast<int>(m_Simulation->GetNumberOfFocalPlanes());

  return 0;
}


int
QFocalPlanePositionsTableModel
::columnCount(const QModelIndex& parent) const {
  return 2;
}


void
QFocalPlanePositionsTableModel
::Refresh() {
  reset();
}
