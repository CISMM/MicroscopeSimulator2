#include <QModelObjectPropertyListTableModel.h>

#include <ModelObject.h>
#include <ModelObjectProperty.h>
#include <ModelObjectPropertyList.h>

#include <iostream>


QModelObjectPropertyListTableModel
::QModelObjectPropertyListTableModel(QObject* parent)
  : QAbstractTableModel(parent) {
  m_ModelObject = NULL;
}


QModelObjectPropertyListTableModel
::~QModelObjectPropertyListTableModel() {

}


void
QModelObjectPropertyListTableModel
::SetModelObject(ModelObject* object) {
  m_ModelObject = object;

  Refresh();
}


ModelObject*
QModelObjectPropertyListTableModel
::GetModelObject() {
  return m_ModelObject;
}


bool
QModelObjectPropertyListTableModel
::setData(const QModelIndex& index, const QVariant& value, int role) {
  ModelObjectProperty* mop = m_ModelObject->GetProperty(index.row());

  if (index.column() == 1) {
    if (mop->GetType() == ModelObjectProperty::BOOL_TYPE) {
      mop->SetBoolValue(value.toBool());
    } else if (mop->GetType() == ModelObjectProperty::INT_TYPE) {
      mop->SetIntValue(value.toInt());
    } else if (mop->GetType() == ModelObjectProperty::DOUBLE_TYPE) {
      mop->SetDoubleValue(value.toDouble());
    } else if (mop->GetType() == ModelObjectProperty::STRING_TYPE) {
      mop->SetStringValue(value.toString().toStdString());
    } else if (mop->GetType() == ModelObjectProperty::FLUOROPHORE_MODEL_TYPE) {
      // Don't do anything
    } else {
      std::cout << "Unknown property type" << std::endl;
    }
  } else if (index.column() == 3) {
    mop->SetOptimize(value.toBool());
  }

  reset();

  // Sully the model object so that they are updated before any views
  // refresh. This will ensure consistency.
  m_ModelObject->Sully();

  // Notify Qt items that the data has changed.
  emit dataChanged(index, index);

  return true;
}


QVariant
QModelObjectPropertyListTableModel
::data(const QModelIndex& index, int role) const {
  if (m_ModelObject == NULL) {
    return QVariant();
  }

  int row = index.row();
  int col = index.column();
  if (row < 0 || row >= this->rowCount() || col < 0 || col >= this->columnCount())
    return QVariant();

  ModelObjectProperty* prop = m_ModelObject->GetProperty(index.row());

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (col == 0) {
      return QVariant(prop->GetName().c_str());
    } else if (col == 1) {
      switch (prop->GetType()) {
        //case ModelObjectProperty::BOOL_TYPE:   return QVariant(prop->GetBoolValue());   break;
      case ModelObjectProperty::INT_TYPE:    return QVariant(prop->GetIntValue());    break;
      case ModelObjectProperty::DOUBLE_TYPE: return QVariant(prop->GetDoubleValue()); break;
      case ModelObjectProperty::STRING_TYPE: return QVariant(prop->GetStringValue().c_str()); break;
      case ModelObjectProperty::FLUOROPHORE_MODEL_TYPE: return QVariant(tr("Click to edit")); break;
      default: return QVariant();
      }
    } else if (col == 2) {
      return QVariant(tr(prop->GetUnits().c_str()));
    } else {
      return QVariant();
    }
  } else if (role == Qt::CheckStateRole) {
    if (prop->GetType() == ModelObjectProperty::BOOL_TYPE && col == 1) {
      return (prop->GetBoolValue() ? Qt::Checked : Qt::Unchecked);
    } else if (col == 3 && prop->IsOptimizable()) {
      return (prop->GetOptimize() ? Qt::Checked : Qt::Unchecked);
    } else {
      return QVariant();
    }
  } else {
    return QVariant();
  }
}


Qt::ItemFlags
QModelObjectPropertyListTableModel
::flags(const QModelIndex& index) const {
  if (index.column() == 1) {
    Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    ModelObjectProperty* prop = m_ModelObject->GetProperty(index.row());
    if (prop && prop->IsEditable()) {
      if (prop->GetType() == ModelObjectProperty::BOOL_TYPE) {
        flag = flag | Qt::ItemIsUserCheckable;
      } else if (prop) {
        flag = flag | Qt::ItemIsEditable;
      }
    }
    return flag;
  } else if (index.column() == 3) {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  }

  return Qt::ItemIsEnabled;
}


QVariant
QModelObjectPropertyListTableModel
::headerData(int section, Qt::Orientation orientation,
             int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0: return QVariant("Property"); break;
    case 1: return QVariant("Value");    break;
    case 2: return QVariant("Units");    break;
    case 3: return QVariant("Optimize"); break;
    default: return QVariant(section);   break;
    }
  } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
    return QVariant(section);
  } else {
    return QVariant();
  }
}


int
QModelObjectPropertyListTableModel
::rowCount(const QModelIndex& parent) const {
  if (m_ModelObject == NULL || m_ModelObject->GetPropertyList() == NULL)
    return 0;

  return m_ModelObject->GetPropertyList()->GetSize();
}


int
QModelObjectPropertyListTableModel
::columnCount(const QModelIndex& parent) const {
  return 4;
}


void
QModelObjectPropertyListTableModel
::Refresh() {
  reset();
}
