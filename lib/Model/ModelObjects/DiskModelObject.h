#ifndef _DISK_MODEL_OBJECT_H_
#define _DISK_MODEL_OBJECT_H_

#include <ModelObject.h>
#include <vtkSmartPointer.h>

class vtkDiskSource;
class vtkTriangleFilter;


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

  vtkSmartPointer<vtkDiskSource>     m_DiskSource;
  vtkSmartPointer<vtkTriangleFilter> m_GeometrySource;
};

#endif // _DISK_MODEL_OBJECT_H_
