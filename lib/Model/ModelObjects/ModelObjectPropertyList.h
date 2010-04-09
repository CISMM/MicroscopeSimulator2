#ifndef _MODEL_OBJECT_PROPERTY_LIST_H_
#define _MODEL_OBJECT_PROPERTY_LIST_H_

#include <vector>

// Forward declarations
class ModelObjectProperty;


class ModelObjectPropertyList {

 public:
  ModelObjectPropertyList();
  virtual ~ModelObjectPropertyList();

  int GetSize();

  void AddProperty(ModelObjectProperty* property);

  void PopProperty();

  void DeleteAndPopProperty();

  ModelObjectProperty* GetProperty(int i);

  ModelObjectProperty* GetProperty(const std::string& name);

 protected:

  std::vector<ModelObjectProperty*> m_List;

};

#endif // _MODEL_OBJECT_PROPERTY_LIST_H_
