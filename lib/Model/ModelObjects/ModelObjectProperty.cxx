#include <ModelObjectProperty.h>

#include <iostream>


ModelObjectProperty
::ModelObjectProperty(const std::string& name, ModelObjectPropertyType type,
                      const std::string& units, bool editable, bool optimizable) {
  Initialize(name, type, units, editable, optimizable);
}


ModelObjectProperty
::ModelObjectProperty(const std::string& name, bool value,
                      const std::string& units, bool editable, bool optimizable) {
  Initialize(name, ModelObjectProperty::BOOL_TYPE, units, editable, optimizable);  
  m_BoolValue = value;
}


ModelObjectProperty
::ModelObjectProperty(const std::string& name, int value, 
                      const std::string& units, bool editable, bool optimizable) {
  Initialize(name, ModelObjectProperty::INT_TYPE, units, editable, optimizable);  
  m_IntValue = value;
}


ModelObjectProperty
::ModelObjectProperty(const std::string& name, double value,
                      const std::string& units, bool editable, bool optimizable) {
  Initialize(name, ModelObjectProperty::DOUBLE_TYPE, units, editable, optimizable);  
  m_DoubleValue = value;
}


ModelObjectProperty
::ModelObjectProperty(const std::string& name, const std::string& value,
                      const std::string& units, bool editable, bool optimizable) {
  Initialize(name, ModelObjectProperty::STRING_TYPE, units, editable, optimizable);  
  m_StringValue = value;
}


void
ModelObjectProperty
::Initialize(const std::string& name, ModelObjectPropertyType type,
             const std::string& units, bool editable, bool optimizable) {
  m_Name = name;
  m_Type = type;
  m_Editable = editable;
  m_Optimizable = optimizable;
  m_Units = units;

  m_BoolValue = true;
  m_IntValue = 0.0;
  m_DoubleValue = 0.0;
  m_StringValue = "";
}


ModelObjectProperty
::~ModelObjectProperty() {

}


std::string&
ModelObjectProperty
::GetName() {
  return m_Name;
}


ModelObjectProperty::ModelObjectPropertyType
ModelObjectProperty
::GetType() {
  return m_Type;
}


bool
ModelObjectProperty
::IsEditable() {
  return m_Editable;
}


bool
ModelObjectProperty
::IsOptimizable() {
  return m_Optimizable;
}


void
ModelObjectProperty
::SetBoolValue(bool value) {
  m_BoolValue = value;
}


bool
ModelObjectProperty
::GetBoolValue() {
  return m_BoolValue;
}


void
ModelObjectProperty
::SetIntValue(int value) {
  m_IntValue = value;
}


int
ModelObjectProperty
::GetIntValue() {
  return m_IntValue;
}


void
ModelObjectProperty
::SetDoubleValue(double value) {
  m_DoubleValue = value;
}


double
ModelObjectProperty
::GetDoubleValue() {
  return m_DoubleValue;
}


void
ModelObjectProperty
::SetStringValue(const std::string& value) {
  m_StringValue = value;
}


std::string&
ModelObjectProperty
::GetStringValue() {
  return m_StringValue;
}


std::string&
ModelObjectProperty
::GetUnits() {
  return m_Units;
}
