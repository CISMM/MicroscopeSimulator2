#include <QFluorescenceOptimizerSettingsTableModel.h>

#include <FluorescenceOptimizer.h>


QFluorescenceOptimizerSettingsTableModel
::QFluorescenceOptimizerSettingsTableModel(QObject* parent)
  : QAbstractTableModel(parent) {
  m_FluorescenceOptimizer = NULL;
}


QFluorescenceOptimizerSettingsTableModel
::~QFluorescenceOptimizerSettingsTableModel() {

}


void
QFluorescenceOptimizerSettingsTableModel
::SetFluorescenceOptimizer(FluorescenceOptimizer* optimizer) {
  m_FluorescenceOptimizer = optimizer;

  Refresh();
}


FluorescenceOptimizer*
QFluorescenceOptimizerSettingsTableModel
::GetFluorescenceOptimizer() {
  return m_FluorescenceOptimizer;
}


bool
QFluorescenceOptimizerSettingsTableModel
::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.column() == 1) {
    FluorescenceOptimizer::NumericType type = 
      m_FluorescenceOptimizer->GetOptimizerParameterNumericType(index.row());

    FluorescenceOptimizer::Variant newValue;
    switch (type) {
    case FluorescenceOptimizer::INT_TYPE:
      newValue.iValue = value.toInt();
      break;

    case FluorescenceOptimizer::FLOAT_TYPE:
      newValue.fValue = static_cast<float>(value.toDouble());
      break;

    case FluorescenceOptimizer::DOUBLE_TYPE:
      newValue.dValue = value.toDouble();
      break;

    default:
      newValue.iValue = 0;
      newValue.fValue = 0.0f;
      newValue.dValue = 0.0;
    }

    m_FluorescenceOptimizer->SetOptimizerParameterValue(index.row(), newValue);

    reset();

    m_FluorescenceOptimizer->Sully();
    
    // Notify Qt items that the data has changed.
    emit dataChanged(index, index);
  }

  return true;
}


QVariant
QFluorescenceOptimizerSettingsTableModel
::data(const QModelIndex& index, int role) const {
  if (m_FluorescenceOptimizer == NULL) {
    return QVariant();
  }

  int row = index.row();
  int col = index.column();
  if (row < 0 || row > this->rowCount() || col < 0 || col >= this->columnCount())
    return QVariant();

  FluorescenceOptimizer::Parameter p = m_FluorescenceOptimizer->GetOptimizerParameter(row);
  
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (col == 0) {
      return QVariant(p.name.c_str());
    } else if (col == 1) {
      switch (p.type) {
      case FluorescenceOptimizer::INT_TYPE:    return QVariant(p.value.iValue); break;
      case FluorescenceOptimizer::FLOAT_TYPE:  return QVariant(p.value.fValue); break;
      case FluorescenceOptimizer::DOUBLE_TYPE: return QVariant(p.value.dValue); break;
      default: return QVariant();
      }
    }
  }

  return QVariant();  
}


Qt::ItemFlags
QFluorescenceOptimizerSettingsTableModel
::flags(const QModelIndex& index) const {
  if (index.column() == 1) {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  return Qt::ItemIsEnabled;
}


QVariant
QFluorescenceOptimizerSettingsTableModel
::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0: return QVariant("Setting"); break;
    case 1: return QVariant("Value"); break;
    default: return QVariant(); break;
    }
  }

  return QVariant();
}


int
QFluorescenceOptimizerSettingsTableModel
::rowCount(const QModelIndex& parent) const {
  if (m_FluorescenceOptimizer == NULL)
    return 0;

  return m_FluorescenceOptimizer->GetNumberOfOptimizerParameters();
}


int
QFluorescenceOptimizerSettingsTableModel
::columnCount(const QModelIndex& parent) const {
  return 2;
}

void
QFluorescenceOptimizerSettingsTableModel
::Refresh() {
  reset();
}
