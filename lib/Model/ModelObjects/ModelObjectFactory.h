#ifndef _MODEL_OBJECT_FACTORY_H_
#define _MODEL_OBJECT_FACTORY_H_

#include "ModelObject.h"

class DirtyListener;

class ModelObjectFactory {

 public:
  ModelObjectFactory(DirtyListener* dirtyListener);
  virtual ~ModelObjectFactory();

  ModelObjectPtr CreateModelObject(const std::string& objectName) const;

 protected:
  ModelObjectFactory() {};

  DirtyListener* m_DirtyListener;

};


#endif // _MODEL_OBJECT_FACTORY_H_
