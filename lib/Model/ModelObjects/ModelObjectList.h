#ifndef _MODEL_OBJECT_LIST_H_
#define _MODEL_OBJECT_LIST_H_

#include <string>
#include <list>

#include "DirtyListener.h"
#include "ModelObject.h"
#include "ModelObjectFactory.h"
#include "XMLStorable.h"


class ModelObjectList : public DirtyListener, public XMLStorable {


 public:
  ModelObjectList(DirtyListener* dirtyListener);
  virtual ~ModelObjectList();

  virtual void Sully();
  virtual void SetStatusMessage(const std::string& status);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  size_t GetSize();
  size_t GetSize(const std::string& objectType);

  ModelObjectPtr AddModelObject(const std::string& objectTypeName);
  ModelObjectPtr ImportModelObject(const std::string& objectTypeName, const std::string& fileName);

  ModelObjectPtr GetModelObjectByName(const std::string& objectName);
  ModelObjectPtr GetModelObjectAtIndex(int index);
  
  // Returns nth model object of a particular type
  ModelObjectPtr GetModelObjectAtIndex(int index, const std::string& objectType);

  void Add(ModelObjectPtr mo);
  void Delete(ModelObjectPtr mo);
  void DeleteAll();

  std::string GenerateUniqueName(const std::string& name);

 protected:
  ModelObjectList() {};

  DirtyListener*          m_DirtyListener;

  std::list<ModelObjectPtr> m_ObjectList;

  ModelObjectFactory* m_ModelObjectFactory;

  bool IsNameUnique(const std::string& name);

};

typedef std::list<ModelObject*>::iterator ModelObjectListIterator;
typedef ModelObjectList* ModelObjectListPtr;

#endif // _MODEL_OBJECT_LIST_H_
