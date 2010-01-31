#ifndef _MODEL_OBJECT_LIST_H_
#define _MODEL_OBJECT_LIST_H_

#include "StdLibGuard.h"
#include <string>
#include <list>

#include "DirtyListener.h"
#include "ModelObject.h"
#include "XMLStorable.h"


class ModelObjectList : public DirtyListener, public XMLStorable {


 public:
  ModelObjectList(DirtyListener* dirtyListener);
  virtual ~ModelObjectList();

  virtual void Sully();
  virtual void SetStatusMessage(const std::string& status);

  virtual void GetXMLConfiguration(xmlNodePtr node);

  size_t GetSize();

  void AddModelObject(const std::string& objectName);
  void ImportModelObject(const std::string& objectName);

  ModelObjectPtr GetModelObjectByName(const std::string& objectName);
  ModelObjectPtr GetModelObjectAtIndex(int index);

  void Add(ModelObjectPtr mo);
  void Delete(ModelObjectPtr mo);

  // TODO - add GetMaximumObjectHeight - see ModelObjectList.java

  std::string GenerateUniqueName(const std::string& name);

 protected:
  ModelObjectList() {};

  DirtyListener*          m_DirtyListener;

  std::list<ModelObjectPtr> m_ObjectList;

  // TODO - add reference to ModelObjectFactory


  bool IsNameUnique(const std::string& name);

};

typedef std::list<ModelObject*>::iterator ModelObjectListIterator;
typedef ModelObjectList* ModelObjectListPtr;

#endif // _MODEL_OBJECT_LIST_H_
