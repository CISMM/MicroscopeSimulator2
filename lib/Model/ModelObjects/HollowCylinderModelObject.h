#ifndef _HOLLOW_CYLINDER_MODEL_OBJECT_H_
#define _HOLLOW_CYLINDER_MODEL_OBJECT_H_

#include <ModelObject.h>

#include <vtkSmartPointer.h>

class vtkDiskSource;
class vtkLinearExtrusionFilter;
class vtkPolyDataNormals;
class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkTriangleFilter;


class HollowCylinderModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;
  
  static const char* OUTER_RADIUS_PROP;
  static const char* THICKNESS_PROP;
  static const char* LENGTH_PROP;
  static const char* SURFACE_FLUOR_PROP;
  static const char* VOLUME_FLUOR_PROP;

  HollowCylinderModelObject(DirtyListener* dirtyListener);
  virtual ~HollowCylinderModelObject();

  virtual void Update();

 protected:
  HollowCylinderModelObject() {};

  vtkSmartPointer<vtkDiskSource>              m_DiskSource;
  vtkSmartPointer<vtkTransform>               m_Transform;
  vtkSmartPointer<vtkTransformPolyDataFilter> m_TransformFilter;
  vtkSmartPointer<vtkLinearExtrusionFilter>   m_ExtrusionSource;
  vtkSmartPointer<vtkTriangleFilter>          m_TriangleFilter;
  vtkSmartPointer<vtkPolyDataNormals>         m_GeometrySource;

};

typedef HollowCylinderModelObject* HollowCylinderModelObjectPtr;

#endif // _HOLLOW_CYLINDER_MODEL_OBJECT_H_
