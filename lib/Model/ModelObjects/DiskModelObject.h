#ifndef _DISK_MODEL_OBJECT_H_
#define _DISK_MODEL_OBJECT_H_

#include <ModelObject.h>
#include <vtkSmartPointer.h>

class vtkDiskSource2;


class DiskModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* RADIUS_PROP;
  static const char* SURFACE_FLUOR_PROP;


  DiskModelObject(DirtyListener* dirtyListener);
  virtual ~DiskModelObject();

  virtual void Update();

 protected:
  DiskModelObject() {};

  vtkSmartPointer<vtkDiskSource2> m_GeometrySource;
};

#endif // _DISK_MODEL_OBJECT_H_
