#ifndef _TORUS_MODEL_OBJECT_H_
#define _TORUS_MODEL_OBJECT_H_

#include <ModelObject.h>
#include <vtkSmartPointer.h>

class vtkPolyDataNormals;
class vtkVolumetricTorusSource;


class TorusModelObject : public ModelObject {

 public:
  static const char* OBJECT_TYPE_NAME;

  static const char* CROSS_SECTION_RADIUS_PROP;
  static const char* RING_RADIUS_PROP;
  static const char* SURFACE_FLUOR_PROP;
  static const char* VOLUME_FLUOR_PROP;


  TorusModelObject(DirtyListener* dirtyListener);
  virtual ~TorusModelObject();

  virtual void Update();

 protected:
  TorusModelObject() {};

  vtkSmartPointer<vtkVolumetricTorusSource> m_TorusSource;
  vtkSmartPointer<vtkPolyDataNormals>       m_GeometrySource;

};

#endif // _TORUS_MODEL_OBJECT_H_
