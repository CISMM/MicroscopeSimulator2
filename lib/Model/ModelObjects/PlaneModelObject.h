#ifndef _PLANE_MODEL_OBJECT_H_
#define _PLANE_MODEL_OBJECT_H_

#include <ModelObject.h>
#include <vtkSmartPointer.h>

class vtkPlaneSource;
class vtkTriangleFilter;


class PlaneModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* WIDTH_PROP;
  static const char* HEIGHT_PROP;
  static const char* SURFACE_FLUOR_PROP;


  PlaneModelObject(DirtyListener* dirtyListener);
  virtual ~PlaneModelObject();

  virtual void Update();

 protected:
  PlaneModelObject() {};

  vtkSmartPointer<vtkPlaneSource>    m_PlaneSource;
  vtkSmartPointer<vtkTriangleFilter> m_GeometrySource;

};

typedef PlaneModelObject* PlaneModelObjectPtr;

#endif // _PLANE_MODEL_OBJECT_H_
