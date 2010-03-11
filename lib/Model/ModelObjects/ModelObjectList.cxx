#include "ModelObjectList.h"

#include "ModelObjectFactory.h"


ModelObjectList
::ModelObjectList(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;
  m_ModelObjectFactory = new ModelObjectFactory(dirtyListener);
}


ModelObjectList
::~ModelObjectList() {

  // Delete the objects.
  std::list<ModelObject*>::iterator iter;
  for (iter = m_ObjectList.begin(); iter != m_ObjectList.end(); iter++) {
    delete *iter;
  }
  delete m_ModelObjectFactory;
}


void
ModelObjectList
::Sully() {
  if (m_DirtyListener) {
    m_DirtyListener->Sully();
  }
}


void
ModelObjectList
::SetStatusMessage(const std::string& status) {
  if (m_DirtyListener) {
    m_DirtyListener->SetStatusMessage(status);
  }
}


void
ModelObjectList
::GetXMLConfiguration(xmlNodePtr node) {

  // Iterate over model objects and get their configurations.
  ModelObjectListIterator iter;
  for (iter = m_ObjectList.begin(); iter != m_ObjectList.end(); iter++) {
    (*iter)->GetXMLConfiguration(node);
  }
}


void
ModelObjectList
::RestoreFromXML(xmlNodePtr node) {
  xmlNodePtr modelObjectNode = node->children;

  // Iterate over model objects
  while (modelObjectNode) {
    ModelObjectPtr mo = AddModelObject(std::string((char*) modelObjectNode->name));
    mo->RestoreFromXML(modelObjectNode);

    modelObjectNode = modelObjectNode->next;
  }
}


size_t
ModelObjectList
::GetSize() {
  return m_ObjectList.size();
}


ModelObjectPtr
ModelObjectList
::AddModelObject(const std::string& objectTypeName) {
  std::cout << "Adding model object: " << objectTypeName << std::endl;

  ModelObjectPtr mo = m_ModelObjectFactory->CreateModelObject(objectTypeName);
  if (mo) {
    std::string name = GenerateUniqueName(mo->GetName());
    mo->SetName(name);
    Add(mo);
  }

  Sully();
  SetStatusMessage("Added object to the scene.");
  
  return mo;
}


void
ModelObjectList
::ImportModelObject(const std::string& objectTypeName) {
  // TODO - here the ModelObjectFactory will import a model

  // TODO - add the created model to the list

  Sully();
  SetStatusMessage("Imported object to the scene.");
}


ModelObjectPtr
ModelObjectList
::GetModelObjectByName(const std::string& objectName) {
  ModelObjectListIterator iter;
  for (iter = m_ObjectList.begin(); iter != m_ObjectList.end(); iter++) {
    if ((*iter)->GetName() == objectName)
      return *iter;
  }

  return NULL;
}


ModelObjectPtr
ModelObjectList
::GetModelObjectAtIndex(int index) {
  int i = 0;
  ModelObjectListIterator iter;
  for (iter = m_ObjectList.begin(); iter != m_ObjectList.end(); iter++) {
    if (i == index) {
      return *iter;
    }
    i++;
  }

  return NULL;
}


void
ModelObjectList
::Add(ModelObjectPtr mo) {
  m_ObjectList.push_back(mo);

  Sully();
}


void
ModelObjectList
::Delete(ModelObjectPtr mo) {
  m_ObjectList.remove(mo);

  Sully();
}


void
ModelObjectList
::DeleteAll() {
  std::list<ModelObjectPtr>::iterator iter = m_ObjectList.begin();
  for (iter = m_ObjectList.begin(); iter != m_ObjectList.end(); iter++) {
    delete *iter;
  }
  m_ObjectList.erase(m_ObjectList.begin(), m_ObjectList.end());
}


std::string
ModelObjectList
::GenerateUniqueName(const std::string& name) {
  if (IsNameUnique(name)) {
    return name;
  }

  // Add a number to the end of the name.
  int i = 1;
  while (true) {
    char buf[128];
    sprintf(buf, "%s - %d", name.c_str(), i);
    std::string stringBuffer(buf);

    if (IsNameUnique(stringBuffer))
      return stringBuffer;
    i++;
  }
}


bool
ModelObjectList
::IsNameUnique(const std::string& name) {
  std::list<ModelObjectPtr>::iterator iter;
  for (iter = m_ObjectList.begin(); iter != m_ObjectList.end(); iter++) {
    if ((*iter)->GetName() == name)
      return false;
  }

  return true;
}

// TODO - add GetMaximumObjectHeight

