#include "ModelObjectPropertyList.h"

#include "ModelObjectProperty.h"

#include <iostream>

ModelObjectPropertyList
::ModelObjectPropertyList() {
}


ModelObjectPropertyList
::~ModelObjectPropertyList() {

}


int
ModelObjectPropertyList
::GetSize() {
  return static_cast<int>(m_List.size());
}


void
ModelObjectPropertyList
::AddProperty(ModelObjectProperty* property) {
  m_List.push_back(property);
}


ModelObjectProperty*
ModelObjectPropertyList
::GetProperty(int i) {
  if (i < 0 || i > GetSize()) {
    return NULL;
  }

  ModelObjectProperty* property = NULL;
  try {
    property = m_List.at(i);
  } catch (...) {
  }

  return property;
}


void
ModelObjectPropertyList
::PopProperty() {
  m_List.pop_back();
}


void
ModelObjectPropertyList
::DeleteAndPopProperty() {
  ModelObjectProperty* prop = m_List.back();
  delete prop;
  m_List.pop_back();
}


ModelObjectProperty*
ModelObjectPropertyList
::GetProperty(const std::string& name) {
  std::vector<ModelObjectProperty*>::iterator iter;
  
  for (iter = m_List.begin(); iter != m_List.end(); iter++) {
    if ((*iter)->GetName() == name) {
      return (*iter);
    }
  }

  std::cout << "ModelObjectPropertyList could not find property '"
            << name << "'" << std::endl;

  return NULL;
}
