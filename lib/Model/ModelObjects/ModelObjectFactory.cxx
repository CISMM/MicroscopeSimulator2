#include <ModelObjectFactory.h>

#include <DirtyListener.h>

#include <CylinderModelObject.h>
#include <DiskModelObject.h>
#include <EllipsoidModelObject.h>
#include <FlexibleTubeModelObject.h>
#include <HollowCylinderModelObject.h>
#include <ImageModelObject.h>
#include <ImportedGeometryModelObject.h>
#include <PlaneModelObject.h>
#include <PointRingModelObject.h>
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
  } else if (objectName == EllipsoidModelObject::OBJECT_TYPE_NAME) {
    object = new EllipsoidModelObject(m_DirtyListener);
  } else if (objectName == HollowCylinderModelObject::OBJECT_TYPE_NAME) {
    object = new HollowCylinderModelObject(m_DirtyListener);
  } else if (objectName == DiskModelObject::OBJECT_TYPE_NAME) {
    object = new DiskModelObject(m_DirtyListener);
  } else if (objectName == FlexibleTubeModelObject::OBJECT_TYPE_NAME) {
    object = new FlexibleTubeModelObject(m_DirtyListener);
  } else if (objectName == PlaneModelObject::OBJECT_TYPE_NAME) {
    object = new PlaneModelObject(m_DirtyListener);
  } else if (objectName == PointRingModelObject::OBJECT_TYPE_NAME) {
    object = new PointRingModelObject(m_DirtyListener);
  } else if (objectName == PointSetModelObject::OBJECT_TYPE_NAME) {
    object = new PointSetModelObject(m_DirtyListener);
  } else if (objectName == SphereModelObject::OBJECT_TYPE_NAME) {
    object = new SphereModelObject(m_DirtyListener);
  } else if (objectName == TorusModelObject::OBJECT_TYPE_NAME) {
    object = new TorusModelObject(m_DirtyListener);

  // We need the following to create model objects of this type
  // when opening a simulation file.
  } else if (objectName == ImageModelObject::OBJECT_TYPE_NAME) {
    object = new ImageModelObject(m_DirtyListener);
  } else if (objectName == ImportedGeometryModelObject::OBJECT_TYPE_NAME) {
    object = new ImportedGeometryModelObject(m_DirtyListener);
  }
  return object;
}


ModelObjectPtr
ModelObjectFactory
::ImportModelObject(const std::string& objectName, const std::string& fileName) const {
  ModelObjectPtr object = NULL;
  if (objectName == ImageModelObject::OBJECT_TYPE_NAME) {
    ImageModelObject* imageModel = new ImageModelObject(m_DirtyListener);
    imageModel->LoadFile(fileName);

    object = imageModel;
  } else if (objectName == ImportedGeometryModelObject::OBJECT_TYPE_NAME) {
    ImportedGeometryModelObject* geometryModel = new ImportedGeometryModelObject(m_DirtyListener);
    geometryModel->LoadFile(fileName);

    object = geometryModel;
  }

  return object;
}
