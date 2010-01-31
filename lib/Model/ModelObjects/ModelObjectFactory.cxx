#include <ModelObjectFactory.h>

#include <DirtyListener.h>

#include <CylinderModelObject.h>
#include <DiskModelObject.h>
#include <FlexibleTubeModelObject.h>
#include <PlaneModelObject.h>
#include <PointSetModelObject.h>
#include <SphereModelObject.h>
#include <TorusModelObject.h>


ModelObjectFactory
::ModelObjectFactory(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;
}


ModelObjectFactory
::~ModelObjectFactory() {
}


ModelObjectPtr
ModelObjectFactory
::CreateModelObject(const std::string& objectName) const {
  ModelObjectPtr object = NULL;
  if (objectName == CylinderModelObject::OBJECT_TYPE_NAME) {
    object = new CylinderModelObject(m_DirtyListener);
  } else if (objectName == DiskModelObject::OBJECT_TYPE_NAME) {
    object = new DiskModelObject(m_DirtyListener);
  } else if (objectName == FlexibleTubeModelObject::OBJECT_TYPE_NAME) {
    object = new FlexibleTubeModelObject(m_DirtyListener);
  } else if (objectName == PlaneModelObject::OBJECT_TYPE_NAME) {
    object = new PlaneModelObject(m_DirtyListener);
  } else if (objectName == PointSetModelObject::OBJECT_TYPE_NAME) {
    object = new PointSetModelObject(m_DirtyListener);
  } else if (objectName == SphereModelObject::OBJECT_TYPE_NAME) {
    object = new SphereModelObject(m_DirtyListener);
  } else if (objectName == TorusModelObject::OBJECT_TYPE_NAME) {
    object = new TorusModelObject(m_DirtyListener);
  }
  return object;
}
